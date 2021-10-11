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
#include"ujsonin/ujsonin.h"
#include"archiver/plist.h"
//#include<openssl/ssl.h>

//int g_msg_id = -1;

channelT *channel__new( void *service, char secure, int id ) {
  channelT *self = (channelT *) malloc( sizeof( channelT ) );
  self->service = service;
  self->secure = secure;
  self->channel = id;
  self->nextMsgId = 0;
  return self;
}

char channel__send( channelT *chan, const char *method, tBASE *args, uint8_t flags ) {
  uint32_t id = chan->nextMsgId++;//++g_msg_id;
  return channel__send_withid( chan, method, args, flags, id );
}

//char channel__call( channelT *chan, const char *method, tBASE *args,
//    uint8_t flags, CFTypeRef *msgOut, CFArrayRef *auxOut ) {
char channel__call( channelT *chan, const char *method, tBASE *args,
    uint8_t flags, tBASE **msgOut, tARR **auxOut ) {
  
  int id = chan->nextMsgId++;//g_msg_id;
  #ifdef DEBUG
  printf("Channel call:\n  channel=%d\n  method=%s\n  id=%d\n  args=", chan->channel, method, id);
  if( args ) tBASE__dump( args, 2 );
  else printf("nil\n");
  #endif
  
  channel__send_withid( chan, method, args, flags | SEND_HASREPLY, id );
    
  int idout = 0;
  int convIndex;
  int res;
  //for( int i=0;i<10;i++ ) {
    res = channel__recv_withid( chan, msgOut, auxOut, &idout, &convIndex, NULL );
    //if( idout != id ) {
    //  printf("Response of previous message; fetching another\n");
    //  continue;
    //}
    if( !res ) {
      #ifdef DEBUG
      printf("Failed recv on channel call\n");
      #endif
    }
    //break;
  //}
  
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

char channel__reply( channelT *self, tBASE *argsT, uint32_t id, int convIndex ) {
  uint32_t argsLen = 0;
  //tARR *empty = tARR__new();
  //uint8_t *argsBytes = argsT ? args_to_aux_bytes( (tBASE *) empty, &argsLen ) : NULL;
  uint8_t argsBytes = 0;
  #ifdef DEBUG2
  printf("\n");
  printf("Reply %i.%i.%i\n", self->channel, id, convIndex );
  printf("made aux bytes %d\n", argsLen );
  #endif
  
  #ifdef DEBUG2
  if( argsT ) {
    printf("Replying with: ");
    tBASE__dump( argsT, 1 );
  }
  #endif
  
  uint32_t methodLen = 0;
  //char *method = argsT;
  //uint8_t *methodBytes = method ? ( uint8_t * ) strArchive( method, &methodLen ) : NULL;
  uint8_t *methodBytes = NULL;
  if( argsT ) {
    methodBytes = tBASE__archivebin( argsT, &methodLen );
    
    //int flen;
    //methodBytes = (uint8_t *) slurp_file( "response.msg", &flen );
    //methodLen = flen;
  }
  
  #ifdef DEBUG2
  printf("made method bytes %d\n", methodLen );
  #endif
  
  char expects_reply = 0;//flags & SEND_HASREPLY;
  
  dtxpayload *payload = dtxpayload__new(
    expects_reply,
    argsLen,
    argsLen + methodLen
  );
  payload->type = argsT ? 3 : 0; // 0 = ack, 3 = response
  
  dtxmsg *msg = dtxmsg__new(
    self->channel,
    id,
    expects_reply,
    sizeof(dtxpayload) + payload->totlen
  );
  msg->conversationIndex = convIndex;
  
  bytearr *msgb = bytearr__new();
  bytearr__append( msgb, (uint8_t *) msg,     sizeof( dtxmsg ),     0 );
  bytearr__append( msgb, (uint8_t *) payload, sizeof( dtxpayload ), 0 );
  //if( argsBytes ) bytearr__append( msgb, argsBytes,   argsLen,   0 );
  if( methodBytes ) bytearr__append( msgb, methodBytes, methodLen, 0 );
  
  uint32_t msgLen;
  uint8_t *msgBytes = bytearr__bytes( msgb, &msgLen );
  
  ssize_t sentLen = self->secure
                  ? AMDServiceConnectionSend( self->service, msgBytes, msgLen )
                  : write( AMDServiceConnectionGetSocket( self->service ), msgBytes, msgLen );
  /*ssize_t sentLen;
  if( self->secure ) {
    SSL *ssl = AMDServiceConnectionGetSecureIOContext( self->service );
    SSL_write( ssl, msgBytes, msgLen );
  }
  else {
    sentLen = write( AMDServiceConnectionGetSocket( self->service ), msgBytes, msgLen );
  }*/
  
  #ifdef DEBUG2
  printf("Sent %d Sentok %d\n", msgLen, (int) sentLen );
  #endif
  
  if( sentLen != msgLen ) {
    fprintf(stderr, "Failed to send 0x%lx bytes of message: %s\n", (unsigned long) msgLen, strerror(errno));
    exit(0);
    return 0;
  }

  return 1;
}

char channel__send_withid( channelT *self, const char *method, tBASE *argsT, uint8_t flags, uint32_t id ) {
  uint32_t argsLen = 0;
  
  #ifdef DEBUG2
  //printf("aux for channel send:\n");
  //if( argsT ) tBASE__dump( argsT, 1 );
  //else printf("  NONE\n");
  #endif
  
  uint8_t *argsBytes = args_to_aux_bytes( argsT, &argsLen );
  #ifdef DEBUG2
  printf("\n");
  printf("made aux bytes %d\n", argsLen );
  #endif
  
  uint32_t methodLen;
  //uint8_t *methodBytes = method ? ( uint8_t * ) strArchive( method, &methodLen ) : NULL;
  uint8_t *methodBytes = method ? tBASE__archivebin(
    (tBASE *) tSTR__new( method ),
    &methodLen ) : NULL;
  
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
  printf("Send msg %i.%i.0: %d / %d \n", self->channel, id, (int) sentLen, msgLen );
  #endif
  
  if( sentLen != msgLen ) {
    fprintf(stderr, "Failed to send 0x%lx bytes of message: %s\n", (unsigned long) msgLen, strerror(errno));
    return 0;
  }

  return 1;
}

char channel__recv( channelT *self, tBASE **msgOut, tARR **argOut ) {
  return channel__recv_withid( self, msgOut, argOut, NULL, NULL, NULL );
}

char channel__recv_withid( channelT *self, tBASE **msgOut, tARR **argOut, int *idout, int *convIndexOut, int *channelOut ) {
  uint32_t id = 0;
  
  bytearr *payloadb = bytearr__new();

  int sock = self->secure ? 0 : AMDServiceConnectionGetSocket( self->service );

  #ifdef DEBUG
  printf("\nRecv Msg\n");
  #endif
    
  int lastConvIndex;
  for(;;) {
    dtxmsg msg;
    ssize_t nrecv = self->secure
                  ? AMDServiceConnectionReceive( self->service, (char *) &msg, sizeof(dtxmsg) )
                  : read(sock, &msg, sizeof(dtxmsg));
    if( nrecv != sizeof(dtxmsg) ) { fprintf(stderr, "Initial read error: %s, nrecv = %lx\n", strerror(errno), nrecv); return false; }
    
    //printf("Received header - size: %d\n", (int) nrecv);

    if( msg.magic != 0x1F3D5B79 ) { fprintf(stderr, "invalid magic: %x\n", msg.magic); return 0; }
    
    //printf("Header magic ok\n");
    #ifdef DEBUG
    printf( "Channel code: %d\n", msg.channelCode );
    if( channelOut ) *channelOut = msg.channelCode;
    #endif
    
    if( !msg.conversationIndex ) { // start of conversation
      if( msg.id > self->nextMsgId ) {
        self->nextMsgId = msg.id+1; // fast forward
        //printf("Advancing to msgId %i\n", msg.id );
      }
      //else if( msg.id < g_msg_id ) {
        //fprintf( stderr, "bad msg.id: %d\n", msg.id );
      //}
    }
    if( convIndexOut ) {
      *convIndexOut = msg.conversationIndex;
    }
    #ifdef DEBUG
    printf("Recv Msg: (%i)%i.%i.%i\n", self->channel, msg.channelCode, msg.id, msg.conversationIndex);
    #endif
    
    if( !msg.fragId ) {
      id = msg.id;
      if( msg.fragCount > 1 ) continue; // frag 0 contains only header
    }

    bytearr *frag = bytearr__new();
    
    bytearr__append( frag, (uint8_t *) &msg, sizeof( dtxmsg ), 0 );
    
    uint32_t dataLen = msg.length;
    #ifdef DEBUG
    printf("Message length: %i\n", (int) dataLen );
    #endif
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
  if( idout ) *idout = id;
  
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
    
    #ifdef DEBUG
    printf("Received DTX aux/arr:");
    if( auxArr ) {
      //cfdump( 1, cfMsg );
      tBASE__dump( (tBASE *) auxArr, 1 );
    }
    else printf("nil/not archive\n");
    
    #endif
    
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
    printf("Received DTX msg:");
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