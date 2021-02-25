// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include<stdlib.h>
#include"cfutil.h"
#include"uclop.h"
#include"mobiledevice.h"
#include"service.h"
#include"services.h"

static ucmd *g_cmd = NULL;
void runMg( void *device );
void run_mg( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runMg ); }

CFDictionaryRef runMgRaw( void *device, CFArrayRef keys ) {
  void *diagService = activateDiagService( device );
  
  CFDictionaryRef plist = genmap( 4,
    "Request", CFSTR("MobileGestalt"),
    "MobileGestaltKeys", keys
  );
  
  exitOnError(
    AMDServiceConnectionSendMessage( diagService, plist, kCFPropertyListXMLFormat_v1_0 ),
    "Send X" );
  
  CFDictionaryRef info = NULL;
  exitOnError(
    AMDServiceConnectionReceiveMessage( diagService, &info, nil ),
    "Request Receive" );
  
  CFStringRef status = CFDictionaryGetValue( info, CFSTR("Status") );
  if( !status ) {
    printf("Failure; did not get status back\n");
    exit(1);
  }
  char *statc = str_cf2c( status );
  if( strcmp( statc, "Success" ) ) {
    printf("Failure:%s\n", statc);
    exit(1);
  }
    
  CFDictionaryRef diag = CFDictionaryGetValue( info, CFSTR("Diagnostics") );
  if( !diag ) {
    printf("No Diagnostics node in result\n");
    exit(1);
  }
  
  CFDictionaryRef mg = CFDictionaryGetValue( diag, CFSTR("MobileGestalt") );
  if( !mg ) {
    printf("No Diagnostics.MobileGestalt in result\n");
    exit(1);
  }
  
  return mg;
}

void runMg( void *device ) {
  CFStringRef udidCf = AMDeviceCopyDeviceIdentifier( device );
  char *udid = str_cf2c( udidCf );
  char *goalUdid = ucmd__get( g_cmd, "-id" );
  if( goalUdid && strcmp( udid, goalUdid ) ) return;
  
  char useJson = ucmd__get( g_cmd, "-json" ) ? 1 : 0;
  
  int count = g_cmd->argc;
  
  char *defaultNames[] = { "UniqueDeviceID","main-screen-width","main-screen-height" };
  char **namesC;
  if( count == 0 ) {
    count = 3;
    namesC = defaultNames;
  }
  else namesC = g_cmd->argv;
  
  CFArrayRef names = strsToArr( count, namesC );
  
  CFDictionaryRef mg = runMgRaw( device, names ); 
  
  if( useJson ) printf("{\n");
  for( int i=0;i<count;i++ ) {
    char *name = namesC[i];
    CFTypeRef val = CFDictionaryGetValue( mg, str_c2cf( name ) );
    if( !val ) {
      printf("\"%s\":NOT FOUND\n",name);
      continue;
    }
    if( useJson ) {
      printf("\"%s\":",name);
      cfdump( 1, val );
      if( i != ( count - 1 ) ) printf(",\n");
      else printf("\n");
    }
    else {
      printf("%s:",name);
      cfdump( 1, val );
    }
  }
  if( useJson ) printf("}\n");
    
  exit(0);
}