#include<CoreFoundation/CoreFoundation.h>
#include"bytearr.h"
#include<unistd.h>
#include"services.h"
#include"service.h"
#include"uclop.h"
#include"cfutil.h"
#include"archiver.h"

static ucmd *g_cmd = NULL;
void runXctest( void *device ) {
  serviceT *testman = service__new_testmanagerd( device );
  
  channelT *testchan = service__connect_channel( testman,
    "dtxproxy:XCTestManager_IDEInterface:XCTestManager_DaemonConnectionInterface" );
  
  tARR *initProt = tARR__newVals( 2,
    tI32__new( 1 ),
    tI32__new( 29 )
  );
  
  channel__send( testchan,
    "_IDE_initiateControlSessionWithProtocolVersion:",
    (tBASE *) initProt,
    SEND_HASREPLY
  );
  
  tARR__del( initProt );
  
  //sid = genuuid();
  char *sid = "fjdkljfds";
  
  tARR *initSessionArgs = tARR__newVals( 4,
    tSTR__new( sid ),
    tSTR__new( "blahblah" ),
    tSTR__new( "/Applications/Xcode.app"),
    tI32__new( 36 )
  );
    
  channel__send( testchan,
    "_IDE_initiateSessionWithIdentifier:forClient:atPath:protocolVersion:",
    (tBASE *) initSessionArgs,
    SEND_HASREPLY
  );
  
  tARR__del( initSessionArgs );
  
  // launch app runner
  
  //  CFSTR("dtxproxy:XCTestManager_IDEInterface:XCTestManager_DaemonConnectionInterface"), NULL );
}