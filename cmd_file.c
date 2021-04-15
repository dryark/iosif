// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include"cfutil.h"
#include"uclop.h"
#include"service.h"
#include"services.h"

static ucmd *g_cmd = NULL;
void runGetFile( void *device );
void run_getfile( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runGetFile ); }

void *houseArrest( void *device, char *bi ) {
  void *conn = NULL;

  CFStringRef cfbi = str_c2cf( bi );
  
  int err1 = AMDeviceCreateHouseArrestService( device, cfbi, 0, &conn );
  if( err1 ) {
    //fprintf( stderr, "Err1\n" );
    CFDictionaryRef args = genmap( 2,
      "Command", CFSTR("VendDocuments") );
    
    int err2 = AMDeviceCreateHouseArrestService( device, cfbi, args, &conn );
    if( err2 ) {
      mobdev_err *info = mobdev_geterr( err2 );
      if( info ) {
        fprintf( stderr, "Error on x12: %s - %s\n", info->name, info->error );
      }
      fprintf( stderr, "Err2: %d\n", err2 );
      exit(0);
    }
  }
  
  return conn;
}

void runGetFile( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  devUp( device );
  
  //serviceT *afcService = service__new_afc( device );
  //void *afc = afcService->service;
  
  void *afc = houseArrest( device, g_cmd->argv[0] );
  
  devDown( device );
  
  char *remoteFile = g_cmd->argv[1];
  char *localFile = g_cmd->argv[2];
  
  unsigned long long fhr;
  int err = AFCFileRefOpen( afc, remoteFile, 1, &fhr );
  if( err ) {
    mobdev_err *info = mobdev_geterr( err );
    if( info ) {
      fprintf( stderr, "Error on x: %s - %s\n", info->name, info->error );
    }
    else {
      fprintf(stderr,"err other %d - %s\n", err, strerror( err ) );
    }
    exit(0);
  }

  FILE *fhl = fopen( localFile, "w" );

  if( !fhl ) {
    fprintf( stderr,"err %s\n", strerror( errno ) );
    AFCFileRefClose( afc, fhr );
    exit(0);
  }

  char buffer[4096];
  size_t bufSize = 4096;
  size_t bytesRead = 4096;
  
  while( 1 ) {
    int err = AFCFileRefRead( afc, fhr, buffer, &bytesRead );
    if( !bytesRead ) break;
    fwrite( buffer, bytesRead, 1, fhl );
    bytesRead = 4096;
  }

  AFCFileRefClose( afc, fhr );
  fclose( fhl );
    
  exit(0);
}

