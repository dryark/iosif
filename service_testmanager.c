// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include"bytearr.h"
#include<unistd.h>
#include"services.h"
#include"service.h"
#include"uclop.h"
#include"cfutil.h"
#include"archiver/archiver.h"
#include"cmd_process.h"
#include"uuid.h"
#include"cmd_listapps.h"
#include"service.h"
#include"cmd_file.h"
#include"archiver/plist.h"
#include<pthread.h>

static ucmd *g_cmd = NULL;
void runWda( void *device );
void run_wda( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runWda ); }

void launchTest(
  void *device,
  char *sessionId,
  char *xcTestPath,
  //char *bundleID,
  char *testRunnerBundleID,
  char *xctestConfigPath, // blah.xctest
  tTESTCONF *xcTestConf
);

tTESTCONF *DictXctestConfig(
	char *productModuleName,
	char *sessionIdentifier,
	char *targetApplicationBundleID,
	char *targetApplicationPath,
	char *testBundleURL
);

tTESTCONF *DictXctestConfigMin(
	char *productModuleName,
	char *sessionIdentifier,
	char *targetApplicationBundleID,
	char *targetApplicationPath,
	char *testBundleURL
);

void writeXctestConf( void *device, char *bundleId, tTESTCONF *testConf, char *relFile ) {
  uint32_t len = 0;
  uint8_t *bytes = tBASE__archivebin( (tBASE *) testConf, &len );
  char isErr = writeDataToAppFile( device, bundleId, relFile, bytes, len );
  if( !isErr ) printf( "Wrote %s - %s\n", bundleId, relFile );
  else printf( "Failed to write %s - %s\n", bundleId, relFile );
}

void runWda( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  devUp( device );
  CFDictionaryRef runnerInfo = getAppInfo( device, "com.facebook.WebDriverAgentRunner.xctrunner" );
  devDown( device );
  
  //cfdump( 0, runnerInfo );

  CFDictionaryRef runnerEnv = cfmapget( runnerInfo, "EnvironmentVariables" );
  CFStringRef runnerHomeCf = cfmapget( runnerEnv, "HOME" );
  char *runnerHome = str_cf2c( runnerHomeCf );
  CFStringRef runnerAppPathCf = cfmapget( runnerInfo, "Path" );
  char *runnerAppPath = str_cf2c( runnerAppPathCf );
  
  char confFileName[300];
  char *sessionId = uuid_generate();
  sprintf( confFileName, "%s.xctestconfiguration", sessionId );
  
  char xcTestPath[400];
  sprintf( xcTestPath, "%s/PlugIns/WebDriverAgentRunner.xctest", runnerAppPath );
  
  char xcTestConfFile[300];
  sprintf( xcTestConfFile, "tmp/%s.xctestconfiguration", sessionId );
  
  char xcTestConfPath[500];
  sprintf( xcTestConfPath, "%s/%s", runnerHome, xcTestConfFile );
  
  char relUrl[300];
  sprintf( relUrl, "file://%s", xcTestPath );
  
  char *runnerBundleId = "com.facebook.WebDriverAgentRunner.xctrunner";
  
  tTESTCONF *xcTestConf = DictXctestConfig(
    "WebDriverAgentRunner-Runner",
    sessionId,
    runnerBundleId,
    runnerAppPath,
    relUrl
  );
  
  writeXctestConf( device, runnerBundleId, xcTestConf, xcTestConfFile );
  
  launchTest(
    device,
    sessionId,
    xcTestPath,
    "com.facebook.WebDriverAgentRunner.xctrunner",
    xcTestConfPath,
    xcTestConf
  );
	
	exit(0);
}

void dci__startExecutingTestPlan__protocolVersion( channelT *dcichan, uint8_t version ) {
  tBASE *msg = NULL;
  channel__call( dcichan,
	  "_IDE_startExecutingTestPlanWithProtocolVersion:",
	  (tBASE *) tI8__new( version ),
	  0, &msg, NULL
	);
}

void dci__initSession__identifier__capabilities( 
  channelT *dcichan,
  char *id,
  tCAPS *capabilities
) {
  tBASE *msg = NULL;
	channel__call( dcichan,
	  "_IDE_initiateSessionWithIdentifier:capabilities:",
	  (tBASE *) tARR__newVals(
      2,
      //tSTR__new( id ),
      tUUID__new( id ),
      capabilities
	  ),
	  0, &msg, NULL
	);
}

void dci__initSession__identifier__protocolVersion(
  channelT *dcichan,
  char *sessionId,
  uint16_t protocolVersion
) {
  tBASE *msg = NULL;
  
  char nodash[40];
  int j = 0;
  for( int i=0;i<strlen( sessionId);i++ ) {
    char let = sessionId[i];
    if( let != '-' ) nodash[ j++ ] = let;
  }
  nodash[j] = 0x00;
  
  char blah[200];
  sprintf( blah, "%s-6722-000247F15966B083", sessionId );
  
	channel__call( dcichan,
	  "_IDE_initiateSessionWithIdentifier:forClient:atPath:protocolVersion:",
	  (tBASE *) tARR__newVals( 4,
	    tUUID__new( sessionId ),
	    //tSTR__new( nodash ),
	    tSTR__new( blah ),
	    tSTR__new( "/Applications/Xcode.app" ),
	    tI64__new( protocolVersion )
	  ),
	  0, &msg, NULL
	);
}

uint64_t launchTestRunner(
  void *device,
  char *xctestConfigPath,
  char *bundleIdentifier,
  char *sessionId,
  char *xcTestPath,
  channelT *processChan
) {
	tARR *arguments = tARR__newStrs( 4,
		"-NSTreatUnknownArgumentsAsOpen", "NO",
		"-ApplePersistenceIgnoreState", "YES"
	);
	
	tDICT *environment = tDICT__newPairs( 20,
		"CA_ASSERT_MAIN_THREAD_TRANSACTIONS", tSTR__new( "0" ),
		"CA_DEBUG_TRANSACTIONS",              tSTR__new( "0" ),
		"DYLD_INSERT_LIBRARIES",              tSTR__new( "/Developer/usr/lib/libMainThreadChecker.dylib" ),
		"MTC_CRASH_ON_REPORT",                tSTR__new( "1" ),
		"NSUnbufferedIO",                     tSTR__new( "YES" ),
		"OS_ACTIVITY_DT_MODE",                tSTR__new( "YES" ),
		"SQLITE_ENABLE_THREAD_ASSERTIONS",    tSTR__new( "1" ),
		"XCTestBundlePath",                   tSTR__new( xcTestPath ),
		"XCTestConfigurationFilePath",        tSTR__new(""),//tSTR__new( xctestConfigPath ),
		"XCTestSessionIdentifier",            tSTR__new( sessionId )
	);
	
	tDICT *options = tDICT__newPairs( 4,
		"StartSuspendedKey", tBOOL__new( 0 ),
		"ActivateSuspended", tBOOL__new( 1 )
	);

	return launchProcess_withchan( device, bundleIdentifier, environment, arguments, options, processChan );
}

void dci__authorizeTestSession__pid( channelT *dcichan, uint16_t pid ) {
  tBASE *msg = NULL;
  
	channel__call( dcichan,
	  "_IDE_authorizeTestSessionWithProcessID:",
	  (tBASE *) tI16__new( (uint32_t) pid ),
	  0, &msg, NULL
	);
}

void dci__initControlSession__capabilities(
  channelT *dcichan,
  tCAPS *capabilites
) {
  tBASE *msg = NULL;
  tARR *aux = NULL;
	channel__call( dcichan,
	  "_IDE_initiateControlSessionWithCapabilities:",
	  (tBASE *) capabilites,
	  //(tBASE *) tARR__newVals( 1, tNULL__new() ),
	  0, &msg, &aux
	);
	
	tBASE__dump( msg, 1 );
	//tBASE__dump( aux, 1 );
}

void dci__initControlSession__protocolVersion(
  channelT *dcichan,
  uint16_t version
) {
  tBASE *msg = NULL;
  
	channel__call( dcichan,
	  "_IDE_initiateControlSessionWithProtocolVersion:",
	  (tBASE *) tI16__new( version ),
	  //(tBASE *) tARR__newVals( 1, tNULL__new() ),
	  0, &msg, NULL
	);
	
	tBASE__dump( msg, 1 );
}

typedef struct {
  int pid;
  channelT *procChan;
  void *device;
} watchargs;

void *watchWda( void *voidArgs ) {
  watchargs *args = (watchargs *) voidArgs;
  int pid = args->pid;
  printf("Watch pid: %d\n", pid );
  //channelT *procChan = args->procChan;
  void *device = args->device;
  //devUp( device );
  //channelT *processChan = channel__new_processcontrol( device );
  //startObserving( processChan, pid );
  
  while(1) {
    tBASE *msg = NULL;
    tARR *aux = NULL;
    int msgid = 0;
    int convIndex = 0;
    sleep(1);
    //channel__recv_withid( processChan, &msg, &aux, &msgid, &convIndex );
  }
}

void launchTest(
  void *device,
  char *sessionId,
  char *xcTestPath, // blah.xctest
  //char *bundleID,
  char *testRunnerBundleID,
  char *xcTestConfPath, // tmp/blah.config
  tTESTCONF *xcTestConf
) {
  //char *sessionId = uuid_generate();
  char *testrunnerAppPath; // .app/PlugIns/blah.xctest
  
  serviceT *testman = service__new_testmanagerd( device );
  serviceT *testman2 = service__new_testmanagerd( device );
  //serviceT *testman3 = service__new_testmanagerd( device );
  
  printf("\n===== Creating dciChan =====\n");
  
  channelT *dciChan = service__connect_channel_int( testman,
    "dtxproxy:XCTestManager_IDEInterface:XCTestManager_DaemonConnectionInterface", 1 );
  
  //printf("\n===== Init Control Session with caps =====\n");
	tDICT *emptycaps = tDICT__new();
	tCAPS *capob = tCAPS__new( emptycaps );
	dci__initControlSession__capabilities( dciChan, capob );
  
  printf("\n===== Creating dciChan2 =====\n");
	
	//serviceT *testman2 = service__new_testmanagerd( device );
	channelT *dciChan2 = service__connect_channel_int( testman2,
    "dtxproxy:XCTestManager_IDEInterface:XCTestManager_DaemonConnectionInterface", 1 );
	//channel__handshake2( dciChan2 );
  
	tBASE *msg = NULL;
	tARR *aux = NULL;
	
	printf("\n===== Init Session with id and caps =====\n");
	tDICT *capabilities = tDICT__newPairs( 6,
	  "XCTIssue capability", tI8__new( 1 ),
		"skipped test capability", tI8__new( 1 ),
		"test timeout capability", tI8__new( 1 )
	);
	
	dci__initSession__identifier__capabilities(
	  dciChan2,
	  sessionId,
	  tCAPS__new( capabilities ) );
	
	//printf("\n===== Creating Control Session for dciChan =====\n");
	
	//dci__initControlSession__protocolVersion( dciChan, 35 );
	
	
	int msgid = 0;
	int conversationIndex = 0;
	int channelCode = 0;
	
	//printf("\n===== Init Session for dciChan2 =====\n");
	
	//dci__initSession__identifier__protocolVersion(
	//  dciChan2,
	//  sessionId,
	//  36 );
	
	printf("ok\n");
	
	printf("\n===== Launching Test Runner =====\n");
		
	channelT *processChan = channel__new_processcontrol( device );
	
	uint32_t pid = launchTestRunner(
	  device,
	  xcTestConfPath,
	  testRunnerBundleID,
	  sessionId,
	  xcTestPath,
	  processChan
	);
	//uint32_t pid = 3;
	
	//printf("\n===== Throwing out 2 messages =====\n");
  //channel__recv_withid( dciChan2, &msg, NULL, &msgid, &conversationIndex );
  //channel__recv_withid( dciChan2, &msg, NULL, &msgid, &conversationIndex );
	
	printf("xctest pid:%i\n", (uint16_t) pid );
	
	/*pthread_t thread_id;
	watchargs args;
	args.pid = pid;
	args.procChan = processChan;
	args.device = device;
	
	printf("Launching thread to monitor process\n");
	pthread_create( &thread_id, NULL, watchWda, &args );*/
	
	printf("\n===== Waiting for test bundle ready =====\n");
	
	int readyId = 0;
	
	while( 1 ) {
	  aux = NULL;
	  
	  //gettimeofday( &start, NULL );
	  int res = channel__recv_withid( dciChan2, &msg, &aux, &msgid, &conversationIndex, &channelCode );
	  if( res == 0 ) {
	    exit(0);
	  }
	  
	  if( msg->type == xfSTR ) {
	    tSTR *msgSTR = (tSTR *) msg;
	    const char *msgc = msgSTR->val;
	    
	    printf("Got msg `%s`\n", msgc );
	    
	    if( !strcmp( msgc, "_XCT_testRunnerReadyWithCapabilities:" ) ) {
	      printf("%i: _XCT_testRunnerReadyWithCapabilities:\n", msgid );
	      if( aux != NULL ) {
	        tBASE__dump( (tBASE *) aux, 1 );
	      }
	      
	      readyId = msgid;
	      
	      printf("\n===== Authorizing Test Session =====\n");
	      dci__authorizeTestSession__pid( dciChan, pid );
	    }
	    
	    else if( !strcmp( msgc, "_requestChannelWithCode:identifier:" ) ){
	      channelT *fakeChan = channel__new( dciChan2->service, 1, channelCode );
	      channel__reply( fakeChan, NULL, msgid, 1 );
	    }
	    
	    else if( !strcmp( msgc, "_notifyOfPublishedCapabilities:" ) ){
	      channelT *fakeChan = channel__new( dciChan2->service, 1, channelCode );
	      channel__reply( fakeChan, NULL, msgid, 1 );
	    }
	    
	    else if( !strcmp( msgc, "_XCT_logDebugMessage:" ) ) {
	      if( aux && aux->type == xfARR ) {
	        tBASE *a1 = aux->head;
	        if( a1->type == xfSTR ) {
	          tSTR *a1str = (tSTR *) a1;
	          if( strstr( a1str->val, "test runner ready" ) != NULL ) {
	            channel__reply( dciChan2, NULL, msgid, 1 );
	            break;
	          }
	          if( strstr( a1str->val, "future for 'requesting test configuration" ) != NULL ) {
	            printf("\n===== Sending Test Conf =====\n");
	            channel__reply( dciChan2, (tBASE *) xcTestConf, readyId, 1 );
	          }
	        }
	      }
	      channel__reply( dciChan2, NULL, msgid, 1 );
	    }
	    
	    else {
	      channel__reply( dciChan2, NULL, msgid, 1 );
	    }
	  }
	}
	
	printf("\n===== Test runner is ready! =====\n");
	
	//printf("\n===== Receiving on dciChan2 =====\n");
	// this will be _requestChannelWithCode:identifier:
	//channel__recv_withid( dciChan2, &msg, NULL, &msgid, &conversationIndex );
	
	//channel__reply( dciChan2, (tBASE *) tI32__new( 2 ), msgid, 1 );
	
	//printf("\n===== Receiving on dciChan2 2 =====\n");
	// this will be _notifyOfPublishedCapabilities:
	//channel__recv_withid( dciChan2, &msg, NULL, &msgid, &conversationIndex );
		
  printf("\n===== Waiting for Running tests =====\n");
  
  //channelT *dciChan3 = channel__connect_channel_int( dciChan2,
  //  "dtxproxy:XCTestDriverInterface:XCTestManager_IDEInterface", 1 );
  
  /*int negChanCode = 0;
  while( 1 ) {
	  aux = NULL;
	  
	  //gettimeofday( &start, NULL );
	  int res = channel__recv_withid( dciChan2, &msg, &aux, &msgid, &conversationIndex, &channelCode );
	  if( res == 0 ) {
	    exit(0);
	  }
	  
	  channelT *fakeChan = channel__new( dciChan2->service, 1, channelCode );
	  if( msg->type == xfSTR ) {
	    tSTR *msgSTR = (tSTR *) msg;
	    const char *msgc = msgSTR->val;
	    
	    if( !strcmp( msgc, "outputReceived:fromProcess:atTime:" ) ) {
	      if( aux && aux->type == xfARR ) {
	        tBASE *a1 = aux->head;
	        if( a1->type == xfSTR ) {
	          tSTR *a1str = (tSTR *) a1;
	          if( strstr( a1str->val, "Running tests" ) != NULL ) {
	            negChanCode = channelCode;
	            break;
	          }
	        }
	      }
	      channel__reply( dciChan2, NULL, msgid, 1 );
	      break;
	    }
	    
	    else if( !strcmp( msgc, "_XCT_logDebugMessage:" ) ) {
	      if( aux && aux->type == xfARR ) {
	        tBASE *a1 = aux->head;
	        if( a1->type == xfSTR ) {
	          tSTR *a1str = (tSTR *) a1;
	          if( strstr( a1str->val, "requesting test configuration" ) != NULL ) {
	            //continue;
	          }
	        }
	      }
	      channel__reply( dciChan2, NULL, msgid, 1 );
	    }
	    
	    else {
	      channel__reply( dciChan2, NULL, msgid, 1 );
	    }
	  }
	}*/
  
	printf("\n===== Start Executing =====\n");
	
  //channel__recv_withid( fakeChan, &msg, &aux, &msgid, &conversationIndex );
  //channel__recv_withid( fakeChan, &msg, &aux, &msgid, &conversationIndex );
	channelT *negChan = channel__new( dciChan2->service, dciChan->secure, -1 );
  dci__startExecutingTestPlan__protocolVersion( negChan, 36 ); // ideInterfaceChannel
	
  printf("\n===== Receive Running Tests Message =====\n");
  /*
  After "start executing" is run, "Running tests..." will be received.
  */
  channel__recv_withid( processChan, &msg, &aux, &msgid, &conversationIndex, NULL );
  channel__reply( processChan, NULL, msgid, 1 );
  //channel__recv_withid( processChan, &msg, &aux, &msgid, &conversationIndex, NULL );
  
	//channel__reply( dciChan2, (tBASE *) xcTestConf, readyId, 1 );
	//channel__reply( dciChan2, (tBASE *) xcTestConf, readyId, 1 );
		
	struct timeval start, end;
	
  //exit(0);
	//startObserving( processChan, pid );
	
	//while(1) {
  //  channel__recv_withid( processChan, &msg, &aux, &msgid, &conversationIndex, NULL );
  //  channel__reply( processChan, NULL, msgid, 1 );
  //}
	
  printf("\n===== Looping =====\n");
  
	while( 1 ) {
	  aux = NULL;
	  
	  //gettimeofday( &start, NULL );
	  channel__recv_withid( dciChan2, &msg, &aux, &msgid, &conversationIndex, NULL );
	  //gettimeofday( &stop, NULL );
	  //int sdif = stop.tv_sec - start.tv_sec;
	  
	  //if( msgid == 20 ) channel__reply( dciChan2, (tBASE *) xcTestConf, readyId, 1 );
	  if( !msg ) continue;
	  
	  if( msg->type == xfSTR ) {
	    tSTR *msgSTR = (tSTR *) msg;
	    const char *msgc = msgSTR->val;
	    if( !strcmp( msgc, "_XCT_testRunnerReadyWithCapabilities:" ) ) {
	      printf("%i: _XCT_testRunnerReadyWithCapabilities:\n", msgid );
	      if( aux != NULL ) {
	        tBASE__dump( (tBASE *) aux, 1 );
	      }
	      
	      readyId = msgid;
	      channel__reply( dciChan2, (tBASE *) xcTestConf, msgid, 1 );
	      
	      //channel__send_withid( dciChan2, "_XCT_testRunnerReadyWithCapabilities:", , 0, msgid );
	    }
	    
	    else if( !strcmp( msgc, "_XCT_logDebugMessage:" ) ) {
	      if( aux && aux->type == xfARR ) {
	        tBASE *a1 = aux->head;
	        if( a1->type == xfSTR ) {
	          tSTR *a1str = (tSTR *) a1;
	          if( strstr( a1str->val, "Received test runner ready reply with error" ) != NULL ) {
	            printf("Error\n");
	            exit(0);
	          }
	        }
	      }
	      channel__reply( dciChan2, NULL, msgid, 1 );
	    }
	    
	    else if( !strcmp( msgc, "_XCT_testBundleReadyWithProtocolVersion:minimumVersion:" ) ) {
	      printf("%i: _XCT_testBundleReadyWithProtocolVersion:minimumVersion::\n", msgid );
	      
	      //channel__send_withid( dciChan2, "_XCT_testRunnerReadyWithCapabilities:", , 0, msgid );
	    }
	    
	    else {
	      //printf("%i: %s\n", msgid, msgc );
	      //if( aux != NULL ) {
	      //  tBASE__dump( (tBASE *) aux, 1 );
	      //}
	      channel__reply( dciChan2, NULL, msgid, 1 );
	    }
	  }
	  else tBASE__dump( msg, 1 );
	  //sleep(1);
	  printf(". ");
	}
}

tTESTCONF *DictXctestConfig(
	char *productModuleName,
	char *sessionIdentifier,
	char *targetApplicationBundleID,
	char *targetApplicationPath,
	char *testBundleURL
) {
  tDICT *conf = tDICT__new();

  // What go-ios uses
	tDICT__set( conf, "aggregateStatisticsBeforeCrash",
	  tDICT__newPairs( 2, "XCSuiteRecordsKey", tDICT__new() )
	);
	tDICT__set( conf, "automationFrameworkPath",
	  tSTR__new( "/Developer/Library/PrivateFrameworks/XCTAutomationSupport.framework" )
	);
	tDICT__set( conf, "baselineFileRelativePath",          tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "baselineFileURL",                   tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "defaultTestExecutionTimeAllowance", tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "disablePerformanceMetrics",         tBOOL__new(0) );
	tDICT__set( conf, "emitOSLogs",                        tBOOL__new(0) );
	//x tDICT__set( conf, "formatVersion", tI8__new( 2 ) );
	tDICT__set( conf, "gatherLocalizableStringsData",      tBOOL__new(0) );
	tDICT__set( conf, "initializeForUITesting",            tBOOL__new(1) );
	tDICT__set( conf, "maximumTestExecutionTimeAllowance", tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "productModuleName",                 tSTR__new( productModuleName ) );
	tDICT__set( conf, "randomExecutionOrderingSeed",       tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "reportActivities",                  tBOOL__new(1) );
	tDICT__set( conf, "reportResultsToIDE",                tBOOL__new(1) );
	tDICT__set( conf, "sessionIdentifier",                 tUUID__new( sessionIdentifier ) );
	tDICT__set( conf, "systemAttachmentLifetime",          tI8__new(2)   );
	//x tDICT__set( conf, "targetApplicationArguments", tDICT__new() );
	tDICT__set( conf, "targetApplicationBundleID",         tSTR__new( targetApplicationBundleID ) );
	//x tDICT__set( conf, "targetApplicationEnvironment", tDICT__new() );
	tDICT__set( conf, "targetApplicationPath",             tSTR__new( targetApplicationPath ) );
	tDICT__set( conf, "testApplicationUserOverrides",      tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "testBundleRelativePath",            tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "testBundleURL",                     tURL__new( testBundleURL ) );
	tDICT__set( conf, "testExecutionOrdering",             tI8__new(0)   );
	tDICT__set( conf, "testTimeoutsEnabled",               tBOOL__new(0) );
	tDICT__set( conf, "testsDrivenByIDE",                  tBOOL__new(0) );
	tDICT__set( conf, "testsMustRunOnMainThread",          tBOOL__new(1) );
	tDICT__set( conf, "testsToRun",                        tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "testsToSkip",                       tSTR__new("$null") ); // tNULL__new()  );
	tDICT__set( conf, "treatMissingBaselinesAsFailures",   tBOOL__new(0) );
	tDICT__set( conf, "userAttachmentLifetime",            tI8__new(1)   );
	
	// What IDB uses
	// See https://github.com/facebook/idb/blob/88d59fe624621610331c599f7e4bd41e6e70d4c2/XCTestBootstrap/Configuration/FBTestConfiguration.m
	/*tDICT__set( conf, "sessionIdentifier",                 tUUID__new( sessionIdentifier ) );
	tDICT__set( conf, "testBundleURL",                     tURL__new( testBundleURL ) );
	tDICT__set( conf, "treatMissingBaselinesAsFailures",   tBOOL__new(0) );
	tDICT__set( conf, "productModuleName",                 tSTR__new( productModuleName ) );
	tDICT__set( conf, "reportResultsToIDE",                tBOOL__new(1) );
	tDICT__set( conf, "testsMustRunOnMainThread",          tBOOL__new(1) );
	tDICT__set( conf, "initializeForUITesting",            tBOOL__new(1) );
	tDICT__set( conf, "testsToRun",                        tSTR__new("$null") );
	tDICT__set( conf, "testsToSkip",                       tSTR__new("$null") );
	tDICT__set( conf, "targetApplicationPath",             tSTR__new( targetApplicationPath ) );
	tDICT__set( conf, "targetApplicationBundleID",         tSTR__new( targetApplicationBundleID ) );
	tDICT__set( conf, "automationFrameworkPath",
	  tSTR__new( "/Developer/Library/PrivateFrameworks/XCTAutomationSupport.framework" )
	);
	tDICT__set( conf, "reportActivities",                  tBOOL__new(1) );*/
	
	// What XCode itself uses
	/*tDICT__set( conf, "sessionIdentifier",                 tUUID__new( sessionIdentifier ) );
	tDICT__set( conf, "testBundleRelativePath",            tSTR__new("$null") );
	tDICT__set( conf, "testsToRun",                        tSTR__new("$null") );
	tDICT__set( conf, "userAttachmentLifetime",            tI8__new(1)   );
	tDICT__set( conf, "targetApplicationPath",             tSTR__new("$null") );
	tDICT__set( conf, "targetApplicationEnvironment",      tDICT__new() );
	tDICT__set( conf, "testsToSkip",                       tSTR__new("$null") );
	tDICT__set( conf, "testBundleURL",                     tURL__new( testBundleURL ) );
	tDICT__set( conf, "baselineFileURL",                   tSTR__new("$null") );
	tDICT__set( conf, "testsMustRunOnMainThread",          tBOOL__new(1) );
	tDICT__set( conf, "randomExecutionOrderingSeed",       tSTR__new("$null") );
	tDICT__set( conf, "productModuleName",                 tSTR__new( "WebDriverAgentRunner" ) );
	tDICT__set( conf, "testTimeoutsEnabled",               tBOOL__new(0) );
	tDICT__set( conf, "testsDrivenByIDE",                  tBOOL__new(0) );
	tDICT__set( conf, "reportResultsToIDE",                tBOOL__new(1) );
	tDICT__set( conf, "targetApplicationArguments",        tARR__new() );
	tDICT__set( conf, "reportActivities",                  tBOOL__new(1) );
	tDICT__set( conf, "testApplicationDependencies",
	  tDICT__newPairs( 2, "com.facebook.WebDriverAgentRunner.xctrunner",
	    tSTR__new("/Users/user/Library/Developer/Xcode/DerivedData/WebDriverAgent-amngfwpzinssypapwcfadnjclcft/Build/Products/Debug-iphoneos/WebDriverAgentRunner-Runner.app") ) );
	tDICT__set( conf, "formatVersion",                     tI8__new(2) );
	tDICT__set( conf, "disablePerformanceMetrics",         tBOOL__new(0) );
	tDICT__set( conf, "baselineFileRelativePath",          tSTR__new("$null") );
	tDICT__set( conf, "testExecutionOrdering",             tI8__new(0)   );
	
	tDICT *caps = tDICT__newPairs( 6,
	  "XCTIssue capability", tI8__new( 1 ),
		"skipped test capability", tI8__new( 1 ),
		"test timeout capability", tI8__new( 1 )
	);
	//tDICT *caps = tDICT__new();
	tDICT__set( conf, "IDECapabilities",
	  tCAPS__new( caps )
	);
	
	tDICT__set( conf, "defaultTestExecutionTimeAllowance", tI16__new(600) );
	tDICT__set( conf, "targetApplicationBundleID",         tSTR__new("$null") );//tSTR__new( targetApplicationBundleID ) );
	tDICT__set( conf, "emitOSLogs",                        tBOOL__new(0) );
	tDICT__set( conf, "maximumTestExecutionTimeAllowance", tSTR__new("$null") );
	tDICT__set( conf, "automationFrameworkPath",
	  tSTR__new( "/Developer/Library/PrivateFrameworks/XCTAutomationSupport.framework" )
	);
	tDICT__set( conf, "performanceTestConfiguration",      tSTR__new("$null") );
	tDICT__set( conf, "treatMissingBaselinesAsFailures",   tBOOL__new(0) );
	tDICT__set( conf, "aggregateStatisticsBeforeCrash",
	  tDICT__newPairs( 2, "XCSuiteRecordsKey", tDICT__new() )
	);
	tDICT__set( conf, "gatherLocalizableStringsData",      tBOOL__new(0) );
	tDICT__set( conf, "traceCollectionEnabled_v2",         tSTR__new("$null") );
	tDICT__set( conf, "systemAttachmentLifetime",          tI8__new(2)   );
	tDICT__set( conf, "initializeForUITesting",            tBOOL__new(1) );
	tDICT__set( conf, "testApplicationUserOverrides",      tSTR__new("$null") );*/
	
	//tDICT__set( conf, "sessionIdentifier",                 tUUID__new( sessionIdentifier ) );
	
	/*
	Also see:
	https://github.com/linkedin/bluepill/blob/76d758d7038ea6cfe39cdc079c3eb8cb1777329e/bp/src/SimulatorHelper.m
	https://github.com/alibaba/taobao-iphone-device/blob/19f128df694d510fdf5b2039e6826d7bb2309452/tidevice/bplist.py#L162
	https://github.com/calabash/iOSDeviceManager/blob/bd52607cd09554c361a622fad41e7a255a18fb91/iOSDeviceManager/Utilities/XCTestConfigurationPlist.m
	*/
	
	return tTESTCONF__new( conf );
}
