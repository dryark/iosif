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

void run_ls(           ucmd *cmd ) { g_cmd = cmd; waitForConnect( runLs           ); }
void run_ps(           ucmd *cmd ) { g_cmd = cmd; waitForConnect( runPs           ); }
void run_smProcList(   ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSmProcList   ); }
void run_smSysList(    ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSmSysList    ); }
void run_smCoalList(   ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSmCoalList   ); }
void run_machTimeInfo( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runMachTimeInfo ); }

void runSmProcList( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  CFTypeRef ls;
  channel__call( chan,
    "sysmonProcessAttributes", NULL, 0,
    &ls, NULL
  );
  printf("%s",cftype( ls ) );
  
  service__del( service );
  exit(0);
}

void runSmSysList( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  CFTypeRef ls;
  channel__call( chan,
    "sysmonSystemAttributes", NULL, 0,
    &ls, NULL
  );
  printf("%s",cftype( ls ) );
  
  service__del( service );
  exit(0);
}

void runSmCoalList( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  CFTypeRef ls;
  channel__call( chan,
    "sysmonCoalitionAttributes", NULL, 0,
    &ls, NULL
  );
  printf("%s",cftype( ls ) );
  
  service__del( service );
  exit(0);
}

void runMachTimeInfo( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  CFTypeRef ls;
  channel__call( chan, "machTimeInfo", NULL, 0, &ls, NULL );
  
  uint16_t numerator   = cfi16( CFArrayGetValueAtIndex( ls, 1 ) );
  uint16_t denominator = cfi16( CFArrayGetValueAtIndex( ls, 2 ) );
  uint64_t upTimeMach  = cfi64( CFArrayGetValueAtIndex( ls, 3 ) );
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
  cfdump(0,CFArrayGetValueAtIndex( ls, 0 ));
    
  service__del( service );
  exit(0);
}

void runLs( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.deviceinfo" );
  
  CFTypeRef ls;
  channel__call( chan,
    "directoryListingForPath:", (tBASE *) tSTR__new( g_cmd->argv[0] ), 0,
    &ls, NULL
  );
  cfdump( 1, ls );
  
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
  
  CFTypeRef msg;
  
  channel__call( chan,
    "runningProcesses", NULL, 0,
    &msg, NULL
  );
  if( !msg ) {
    service__del( service );
    exit(0);
  }
  
  cfdump( 1, msg );
  
  char ok = 1;
  if( iscfarr( msg ) ) {
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
  }
  else {
    fprintf(stderr, "Process result not array; is: %s\n", cftype(msg) );
    ok = 0;
  }

  CFRelease( msg );
  
  service__del( service );
  exit(0);
}