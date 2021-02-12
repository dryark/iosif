// Copyright (c) 2021 David Helkowski
// Anti-Corruption License
#include<CoreFoundation/CoreFoundation.h>

CFNumberRef i8cf(  char num    );
CFNumberRef i32cf( int32_t num );
CFNumberRef i64cf( int64_t num );

char cfstreq( CFStringRef cf, const char *to );
char *str_cf2c( CFStringRef cfstr );

char iscfstr( CFTypeRef cf );
char iscfdict( CFTypeRef cf );
char iscfarr( CFTypeRef cf );
char iscfnum( CFTypeRef cf );

CFStringRef str_c2cf( char *str );

void cfdump( int depth, CFTypeRef node );

void cf2json( CFTypeRef node );

CFDictionaryRef genmap( int count, ... );

char *cftype( CFTypeRef cf );

CFArrayRef strsToArr( int count, char **strs );

CFArrayRef genarr( int count, ... );

CFArrayRef deserialize( const uint8_t *buf, size_t bufsize );

uint32_t crc32( uint32_t crc, const char *buf, size_t len );

char *strArchive( const char *str, int *strLen );

void dumparchive( uint8_t *data, int len );