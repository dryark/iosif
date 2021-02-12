// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

void *activateService( void *device, CFTypeRef serviceName, CFDictionaryRef options ) {
  devUp( device );
  void *service;
  exitOnError( AMDeviceSecureStartService( device, serviceName, options, &service ), "Start Syslog Service" );
  devDown( device );
  return service;
}

void *activateSyslog(            void *device ) {
    CFDictionaryRef unlockEscrow = genmap( 2, "UnlockEscrowBag", kCFBooleanTrue ) ;
    return activateService( device, CFSTR("com.apple.syslog_relay"), unlockEscrow );
}
void *activateScreenshotService( void *device ) { return activateService( device, CFSTR("com.apple.mobile.screenshotr"), NULL ); }
void *activateDiagService(       void *device ) { return activateService( device, CFSTR("com.apple.mobile.diagnostics_relay"), NULL ); }

void *activateInstrumentsService( void *device, char *issecure ) {
  devUp( device );
  void *service;
  int err = AMDeviceSecureStartService( device, CFSTR("com.apple.instruments.remoteserver"), NULL, &service );
  if( err ) {
    exitOnError( 
      AMDeviceSecureStartService( 
        device, 
        CFSTR("com.apple.instruments.remoteserver.DVTSecureSocketProxy"), 
        NULL, 
        &service
      ),
      "Start Syslog Service"
    );
    *issecure = 1;
  }

  devDown( device );
  return service;
}

void *activateTestManagerService( void *device, char *issecure ) {
  devUp( device );
  void *service;
  int err = AMDeviceSecureStartService( device, CFSTR("com.apple.testmanagerd.lockdown"), NULL, &service );
  if( err ) {
    exitOnError( 
      AMDeviceSecureStartService( 
        device, 
        CFSTR("com.apple.testmanagerd.lockdown.secure"), 
        NULL, 
        &service
      ),
      "Start Syslog Service"
    );
    *issecure = 1;
  }

  devDown( device );
  return service;
}

void *activateTestRemote( void *device, char *issecure ) {
  devUp( device );
  void *service;
  exitOnError( AMDeviceStartService( device, CFSTR("com.apple.testmanagerd.remote-automation.lockdown.secure"), &service ), "Start Syslog Service" );
  devDown( device );
  return service;
  /*devUp( device );
  void *service;
  //int err = AMDeviceSecureStartService( device, CFSTR("com.apple.testmanagerd.remote-automation.lockdown"), NULL, &service );
  //if( err ) {
    exitOnError( 
      AMDeviceSecureStartService( 
        device, 
        CFSTR("com.apple.testmanagerd.remote-automation.lockdown.secure"), 
        NULL, 
        &service
      ),
      "Start Syslog Service"
    );
    *issecure = 1;
  //}

  devDown( device );
  return service;*/
  //CFDictionaryRef unlockEscrow = genmap( 2, "UnlockEscrowBag", kCFBooleanTrue ) ;
  //return activateService( device, CFSTR("com.apple.testmanagerd.remote-automation.lockdown.secure"), unlockEscrow );
}