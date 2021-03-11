// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
#include"service.h"
#include"bytearr.h"
#include"dtxmsg.h"
#include"cfutil.h"
//#include"nsutil.h"
#include"service.h"
#include"archiver/archiver.h"
#include"archiver/unarchiver.h"

int g_msg_id = 0;

channelT *channel__new( void *service, char secure, int id ) {
  channelT *self = (channelT *) malloc( sizeof( channelT ) );
  self->service = service;
  self->secure = secure;
  self->channel = id;
  return self;
}

char channel__send( channelT *chan, const char *method, tBASE *args, uint8_t flags ) {
  uint32_t id = ++g_msg_id;
  return channel__send_withid( chan, method, args, flags, id );
}

//char channel__call( channelT *chan, const char *method, tBASE *args,
//    uint8_t flags, CFTypeRef *msgOut, CFArrayRef *auxOut ) {
char channel__call( channelT *chan, const char *method, tBASE *args,
    uint8_t flags, tBASE **msgOut, tARR **auxOut ) {
  #ifdef DEBUG
  printf("Channel call:\n  channel=%d\n  method=%s\n  args=", chan->channel, method);
  //if( args ) cfdump( 1, args );
  //else printf("nil\n");
  #endif
  channel__send( chan, method, args, flags | SEND_HASREPLY );
  int res = channel__recv( chan, msgOut, auxOut );
  if( !res ) {
    #ifdef DEBUG
    printf("Failed recv on channel call\n");
    #endif
  }
  return res;
}

uint8_t *args_to_aux_bytes( tBASE *argsT, uint32_t *len ) {
  uint8_t *argsBytes = NULL;
  if( argsT ) {
    bytearr *ba = tBASE__asaux( argsT );
    //cfarr2aux( argsCf, argSecure );
    argsBytes = bytearr__asaux( ba, len );
  }
  else *len = 0;
  return argsBytes;
}

char channel__send_withid( channelT *self, const char *method, tBASE *argsT, uint8_t flags, uint32_t id ) {
  uint32_t argsLen = 0;
  uint8_t *argsBytes = args_to_aux_bytes( argsT, &argsLen );
  #ifdef DEBUG2
  printf("made aux bytes %d\n", argsLen );
  #endif
  
  int methodLen;
  uint8_t *methodBytes = method ? ( uint8_t * ) strArchive( method, &methodLen ) : NULL;
  
  #ifdef DEBUG2
  printf("made method bytes %d\n", methodLen );
  #endif
  
  char expects_reply = flags & SEND_HASREPLY;
  
  dtxpayload *payload = dtxpayload__new( expects_reply, argsLen, argsLen + methodLen );
  
  dtxmsg *msg = dtxmsg__new( self->channel, id, expects_reply, sizeof(dtxpayload) + payload->totlen );
  
  bytearr *msgb = bytearr__new();
  bytearr__append( msgb, (uint8_t *) msg,     sizeof( dtxmsg ),     0 );
  bytearr__append( msgb, (uint8_t *) payload, sizeof( dtxpayload ), 0 );
  bytearr__append( msgb, argsBytes,   argsLen,   0 );
  bytearr__append( msgb, methodBytes, methodLen, 0 );
  
  uint32_t msgLen;
  uint8_t *msgBytes = bytearr__bytes( msgb, &msgLen );
  
  ssize_t sentLen = self->secure
                  ? AMDServiceConnectionSend( self->service, msgBytes, msgLen )
                  : write( AMDServiceConnectionGetSocket( self->service ), msgBytes, msgLen );
  #ifdef DEBUG2
  printf("Sent %d Sentok %d\n", msgLen, (int) sentLen );
  #endif
  
  if( sentLen != msgLen ) {
    fprintf(stderr, "Failed to send 0x%lx bytes of message: %s\n", (unsigned long) msgLen, strerror(errno));
    return 0;
  }

  return 1;
}

char channel__recv( channelT *self, tBASE **msgOut, tARR **argOut ) {
  uint32_t id = 0;
  
  bytearr *payloadb = bytearr__new();

  int sock = self->secure ? 0 : AMDServiceConnectionGetSocket( self->service );

  for(;;) {
    dtxmsg msg;
    ssize_t nrecv = self->secure
                  ? AMDServiceConnectionReceive( self->service, (char *) &msg, sizeof(dtxmsg) )
                  : read(sock, &msg, sizeof(dtxmsg));
    if( nrecv != sizeof(dtxmsg) ) { fprintf(stderr, "read error: %s, nrecv = %lx\n", strerror(errno), nrecv); return false; }
    
    //printf("Received header - size: %d\n", (int) nrecv);

    if( msg.magic != 0x1F3D5B79 ) { fprintf(stderr, "invalid magic: %x\n", msg.magic); return 0; }
    
    //printf("Header magic ok\n");

    //printf( "Channel code: %d\n", msg.channelCode );
    if( !msg.conversationIndex ) { // start of conversation
      if( msg.id > g_msg_id ) {
        g_msg_id = msg.id; // fast forward
        //printf("Advancing to msgId %i\n", msg.id );
      }
      else if( msg.id < g_msg_id ) {
        //fprintf( stderr, "bad msg.id: %d\n", msg.id );
      }
    }
    
    if( !msg.fragId ) {
      id = msg.id;
      if( msg.fragCount > 1 ) continue; // frag 0 contains only header
    }

    bytearr *frag = bytearr__new();
    
    bytearr__append( frag, (uint8_t *) &msg, sizeof( dtxmsg ), 0 );
    
    uint32_t dataLen = msg.length;
    //printf("Message length: %i\n", (int) dataLen );
    uint8_t *dataBlock = (uint8_t *) malloc( dataLen );

    uint32_t received = 0;
    while ( received < dataLen ) {
      uint8_t *ptr = dataBlock + received;
      size_t remaining = dataLen - received;
      uint32_t part = self->secure
        ? AMDServiceConnectionReceive( self->service, (char *) ptr, remaining )
        : read( sock, ptr, remaining );
      if( nrecv <= 0 ) { fprintf( stderr, "read error: %s\n", strerror(errno) ); return 0; }
      received += part;
    }
    //printf("Received all bytes\n");
    if( received > dataLen ) {
      printf("Received extra\n");
    }
    
    bytearr__append( payloadb, dataBlock, dataLen, 0 );

    if ( msg.fragId == msg.fragCount - 1 ) break;
  }
  //printf("Got all fragments\n");

  uint32_t payloadLen;
  uint8_t *payloadBytes = bytearr__bytes( payloadb, &payloadLen );
  
  #ifdef DEBUG
  printf("Size of received payload: %i\n", payloadLen );
  #endif
  
  //printf("Size of payload generally: %i\n", (int) sizeof( dtxpayload ) );
  
  // payload header
  const dtxpayload *payloadHeader = (const dtxpayload *) payloadBytes;

  //if( payload->flags & 0xFF000 ) { fprintf( stderr, "compression unsupported\n" ); return 0; }

  // aux array is after payload header
  const uint8_t *aux = payloadBytes + sizeof( dtxpayload );
  uint32_t auxlen = payloadHeader->auxlen;
  #ifdef DEBUG
  printf("Recv Aux len:%i\n", auxlen );
  #endif
  
  // output message is after the argument array
  const uint8_t *objptr = aux + auxlen;
  uint32_t objlen = payloadHeader->totlen - auxlen;
  
  #ifdef DEBUG
  printf("Recv msg len: %d\n", objlen );
  #endif

  if( auxlen && argOut ) {
    //CFArrayRef auxArr = deserialize2cf( aux, auxlen );
    tARR *auxArr = (tARR *) deserialize2t( aux, auxlen );
    if( !auxArr) return 0;
    if( auxArr ) *argOut = auxArr;
  }
  else if( argOut ) *argOut = NULL;

  //printf("Sizeof obj: %i\n", (int) objlen );
  if( objlen && msgOut ) {
    //for( int i=0;i<objlen;i++ ) {
    //  printf("%02x", objptr[i] );
    //}
    
    //CFTypeRef cfMsg = archive2cf( (uint8_t *) objptr, objlen );
    tBASE *tMsg = dearchive( (uint8_t *) objptr, objlen );
    #ifdef DEBUG
    printf("Recevived DTX msg:");
    if( tMsg ) {
      //cfdump( 1, cfMsg );
      tBASE__dump( tMsg, 1 );
    }
    else printf("nil/not archive\n");
    #endif
    
    //printf( "%s", cftype( cfMsg ) );
    //exit(0);
    //if( cfMsg == NULL ) {
    if( tMsg == NULL ) {
      printf("Could not unarchive. Dumping\n");
      dumparchive( objptr, objlen );
    }
    //*msgOut = cfMsg;
    *msgOut = tMsg;
  }
  else if( msgOut ) {
    *msgOut = NULL;
  }

  return 1;
}