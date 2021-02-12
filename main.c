// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdio.h>
#include<stdarg.h>
#include<CoreFoundation/CoreFoundation.h>
#include"mobdev_err.h"
#include"cfutil.h"
#include"nsutil.h"
#include"uclop.h"
#include"mobiledevice.h"
#include"bytearr.h"

#define BUFSIZE 4096

void exitOnError( int rc, const char *src ) {
  if( !rc ) return;
  mobdev_err *info = mobdev_geterr( rc );
  fprintf( stderr, "Error on %s: %s - %s\n", src, info->name, info->error );
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
void (*onDeviceDisconnect)(void *device );
void onNotice( noticeInfo *info ) {
  switch( info->type )  {
    case 1: onDeviceConnect( info->device ); break;
    case 2: onDeviceDisconnect( info->device ); break; // disconnect
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

void waitForConnectDisconnect( void (*onConnect)(void *device), void (*onDisconnect)(void *device)  ) {
  onDeviceConnect = onConnect;
  onDeviceDisconnect = onDisconnect;
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
#include "cmd_instruments.c"
#include "cmd_bytetest.c"

void nopDeviceAction( void *device ) {
}

int main (int argc, char **argv) {
  uopt *list_options[] = {
    UOPT_FLAG("-json","Output JSON format"),
    NULL
  };
  uopt *udid_option[] = {
    UOPT("-id","UDID of device"),
    NULL
  };
  uopt *udid_w_json_option[] = {
    UOPT_FLAG("-json","Output JSON format"),
    UOPT("-id","UDID of device"),
    NULL
  };
  
  onDeviceConnect = nopDeviceAction;
  onDeviceDisconnect = nopDeviceAction;
  //CFPreferencesSetAppValue(CFSTR("LogLevel"), easynum(9), CFSTR("com.apple.MobileDevice"));
  
  uclop *opts = uclop__new( NULL, NULL );
  uclop__addcmd( opts, "log",        "Syslog",         &run_syslog,      udid_option );
  uclop__addcmd( opts, "list",       "List Devices",   &run_list,        list_options );
  uclop__addcmd( opts, "detectloop", "Detect Loop",    &run_detect,      NULL );
  uclop__addcmd( opts, "img",        "Get screenshot", &run_img,         NULL );
  uclop__addcmd( opts, "mg",         "Mobile Gestalt", &run_mg,          udid_w_json_option );
  uclop__addcmd( opts, "gas",        "Gas Guage",      &run_gg,          NULL );
  uclop__addcmd( opts, "info",       "Info",           &run_info,        udid_w_json_option );
  uclop__addcmd( opts, "syscfg",     "Get SysCfg",     &run_syscfg,      NULL );
  uclop__addcmd( opts, "inst",       "Instruments",    &run_instruments, NULL );
  uclop__addcmd( opts, "ls",         "ls",             &run_ls,          NULL );
  uclop__addcmd( opts, "ps",         "Process list",   &run_ps,          NULL );
  uclop__addcmd( opts, "btest",      "Bytearr Test",   &run_bytetest,    NULL );
  ucmd *tun = uclop__addcmd( opts, "tunnel", "Tunnel", &run_tunnel, udid_option );
  tun->extrahelp = "[from]:[to] [[from]:[to]...]";
  uclop__run( opts, argc, argv );
}
