// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include"cfutil.h"
#include"uclop.h"
#include"service.h"

static ucmd *g_cmd = NULL;
void runListApps( void *device );
void run_listapps( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runListApps ); }

void runListApps( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  devUp( device );
  
  CFDictionaryRef res;
  
  // could also use installation_proxy -> Browse ?
  // Entitlements."com.apple.developer.team-identifier"
  // SignerIdentity
  AMDeviceLookupApplications( device, genmap( 6,
    "ApplicationType", CFSTR("Any"),
    "ReturnAttributes", boolcf(1),
    "ShowLaunchProhibitedApps", boolcf(1)
  ), &res );
  
  devDown( device );
  
  char *bi = ucmd__get( g_cmd, "-bi" );
  if( bi ) {
    int keyCnt = CFDictionaryGetCount( res );
    CFTypeRef *keysTypeRef = (CFTypeRef *) malloc( keyCnt * sizeof(CFTypeRef) );
    CFDictionaryGetKeysAndValues( res, (const void **) keysTypeRef, NULL);
    const void **keys = (const void **) keysTypeRef;
    uint8_t found = 0;
    for( int i=0;i<keyCnt;i++ ) {
      CFStringRef cfkey = keys[ i ];
      char *key = str_cf2c( cfkey );
      if( !strcmp( key, bi ) ) {
        CFDictionaryRef item = CFDictionaryGetValue( res, cfkey );
        cfdump( 1, item );
        found = 1;
      }
    }
    if( !found ) {
      fprintf( stderr, "Could not find installed app with bundle identifier %s\n", bi );
      exit(1);
    }
  }
  else {
    cfdump( 1, res );
  }
  
  exit(0);
}