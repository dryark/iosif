// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdio.h>
#include"cfutil.h"
#include"mobiledevice.h"
#include"uclop.h"
#include"service.h"

static ucmd *g_cmd = NULL;
void runList( void *device );
void run_list( ucmd *cmd ) {
  g_cmd = cmd;
  setupTimeout(1);
  waitForConnect( runList );
}

void detectConnect( void *device );
void detectDisconnect( void *device );
void run_detect( ucmd *cmd ) {
  g_cmd = cmd;
  waitForConnectDisconnect( detectConnect, detectDisconnect );
}

void detectConnect( void *device ) {
  devUp( device );
  CFStringRef udidCf = AMDeviceCopyDeviceIdentifier( device );
  char *udid = str_cf2c( udidCf );
  CFTypeRef nameCf = AMDeviceCopyValue( device, NULL, CFSTR("DeviceName") );
  char *name = str_cf2c( nameCf );
  devDown( device );
  for( int i=0;i<strlen(name);i++ ) if( name[i]=='\"' ) name[i]='`';
  fprintf(stderr,"{type:\"connect\",udid:\"%s\",name:\"%s\"}\n",udid,name);
}

void detectDisconnect( void *device ) {
  CFStringRef udidCf = AMDeviceCopyDeviceIdentifier( device );
  char *udid = str_cf2c( udidCf );
  fprintf(stderr,"{type:\"disconnect\",udid:\"%s\"}\n",udid);
}

void runList( void *device ) {
  char g_json = ucmd__get( g_cmd, "-json" ) ? 1 : 0;
  
  CFStringRef devId = AMDeviceCopyDeviceIdentifier( device );
  char *udid = str_cf2c( devId );
  if( g_json ) {
    printf("[udid:\"%s\"],\n", udid );
  } else {
    fprintf(stderr,"udid:%s\n", udid );
  }
}