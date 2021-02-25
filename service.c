#include"service.h"
#include"cfutil.h"
#include"uclop.h"

int g_chan_id = 0;

serviceT *service__new( void *device, char *name, char *secureName ) {
  serviceT *self = (serviceT *) malloc( sizeof( serviceT ) );
  
  devUp( device );
  int err = AMDeviceSecureStartService( device, str_c2cf( name ), NULL, &(self->service) );
  if( err ) {
    exitOnError(
      AMDeviceSecureStartService( device, str_c2cf(secureName), NULL, &(self->service) ),
      "Start Service"
    );
    self->secure = 1;
  }
  else self->secure = 0;
  
  #ifdef DEBUG
  printf("Started service %s %s\n", name, self->secure ? "securely" : "" );
  #endif
  
  devDown( device );
  
  return self;
}

channelT *service__connect_channel( serviceT *service, char *name ) {
  // testmanagerd uses hidden channels :(
  if( !CFDictionaryContainsKey( service->channels, str_c2cf(name) ) ) {
    fprintf( stderr, "channel %s not in list\n", name );
    return NULL;
  }
  //printf("Channel in list\n");

  int code = ++g_chan_id;

  #ifdef DEBUG
  printf("Connecting channel %s to code %d\n", name, code );
  #endif
  
  tARR *args = tARR__newVals( 2,
    tI32__new( code ),
    tSTR__new(name)
  );
  CFTypeRef msg = NULL;
  
  if( !service__send( service, "_requestChannelWithCode:identifier:", (tBASE *) args, SEND_HASREPLY ) ) {
    printf("Could not send channel request\n");
    return NULL;
  }
  if( !service__recv( service, &msg, NULL ) ) {
    printf("No response to connect channel\n");
    return NULL;
  }
  
  if( msg ) {
    fprintf( stderr, "Error: _requestChannelWithCode:identifier: returned %s\n", cftype(msg) );
    cfdump( 0, msg );
    CFRelease( msg );
    return NULL;
  }
  
  #ifdef DEBUG
  printf("  Successful\n");
  #endif
  
  channelT *chan = channel__new( service->service, service->secure, code );
  
  channel__recv( chan, &msg, NULL );
  
  return chan;
}

char service__handshake( serviceT *self ) {
  tDICT *caps = tDICT__newPairs( 4,
    "com.apple.private.DTXConnection", tI32__new( 1 ),
    "com.apple.private.DTXBlockCompression", tI32__new( 2 )
  );
    
  //printf("Handshake msgId:%i\n", cur_message + 1 );
  
  if( !service__send( self, "_notifyOfPublishedCapabilities:", (tBASE *) caps, SEND_HASREPLY ) ) {
    printf("Service handshake failed\n");
    return false;
  }
  
  #ifdef DEBUG2
  printf("Sent handshake\n");
  #endif
  
  //CFRelease( caps );

  CFTypeRef msg = NULL;
  CFArrayRef arg = NULL;

  if( !service__recv( self, &msg, &arg ) || !msg || !arg ) {
    fprintf(stderr, "recvDtxMessage failed:\n");
    return 0;
  }
  
  #ifdef DEBUG
  printf("Handshake response: ");
  cfdump( 1, msg );
  #endif
  //printf("Received response\n");

  CFDictionaryRef channels = NULL;
  do {
    if( !iscfstr( msg ) ) {
      fprintf( stderr, "Handshake response isn't a message: %s\n", cftype(msg) );
      break;
    }
    if( !cfstreq( (CFStringRef) msg, "_notifyOfPublishedCapabilities:" ) ) {
      fprintf(stderr, "Handshake response is wrong: %s\n", cftype(msg) );
      break;
    }

    if( !iscfarr( arg ) ) {
      fprintf(stderr,"handshake arg response is not an array; is: %s\n", cftype( arg ) );
      break;
    }
    
    if( CFArrayGetCount( (CFArrayRef) arg ) != 1 ) {
      fprintf(stderr,"handshake arg response is not an array of len 1; is: %s\n", cftype( arg ) );
      break;
    }
    
    channels = (CFDictionaryRef) CFArrayGetValueAtIndex( (CFArrayRef) arg, 0 );
    if( !channels ) {
      fprintf(stderr,"handshake arg[0] is empty\n" );
      channels = NULL;
      break;
    }
    
    if( !iscfdict( channels ) ) {
      fprintf(stderr,"handshake arg[0] is not a dict; is: %s\n", cftype( arg ) );
      channels = NULL;
      break;
    }
    
    if( ! CFDictionaryGetCount( channels ) ) {
      fprintf(stderr, "handshake arg[0] is empty dict\n" );
      channels = NULL;
      break;
    }

    CFRetain( channels );

    #ifdef DEBUG2
    printf("Channel list:");
    cfdump(1,channels);
    #endif
  }
  while( false );
  
  self->channels = channels;

  CFRelease( msg );
  CFRelease( arg );
  
  return 1;
}

void devUp( void *device ) {
  exitOnError( AMDeviceConnect( device ), "Connect to Device" );
  exitOnError( AMDeviceValidatePairing( device ), "Validate Device Pairing" );
  exitOnError( AMDeviceStartSession( device ), "Start Device Session" );
}

void devDown( void *device ) {
  exitOnError( AMDeviceStopSession(device), "Stop Device Session" );
  exitOnError( AMDeviceDisconnect(device), "Disconnect Device" );
}

char service__send( serviceT *self, CFTypeRef msg, tBASE * aux, uint8_t flags ) {
  channelT *chan = channel__new( self->service, self->secure, 0 );
  return channel__send( chan, msg, aux, flags );
}

char service__recv( serviceT *self, CFTypeRef *msg, CFArrayRef *aux ) {
  channelT *chan = channel__new( self->service, self->secure, 0 );
  return channel__recv( chan, msg, aux );
}

char service__call( serviceT *self, 
    const char *method, tBASE * args, uint8_t flags,
    CFTypeRef *msgOut, CFArrayRef *auxOut ) {
  channelT *chan = channel__new( self->service, self->secure, 0 );
  channel__send( chan, method, args, flags );
  return channel__recv( chan, msgOut, auxOut );
}

void service__del( serviceT *self ) {
  AMDServiceConnectionInvalidate( self->service );
}

void exitOnError( int rc, const char *src ) {
  if( !rc ) return;
  mobdev_err *info = mobdev_geterr( rc );
  fprintf( stderr, "Error on %s: %s - %s\n", src, info->name, info->error );
}

void (*onDeviceConnect)( void *device ) = NULL;
void (*onDeviceDisconnect)(void *device ) = NULL;
void onNotice( noticeInfo *info ) {
  switch( info->type )  {
    case 1: if(onDeviceConnect) onDeviceConnect( info->device ); break;
    case 2: if(onDeviceDisconnect) onDeviceDisconnect( info->device ); break; // disconnect
    case 3: break; // unsubscribed
  }
}

void waitForConnect( void (*onConnect)(void *device) ) {
  onDeviceConnect = onConnect;
  void *subscribe;
  int rc = AMDeviceNotificationSubscribe( onNotice, 0, 0, 0, &subscribe );
  if( rc < 0 ) { fprintf(stderr, "AMDeviceNotificationSubscribe err %d\n", rc); exit(1); }
  CFRunLoopRun();
}

void waitForConnectDisconnect( void (*onConnect)(void *device), void (*onDisconnect)(void *device)  ) {
  onDeviceConnect = onConnect;
  onDeviceDisconnect = onDisconnect;
  void *subscribe;
  int rc = AMDeviceNotificationSubscribe( onNotice, 0,0,0, &subscribe );
  if( rc < 0 ) { fprintf(stderr, "AMDeviceNotificationSubscribe err %d\n", rc); exit(1); }
  CFRunLoopRun();
}

void onTimeout() {
  exit(0);
}

void setupTimeout( int seconds ) {
  CFRunLoopTimerRef timer = CFRunLoopTimerCreate( NULL, CFAbsoluteTimeGetCurrent() + seconds, 0, 0, 0, onTimeout, NULL );
  CFRunLoopAddTimer( CFRunLoopGetCurrent(), timer, kCFRunLoopCommonModes );
}

char desired_device( void *device, ucmd *g_cmd ) {
  CFStringRef udidCf = AMDeviceCopyDeviceIdentifier( device );
  char *udid = str_cf2c( udidCf );
  char *goalUdid = ucmd__get( g_cmd, "-id" );
  if( goalUdid && strcmp( udid, goalUdid ) ) return 0;
  return 1;
}