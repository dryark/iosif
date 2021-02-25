// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include"bytearr.h"
#include"nsutil.h"
#include"cfutil.h"
#include"archiver.h"

bytechunk *bytechunk__new( uint8_t *data, uint32_t len, char alloc ) {
  bytechunk *self = (bytechunk *) malloc( sizeof( bytechunk ) );
  uint8_t *datacopy = (uint8_t *) malloc( len );
  memcpy( datacopy, data, len );
  self->data = datacopy;
  self->len = len;
  self->alloc = alloc;
  self->next = NULL;
  return self;
}

bytearr *bytearr__new() {
  bytearr *self = (bytearr *) calloc( 1, sizeof( bytearr ) );
  return self;
}

void ba__print( bytearr *self, char *fmt, ... ) {
  va_list args;
  va_start( args, fmt );
  char *data;
  int len = vasprintf( &data, fmt, args );
  va_end( args );
  bytearr__append( self, (uint8_t *) data, len, 0 );
}

void bytearr__append( bytearr *self, uint8_t *data, uint32_t len, char alloc ) {
  //printf("Appending %i bytes\n", len );
  bytechunk *chunk = bytechunk__new( data, len, alloc );
  if( !self->head ) {
    self->head = self->tail = chunk;
    return;
  }
  if( self->head == self->tail ) {
    self->head->next = chunk;
    self->tail = chunk;
    return;
  }
  bytechunk *oldtail = self->tail;
  self->tail = chunk;
  oldtail->next = chunk;
}

void bytearr__appendi32( bytearr *self, int32_t num ) {
  //printf("Appending i32 %i\n", num );
  int32_t *nump = (int32_t *) malloc(4);
  *nump = num;
  bytearr__append( self, (uint8_t *) nump, 4, 1 );
}

void bytearr__appendi64( bytearr *self, int64_t num ) {
  //printf("Appending i64 %" PRId64 "\n", num );
  int64_t *nump = (int64_t *) malloc(8);
  *nump = num;
  bytearr__append( self, (uint8_t *) nump, 8, 1 );
}

uint8_t *bytearr__bytes( bytearr *self, uint32_t *len ) {
  bytechunk *chunk = self->head;
  uint32_t total = 0;
  while( chunk ) {
    total += chunk->len;
    chunk = chunk->next;
  }
  uint8_t *bytes = (uint8_t *) malloc( total );
  *len = total;
  chunk = self->head;
  uint32_t pos = 0;
  while( chunk ) {
    memcpy( &bytes[ pos ], chunk->data, chunk->len );
    pos += chunk->len;
    chunk = chunk->next;
  }
  return bytes;
}

void bytearr__auxi32( bytearr *self, int32_t val ) {
  bytearr__appendi32( self, 10 ); // empty dict
  bytearr__appendi32( self, 3 ); // i32
  bytearr__appendi32( self, val );
}

void bytearr__auxi64( bytearr *self, int64_t val ) {
  bytearr__appendi32( self, 10 ); // empty dict
  bytearr__appendi32( self, 4 ); // i64
  bytearr__appendi64( self, val );
}

void bytearr__auxcf( bytearr *self, CFTypeRef cf, char secure ) {
  bytearr__appendi32( self, 10 ); // empty dict
  bytearr__appendi32( self, 2 ); // cfTypeRef
  
  int len = 0;
  uint8_t *data = cf2archive( cf, &len, secure );
  
  bytearr__appendi32( self, len );
  bytearr__append( self, data, len, 1 );
}

uint8_t *bytearr__asaux( bytearr *self, uint32_t *len ) {
  bytechunk *chunk = self->head;
  int64_t total = 16;
  while( chunk ) {
    total += chunk->len;
    chunk = chunk->next;
  }
  //printf("Aux total size: %" PRId64 "\n", total );
  //printf("Aux data size: %" PRId64 "\n", total-16 );
  uint8_t *bytes = (uint8_t *) calloc( 1, total );
  int64_t magic = 0x1F0;
  memcpy( bytes, &magic, 8 );
  int64_t justDataLen = total - 16;
  memcpy( &bytes[8], &justDataLen, 8 );
  
  *len = total;
  chunk = self->head;
  int pos = 16;
  while( chunk ) {
    memcpy( &bytes[ pos ], chunk->data, chunk->len );
    pos += chunk->len;
    chunk = chunk->next;
  }
  return bytes;
}

bytearr *cfarr2aux( CFTypeRef argsCf, char secure ) {
  bytearr *args = bytearr__new();
  if( iscfarr( argsCf ) ) {
    int count = CFArrayGetCount( (CFArrayRef) argsCf );
    //printf("Count = %i\n", count );
    for( int i=0;i<count;i++ ) {
      CFTypeRef el = CFArrayGetValueAtIndex( (CFArrayRef) argsCf, i );
      if( iscfnum( el ) ) {
        //printf("Is number\n");
        CFIndex numt = CFNumberGetType( el );
        switch( numt ) {
          case kCFNumberSInt32Type: { 
            SInt32 num;
            CFNumberGetValue( el, numt, &num);
            //printf("Number value:%i\n", num );
            bytearr__auxi32( args, num ); }
          }
      }
      else bytearr__auxcf( args, el, secure );
    }
  }
  else {
    bytearr__auxcf( args, argsCf, secure );
  }
  return args;
}