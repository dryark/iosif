// Copyright (c) 2021 David Helkowski
// Anti-Corruption License
#ifndef __CFUTIL_H
#define __CFUTIL_H
#include<CoreFoundation/CoreFoundation.h>

CFNumberRef i8cf(  char num    );
CFNumberRef i16cf( int16_t num );
CFNumberRef i32cf( int32_t num );
CFNumberRef i64cf( int64_t num );
CFBooleanRef boolcf( char val );
uint64_t cfi64( CFNumberRef num );
uint16_t cfi16( CFNumberRef num );
uint8_t cfi8( CFNumberRef num );

char cfstreq( CFStringRef cf, const char *to );
char *str_cf2c( CFStringRef cfstr );

char iscfstr( CFTypeRef cf );
char iscfdict( CFTypeRef cf );
char iscfarr( CFTypeRef cf );
char iscfnum( CFTypeRef cf );

CFStringRef str_c2cf( char *str );

CFTypeRef cfmapget( CFDictionaryRef node, char *key );
void cfdump( int depth, CFTypeRef node );

CFDictionaryRef genmap( int count, ... );

char *cftype( CFTypeRef cf );

CFArrayRef strsToArr( int count, char **strs );

CFArrayRef genarr( int count, ... );
CFArrayRef genstrarr( int count, ... );

CFArrayRef deserialize( const uint8_t *buf, size_t bufsize );

uint32_t crc32( uint32_t crc, const char *buf, size_t len );

char *strArchive( const char *str, int *strLen );

void dumparchive( const uint8_t *data, uint32_t len );

CFPropertyListRef xml2plist( const char *data );
CFPropertyListRef data2plist( const uint8_t *data, int len );
#endif