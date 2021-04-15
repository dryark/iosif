// Copyright (c) 2021 David Helkowski
// Anti-Corruption License
#include<CoreFoundation/CoreFoundation.h>
#include"service.h"
#include"cfutil.h"

void *activateService( void *device, CFTypeRef serviceName, CFDictionaryRef options ) {
  devUp( device );
  void *service;
  exitOnError( AMDeviceSecureStartService( device, serviceName, options, &service ), "Some Service" );
  devDown( device );
  return service;
}

void *activateSyslog( void *device ) {
    CFDictionaryRef unlockEscrow = genmap( 2, "UnlockEscrowBag", kCFBooleanTrue ) ;
    return activateService( device, CFSTR("com.apple.syslog_relay"), unlockEscrow );
}
void *activateScreenshotService( void *device ) {
  return activateService( device, CFSTR("com.apple.mobile.screenshotr"), NULL );
}
void *activateDiagService( void *device ) {
  return activateService( device, CFSTR("com.apple.mobile.diagnostics_relay"), NULL );
}

serviceT *service__new_instruments( void *device ) {
  serviceT *self = service__new( device,
    "com.apple.instruments.remoteserver",
    "com.apple.instruments.remoteserver.DVTSecureSocketProxy" );
  service__handshake( self );
  return self;
}

serviceT *service__new_afc( void *device ) {
  serviceT *self = service__new( device,
    "com.apple.afc",
    NULL );
  return self;
}

serviceT *service__new_testmanagerd( void *device ) {
  serviceT *self = service__new( device,
    "com.apple.testmanagerd.lockdown",
    "com.apple.testmanagerd.lockdown.secure" );
  service__handshake( self );
  return self;
}

serviceT *service__new_remoteautomation( void *device ) {
  serviceT *self = service__new( device,
    "com.apple.testmanagerd.remote-automation.lockdown",
    "com.apple.testmanagerd.remote-automation.lockdown.secure" );
  return self;
}