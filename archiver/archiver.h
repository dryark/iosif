// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#ifndef __ARCHIVER_H
#define __ARCHIVER_H

#include"types.h"
#include "../bytearr.h"

tBOOL * tBOOL__new( uint8_t val );
tNULL * tNULL__new();

#define tDICT__set( a,b,c ) _tDICT__set( a,b, (tBASE *) c )
void _tDICT__set( tDICT *self, const char *key, tBASE *val );

#define tDICT__seto( a,b,c ) _tDICT__seto( a,(tBASE *) b, (tBASE *) c )
void _tDICT__seto( tDICT *self, tBASE *key, tBASE *val );

tBASE *tDICT__get( tDICT *self, const char *key );

tDICT *tDICT__newPairs( int count, ... );

#define tOBS__add( a,b ) _tOBS__add( a, (tBASE *) b )
uint32_t _tOBS__add( tOBS *self, tBASE *ob );

void tBASE__dump( tBASE *ob, uint8_t depth );
void tBASE__dump_nocr( tBASE *ob, uint8_t depth );
void tBASE__dumpxml( tBASE *self );

#define tARR__add( a,b) _tARR__add( a, (tBASE *) b )
void _tARR__add( tARR *self, tBASE *ob );

void **tARR__flatten( tARR *self );
tBASE *tARR__get( tARR *self, uint32_t pos );

tARR *tARR__newVals( int count, ... );
tARR *tARR__newStrs( int count, ... );

void tOBS__xml( tOBS *self, bytearr *ba, uint8_t depth );
void tBASE__xml( tBASE *ob, bytearr *ba, uint8_t depth );

#define tBASE__del( a ) _tBASE__del( (tBASE *) a )
void _tBASE__del( tBASE *self );

#define tBASE__dup( a ) _tBASE__dup( (tBASE *) a )
tBASE *_tBASE__dup( tBASE *self );

void tOBS__dump( tOBS *self, uint8_t depth );

#define tOBS__new( a ) _tOBS__new( (tBASE *) a )
tOBS *_tOBS__new( tBASE *root );

tOFF *tOFF__new( uint16_t off );
tOFFS *tOFFS__new();
uint16_t tOFFS__add( tOFFS *self, uint16_t off );

tSTR *tSTR__new( const char *val );
tSTR *tSTR__newl( const char *val, int len );
tI8 *tI8__new( uint8_t val );
tI16 *tI16__new( uint16_t val );
tI32 *tI32__new( uint32_t val );
tI64 *tI64__new( int64_t val );
tTIME *tTIME__new( double val );
tARR *tARR__new();
tARCID *tARCID__new( uint32_t val );
tDICT *tDICT__new();
tBASE *tREF__new( tBASE *val );
tF1 *tF1__new( float val );
tF2 *tF2__new( double val );
tDATA *tDATA__new( uint8_t *val, uint32_t len );
tUUID *tUUID__new( char *str );
tURL *tURL__new( char *relative );
tCAPS *tCAPS__new( tDICT *val );
tTESTCONF *tTESTCONF__new( tDICT *val );
uint32_t tI__val32( tBASE *self );
int64_t tI__val64( tBASE *self );
char tIsnum( tBASE *self );

void tARR__del( tARR *self );

bytearr *tARR__asaux( tARR *self );
bytearr *tDICT__asaux( tDICT *self );
void tBASE__toaux( tBASE *self, bytearr *out );
bytearr *tBASE__asaux( tBASE *self );
void tI32__toaux( tI32 *self, bytearr *out );
void tI16__toaux( tI16 *self, bytearr *out );
void tI8__toaux( tI8 *self, bytearr *out );
void tDATA__toaux( tDATA *self, bytearr *out );
void tBASE__archiveToAux( tBASE *self, bytearr *out );
uint8_t *tBASE__archive( tBASE *self, uint32_t *len );

#define tARR__addI32( a,b ) tARR__add( a, tI32__new( b ) )
#define tARR__addSTR( a,b ) tARR__add( a, tSTR__new( b ) )
#endif