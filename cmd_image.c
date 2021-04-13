// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include"uclop.h"
#include"cfutil.h"
#include"mobiledevice.h"
#include"service.h"
#include"services.h"
#ifdef NNG
#include<nng/nng.h>
#include<nng/protocol/reqrep0/rep.h>
#endif

static ucmd *g_cmd = NULL;
void runImage( void *device );
void run_img( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runImage ); }

#ifdef NNG
void runIServer( void *device );
void run_iserver( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runIServer ); }
#endif

void fatal( char *str, int rv ) {
  fprintf(stderr,"Err %s ; rv=%d\n", str, rv );
  //exit(1);
}

void *initScreenshotService( void *device ) {
  void *imgService = activateScreenshotService( device );
  
  CFTypeRef info = NULL;
  exitOnError( 
    AMDServiceConnectionReceiveMessage( imgService, &info, nil ),
    "Receive Intro" );
  
  exitOnError(
    AMDServiceConnectionSendMessage(
      imgService,
      genarr( 2,
        CFSTR("DLMessageVersionExchange"),
        CFSTR("DLVersionsOk")
      ), kCFPropertyListXMLFormat_v1_0
    ),
    "Accept Version"
  );
  
  exitOnError( 
    AMDServiceConnectionReceiveMessage(imgService, &info, nil),
    "Receive Version Accept" );
    
  return imgService;
}

void *getImage( void *imgService, size_t *lenOut ) {
  exitOnError(
    AMDServiceConnectionSendMessage(
      imgService,
      genarr( 2,
        CFSTR( "DLMessageProcessMessage" ),
        genmap( 2, "MessageType", CFSTR("ScreenShotRequest") )
      ), kCFPropertyListXMLFormat_v1_0
    ),
    "Send SC Request"
  );
    
  CFTypeRef result = NULL;
  exitOnError(
    AMDServiceConnectionReceiveMessage( imgService, &result, nil ),
    "Get SC"
  );
  
  if( result ) {
    CFDictionaryRef data = CFArrayGetValueAtIndex( result, 1 );
    if( data ) {
      CFTypeRef image = CFDictionaryGetValue( data, CFSTR("ScreenShotData") );
      if( image ) {
        int len = CFDataGetLength( image );
        
        CFRange range = CFRangeMake(0,len);
        char *bytes = malloc( len );
        CFDataGetBytes( image, range, (UInt8 *) bytes );
        
        *lenOut = len;
        CFRelease( result );
        return bytes;
      }
    }
  }
  CFRelease( result );
  return 0;
}

#ifdef NNG
void runIServer( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  void *imgService = initScreenshotService( device );
  
  int port = 8271;
  char *portStr = ucmd__get( g_cmd, "-port" );
  if( portStr ) port = atoi( portStr );
  
  char url[50];
  sprintf( url, "tcp://127.0.0.1:%d", port );
  
  nng_socket sock;
  int rv;
  
  rv = nng_rep0_open( &sock );
  if( rv ) fatal("nng_rep0_open",rv);
  
  rv = nng_listen( sock, url, NULL, 0 );
  if( rv ) fatal("nng_listen",rv);
  
  printf("listening on %s\n", url );
  fflush( stdout );
  
  void *lastdata;
  size_t lastlen;
  
  int n = 0;
  for(;;) {
    //printf("n:%i\n", n );
    char *buf = NULL;
    size_t size;
    rv = nng_recv( sock, &buf, &size, NNG_FLAG_ALLOC );
    if( rv ) fatal("nng_recv",rv);
    if( !size ) fatal("recv empty",0);
    
    //printf("Received %.*s\n", (int) size, buf );
    
    if( !strncmp( buf, "img", 3 ) ) {
      char *left = &buf[ 4 ];
      int id = atoi( left );
      printf("id:%d\n", id );
      if( id <= n ) {
        nng_free( buf, size );
        printf("Duplicate\n");
        nng_send( sock, lastdata, lastlen, 0 );
        continue;
      }
      n = id;
      
      size_t imgLen;
      
      //printf("Fetching image\n");
      void *data = getImage( imgService, &imgLen );
      //printf("Fetch done\n");
      
      if( data ) {
        rv = nng_send( sock, data, imgLen, 0 );
        if( lastdata ) free( lastdata );
        lastdata = data;
        lastlen = imgLen;
        //free( data );
      }
      else {
        fprintf(stderr, "Did not get image\n" );
      }
      //nng_send( sock, "test", 5, 0 );
    }
    else {
      rv = nng_send( sock, "what?", 6, 0 ); 
    }
    nng_free( buf, size );
  }
  
  exit(0);
}
#endif

void runImage( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  void *imgService = initScreenshotService( device );
  
  size_t imgLen;
  void *data = getImage( imgService, &imgLen );
  
  if( data ) {
    FILE *fh = fopen( "test.png", "w" );
    if( !fh ) {
      printf("Could not open test.png for writing\n");
      exit(1);
    }
    fwrite( data, imgLen, 1, fh );
    free( data );
    fclose( fh );
  }
    
  exit(0);
}