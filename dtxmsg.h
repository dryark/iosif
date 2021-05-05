// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#ifndef __DTXMSG_H
#define __DTXMSG_H
typedef struct {
  uint32_t magic;
  uint32_t size; // 4
  uint16_t fragId; // 8
  uint16_t fragCount; // 10
  uint32_t length; // 12
  uint32_t id; // 16
  uint32_t conversationIndex ; // 20
  int32_t channelCode; // 24
  uint32_t expectsReply; // 28
} dtxmsg;

typedef struct {
  uint32_t type;
  uint32_t auxlen; // 4
  uint32_t totlen; // 8
  uint32_t flags; // 12
} dtxpayload;

dtxmsg *dtxmsg__new( int channel, uint32_t id, char expectsReply, uint32_t length );
dtxpayload *dtxpayload__new( char expectsReply, uint32_t auxlen, uint64_t totlen );
#endif