#ifndef __BYTEARR_H
#define __BYTEARR_H
#include<stdint.h>
#include<CoreFoundation/CoreFoundation.h>

typedef struct bytechunk_s bytechunk;
struct bytechunk_s {
  uint8_t *data;
  int len;
  char alloc;
  struct bytechunk_s *next;
};

typedef struct {
  bytechunk *head;
  bytechunk *tail;
} bytearr;

bytechunk *bytechunk__new( uint8_t *data, int len, char alloc );

bytearr *bytearr__new();

void bytearr__append( bytearr *self, uint8_t *data, int len, char alloc );

void bytearr__appendi32( bytearr *self, int32_t num );

void bytearr__appendi64( bytearr *self, int64_t num );

uint8_t *bytearr__bytes( bytearr *self, int *len );

void bytearr__auxi32( bytearr *self, int32_t val );

void bytearr__auxi64( bytearr *self, int64_t val );

void bytearr__auxcf( bytearr *self, CFTypeRef cf, char secure );

uint8_t *bytearr__asaux( bytearr *self, int *len );

bytearr *cfarr2aux( CFTypeRef argsCf, char secure );

#endif