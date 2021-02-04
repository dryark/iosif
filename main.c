// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdio.h>
#include<stdarg.h>
#include<CoreFoundation/CoreFoundation.h>
#include"errors.h"
#include"cfutil.c"
#include"uclop.h"
#include"mobiledevice.h"

#define BUFSIZE 4096

void exitOnError( int rc, const char *src ) {
  if( !rc ) return;
  const char *error_message = get_error_message(rc);                           \
  fprintf( stderr, "%s: %s\n", src, error_message );
}

void devUp( void *device ) {
  exitOnError( AMDeviceConnect( device ), "Connect to Device" );
  exitOnError( AMDeviceValidatePairing( device ), "Validate Device Pairing" );
  exitOnError( AMDeviceStartSession( device ), "Start Device Session" );
}

void devDown( void *device ) {
  exitOnError( AMDeviceStopSession(device), "Stop Device Session" );
  exitOnError( AMDeviceDisconnect(device), "Disconnect Device" );
}

void onTimeout() {
  exit(0);
}

void setupTimeout( int seconds ) {
  CFRunLoopTimerRef timer = CFRunLoopTimerCreate( NULL, CFAbsoluteTimeGetCurrent() + seconds, 0, 0, 0, onTimeout, NULL );
  CFRunLoopAddTimer( CFRunLoopGetCurrent(), timer, kCFRunLoopCommonModes );
}

ucmd *g_cmd = NULL;

void (*onDeviceConnect)( void *device );
void onNotice( noticeInfo *info ) {
  switch( info->type )  {
    case 1: onDeviceConnect( info->device ); break;
    case 2: break; // disconnect
    case 3: break; // unsubscribed
  }
}

void waitForConnect( void (*onConnect)(void *device) ) {
  onDeviceConnect = onConnect;
  void *subscribe;
  int rc = AMDeviceNotificationSubscribe( onNotice, 0,0,0, &subscribe );
  if( rc < 0 ) { fprintf(stderr, "AMDeviceNotificationSubscribe err %d\n", rc); exit(1); }
  CFRunLoopRun();
}

#include "services.c"
#include "cmd_syslog.c"
#include "cmd_list.c"
#include "cmd_mg.c"
#include "cmd_gg.c"
#include "cmd_image.c"
#include "cmd_tunnel.c"
#include "cmd_info.c"
#include "cmd_syscfg.c"

int main (int argc, char **argv) {
  uopt *list_options[] = {
    UOPT_FLAG("-json","Output JSON format"),
    NULL
  };
  uopt *log_options[] = {
    UOPT("-id","UDID of device"),
    NULL
  };
  
  //CFPreferencesSetAppValue(CFSTR("LogLevel"), easynum(9), CFSTR("com.apple.MobileDevice"));
  
  uclop *opts = uclop__new( NULL, NULL );
  uclop__addcmd( opts, "log",  "Syslog",         &run_syslog, log_options );
  uclop__addcmd( opts, "list", "List Devices",   &run_list,   list_options );
  uclop__addcmd( opts, "img",  "Get screenshot", &run_img,    NULL );
  uclop__addcmd( opts, "mg",   "Mobile Gestalt", &run_mg,     list_options );
  uclop__addcmd( opts, "gas",  "Gas Guage",      &run_gg,     NULL );
  uclop__addcmd( opts, "info", "Info",           &run_info,   NULL );
  uclop__addcmd( opts, "syscfg", "Get SysCfg",   &run_syscfg, NULL );
  ucmd *tun = uclop__addcmd( opts, "tunnel", "Tunnel",       &run_tunnel, NULL );
  tun->extrahelp = "[from] [to] [[from] [to]...]";
  uclop__run( opts, argc, argv );
}
