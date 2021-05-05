// Copyright (c) 2021 David Helkowski
// MIT License

#include<stdint.h>
uint16_t bswap_u16( uint16_t v ) { return (v << 8) | (v >> 8 ); }
int16_t  bswap_i16(  int16_t  v ) { return (v << 8) | ((v >> 8) & 0xFF); }

int32_t bswap_i32( int32_t v ) {
  v = ((v << 8) & 0xFF00FF00) | ((v >> 8) & 0xFF00FF ); 
  return (v << 16) | ((v >> 16) & 0xFFFF);
}

uint32_t bswap_u32( uint32_t v ) {
  v = ((v << 8) & 0xFF00FF00 ) | ((v >> 8) & 0xFF00FF ); 
  return (v << 16) | (v >> 16);
}

int64_t bswap_i64( int64_t v ) {
  v = ((v << 8 ) & 0xFF00FF00FF00FF00ULL ) | ((v >> 8 ) & 0x00FF00FF00FF00FFULL );
  v = ((v << 16) & 0xFFFF0000FFFF0000ULL ) | ((v >> 16) & 0x0000FFFF0000FFFFULL );
  return (v << 32) | ((v >> 32) & 0xFFFFFFFFULL);
}

uint64_t bswap_u64( uint64_t v ) {
  v = ((v << 8 ) & 0xFF00FF00FF00FF00ULL ) | ((v >> 8 ) & 0x00FF00FF00FF00FFULL );
  v = ((v << 16) & 0xFFFF0000FFFF0000ULL ) | ((v >> 16) & 0x0000FFFF0000FFFFULL );
  return (v << 32) | (v >> 32);
}