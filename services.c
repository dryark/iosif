// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

void *activateService( void *device, CFTypeRef serviceName, char escrow ) {
  devUp( device );
  
  void *service;
  
  CFDictionaryRef unlockEscrow = escrow ? genmap( 2, "UnlockEscrowBag", kCFBooleanTrue ) : NULL;
  
  exitOnError(
    AMDeviceSecureStartService( device, serviceName, unlockEscrow, &service ),
    "Start Syslog Service"
  );
  
  devDown( device );
  
  return service;
}

void *activateSyslog(            void *device ) { return activateService( device, CFSTR("com.apple.syslog_relay"), 1 ); }
void *activateScreenshotService( void *device ) { return activateService( device, CFSTR("com.apple.mobile.screenshotr"), 0 ); }
void *activateDiagService(       void *device ) { return activateService( device, CFSTR("com.apple.mobile.diagnostics_relay"), 0 ); }