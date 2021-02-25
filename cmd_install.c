// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include<stdlib.h>
#include"service.h"
#include"cfutil.h"
#include"uclop.h"

static ucmd *g_cmd = NULL;
void runInstall( void *device );
void run_install( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runInstall ); }

int transferStatus( CFDictionaryRef dict, int x ) {
  //cfdump( 1, dict );
  CFNumberRef pCf = cfmapget( dict, "PercentComplete" );
  if( pCf ) {
    uint16_t percent = cfi16( pCf );
    printf("\rCopying:%d%%", percent );
  }
  return 0;
}

int installStatus( CFDictionaryRef dict, int x ) {
  CFNumberRef pCf = cfmapget( dict, "PercentComplete" );
  if( pCf ) {
    uint16_t percent = cfi16( pCf );
    printf("\rInstalling:%d%%", percent );
  }
  //cfdump( 1, dict );
  return 0;
}

void runInstall( void *device ) {
  char *pathAbs = realpath( ucmd__get( g_cmd, "-path" ), NULL );
  CFStringRef pathCf = str_c2cf( pathAbs );
  CFURLRef absUrl = CFURLCreateWithFileSystemPath( NULL, pathCf, kCFURLPOSIXPathStyle, 0 );
  
  devUp( device );
  
  void *afcConn;
  AMDeviceSecureStartService( device, CFSTR("com.apple.afc"), NULL, &afcConn );
  
  CFDictionaryRef map = genmap( 2, "PackageType", CFSTR("Developer") );
  AMDeviceSecureTransferPath( 0, device, absUrl, map, transferStatus, 0 );
  printf("\rCopying:100%%\n");
  close( *( (int*) afcConn ) );
  
  AMDeviceSecureInstallApplication( 0, device, absUrl, map, installStatus, 0 );
  printf("\rInstalling:100%%\n");
  
  free( pathAbs );
  
  devDown( device );
  
  exit(0);
}