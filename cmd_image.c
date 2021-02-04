// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

void runImage( void *device );
void run_img( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runImage ); }

void runImage( void *device ) {
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
        FILE *fh = fopen( "test.png", "w" );
        if( !fh ) {
          printf("Could not open test.png for writing\n");
          exit(1);
        }
        fwrite( bytes, len, 1, fh );
        free( bytes );
        fclose( fh );
      }
    }
  }
    
  exit(0);
}