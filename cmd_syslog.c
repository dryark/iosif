// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include<stdio.h>
#include<string.h>
#include"mobiledevice.h"
#include"uclop.h"
#include"services.h"
#include"service.h"
#include"cfutil.h"

#define BUFSIZE 40//4096

static ucmd *g_cmd = NULL;
void runSysLog( void *device );
void run_syslog( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSysLog ); }

typedef struct {
  char *data;
  uint16_t maxsize;
  uint16_t size;
} msg;

msg *msg__new() {
  msg *self = (msg *) calloc( 1, sizeof( msg ) );
  self->data = (char *) calloc( 1, BUFSIZE + 1 );
  self->maxsize = BUFSIZE;
  return self;
}

void msg__append( msg *self, char *data, uint16_t size ) {
  if( ( self->size + size ) > self->maxsize ) {
    uint16_t newsize = self->size * 2;
    while( newsize < ( self->size + size ) ) newsize *= 2;
    char *newdata = (char *) calloc( 1, newsize );
    memcpy( newdata, self->data, self->size );
    free( self->data );
    self->data = newdata;
  }
  memcpy( &(self->data[ self->size ]), data, size );
  self->size += size;
}

void msg__reset( msg *self ) {
  self->size = 0;
}

void msg__parse( msg *self ) {
}

void msg__output( msg *self, int procCount, char **procs ) {
  char month[4] = {};
  memcpy( month, self->data, 3 );
  char day[3] = {};
  memcpy( day, &(self->data[4]), 2 );
  char time[9] = {};
  memcpy( time, &(self->data[7]), 8 );
  //printf("Month:[%s]\n", month );
  //printf("Day:[%s]\n", day );
  //printf("Time:[%s]\n", time );
  
  // Process
  int i=18;
  for( ;i<self->size;i++ ) {
    if( self->data[i] == ' ' ) break;
  }
  int k;
  int pidEnd = i - 2;
  //printf("pidEnd char: %c\n", self->data[ i - 1 ] );
  for( k=pidEnd;k>18;k-- ) {
    if( self->data[k] == '[' ) break;
  }
  k++;
  int pidStart = k;
  int pidLen = pidEnd - pidStart + 1; 
  char *pid = &self->data[pidStart];
  
  // Process name
  int procLen = i-18-(pidLen+2);
  char *proc = &self->data[18];
  
  if( procCount ) {
    int procOk = 0;
    for( int ii=0; ii<procCount; ii++ ) {
      if( !strncmp( procs[ii], proc, procLen ) ) {
        procOk = 1;
        break;
      }
    }
    if( !procOk ) return;
  }
  
  // PID
  //printf("[%.*s,", pidLen, pid );  
  //printf("\"%.*s\",", procLen, proc );
  
  // Type ( Notice etc )
  int j=i+2;
  for( ;j<self->size;j++ ) {
    if( self->data[j] == ':' ) break;
  }
  int typeStart = i+2;
  int typeEnd = j-1;
  int typeLen = typeEnd - typeStart;
  char *type = &self->data[typeStart];
  //printf("\"%.*s\",", typeLen, type );
  
  // Message itself
  int msgLen = self->size-(typeEnd+3)-1;
  char *msg = &self->data[typeEnd+3];
  //printf("`%.*s`]\n", msgLen, msg );
  
  char *buffer = (char *) malloc( pidLen + 2 + procLen + 3 + typeLen + 3 + msgLen + 3 + 10 );
  sprintf( buffer, "[%.*s,\"%.*s\",\"%.*s\",`%.*s`]\n",
    pidLen, pid,
    procLen, proc,
    typeLen, type,
    msgLen, msg );
  int bufLen = strlen( buffer );
  printf("*%d%s", bufLen, buffer );
  free( buffer );
}

void runSysLog( void *device ) {
  char *udid = ucmd__get( g_cmd, "-id" );
  
  int procCount = 0;
  char **procs = NULL;
  int argc = g_cmd->argc;
  if( argc > 0 ) {
    for( int i=0;i<argc;i+=2 ) {
      char *filter = g_cmd->argv[i];
      //char *val = g_cmd->argv[i+1];
      if( !strncmp( filter, "proc", 4 ) ) {
        procCount++;
      }
    }
    
    if( procCount ) {
      procs = ( char ** ) malloc( sizeof( char * ) );
      int procI = 0;
      for( int i=0;i<argc;i+=2 ) {
        char *filter = g_cmd->argv[i];
        char *val = g_cmd->argv[i+1];
        if( !strncmp( filter, "proc", 4 ) ) {
          procs[ procI++ ] = val;
          //printf("Filtering by proc %s\n", val );
        }
      }
    }
  }
  //exit(0);
  
  CFStringRef devId = AMDeviceCopyDeviceIdentifier( device );
  char *audid = str_cf2c( devId );
  if( udid && strcmp( audid, udid ) ) return;
  //fprintf(stderr,"udid:%s\n", udid );
  
  void *service = activateSyslog( device );

  msg *curmsg = msg__new();
  
  char buf[ BUFSIZE ];
  
  int len, rc;
  while( 1 ) {
    rc = AMDServiceConnectionReceive( service, buf, BUFSIZE-1 );
    if( rc <= 0 ) break;
    //printf("rc:%d\n", rc );
    //continue;
    
    if( buf[0] == 0 ) {
      if( curmsg->size ) {
        //fwrite( curmsg->data, curmsg->size, 1, stdout );
        //fwrite( "\n", 1, 1, stdout );
        msg__output( curmsg, procCount, procs );
        fflush( stdout );
        msg__reset( curmsg );
      }
      //len = strlen( &buf[1] );
      msg__append( curmsg, &buf[1], rc-1 );
      //fwrite( &buf[1], len, 1, stdout );
    }
    else {
      len = strlen( buf );
      msg__append( curmsg, buf, rc );
      //fputs( buf, stdout );
    }
    
    // We only need to clear the portion of the buffer used
    //memset( buf, 0, len );
  }
}