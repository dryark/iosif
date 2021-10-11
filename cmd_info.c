// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include"cfutil.h"
#include"uclop.h"
#include"service.h"
#include"cmd_info.h"

static ucmd *g_cmd = NULL;
void runInfo( void *device );
void run_info( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runInfo ); }
void runIosVersion( void *device );
void run_ios_version( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runIosVersion ); }

void runInfo( void *device ) {
  CFStringRef udidCf = AMDeviceCopyDeviceIdentifier( device );
  char *udid = str_cf2c( udidCf );
  char *goalUdid = ucmd__get( g_cmd, "-id" );
  if( goalUdid && strcmp( udid, goalUdid ) ) return;
  
  char useJson = ucmd__get( g_cmd, "-json" ) ? 1 : 0;
  
  devUp( device );
  
  if( useJson ) printf("{\n");
  for( int i=0;i<g_cmd->argc;i++ ) {
      char *name = strdup(g_cmd->argv[i]);
      char *dom = NULL;
      char *name2 = name;
      for( int j=0;1;j++ ) {
          char let = name[j];
          if( !let ) break;
          if( let == ':' ) {
              name[j] = 0x00;
              dom = name;
              name2 = &name[j+1];
          }
      }
      //printf("Dom=%s Name=%s\n", dom, name2 );
      CFTypeRef val = AMDeviceCopyValue( device, dom ? str_c2cf( dom ) : NULL, (name2[0]==0)?NULL:str_c2cf(name2) );
      if( val ) {
        if( useJson ) {
          printf("%s:",name);
          cfdump( 1, val );
          if( i != g_cmd->argc-1 ) {
            printf(",\n");
          }
        }
        else {
          if( g_cmd->argc>1) printf("%s:",name);
          cfdump( 1, val );
        }
      }
      free( name );
  }
  if( useJson ) printf("}\n");
  
  devDown( device );
  
  exit(0);
}

iosVersion getIosVersion( void *device ) {
  iosVersion version = {0,0,0};
  
  devUp( device );
  CFTypeRef val = AMDeviceCopyValue( device, NULL, str_c2cf( "ProductVersion" ) );
  devDown( device );
  
  //cfdump( 1, val );
  char *vStr = str_cf2c( val );
  
  //printf("Raw ios version:%s\n", vStr );
  int dots[3] = {0,0,0};
  int dotI = 0;
  for( int i=0;i<strlen(vStr);i++ ) {
    if( vStr[i] == '.' ) {
      dots[dotI++] = i;
    }
  }
  if( !dots[0] ) {
    version.major = atoi( vStr );
    return version;
  }
  char majorStr[5];
  sprintf( majorStr, "%.*s", dots[0], vStr );
  version.major = atoi( majorStr );
  
  if( !dots[1] ) {
    version.medium = atoi( &vStr[dots[0]+1] );
    return version;
  }
  char mediumStr[5];
  sprintf( mediumStr, "%.*s", dots[1]-dots[0], &vStr[dots[0]+1] );
  version.medium = atoi( mediumStr );
  version.minor = atoi( &vStr[dots[1]+1] );
  
  return version;
}

void runIosVersion( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  iosVersion version = getIosVersion( device );
  printf("Major %d\nMedium %d\nMinor %d\n", version.major, version.medium, version.minor );
  
  exit(0);
}