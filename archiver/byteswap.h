// Copyright (c) 2021 David Helkowski
// MIT License

#ifndef __BYTESWAP_H
#define __BYTESWAP_H
#include<stdint.h>
uint16_t bswap_u16( uint16_t v );
int16_t  bswap_i16( int16_t  v );
int32_t  bswap_i32( int32_t  v );
uint32_t bswap_u32( uint32_t v );
int64_t  bswap_i64( int64_t  v );
uint64_t bswap_u64( uint64_t v );
#endif