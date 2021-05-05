// Copyright (c) 2021 David Helkowski
// Based on https://github.com/rxi/uuid4 - Copyright (c) 2018 rxi
// MIT license

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include"uuid.h"

// ***** START PRNG *****
// License CC0 - http://creativecommons.org/publicdomain/zero/1.0/

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static uint64_t s[4];

static uint64_t next() {
	const uint64_t result = rotl(s[0] + s[3], 23) + s[0];
	const uint64_t t = s[1] << 17;

	s[2] ^= s[0]; s[3] ^= s[1]; s[1] ^= s[2]; s[0] ^= s[3];
	s[2] ^= t;
	s[3] = rotl(s[3], 45);

	return result;
}

// ***** END PRNG *****

static char initdone = 0;

static void uuid_init() {
  FILE *fp = fopen("/dev/urandom", "rb");
  if( !fp ) goto ERR;
  int bytes_read = fread(s, 1, sizeof(s), fp);
  fclose( fp );
  if( bytes_read == sizeof(s) ) return;
ERR:
  s[0] = 1; s[1] = 2; s[2] = 3; s[3] = 4;
}

char *uuid_generate() {
  if( !initdone ) uuid_init();
  char *buf = (char *) malloc( 37 );
  char *dst = buf;
  static const char *template = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
  static const char *chars = "0123456789abcdef";
  union { unsigned char b[16]; uint64_t word[2]; } s;
  const char *p;
  int i, n;
  s.word[0] = next();
  s.word[1] = next();
  
  p = template;
  i = 0;
  while( *p ) {
    n = s.b[i >> 1];
    n = (i & 1) ? (n >> 4) : (n & 0xf);
    switch( *p ) {
      case 'x':
        *dst = chars[n]; i++;  break;
      case 'y':
        *dst = chars[(n & 0x3) + 8]; i++;  break;
      default:
        *dst = *p;
    }
    dst++, p++;
  }
  *dst = '\0';
  return buf;
}
