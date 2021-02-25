// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include<stdlib.h>
#include"uclop.h"
#include"cfutil.h"
#include"mobiledevice.h"
#include"service.h"
#include"services.h"

static ucmd *g_cmd = NULL;
void runGG( void *device );
void run_gg( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runGG ); }

void runGG( void *device ) {
  void *diagService = activateDiagService( device );
  CFDictionaryRef plist = genmap( 2, "Request", CFSTR("GasGauge") );
  
  exitOnError(
    AMDServiceConnectionSendMessage( diagService, plist, kCFPropertyListXMLFormat_v1_0 ),
    "Send X" );

  CFDictionaryRef info = NULL;
  exitOnError(
    AMDServiceConnectionReceiveMessage( diagService, &info, nil ),
    "Request Receive" );
  
  cfdump( 0, info );
  
  exit(0);
}