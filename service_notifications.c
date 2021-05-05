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
void runNotices( void *device );
void run_notices( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runNotices      ); }

void runNotices( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  serviceT *service = service__new_instruments( device );

  //CFTypeRef msg = NULL;
  tBASE *msg = NULL;
  
  channelT *chan = service__connect_channel( service, "com.apple.instruments.server.services.mobilenotifications" );
  
  if( !channel__send( chan, "setApplicationStateNotificationsEnabled:", (tBASE *) tI32__new(1), 0 ) ) {
    fprintf( stderr, "setConfig failed\n" );
    return;
  }
  
  for(;;) {
    //CFArrayRef data = NULL;
    tARR *data = NULL;
    if( !channel__recv( chan, &msg, &data ) ) {
      fprintf( stderr, "recvDtx failed\n" );
      return;
    }
    
    //if( data ) cfdump( 0, data );
    if( data ) tBASE__dump( (tBASE *) data, 1 );
    
    if( msg ) {
      //cfdump( 0, msg );
      tBASE__dump( msg, 1 );
    }
    else sleep(1);
  }
    
  service__del( service );
  exit(0);
}