// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#ifndef __BYTEARR_H
#define __BYTEARR_H
#include<stdint.h>
#include<CoreFoundation/CoreFoundation.h>
#include"archiver/types.h"

typedef struct bytechunk_s bytechunk;
struct bytechunk_s {
  uint8_t *data;
  uint32_t len;
  char alloc;
  struct bytechunk_s *next;
};

typedef struct {
  uint32_t len;
  bytechunk *head;
  bytechunk *tail;
} bytearr;

bytechunk *bytechunk__new( uint8_t *data, uint32_t len, char alloc );

bytearr *bytearr__new();

void bytearr__appendba( bytearr *self, bytearr *toadd );

void bytearr__append( bytearr *self, uint8_t *data, uint32_t len, char alloc );
void bytearr__appdup( bytearr *self, uint8_t *data, uint32_t len );

void bytearr__appendu8( bytearr *self, uint8_t num );
void bytearr__appendu16( bytearr *self, uint16_t num );
void bytearr__appendi32( bytearr *self, int32_t num );
void bytearr__appendu32( bytearr *self, uint32_t num );

void bytearr__appendi64( bytearr *self, int64_t num );

uint8_t *bytearr__bytes( bytearr *self, uint32_t *len );

void bytearr__auxi32( bytearr *self, int32_t val );

void bytearr__auxi64( bytearr *self, int64_t val );

//void bytearr__auxcf( bytearr *self, CFTypeRef cf, char secure );
void bytearr__auxT( bytearr *self, tBASE *t );

uint8_t *bytearr__asaux( bytearr *self, uint32_t *len );

//bytearr *cfarr2aux( CFTypeRef argsCf, char secure );
bytearr *tarr2aux( tARR *argsT );

void ba__print( bytearr *self, char *fmt, ... );

#endif