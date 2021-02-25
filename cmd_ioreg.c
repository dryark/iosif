// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdio.h>
#include<stdlib.h>
#include"service.h"
#include"services.h"
#include"cfutil.h"
#include"uclop.h"

static ucmd *g_cmd = NULL;
void runIoreg( void *device );
void runBattery( void *device );

void run_ioreg( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runIoreg ); }
void run_battery( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runBattery ); }

void runIoreg( void *device ) {
  if( g_cmd->argc < 1 ) {
    fprintf(stderr, "Must specify ioreg key to query\n" );
    exit(1);
  }
  
  void *diagService = activateDiagService( device );
  
  CFStringRef keyCf = str_c2cf( g_cmd->argv[0] );  
  CFDictionaryRef plist = genmap( 4,
    "Request", CFSTR("IORegistry"),
    "EntryName", keyCf
  );
  
  exitOnError(
    AMDServiceConnectionSendMessage( diagService, plist, kCFPropertyListXMLFormat_v1_0 ),
    "Send X" );

  CFDictionaryRef info = NULL;
  exitOnError(
    AMDServiceConnectionReceiveMessage( diagService, &info, nil ),
    "Request Receive" );
  
  cfdump( 1, info );
  //printf("%s\n", cftype(info) );
  //cf2json( info );
  //CFPropertyListRef pl = xml2plist( str_cf2c( (CFStringRef) info ) );
  //rintf("%s\n", str_cf2c( infoStr ) );
  //printf("%s\n", cftype( pl ) );
  //CFDictionaryRef dict = CFDictionaryGetValue( pl, CFSTR("IORegistry") );
  //cfdump( 0, dict );
  
  exit(0);
}

void runBattery( void *device ) {
  void *diagService = activateDiagService( device );
  CFDictionaryRef plist = genmap( 4,
    "Request", CFSTR("IORegistry"),
    "EntryName", CFSTR("AppleARMPMUCharger")
  );
  
  exitOnError(
    AMDServiceConnectionSendMessage( diagService, plist, kCFPropertyListXMLFormat_v1_0 ),
    "Send X" );

  CFDictionaryRef info = NULL;
  exitOnError(
    AMDServiceConnectionReceiveMessage( diagService, &info, nil ),
    "Request Receive" );
  
  CFDictionaryRef diag = cfmapget( info, "Diagnostics" );
  CFDictionaryRef ioreg = cfmapget( diag, "IORegistry" );
  
  int16_t temp = cfi16( cfmapget( ioreg, "Temperature" ) );
  float tempC = (float) temp / (float) 100;
  float tempF = tempC * 9.0 / 5.0 + 32.0;
  printf( "Temperature: %.2f C / %.2f F\n", tempC, tempF );
  
  int16_t voltage= cfi16( cfmapget( ioreg, "Voltage" ) );
  float voltageF = (float) voltage / (float) 1000;
  printf( "Voltage: %.3f V\n", voltageF );
  
  int16_t cycles = cfi16( cfmapget( ioreg, "CycleCount" ) );
  printf("Cycle Count: %d\n", cycles );
  
  int16_t curMah = cfi16( cfmapget( ioreg, "AppleRawCurrentCapacity" ) );
  printf("Current Capacity: %d mAh\n", curMah );
  
  int16_t maxMah = cfi16( cfmapget( ioreg, "AppleRawMaxCapacity" ) );
  printf("Max Capacity: %d mAh\n", maxMah );
  
  int16_t designMah = cfi16( cfmapget( ioreg, "DesignCapacity" ) );
  printf("Design Capacity: %d mAh\n", designMah );
  
  CFDictionaryRef batData = cfmapget( ioreg, "BatteryData" );
  CFDictionaryRef lifeData = cfmapget( batData, "LifetimeData" );
  
  int16_t opHours = cfi16( cfmapget( lifeData, "TotalOperatingTime" ) );
  int8_t opHours1 = opHours % 24;
  opHours -= opHours1;
  int16_t opDays = opHours / 24;
  int16_t opDays1 = opDays % 365;
  opDays -= opDays1;
  int8_t opYears = opDays / 365;
  printf("Operating time: %d years, %d days, %d hours\n", opYears, opDays1, opHours1 );
  
  char *serial = str_cf2c( cfmapget( ioreg, "Serial" ) );
  printf("Serial: %s\n", serial );
  
  //cfdump( 1, info );
  //printf("%s\n", cftype(info) );
  //cf2json( info );
  //CFPropertyListRef pl = xml2plist( str_cf2c( (CFStringRef) info ) );
  //rintf("%s\n", str_cf2c( infoStr ) );
  //printf("%s\n", cftype( pl ) );
  //CFDictionaryRef dict = CFDictionaryGetValue( pl, CFSTR("IORegistry") );
  //cfdump( 0, dict );
  
  exit(0);
}