// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include<stdio.h>
#include<string.h>
#include"mobiledevice.h"
#include"uclop.h"
#include"services.h"
#include"service.h"
#include"cfutil.h"

#define BUFSIZE 4096

static ucmd *g_cmd = NULL;
void runSysLog( void *device );
void run_syslog( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSysLog ); }

void runSysLog( void *device ) {
  char *udid = ucmd__get( g_cmd, "-id" );
  
  CFStringRef devId = AMDeviceCopyDeviceIdentifier( device );
  char *audid = str_cf2c( devId );
  if( udid && strcmp( audid, udid ) ) return;
  //fprintf(stderr,"udid:%s\n", udid );
  
  void *service = activateSyslog( device );

  char buf[BUFSIZE]; 
  memset( buf, 0, BUFSIZE );
  
  int len, rc;
  while( ( rc = AMDServiceConnectionReceive( service, buf, BUFSIZE-1 ) > 0) ) {
    if( buf[0] == 0 ) {
      len = strlen( &buf[1] );       
      fwrite( &buf[1], len, 1, stdout );
    }
    else {
      len = strlen( buf );
      fputs( buf, stdout );
    }
    
    fflush(NULL);
    
    // We only need to clear the portion of the buffer used
    memset( buf, 0, len );
  }
}