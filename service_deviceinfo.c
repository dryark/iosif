// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include"bytearr.h"
#include<unistd.h>
#include"services.h"
#include"service.h"
#include"uclop.h"
#include"cfutil.h"

static ucmd *g_cmd = NULL;
void runSmProcList(   void *device );
void runSmSysList(    void *device );
void runSmCoalList(   void *device );
void runMachTimeInfo( void *device );
void runLs(           void *device );
void runPs(           void *device );
void runSysInfo(      void *device );

void run_ls(           ucmd *cmd ) { g_cmd = cmd; waitForConnect( runLs           ); }
void run_ps(           ucmd *cmd ) { g_cmd = cmd; waitForConnect( runPs           ); }
void run_smProcList(   ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSmProcList   ); }
void run_smSysList(    ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSmSysList    ); }
void run_smCoalList(   ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSmCoalList   ); }
void run_machTimeInfo( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runMachTimeInfo ); }
void run_sysinfo(      ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSysInfo      ); }

void runSysInfo( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  tBASE *ref;
  channel__call( chan,
    "systemInformation", NULL, 0,
    &ref, NULL
  );
  tBASE__dump( ref, 1 );
  
  service__del( service );
  exit(0);
}

void runSmProcList( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  //CFTypeRef ref;
  tBASE *ref;
  channel__call( chan,
    "sysmonProcessAttributes", NULL, 0,
    &ref, NULL
  );
  //printf("%s",cftype( ls ) );
  tBASE__dump( ref, 1 );
  
  service__del( service );
  exit(0);
}

void runSmSysList( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  //CFTypeRef ref;
  tBASE *ref;
  channel__call( chan,
    "sysmonSystemAttributes", NULL, 0,
    &ref, NULL
  );
  //printf("%s",cftype( ls ) );
  tBASE__dump( ref, 1 );
  
  service__del( service );
  exit(0);
}

void runSmCoalList( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  //CFTypeRef ref;
  tBASE *ref;
  channel__call( chan,
    "sysmonCoalitionAttributes", NULL, 0,
    &ref, NULL
  );
  //printf("%s",cftype( ref ) );
  tBASE__dump( ref, 1 );
  
  service__del( service );
  exit(0);
}

void runMachTimeInfo( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  //CFTypeRef ls;
  tARR *arr;
  channel__call( chan, "machTimeInfo", NULL, 0, (tBASE **) &arr, NULL );
  
  //uint16_t numerator   = cfi16( CFArrayGetValueAtIndex( arr, 1 ) );
  //uint16_t denominator = cfi16( CFArrayGetValueAtIndex( arr, 2 ) );
  //uint64_t upTimeMach  = cfi64( CFArrayGetValueAtIndex( arr, 3 ) );
  uint16_t numerator   = ( (tI16 *) tARR__get( arr, 1 ) )->val;
  uint16_t denominator = ( (tI16 *) tARR__get( arr, 2 ) )->val;
  uint64_t upTimeMach  = ( (tI64 *) tARR__get( arr, 3 ) )->val;
  upTimeMach /= 1000000000;
  uint32_t cur = upTimeMach * numerator / denominator;
  
  uint8_t seconds = cur % 60;
  cur -= seconds;
  cur /= 60;
  
  uint8_t minutes = cur % 60;
  cur -= minutes;
  cur /= 60;
  
  uint8_t hours = cur % 24;
  cur -= hours;
  cur /= 24;
  
  printf("Uptime: %d days, %02d:%02d.%02d\n", cur, hours, minutes, seconds );
  
  float time_factor = (float)numerator / (float)denominator;
  printf("Time factor: %.2f\n", time_factor );
  
  printf("Mach absolute time: ");
  //cfdump(0,CFArrayGetValueAtIndex( arr, 0 ));
  tBASE__dump( tARR__get( arr, 0 ), 1 );
    
  service__del( service );
  exit(0);
}

void runLs( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  //CFTypeRef ls;
  tBASE *ls;
  channel__call( chan,
    "directoryListingForPath:", (tBASE *) tSTR__new( g_cmd->argv[0] ), 0,
    &ls, NULL
  );
  //cfdump( 1, ls );
  tBASE__dump( ls, 1 );
  
  service__del( service );
  exit(0);
}

void runPs( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  char subsec   = ucmd__get( g_cmd, "-subsec" ) ? 1 : 0;
  char nameonly = ucmd__get( g_cmd, "-short"  ) ? 1 : 0;
  char appsonly = ucmd__get( g_cmd, "-apps"   ) ? 1 : 0;
  
  //CFTypeRef msg;
  tBASE *msg = NULL;
  
  channel__call( chan,
    "runningProcesses", NULL, 0,
    &msg, NULL
  );
  if( !msg ) {
    service__del( service );
    exit(0);
  }
  
  //cfdump( 1, msg );
  
  char ok = 1;
  /*if( iscfarr( msg ) ) {
    CFArrayRef array = (CFArrayRef) msg;

    for( size_t i = 0, size = CFArrayGetCount(array); i < size; i++ ) {
      CFDictionaryRef dict = (CFDictionaryRef) CFArrayGetValueAtIndex(array, i);
      CFStringRef nameCf;
      if( nameonly ) {
        nameCf = (CFStringRef) CFDictionaryGetValue( dict, CFSTR("name") );
      }
      else {
        nameCf = (CFStringRef) CFDictionaryGetValue( dict, CFSTR("realAppName") );
      }
      char *name = str_cf2c( nameCf );
      
      CFNumberRef _pid = (CFNumberRef) CFDictionaryGetValue( dict, CFSTR("pid") );
      int32_t pid = 0;
      CFNumberGetValue( _pid, kCFNumberSInt32Type, &pid );
      
      CFBooleanRef boolCf = (CFBooleanRef) CFDictionaryGetValue( dict, CFSTR("isApplication") );
      char isapp = CFBooleanGetValue( boolCf );
      if( appsonly && !isapp ) continue;
      
      CFDateRef start = (CFDateRef) CFDictionaryGetValue( dict, CFSTR("startDate") );
      double unix = (double) CFDateGetAbsoluteTime( start ) + kCFAbsoluteTimeIntervalSince1970;
      
      if( subsec ) {
        printf( "{ pid:%d,\n  path:\"%s\",\n  start:%.2f },\n", pid, name, unix );
      } 
      else {
        uint32_t unixSec = (uint32_t) unix;
        printf( "{ pid:%d,\n  path:\"%s\",\n  start:%d },\n", pid, name, unixSec );
      }
    }
  }*/
  if( msg->type == xfARR ) {
    tARR *array = (tARR *) msg;

    char *oneApp = ucmd__get( g_cmd, "-appname" );
    char *raw = ucmd__get( g_cmd, "-raw" );
    
    uint8_t found = 0;
    for( size_t i = 0, size = array->count; i < size; i++ ) {
      tDICT *dict = (tDICT *) tARR__get( array, i );
      if( dict->type == xfREF ) dict = (tDICT *) ( (tREF *) dict )->val;
      
      tSTR *shortNameT = (tSTR *) tDICT__get( dict, "name" );
      if( shortNameT->type == xfREF ) shortNameT = (tSTR *) ( (tREF *) shortNameT )->val;
      
      //printf("Comparing %s with %s\n", shortNameT->val, oneApp );
      if( oneApp && strcmp( shortNameT->val, oneApp ) ) continue;
      found = 1;
      if( raw ) {
        tBASE__dump( (tBASE *) dict, 1 );
        break;
      }
      
      const char *shortName = shortNameT->val;
      
      tSTR *nameT;
      if( nameonly ) nameT = shortNameT;
      else           nameT = (tSTR *) tDICT__get( dict, "realAppName" );
      if( nameT->type == xfREF ) nameT = (tSTR *) ( (tREF *) nameT )->val;
      
      const char *name = nameT->val;
      tBASE *pidT = tDICT__get( dict, "pid" );
      if( pidT->type == xfREF ) pidT = (tBASE *) ( (tREF *) pidT )->val;
      
      int32_t pid = tI__val32( pidT );
      //CFNumberGetValue( _pid, kCFNumberSInt32Type, &pid );
      
      //tBOOL *boolT = (tBOOL *) tDICT__get( dict, "isApplication" );
      //char isapp = boolT->val;
      //if( appsonly && !isapp ) continue;
      
      //CFDateRef start = (CFDateRef) CFDictionaryGetValue( dict, CFSTR("startDate") );
      //double unix = (double) CFDateGetAbsoluteTime( start ) + kCFAbsoluteTimeIntervalSince1970;
      tTIME *startT = (tTIME *) tDICT__get( dict, "startDate" );
      double unix = 0;
      if( startT ) {
        if( startT->type == xfREF ) startT = (tTIME *) ( (tREF *) startT )->val;
        unix = startT->val;// + kCFAbsoluteTimeIntervalSince1970;
      }
      else continue; // mach_kernel has no startDate... just don't output it
      if( subsec ) {
        printf( "{ pid:%" PRIu32 ",\n  path:\"%s\",\n  start:%.2f,\n  name:\"%s\" },\n", pid, name, unix, shortName );
      } 
      else {
        uint32_t unixSec = (uint32_t) unix;
        printf( "{ pid:%" PRIu32 ",\n  path:\"%s\",\n  start:%d,\n  name:\"%s\" },\n", pid, name, unixSec, shortName );
      }
    }
    if( oneApp && !found ) {
      fprintf( stderr, "Could not find running app with name %s\n", oneApp ); 
      service__del( service );
      exit(1);
    }
  }
  else {
    fprintf(stderr, "Process result not array; is: %s\n", cftype(msg) );
    ok = 0;
  }

  //CFRelease( msg );
  
  service__del( service );
  exit(0);
}