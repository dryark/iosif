// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#ifndef __ATYPES_H
#define __ATYPES_H

#include<stdint.h>

#define xfI8 1
#define xfI16 2
#define xfI32 3
#define xfSTR 4
#define xfARR 5
#define xfDICT 6
#define xfARCID 7
#define xfOBS 8
#define xfREF 9
#define xfBOOL 10
#define xfNULL 11
#define xfI64 12
#define xfTIME 13
#define xfF1 14
#define xfF2 15
#define xfDATA 16
#define xfUUID 17
#define xfCAPS 18
#define xfURL 19
#define xfTESTCONF 20

struct tBASE_t {
  uint8_t type;
  struct tBASE_t *next;
};
typedef struct tBASE_t tBASE;

#define BASEINFO uint8_t type; tBASE *next;
typedef struct {
  BASEINFO
  uint8_t val;
} tI8;

typedef struct {
  BASEINFO
  uint16_t val;
} tI16;

typedef struct {
  BASEINFO
  int32_t val;
} tI32;

typedef struct {
  BASEINFO
  int64_t val;
} tI64;

typedef struct {
  BASEINFO
  float val;
} tF1;

typedef struct {
  BASEINFO
  double val;
} tF2;

typedef struct {
  BASEINFO
  uint8_t *val;
  uint32_t len;
} tDATA;

typedef struct {
  BASEINFO
  double val;
} tTIME;

typedef struct {
  BASEINFO
  const char *val;
} tSTR;

typedef struct {
  BASEINFO
  uint16_t count;
  tBASE *head;
  tBASE *tail;
} tARR;

typedef struct {
  BASEINFO
  uint8_t len;
  tBASE *head;
  tBASE *tail;
  int num;
} tOBS;

struct tOFF_t {
  struct tOFF_t *next;
  uint16_t offset;
};
typedef struct tOFF_t tOFF;

typedef struct {
  uint16_t len;
  tOFF *head;
  tOFF *tail;
  int num;
} tOFFS;

typedef struct {
  BASEINFO
  int count;
  tBASE *keyHead;
  tBASE *keyTail;
  tBASE *valHead;
  tBASE *valTail;
} tDICT;

typedef struct {
  BASEINFO
  tDICT *dict;
} tCAPS;

typedef struct {
  BASEINFO
  tDICT *dict;
} tTESTCONF;

typedef struct {
  uint8_t type;
  tBASE *next;
  uint32_t val;
} tARCID;

typedef struct {
  BASEINFO
  tBASE *val;
} tREF;

typedef struct {
  BASEINFO
  uint8_t val;
} tBOOL;

typedef struct {
  BASEINFO
} tNULL;

typedef struct {
  BASEINFO
  uint8_t val[16];
} tUUID;

typedef struct {
  BASEINFO
  char *relative;
} tURL;

#endif