#include<CoreFoundation/CoreFoundation.h>
#include"bytearr.h"
#include<unistd.h>
#include"services.h"
#include"service.h"
#include"uclop.h"
#include"cfutil.h"

static ucmd *g_cmd = NULL;
void runSysmon(  void *device );
void run_sysmon(  ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSysmon       ); }

void runSysmon( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  serviceT *service = service__new_instruments( device );
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.sysmontap" );
  
  CFTypeRef msg = NULL;
  
  tARR *procAttrs = tARR__newStrs( 14,
    "memVirtualSize", "cpuUsage", "memRShrd", "memCompressed",
    "cpuTotalSystem", "physFootprint", "cpuTotalUser", "memResidentSize",
    "memRPrvt", "threadCount", "memAnon", "diskBytesWritten",
    "faults", "pid"
  );
  
  tARR *sysAttrs = tARR__newStrs( 1, "threadCount" );
  
  tDICT *dict = tDICT__newPairs( 5,
    "ur", tI32__new( 1000 ),
    "bm", tI32__new( 0 ),
    "procAttrs", procAttrs,
    "cpuUsage", tBOOL__new( 0 ),
    "sampleInterval", tI32__new( 1000000000 )
  );
  /*CFDictionaryRef dict = genmap( 10,
    "ur", i16cf( 1000 ),
    "bm", i16cf( 0 ),
    "procAttrs", procAttrs,
    //"sysAttrs", NULL,//sysAttrs,
    "cpuUsage", kCFBooleanFalse,
    "sampleInterval", i32cf( 1000000000 )
    //"pids", genarr( 1, i16cf( 50906 ) )
  );*/
    
  if( !channel__send( chan, "setConfig:", (tBASE *) dict, 0 ) ) {
    fprintf( stderr, "setConfig failed\n" );
    return;
  }
    
  if( 
    !channel__call( chan,
      "start", 0, 0,
      &msg, NULL
    )
  ) {
    fprintf( stderr, "start failed\n" );
    return;
  }
  
  for( int i=0;i<8; ) {
    if( !channel__recv( chan, &msg, NULL ) ) {
      fprintf( stderr, "recvDtx failed\n" );
      return;
    }
        
    if( msg ) {
      cfdump( 1, CFArrayGetValueAtIndex( msg, 0 ) );
      i++;
    }
    else sleep(1);
  }
    
  if( !channel__send( chan, "stop", 0, 0 ) ) {
    fprintf( stderr, "stop failed\n" );
    return;
  }
  
  service__del( service );
  exit(0);
}