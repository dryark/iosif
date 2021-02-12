// Copyright (c) 2021 David Helkowski
// Anti-Corruption License
#include"bytearr.h"

int g_msg_id = 0;
int g_chan_id = 0;

void runInstruments( void *device );
void run_instruments( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runInstruments ); }

void runLs( void *device );
void run_ls( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runLs ); }

void runPs( void *device );
void run_ps( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runPs ); }

static int connectChannel( CFDictionaryRef channels, void *service, char secure, CFStringRef id, int sourceChannel );
static CFDictionaryRef performHandshake( void *service, char secure );
static char printProclist( CFDictionaryRef channels, void *service, char secure );
static CFTypeRef printLs( CFDictionaryRef channels, void *service, char secure, CFStringRef path );
static CFTypeRef devInfoCall( CFDictionaryRef channels, void *service, char secure, const char *method, CFTypeRef arg );
static CFTypeRef fileBrowserCall( CFDictionaryRef channels, void *service, char secure, const char *method, CFTypeRef arg );
static CFTypeRef channelCall( CFDictionaryRef channels, void *service, char secure, CFStringRef chanName, const char *method, CFTypeRef arg );

void runLs( void *device ) {
  char secure = 0;
  void *iService = activateInstrumentsService( device, &secure );
  CFDictionaryRef channels = performHandshake( iService, secure );
  printLs( channels, iService, secure, str_c2cf( g_cmd->argv[0] ) );
  AMDServiceConnectionInvalidate( iService );
  exit(0);
}

void runPs( void *device ) {
  char secure = 0;
  void *iService = activateInstrumentsService( device, &secure );
  CFDictionaryRef channels = performHandshake( iService, secure );
  printProclist( channels, iService, secure );
  AMDServiceConnectionInvalidate( iService );
  exit(0);
}

void runInstruments( void *device ) {
  char secure = 0;
  
  void *iService = activateInstrumentsService( device, &secure );
  CFDictionaryRef channels = performHandshake( iService, secure );
  
  CFArrayRef arr = genarr( 3,
    CFSTR("/private/var/logs/lockdown_service.log.1"),
    CFSTR("com.dryark.vidtest2"),
    CFSTR("lockdown")
  );
  channelCall( channels, iService, secure, CFSTR("com.apple.instruments.server.services.filetransfer"), 
    "transferData:intoAppContainerForBundleIdentifier:filename", arr );
  
  // Random junk I was trying
  
  /*//printLs( channels, iService, secure );
  // devInfoCall( channels, iService, secure, CFSTR("lookupSysctl:"), CFSTR("kern.boottime") ); not allowed :(
  //devInfoCall( channels, iService, secure, CFSTR("systemInformation"), NULL ); // works
  //devInfoCall( channels, iService, secure, CFSTR("sysmonProcessAttributes"), NULL );
  //fileBrowserCall( channels, iService, secure, CFSTR("dataFromFileAtPath:"), CFSTR("/usr/share/misc/trace.codes") ); 
  channelCall( channels, iService, secure, CFSTR("com.apple.dt.Instruments.inlineCapabilities"), CFSTR("isAppleInternal"), NULL );*/
  
  //void *tService = activateTestManagerService( device, &secure );
  //CFDictionaryRef channels = performHandshake( tService, secure );
  /*channelCall( channels, tService, secure,
    CFSTR("dtxproxy:XCTestManager_IDEInterface:XCTestManager_DaemonConnectionInterface"),
    //CFSTR("dtxproxy:XCTDRemoteAutomationServer"),
    CFSTR("getDeviceOrientation"), NULL );*/
  //int channel = connectChannel( channels, tService, secure, CFSTR("dtxproxy:XCTestManager_IDEInterface:XCTestManager_DaemonConnectionInterface"), 0 );
  //if( channel < 0 ) {
  //  printf("Channel 1 blank\n");
  //}
  //printf("before\n");
  //void *rService = activateTestRemote( device, &secure );
  //printf("after\n");
    
  AMDServiceConnectionInvalidate( iService );
  exit(0);
}

typedef struct {
  uint32_t magic;
  uint32_t size;
  uint16_t fragId;
  uint16_t fragCount;
  uint32_t length;
  uint32_t id;
  uint32_t conversationIndex;
  uint32_t channelCode;
  uint32_t expectsReply;
} dtxmsg;

typedef struct {
  uint32_t flags;
  uint32_t arglen;
  uint64_t totlen;
} dtxpayload;

dtxmsg *dtxmsg__new( int channel, uint32_t id, char expectsReply, uint32_t length ) {
  dtxmsg *self = (dtxmsg *) calloc( 1, sizeof( dtxmsg ) );
  self->magic = 0x1F3D5B79;
  self->size = sizeof( dtxmsg );
  self->fragId = 0;
  self->fragCount = 1;
  self->length = length;
  self->id = id;
  self->conversationIndex = 0;
  self->channelCode = channel;
  self->expectsReply = expectsReply;
  return self;
}

dtxpayload *dtxpayload__new( char expectsReply, uint32_t arglen, uint64_t totlen ) {
  dtxpayload *self = (dtxpayload *) calloc( 1, sizeof( dtxpayload ) );
  self->flags = 0x2 | (expectsReply ? 0x1000 : 0);
  self->arglen = arglen;
  self->totlen = totlen;
  return self;
}

static char sendDtxMessage( void *service, char secure, int channel, const char *method, CFTypeRef argsCf, char expects_reply, char argSecure ) {
  uint32_t id = ++g_msg_id;

  int32_t argsLen = 0;;
  uint8_t *argsBytes;
  if( argsCf ) {
    bytearr *args = cfarr2aux( argsCf, argSecure );
    //bytearr__auxcf( args, argsCf );
    //printf("Made aux cf\n");
    
    argsBytes = bytearr__asaux( args, &argsLen );
  }
  
  int methodLen;
  uint8_t *methodBytes = NULL;
  if( method ) methodBytes = ( uint8_t * ) strArchive( method, &methodLen );
  
  dtxpayload *payload = dtxpayload__new( expects_reply, argsLen, argsLen + methodLen );
  
  //printf("Totallength: %i = argsLen %i + methodLen %i\n", argsLen + methodLen, argsLen, methodLen );

  dtxmsg *msg = dtxmsg__new( channel, id, expects_reply, sizeof(dtxpayload) + payload->totlen );
  
  bytearr *msgb = bytearr__new();
  bytearr__append( msgb, (uint8_t *) msg,     sizeof( dtxmsg ),     0 );
  bytearr__append( msgb, (uint8_t *) payload, sizeof( dtxpayload ), 0 );
  bytearr__append( msgb, argsBytes,   argsLen,   0 );
  bytearr__append( msgb, methodBytes, methodLen, 0 );

  int msgLen;
  uint8_t *msgBytes = bytearr__bytes( msgb, &msgLen ); 

  /*printf("Sending DTXMessage:");
  uint32_t crc = crc32(0,(const char *) msgBytes,msgLen);
  printf("%08x", crc );
  printf("\n");
  for( int i=0;i<msgLen;i++ ) {
    unsigned char byte = msgBytes[i];
    printf("%02x", byte );
  }
  printf("\n\n");*/
  
  ssize_t sentLen = secure
                  ? AMDServiceConnectionSend( service, msgBytes, msgLen )
                  : write( AMDServiceConnectionGetSocket( service ), msgBytes, msgLen );
  
  if( sentLen != msgLen ) {
    fprintf(stderr, "Failed to send 0x%lx bytes of message: %s\n", (unsigned long) msgLen, strerror(errno));
    return 0;
  }

  return 1;
}

static char recvDtxMessage( void *service, char secure, CFTypeRef *msgOut, CFArrayRef *argOut ) {
  uint32_t id = 0;
  
  bytearr *payloadb = bytearr__new();

  int sock = secure ? 0 : AMDServiceConnectionGetSocket( service );

  for(;;) {
    dtxmsg msg;
    ssize_t nrecv = secure
                  ? AMDServiceConnectionReceive( service, (char *) &msg, sizeof(dtxmsg) )
                  : read(sock, &msg, sizeof(dtxmsg));
    if( nrecv != sizeof(dtxmsg) ) { fprintf(stderr, "read error: %s, nrecv = %lx\n", strerror(errno), nrecv); return false; }
    
    //printf("Received header\n");

    if( msg.magic != 0x1F3D5B79 ) { fprintf(stderr, "invalid magic: %x\n", msg.magic); return 0; }
    
    //printf("Header magic ok\n");

    if( !msg.conversationIndex ) { // start of conversation
      if( msg.id > g_msg_id ) {
        g_msg_id = msg.id; // fast forward
        //printf("Advancing to msgId %i\n", msg.id );
      }
      else if( msg.id < g_msg_id ) { fprintf( stderr, "bad msg.id: %d\n", msg.id ); return 0; }
    }
    
    if( !msg.fragId ) {
      id = msg.id;
      if( msg.fragCount > 1 ) continue; // frag 0 contains only header
    }

    bytearr *frag = bytearr__new();
    
    bytearr__append( frag, (uint8_t *) &msg, sizeof( dtxmsg ), 0 );
    
    int64_t dataLen = msg.length;
    //printf("Message length: %i\n", (int) dataLen );
    uint8_t *dataBlock = (uint8_t *) malloc( dataLen );

    uint32_t received = 0;
    while ( received < dataLen ) {
      uint8_t *ptr = dataBlock + received;
      size_t remaining = dataLen - received;
      uint32_t part = secure
        ? AMDServiceConnectionReceive( service, (char *) ptr, remaining )
        : read( sock, ptr, remaining );
      if( nrecv <= 0 ) { fprintf( stderr, "read error: %s\n", strerror(errno) ); return 0; }
      received += part;
    }
    //printf("Received all bytes\n");
    
    bytearr__append( payloadb, dataBlock, dataLen, 0 );

    if ( msg.fragId == msg.fragCount - 1 ) break;
  }
  //printf("Got all fragments\n");

  int payloadLen;
  uint8_t *payloadBytes = bytearr__bytes( payloadb, &payloadLen );
  //printf("Size of received payload: %i\n", payloadLen );
  //printf("Size of payload generally: %i\n", (int) sizeof( dtxpayload ) );
  
  const dtxpayload *payload = (const dtxpayload *) payloadBytes;

  if( payload->flags & 0xFF000 ) { fprintf( stderr, "compression unsupported\n" ); return 0; }

  // argument array is after payload header
  const uint8_t *argptr = payloadBytes + sizeof( dtxpayload );
  uint32_t arglen = payload->arglen;
  //printf("Arg len:%i\n", arglen );

  // output message is after the argument array
  const uint8_t *objptr = argptr + arglen;
  uint64_t objlen = payload->totlen - arglen;

  if( arglen && argOut ) {
    CFArrayRef arg = deserialize( argptr, arglen );
    if( !arg ) return 0;
    *argOut = arg;
  }
  else if( argOut ) *argOut = NULL;

  //printf("Sizeof obj: %i\n", (int) objlen );
  if( objlen && msgOut ) {
    CFTypeRef cfMsg = archive2cf( (uint8_t *) objptr, objlen );
    //cfdump( 0, cfMsg );
    *msgOut = cfMsg;
  }
  else if( msgOut ) {
    *msgOut = NULL;
  }

  return 1;
}

static CFDictionaryRef performHandshake( void *service, char secure ) {
  CFDictionaryRef caps = genmap( 4,
    "com.apple.private.DTXConnection", i64cf( 1 ),
    "com.apple.private.DTXBlockCompression", i64cf( 2 )
  );
    
  //printf("Handshake msgId:%i\n", cur_message + 1 );
  if( !sendDtxMessage( service, secure, 0, "_notifyOfPublishedCapabilities:", caps, 1, 0 ) ) return false;
  
  //printf("Sent handshake\n");
  
  CFRelease( caps );

  CFTypeRef msg = NULL;
  CFArrayRef arg = NULL;

  if( !recvDtxMessage( service, secure, &msg, &arg ) || !msg || !arg ) { fprintf(stderr, "recvDtxMessage failed:\n"); return false; }
  
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

    //printf("channel list:\n%s\n", cftype(channels) );
  }
  while( false );

  CFRelease( msg );
  CFRelease( arg );

  return channels;
}

static int connectChannel( CFDictionaryRef channels, void *service, char secure, CFStringRef id, int sourceChannel ) {
  // testmanagerd uses hidden channels :(
  //if( !CFDictionaryContainsKey( channels, id ) ) {
  //  fprintf( stderr, "channel %s not in list\n", str_cf2c(id) );
  //  return -1;
  //}

  int code = ++g_chan_id;

  //printf("Connecting channel: %i\n", code );
  //printf("Connect msgId:%i\n", cur_message + 1 );
  
  CFArrayRef args = genarr( 2, i32cf( code ), id );
  CFTypeRef msg = NULL;

  if( !sendDtxMessage( service, secure, sourceChannel, "_requestChannelWithCode:identifier:", args, 0, 1 ) ) return -1;
  if( !recvDtxMessage( service, secure, &msg, NULL ) ) return -1;
  
  if( msg ) {
    fprintf( stderr, "Error: _requestChannelWithCode:identifier: returned %s\n", cftype(msg) );
    cfdump( 0, msg );
    CFRelease( msg );
    return -1;
  }
  
  return code;
}

static CFTypeRef channelCall( CFDictionaryRef channels, void *service, char secure, CFStringRef chanName, const char *method, CFTypeRef arg ) {
  int channel = connectChannel( channels, service, secure, chanName, 0 );
  if( channel < 0 ) return 0;

  //printf("Got channel:%i\n", channel );
  CFTypeRef msg = NULL;

  if( !sendDtxMessage( service, secure, channel, method, arg, 1, 0 ) ) {
    fprintf( stderr, "sendDtx to %s failed\n", method );
    return 0;
  }
  
  if( !recvDtxMessage( service, secure, &msg, NULL ) ) {
    fprintf( stderr, "recvDtx from %s failed\n", method );
    return 0;
  }
  if( !msg ) {
    fprintf( stderr, "recvDtx from %s - no response\n", method );
    return 0;
  }
  
  //cfdump( 0, msg );
  printf("%s\n", cftype(msg) );
  
  //CFRelease( msg );
  return msg;
}

static CFTypeRef fileBrowserCall( CFDictionaryRef channels, void *service, char secure, const char *method, CFTypeRef arg ) {
  return channelCall( channels, service, secure, CFSTR("com.apple.instruments.server.services.filebrowser" ), method, arg );
}

static CFTypeRef devInfoCall( CFDictionaryRef channels, void *service, char secure, const char *method, CFTypeRef arg ) {
  return channelCall( channels, service, secure, CFSTR("com.apple.instruments.server.services.deviceinfo" ), method, arg );
}

static CFTypeRef printLs( CFDictionaryRef channels, void *service, char secure, CFStringRef path ) {
  return devInfoCall( channels, service, secure, "directoryListingForPath:", path );
}

static char printProclist( CFDictionaryRef channels, void *service, char secure ) {
  CFTypeRef msg = devInfoCall( channels, service, secure, "runningProcesses", NULL );
  if( !msg ) return 0;
    
  char ok = 1;
  if( iscfarr( msg ) ) {
    CFArrayRef array = (CFArrayRef) msg;

    for( size_t i = 0, size = CFArrayGetCount(array); i < size; i++ ) {
      CFDictionaryRef dict = (CFDictionaryRef) CFArrayGetValueAtIndex(array, i);

      //CFStringRef nameCf = (CFStringRef) CFDictionaryGetValue( dict, CFSTR("name") );
      CFStringRef nameCf = (CFStringRef) CFDictionaryGetValue( dict, CFSTR("realAppName") );
      char *name = str_cf2c( nameCf );
      
      CFNumberRef _pid = (CFNumberRef) CFDictionaryGetValue( dict, CFSTR("pid") );
      int16_t pid = 0;
      CFNumberGetValue( _pid, kCFNumberSInt16Type, &pid );

      printf( "{ pid:%d,\n  path:\"%s\" },\n", pid, name );
    }
  }
  else {
    fprintf(stderr, "Process result not array; is: %s\n", cftype(msg) );
    ok = 0;
  }

  CFRelease( msg );
  return ok;
}