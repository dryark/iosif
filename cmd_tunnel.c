// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include"service.h"
#include"uclop.h"
#include"cfutil.h"

static ucmd *g_cmd = NULL;
void runTunnel( void *device );
void run_tunnel( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runTunnel ); }

typedef struct tunnel_s {
  int inPort;
  int outPort;
  int inSock;
  struct addrinfo *inAddr;
} tunnel;

tunnel *tunnel__new( void *device, int srcPort, int destPort ) {
  tunnel *self = (tunnel *) malloc( sizeof( tunnel ) );
  self->inPort = srcPort;
  self->outPort = destPort;
  
  struct addrinfo hints,*inAddr;
  memset( &hints, 0, sizeof( hints ) );
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  char strAddr[12];
  sprintf( strAddr, "%d", srcPort );
  getaddrinfo( NULL, strAddr, &hints, &inAddr ); 
  self->inAddr = inAddr;
  
  int inSock = socket( inAddr->ai_family, inAddr->ai_socktype, inAddr->ai_protocol );
  if( inSock < 0 ) {
    printf("Error making socket\n");
    exit(1);
  }
  
  int opt = 1;
  int optRes = setsockopt( inSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) );
  if( optRes < 0 ) {
    printf("setsockopt error\n");
    exit(1);
  }
    
  int bindRes = bind( inSock, inAddr->ai_addr, inAddr->ai_addrlen );
  if( bindRes == -1 ) {
    close( inSock );
    printf("failed to bind to %i\n", srcPort );
    exit(1);
  }
  printf("bound to port %s\n", strAddr );
  
  int listenRes = listen( inSock, 3 );
  if( listenRes < 0 ) {
    printf("listen error\n");
    exit(1);
  }
  
  self->inSock = inSock;
  
  return self;
}

#define NETBUFSIZE 1000
void sendBytes( int inSock, int outSock ) {
  char buffer[NETBUFSIZE];
  ssize_t readCount;
  for(;;) {
    readCount = recv( inSock, buffer, NETBUFSIZE, 0 );
    if( readCount <= 0 ) break;
    send( outSock, buffer, readCount, 0 );
  }
  
  if( readCount < 0 ) exit(1);
  
  shutdown( inSock, SHUT_RDWR );
  shutdown( outSock, SHUT_RDWR );
  close( inSock );
  close( outSock );
}

int connectOutput( void *device, int port ) {
  int conId = AMDeviceGetConnectionID( device );
  int sock;
  exitOnError( USBMuxConnectByPort( conId, htons( port ), &sock ),
      "Connecting Port" );
  return sock;
}

void handleClient( void *device, tunnel *tun, int clientSock, struct sockaddr_storage clientAddr ) {
  int outSock = connectOutput( device, tun->outPort );
  if( outSock < 0 ){
    close( clientSock );
    return;
  }
  
  // data flowing to outSock
  if( !fork() ) {
    sendBytes( clientSock, outSock );
    exit(0);
  }
  
  // data coming from outSock
  if( !fork() ) {
    sendBytes( outSock, clientSock );
    exit(0);
  }
  
  close( clientSock );
  close( outSock );
}

void runTunnel( void *device ) {
  CFStringRef udidCf = AMDeviceCopyDeviceIdentifier( device );
  char *udid = str_cf2c( udidCf );
  char *goalUdid = ucmd__get( g_cmd, "-id" );
  if( goalUdid && strcmp( udid, goalUdid ) ) return;
  
  devUp( device );
  
  if( !g_cmd->argc ) {
      fprintf(stderr,"Must specify at least one [fromPort]:[toPort] pair\n");
      devDown( device );
      exit(1);
  }
  
  int tunCount = g_cmd->argc;
  int okCount = 0;
  tunnel *tuns[10];
  
  for( int i=0;i<tunCount;i++ ) {
      char *pair = g_cmd->argv[ i ];
      int j=0;
      char found = 0;
      for( ;j<strlen( pair );j++ ) {
          if( pair[j] == ':' ) {
              found = 1;
              break;
          }
      }
      if( !found ) {
          fprintf(stderr,"Pair \"%s\" does not contain :\n", pair );
          continue;
      }
      pair[j] = 0x00;
      int from = atoi( pair );
      int to = atoi( &pair[j+1] );
      printf("Tunnel from %i to %i\n", from, to );
      tuns[okCount] = tunnel__new( device, from, to );
      okCount++;
  }
    
  fd_set set;
  
  printf("Ready\n");
  fflush(stdout);
  
  while( 1 ) {
    FD_ZERO( &set );
    int maxFd = 0;
    for( int i=0;i<okCount;i++ ) {
      tunnel *tun = tuns[i];
      int inSock = tun->inSock;
      FD_SET( inSock, &set );
      if( inSock > maxFd ) maxFd = inSock;
    }
    
    select( maxFd + 1, &set, NULL, NULL, NULL );
    
    for( int i=0;i<okCount;i++ ) {
      tunnel *tun = tuns[i];
      int inSock = tun->inSock;
      
      if( !FD_ISSET( inSock, &set ) ) continue;
      
      struct sockaddr_storage clientAddr;
      socklen_t clientAddrLen = sizeof( clientAddr );
      int clientSock = accept( inSock, (struct sockaddr*) &clientAddr, &clientAddrLen );
      if( clientSock < 0 ) continue;
      
      printf("Got connection to port\n");
      
      pid_t pid = fork();
      if( pid == -1 ) {
        fprintf(stderr,"fork failed");
        exit(1);
      }
      else if( !pid ) {
        close( inSock );
        handleClient( device, tun, clientSock, clientAddr );
        close( clientSock );
        _exit(0);
      }
      else close( clientSock );
    }
  }
    
  devDown( device );
}