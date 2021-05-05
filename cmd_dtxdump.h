// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#ifndef __DTXDUMP_H
#define __DTXDUMP_H

#include"dtxmsg.h"
struct dmsg_s {
  uint8_t *ptr;
  uint16_t len;
  char *time;
  char dir;
  struct dmsg_s *next;
};
typedef struct dmsg_s dmsg;

typedef struct {
  uint16_t count;
  dmsg *head;
  dmsg *tail;
} dmsgList;

#define DIR_TO 0
#define DIR_FROM 1

void run_dtxdump( ucmd *cmd  );
void run_dtxdumpfolder( ucmd *cmd );
void dump__messages( dmsgList *list, char *jsonFile, char *binFile, char dir );
dmsgList *dmsgList__new();
dmsg **dmsgList__toPtrs( dmsgList *self );
void decode_message( uint8_t *buffer, int len, char *time, char dir );
#endif