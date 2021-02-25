#ifndef __ARCHIVER_H
#define __ARCHIVER_H

#include "bytearr.h"

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

struct tBASE_t {
  uint8_t type;
  struct tBASE_t *next;
};
typedef struct tBASE_t tBASE;

typedef struct {
  uint8_t type;
  tBASE *next;
  uint32_t val;
} tI32;

typedef struct {
  uint8_t type;
  tBASE *next;
  char *val;
} tSTR;

typedef struct {
  uint8_t type;
  tBASE *next;
  uint8_t len;
  tBASE *head;
  tBASE *tail;
} tARR;

typedef struct {
  uint8_t type;
  tBASE *next;
  uint8_t len;
  tBASE *head;
  tBASE *tail;
  int num;
} tOBS;

typedef struct {
  uint8_t type;
  tBASE *next;
  tBASE *keyHead;
  tBASE *keyTail;
  tBASE *valHead;
  tBASE *valTail;
} tDICT;

typedef struct {
  uint8_t type;
  tBASE *next;
  uint32_t val;
} tARCID;

typedef struct {
  uint8_t type;
  tBASE *next;
  tBASE *val;
} tREF;

typedef struct {
  uint8_t type;
  tBASE *next;
  uint8_t val;
} tBOOL;

tBOOL * tBOOL__new( uint8_t val );

#define tDICT__set( a,b,c ) _tDICT__set( a,b, (tBASE *) c )
void _tDICT__set( tDICT *self, char *key, tBASE *val );

tDICT *tDICT__newPairs( int count, ... );

#define tOBS__add( a,b ) _tOBS__add( a, (tBASE *) b )
uint32_t _tOBS__add( tOBS *self, tBASE *ob );

void tBASE__dump( tBASE *ob, uint8_t depth );

#define tARR__add( a,b) _tARR__add( a, (tBASE *) b )
void _tARR__add( tARR *self, tBASE *ob );

tARR *tARR__newVals( int count, ... );
tARR *tARR__newStrs( int count, ... );

void tOBS__xml( tOBS *self, bytearr *ba, uint8_t depth );
void tBASE__xml( tBASE *ob, bytearr *ba, uint8_t depth );

#define tBASE__del( a ) _tBASE__del( (tBASE *) a )
void _tBASE__del( tBASE *self );

void tOBS__dump( tOBS *self, uint8_t depth );

#define tOBS__new( a ) _tOBS__new( (tBASE *) a )
tOBS *_tOBS__new( tBASE *root );

tSTR *tSTR__new( char *val );
tI32 *tI32__new( uint32_t val );
tARR *tARR__new();

void tARR__del( tARR *self );

bytearr *tARR__asaux( tARR *self );
bytearr *tDICT__asaux( tDICT *self );
void tBASE__toaux( tBASE *self, bytearr *out );
bytearr *tBASE__asaux( tBASE *self );
void tI32__toaux( tI32 *self, bytearr *out );
void tBASE__archiveToAux( tBASE *self, bytearr *out );
uint8_t *tBASE__archive( tBASE *self, uint32_t *len );

#define tARR__addI32( a,b ) tARR__add( a, tI32__new( b ) )
#define tARR__addSTR( a,b ) tARR__add( a, tSTR__new( b ) )
#endif