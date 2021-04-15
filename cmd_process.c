// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include<stdlib.h>
#include"cfutil.h"
#include"uclop.h"
#include"mobiledevice.h"
#include"service.h"
#include"services.h"
#include"cmd_process.h"

static ucmd *g_cmd = NULL;
void runLaunch( void *device );
void run_launch( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runLaunch ); }

void runKill( void *device );
void run_kill( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runKill ); }

channelT *channel__new_processcontrol( void *device ) {
  serviceT *inst = service__new_instruments( device );
  
  channelT *chan = service__connect_channel( inst,
    "com.apple.instruments.server.services.processcontrol" );
  
  return chan;
}

void killPid( void *device, int64_t pid ) {
  tBASE *msg = NULL;
  
  channelT *chan = channel__new_processcontrol( device );
  
  channel__call( chan,
    "killPid:", (tBASE *) tI32__new( pid ),
    0, &msg, NULL
  );
  if( !msg ) {
    //channel__del( chan );
    exit(0);
  }
  
  tBASE__dump( msg, 1 );
}

void runKill( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  int pid = atoi( g_cmd->argv[0] );
  killPid( device, pid );
  
  exit(0);
}

void runLaunch( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  tDICT *env = tDICT__new();
  tARR *args = tARR__new();
  tDICT *options = tDICT__new();
  uint64_t pid = launchProcess( device, g_cmd->argv[0], env, args, options );
  
  if( pid ) {
    printf("{Type:\"Success\",PID:%" PRId64 "}\n", pid );
  }
  
  exit(0);
}

uint64_t launchProcess(
  void *device,
  char *bundleIdentifier,
  tDICT *environment,
  tARR *arguments,
  tDICT *options
) {
	tSTR *devicePath = tSTR__new("/private/");

	tARR *args = tARR__newVals( 5,
	  devicePath,
	  tSTR__new( bundleIdentifier ),
	  environment,
	  arguments,
	  options );
	
	tBASE *msg = NULL;
  
  channelT *chan = channel__new_processcontrol( device );
  
  channel__call( chan,
    "launchSuspendedProcessWithDevicePath:bundleIdentifier:environment:arguments:options:", (tBASE *) args,
    0, &msg, NULL
  );
  if( !msg ) {
    //channel__del( chan );
    exit(0);
  }
  
  if( !tIsnum( msg ) ) {
    fprintf(stderr, "Error launching process\n" );
    tBASE__dump( msg, 1 );
    return 0;
  }
  
  return tI__val64( msg );
}