// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdint.h>
#include<stdlib.h>

#include"dtxmsg.h"

dtxmsg *dtxmsg__new( int channel, uint32_t id, char expectsReply, uint32_t length ) {
  dtxmsg *self = (dtxmsg *) calloc( 1, sizeof( dtxmsg ) );
  self->magic = 0x1F3D5B79;
  self->size = sizeof( dtxmsg );
  self->fragId = 0;
  self->fragCount = 1;
  self->length = length;
  self->id = id;
  self->conversationIndex = 0;
  self->channelCode = channel;
  self->expectsReply = expectsReply;
  return self;
}

dtxpayload *dtxpayload__new( char expectsReply, uint32_t auxlen, uint64_t totlen ) {
  dtxpayload *self = (dtxpayload *) calloc( 1, sizeof( dtxpayload ) );
  self->type = 0x2 | (expectsReply ? 0x1000 : 0);
  self->auxlen = auxlen;
  self->totlen = totlen;
  return self;
}