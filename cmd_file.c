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

char writeDataToAppFile( void *device, char *bundleid, char *remoteFile, uint8_t *data, uint32_t len ) {
  devUp( device );
  void *afc = houseArrest( device, bundleid );
  devDown( device );
  
  unsigned long long fhr;
  int err = AFCFileRefOpen( afc, remoteFile, O_WRONLY + 1, &fhr );
  if( err ) {
    mobdev_err *info = mobdev_geterr( err );
    if( info ) {
      fprintf( stderr, "Error on x: %s - %s\n", info->name, info->error );
    }
    else {
      fprintf(stderr,"err other %d - %s\n", err, strerror( err ) );
    }
    return 1;
  }
  
  AFCFileRefWrite( afc, fhr, data, len );
  
  return 0;
}

char ulAppFile( void *device, char *bundleid, char *localFile, char *remoteFile ) {
  devUp( device );
  void *afc = houseArrest( device, bundleid );
  devDown( device );
  
  unsigned long long fhr;
  int err = AFCFileRefOpen( afc, remoteFile, O_WRONLY + 1, &fhr );
  if( err ) {
    mobdev_err *info = mobdev_geterr( err );
    if( info ) {
      fprintf( stderr, "Error on x: %s - %s\n", info->name, info->error );
    }
    else {
      fprintf(stderr,"err other %d - %s\n", err, strerror( err ) );
    }
    return 1;
  }

  FILE *fhl;
  if( !strcmp( localFile, "-" ) ) {
    fhl = stdin;
  }
  else {
    fhl = fopen( localFile, "r" );
  }

  if( !fhl ) {
    fprintf( stderr,"err %s\n", strerror( errno ) );
    AFCFileRefClose( afc, fhr );
    return 2;
  }

  char buffer[4096];
  size_t bytesRead = 4096;
  
  while( 1 ) {
    bytesRead = fread( buffer, 1, 4096, fhl );
    if( !bytesRead ) {
      AFCFileRefClose( afc, fhr );
      fclose( fhl );
      if( feof( fhl ) ) break;
      return 3;
    }
    AFCFileRefWrite( afc, fhr, buffer, bytesRead );
  }
  
  return 0;
}

char dlAppFile( void *device, char *bundleid, char *remoteFile, char *localFile ) {
  devUp( device );
  void *afc = houseArrest( device, bundleid );
  devDown( device );
  
  unsigned long long fhr;
  int err = AFCFileRefOpen( afc, remoteFile, O_RDONLY + 1, &fhr );
  if( err ) {
    mobdev_err *info = mobdev_geterr( err );
    if( info ) {
      fprintf( stderr, "Error on x: %s - %s\n", info->name, info->error );
    }
    else {
      fprintf(stderr,"err other %d - %s\n", err, strerror( err ) );
    }
    return 1;
  }

  FILE *fhl = fopen( localFile, "w" );

  if( !fhl ) {
    fprintf( stderr,"err %s\n", strerror( errno ) );
    AFCFileRefClose( afc, fhr );
    return 2;
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
  
  return 0;
}

void runGetFile( void *device ) {
  if( !desired_device( device, g_cmd ) ) return;
  
  char *remoteFile = g_cmd->argv[1];
  char *localFile = g_cmd->argv[2];
  char *bundleid = g_cmd->argv[0];
  
  dlAppFile( device, bundleid, remoteFile, localFile );
    
  exit(0);
}

