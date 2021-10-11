// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdint.h>
#include"../bytearr.h"
#include"byteswap.h"
#include"types.h"
#include"unarchiver.h"
#include"plist.h"

uint32_t tOBS__plist( tOBS *self, bytearr *ba, tOFFS *offs ) {
  tBASE *cur = self->head;
  uint32_t rootid;
  char doset = 1;
  while( cur ) {
    uint32_t itemid = tBASE__plist( offs, ba, cur );
    if( doset ) {
      rootid = itemid;
      doset = 0;
    }
    cur = cur->next;
  }
  return rootid;
}

tARR * tOBS__2arr( tOBS *self ) {
  tBASE *cur = self->head;
  tARR *arr = tARR__new();
  //tARR__add( arr, tSTR__new("") );
  tARR__add( arr, tSTR__new("$null") );
  //tARR__add( arr, tNULL__new() );
  while( cur ) {
    tARR__add( arr, cur );
    cur = cur->next;
  }
  return arr;
}

void tBOOL__plist( tOFFS *obs, bytearr *ba, tBOOL *self ) {
  uint8_t byte = self->val ? 0b00001001 : 0b00001000;
  bytearr__appendu8( ba, byte );
}

void tNULL__plist( tOFFS *obs, bytearr *ba, tNULL *self ) {
  uint8_t byte = 0x00;
  bytearr__appendu8( ba, byte );
}

void tI8__plist( tOFFS *obs, bytearr *ba, tI8 *self ) {
  uint8_t byte = 0b00010000;
  bytearr__appendu8( ba, byte );
  bytearr__appdup( ba, (uint8_t *) &self->val, 1 );
}

void tI16__plist( tOFFS *offs, bytearr *ba, tI16 *self ) {
  uint8_t byte = 0b00010001;
  bytearr__appendu8( ba, byte );
  int16_t val = bswap_i16( self->val );
  bytearr__appdup( ba, (uint8_t *) &val, 2 );
}

void tI32__plist( tOFFS *offs, bytearr *ba, tI32 *self ) {
  uint8_t byte = 0b00010010;
  bytearr__appendu8( ba, byte );
  int32_t val = bswap_i32( self->val );
  bytearr__appdup( ba, (uint8_t *) &val, 4 );
}

void tI64__plist( tOFFS *offs, bytearr *ba, tI64 *self ) {
  uint8_t byte = 0b00010011;
  bytearr__appendu8( ba, byte );
  int64_t val = bswap_i64( self->val );
  bytearr__appdup( ba, (uint8_t *) &val, 8 );
}

void tF1__plist( tOFFS *offs, bytearr *ba, tF1 *self ) {
  uint8_t byte = 0b00100001;
  // TODO
}

void tF2__plist( tOFFS *offs, bytearr *ba, tF2 *self ) {
  uint8_t byte = 0b00100010;
  // TODO
}

void tTIME__plist( tOFFS *offs, bytearr *ba, tTIME *self ) {
  uint8_t byte = 0b00110000;
  // TODO
}

void tDATA__plist( tOFFS *offs, bytearr *ba, tDATA *self ) {
  uint8_t byte = 0b01000000;
  int sizelen = 0;
  uint8_t *sizebuf = encodeSize( self->len, &sizelen );
  if( sizelen ) {
    byte |= 0b1111;
    bytearr__appendu8( ba, byte );
    bytearr__appdup( ba, sizebuf, sizelen );
  }
  else {
    byte |= self->len;
    bytearr__appendu8( ba, byte );
  }
  if( self->len ) bytearr__appdup( ba, (uint8_t *) self->val, self->len );
}

uint8_t *encodeSize( uint32_t val, int *len ) {
  if( val < 0b1111 ) {
    *len = 0;
    return 0;
  }
  if( val < INT8_MAX ) {
    *len = 2;
    uint8_t *buf = (uint8_t *) malloc(2);
    *buf = 0x10;
    *(buf+1) = val;
    return buf;
  }
  if( val < INT16_MAX ) {
    *len = 3;
    uint8_t *buf = (uint8_t *) malloc(3);
    *buf = 0x11;
    *( (uint16_t *) (buf+1) ) = bswap_u16( val );
    return buf;
  }
  *len = 5;
  uint8_t *buf = (uint8_t *) malloc(5);
  *buf = 0x12;
  *( (uint32_t *) (buf+1) ) = bswap_u32( val );
  return buf;
}

void tSTR__plist( tOFFS *offs, bytearr *ba, tSTR *self ) {
  uint8_t byte = 0b01010000;
  int sizelen = 0;
  uint16_t len = strlen( self->val );
  uint8_t *sizebuf = encodeSize( len, &sizelen );
  if( sizelen ) {
    byte |= 0b1111;
    bytearr__appendu8( ba, byte );
    //if( sizelen == 1 ) bytearr__appendu8( ba, 0x10 );
    //if( sizelen == 2 ) bytearr__appendu8( ba, 0x11 );
    bytearr__appdup( ba, sizebuf, sizelen );
  }
  else {
    if( len ) {
      byte |= len;
    }
    bytearr__appendu8( ba, byte );
  }
  bytearr__appdup( ba, (uint8_t *) self->val, len );
}

void tARCID__plist( tOFFS *offs, bytearr *ba, tARCID *self ) {
  uint8_t byte = 0b10000000;
  int32_t val1 = self->val;
  if( val1 <= INT8_MAX ) {
    //byte |= 0b0000;
    bytearr__appendu8( ba, byte );
    bytearr__appdup( ba, (uint8_t *) &val1, 1 );
    return;
  }
  if( val1 <= INT16_MAX ) {
    byte |= 0b0001;
    bytearr__appendu8( ba, byte );
    int16_t val16 = val1;
    int16_t val = bswap_i16( val16 );
    bytearr__appdup( ba, (uint8_t *) &val, 2 );
    return;
  }
  byte |= 0b0011;
  bytearr__appendu8( ba, byte );
  int16_t val16 = val1;
  int32_t val = bswap_i32( val16 );
  bytearr__appdup( ba, (uint8_t *) &val, 4 );
}

uint16_t tARR__plist( tOFFS *offs, bytearr *ba, tARR *self ) {
  uint8_t byte = 0b10100000;
  
  uint16_t offset = ba->len;
  
  // encode type and length
  int sizelen = 0;
  uint8_t *sizebuf = encodeSize( self->count, &sizelen );
  if( sizelen ) {
    byte |= 0b1111;
    bytearr__appendu8( ba, byte );
    bytearr__appdup( ba, sizebuf, sizelen );
  }
  else {
    byte |= self->count;
    bytearr__appendu8( ba, byte );
  }
  
  bytechunk *chunk = ba->tail;
  for( uint32_t i=0;i<self->count;i++ ) {
    bytearr__appendu16( ba, 0 );
  }
  bytechunk *entry = chunk->next;
  
  tBASE *val = self->head;
  for( uint32_t i=0;i<self->count;i++ ) {
    uint32_t oid = tBASE__plist( offs, ba, val );
    //bytearr__appendu16( ba, bswap_u16( oid ) );
    uint16_t oid2 = bswap_u16( oid );
    memcpy( entry->data, &oid2, 2 );
    entry = entry->next;
    val = val->next;
  }
  
  return offset;
}

uint16_t tDICT__plist( tOFFS *offs, bytearr *ba, tDICT *self ) {
  uint8_t byte = 0b11010000;
    
  uint16_t offset = ba->len;
  
  // encode type and length
  int sizelen = 0;
  uint8_t *sizebuf = encodeSize( self->count, &sizelen );
  if( sizelen ) {
    byte |= 0b1111;
    bytearr__appendu8( ba, byte );
    bytearr__appdup( ba, sizebuf, sizelen );
  }
  else {
    byte |= self->count;
    bytearr__appendu8( ba, byte );
  }
  
  bytechunk *keyentry = ba->tail;
  for( uint32_t i=0;i<self->count;i++ ) {
    bytearr__appendu16( ba, 0 );
  }
  keyentry = keyentry->next;
  
  bytechunk *valentry = ba->tail;
  for( uint32_t i=0;i<self->count;i++ ) {
    bytearr__appendu16( ba, 0 );
  }
  valentry = valentry->next;
  
  tBASE *key = self->keyHead;
  for( uint32_t i=0;i<self->count;i++ ) {
    uint16_t koid = tBASE__plist( offs, ba, key );
    //printf("Key id: %d\n", koid );
    //bytearr__appendu16( ba2, bswap_u16( koid ) );
    uint16_t koid2 = bswap_u16( koid );
    memcpy( keyentry->data, &koid2, 2 );
    key = key->next;
    keyentry = keyentry->next;
  }
  
  tBASE *val = self->valHead;
  for( uint32_t i=0;i<self->count;i++ ) {
    uint16_t oid = tBASE__plist( offs, ba, val );
    //printf("Val id: %d\n", oid );
    //bytearr__appendu16( ba2, bswap_u16( oid ) );
    uint16_t oid2 = bswap_u16( oid );
    memcpy( valentry->data, &oid2, 2 );
    val = val->next;
    valentry = valentry->next;
  }
  
  return offset;
}

char *typeText( uint8_t num ) {
  switch(num) {
    case xfI8:    return "i8";  
    case xfI16:   return "i16";  
    case xfI32:   return "i32";  
    case xfI64:   return "i64";  
    case xfBOOL:  return "bool";  
    case xfSTR:   return "str";  
    case xfARR:   return "arr";  
    case xfDICT:  return "dict";  
    case xfARCID: return "arcid";  
    case xfREF:   return "ref";  
    case xfDATA:  return "data";  
    case xfNULL:  return "null";  
    case xfOBS:   return "obs";  
  }
  return "?";
}

uint16_t tBASE__plist( tOFFS *offs, bytearr *ba, tBASE *ob ) {
  uint16_t offset = ba->len;
  if( ob->type == xfREF ) return tBASE__plist(  offs, ba, ( (tREF *) ob )->val );
  if( ob->type == xfOBS ) return tOBS__plist( (tOBS *) ob, ba, offs );
  uint16_t index = tOFFS__add( offs, offset );
  switch( ob->type ) {
    case xfI8:    tI8__plist(    offs, ba, (tI8 *)    ob ); break;     
    case xfI16:   tI16__plist(   offs, ba, (tI16 *)   ob ); break;
    case xfI32:   tI32__plist(   offs, ba, (tI32 *)   ob ); break;
    case xfI64:   tI64__plist(   offs, ba, (tI64 *)   ob ); break;
    case xfBOOL:  tBOOL__plist(  offs, ba, (tBOOL *)  ob ); break;
    case xfSTR:   tSTR__plist(   offs, ba, (tSTR *)   ob ); break;
    case xfARR:   tARR__plist(   offs, ba, (tARR *)   ob ); break;
    case xfDICT:  tDICT__plist(  offs, ba, (tDICT *)  ob ); break;
    case xfARCID: tARCID__plist( offs, ba, (tARCID *) ob ); break;
    case xfDATA:  tDATA__plist(  offs, ba, (tDATA *)  ob ); break;
    case xfNULL:  tNULL__plist(  offs, ba, (tNULL *)  ob ); break;
    default: return 0;
  }
  //printf("Writing %s to offset %02x\n", typeText( ob->type ), offset );
  return index;
}

uint8_t *tBASE__tobin( tBASE *self, uint32_t *len ) {
  tOFFS *offs = tOFFS__new( self );
  bytearr *ba = bytearr__new();
  bytearr__append( ba, (uint8_t *) "bplist00", 8, 0 );
  
  uint32_t rootNum = tBASE__plist( offs, ba, self );
  
  //printf("Root ob num: %d\n", rootNum );
  
  // Note the location of the offsets table
  uint32_t offsetsPos = ba->len;
  //printf("Offsets position: %d\n", offsetsPos );
  
  uint8_t offBytes = 1;
  if( offsetsPos > INT8_MAX ) {
    offBytes = 2;
  } else if( offsetsPos > INT16_MAX ) {
    offBytes = 4;
  }
  
  // Write the offsets table
  uint16_t numOffs = offs->len;
  tOFF *curOff = offs->head;
  for( uint16_t i=0;i<numOffs;i++ ) {
    //printf("Writing offset %d\n", curOff->offset );
    if( offBytes == 1 ) {
      bytearr__appendu8( ba, curOff->offset );
    } else if( offBytes == 2 ) {
      bytearr__appendu16( ba, bswap_u16( curOff->offset ) );
    } else if( offBytes == 4 ) {
      bytearr__appendu32( ba, bswap_u32( curOff->offset ) );
    }
    curOff = curOff->next;
  }
  
  //printf("Number of objects: %d\n", numOffs );
  
  // Write the trailer
  bpList__encodeTrailer( ba, numOffs, offsetsPos, rootNum, offBytes );
  
  uint8_t *bytes = bytearr__bytes( ba, len );
 
  return bytes;
}

uint8_t *tBASE__archivebin( tBASE *self, uint32_t *len ) {
  tOBS *obs = tOBS__new( self );
  
  tDICT *root = tDICT__newPairs( 8,
    "$objects", (tBASE *) tOBS__2arr( obs ),
    "$top", tDICT__newPairs( 2,
      "root", tARCID__new( 1 )
    ),
    "$version", tI32__new( 100000 ),
    "$archiver", tSTR__new( "NSKeyedArchiver" )
  );
  
  #ifdef DEBUG2
  tBASE__dump( (tBASE *) root, 1 );
  #endif
  //tBASE__dumpxml( (tBASE *) root );

  return tBASE__tobin( (tBASE *) root, len );
}

void bpList__encodeTrailer( bytearr *ba, uint32_t numObs, uint32_t offsetsPos, uint32_t rootNum, uint8_t offBytes ) {
  // Initial blank 6 bytes
  bytearr__appendu32( ba, 0 ); // 0-3
  bytearr__appendu16( ba, 0 ); // 4-5
  
  bytearr__appendu8( ba, offBytes ); // 6 bytes per offset
  bytearr__appendu8( ba, 2 ); // 7 bytes per ref
  
  bytearr__appendu32( ba, 0 ); // 8-11
  bytearr__appendu32( ba, bswap_u32( numObs ) ); // 12-15 max ob num
  
  bytearr__appendu32( ba, 0 ); // 16-19
  bytearr__appendu32( ba, bswap_u32( rootNum ) ); // 20-23 root ob num
  
  bytearr__appendu32( ba, 0 ); // 24-27
  bytearr__appendu32( ba, bswap_u32( offsetsPos ) ); // 28-31 offsets pos
}