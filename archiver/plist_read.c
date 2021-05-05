// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

/*
  This parser does not make use of the "offset table" within bplist format.
  It is not needed, so why demand or use it?
  
  Instead, the parser uses a two-pass approach.
  All objects are read in normally except for dictionaries, arrays, and sets.
  Those are just created as blank on the first pass.
  On the second pass, the contents of the dictionaries, arrays, and sets are populated.
  
  The parser also does not need to know the number of objects present.
  Since the root object is always the first one, it is always possible to tell if more
  objects are present based on references from that objects.
*/

#include<stdint.h>
#include"unarchiver.h"
#include"byteswap.h"
#include"plist_read.h"
#include"lz4.h"

uint32_t readSize( uint8_t **posin, uint8_t *err ) {
  uint8_t *pos = *posin;
  
  uint32_t numBytes;
  //printf("b1:%02x %02x %02x\n", *pos, *(pos+1), *(pos+2) );
  uint8_t size = 1<<(*pos & 0x0F);
  switch( size ) {
    case 1:
      numBytes = *(pos+1);
      pos += 2;
      break;
    case 2:
      numBytes = bswap_u16( *( (uint16_t*) (pos+1) ) );
      pos += 3;
      break;
    case 4:
      numBytes = bswap_u32( *( (uint32_t*) (pos+1) ) );
      pos += 5;
      break;
    default:
      printf("Size error\n");
      *err = 1;
      return 0;
  }
  
  *posin = pos;
  return numBytes;
}

bpList *bpList__new( uint8_t *data, int len ) {
  bpList *self = ( bpList * ) malloc( sizeof( bpList ) );
  uint8_t tok = bpList__readTrailer( self, data, len );
  if( !tok ) {
    if( !strncmp( (char *) &data[4], "bv41", 4 ) ) {
      printf("lz4 encoded\n");
      uint32_t uncompressed = *( (uint32_t *) data );
      uint32_t compressed = *( (uint32_t *) (data+12) );
      printf("Uncompressed: %d\n", uncompressed );
      printf("Compressed: %d\n", compressed );
      void *unc = malloc( uncompressed );
      LZ4_decompress_safe( (char *) &data[16], unc, compressed, uncompressed );
      data = unc;
      self->len = uncompressed;
      bpList__readTrailer( self, data, uncompressed );
      //exit(0);
    }
    else {
      return NULL;
    }
    /*printf("Scan mode\n");
    self->bytesPerRef = 2;
    self->maxObNum = 1000;
    self->scan = 1;
    for( int i=0;i<len;i++ ) {
      if( data[i] == 'b' && data[i+1] == 'p' ) {
        printf("Position of bp:%d\n", i );
        printf("At pos: %.*s\n", 8, &data[i] );
        //printf("Version:%c%c\n", data[5], data[6] );
        data = data+i;
        break;
      } else {
        printf("%02x ", data[i] );
      }
    }*/
  }
  //self->obsArr = (tBASE **) malloc( sizeof( void * ) * ( self->maxObNum + 1 ) );
  //self->obs = tARR__new();
  self->obs = ptrArr__new();
  self->data = data;
  self->len = len;
  bpList__pass1( self, self->bytesPerRef );
  bpList__pass2( self, self->bytesPerRef );
  return self;
}

uint8_t bpList__readTrailer( bpList *self, uint8_t *data, int len ) {
  char begin[30];
  memset( begin, 0, 30 );
  memcpy( begin, data, 8 );
  if( strcmp( begin, "bplist00" ) ) {
    return 0;
  }
  
  int tStart = len - 32;
  //int verPos = tStart + 5; 

  //uint8_t version = *( data + verPos );
  self->bytesPerOffset = *( data + tStart + 6 );
  self->bytesPerRef    = *( data + tStart + 7 );
  self->maxObNum = bswap_u32( *( (uint32_t *) ( data + tStart + 12 ) ) ) - 1;
  self->rootObNum = *( (uint32_t *) ( data + tStart + 20 ) );
  self->offsetsPos = bswap_u32( *( (uint32_t *) ( data + tStart + 28 ) ) ); 
  
  //printf("Version: %d\n", version );
  duprintf("Bytes per offset: %d\n", self->bytesPerOffset );
  duprintf("Bytes per ref: %d\n", self->bytesPerRef );
  duprintf("Num objects: %d\n", self->maxObNum+1 );
  duprintf("Root object num: %d\n", self->rootObNum );
  duprintf("Offset table pos: %d\n\n", self->offsetsPos );
  return 1;
}

uint8_t bpList__pass1( bpList *self, uint8_t refSize ) {
  uint8_t *pos = self->data + 8;
  int tStart = self->len - 32;
  uint8_t *end = self->data + tStart;
  int onum = 0;
  int maxo = 0;
  
  uint32_t numBytes;
  int keyCount = 0;
  tBASE *ob;
  ptrArr *obs = self->obs;
  uint8_t err = 0;
  
  //while( pos < end ) {
  while( onum <= maxo || onum <= self->maxObNum ) {
    uint8_t b1 = *(pos++);
    uint8_t upper = ( b1 & 0xF0 ) >> 4;
    uint8_t lower = b1 & 0xF;
    duprintf("Onum: %d - %x -", onum, upper );
    switch( upper ) {
    case 0b0000: // singleton
      switch( lower ) {
        case 0b0000: duprintf("Null\n");  ob = asBASE tNULL__new();    break;
        case 0b1000: duprintf("False\n"); ob = asBASE tBOOL__new( 0 ); break;
        case 0b1001: duprintf("True\n");  ob = asBASE tBOOL__new( 1 ); break;
        case 0b1111: duprintf("Fill\n");  ob = asBASE tNULL__new( 0 ); break;
        default:
          //continue;
          printf("Null with %x lower\n", lower );
          goto FAIL1;
      }
      break;
    case 0b0001: { // int
      if( lower > 3 ) goto FAIL1;
      uint8_t bytes = 1<<lower;
      switch( bytes ) {
        case 1: {
          int8_t val = *( (int8_t *) pos );
          duprintf("i8 = %d\n", val );
          ob = asBASE tI8__new( val );
        } break;
        case 2: {
          int16_t val = bswap_i16( *( (int16_t *) pos ) );
          duprintf("i16 = %d\n", val );
          ob = asBASE tI16__new( val );
        } break;
        case 4: {
          int32_t val = bswap_i32( *( (int32_t *) pos ) );
          duprintf("i32 = %" PRId32 "\n", val );
          ob = asBASE tI32__new( val );
        } break;
        case 8: {
          int64_t val = bswap_i64( *( (int64_t *) pos ) );
          duprintf("i64 = %" PRId64 "\n", val );
          ob = asBASE tI64__new( val );
        } break;
      }
      pos += bytes;
      break;
    }
    case 0b0010: // float
      numBytes = 1<<lower;
      switch( numBytes ) {
        case 4: { // single precision
            //duprintf("raw: %02x %02x %02x %02x \n", *pos, *(pos+1), *(pos+2), *(pos+3) );
            uint32_t val1 = bswap_u32( *( (uint32_t *) pos ) );
            float val = *( (float *) &val1 );
            duprintf("float = %.2f\n", val );
            ob = asBASE tF1__new( val );
          } break;
        case 8: { // double precision
            uint64_t val1 = bswap_u64( *( (uint64_t *) pos ) );
            double val = *( (double *) &val1 );
            duprintf("double = %.2f\n", val );
            ob = asBASE tF2__new( val );
          } break;
        default:
          printf("Float with numBytes %d\n", numBytes );
          numBytes /= 2;
          goto FAIL1;
      }
      pos += numBytes;
      break;
    case 0b0011: // date
      duprintf("Date\n");
      ob = asBASE tTIME__new( *( (double *) pos ) );
      pos += 8;
      break;
    case 0b0100: // data
      if( lower == 0b1111 ) numBytes = readSize( &pos, &err );
      else                  numBytes = lower;
      
      if( !numBytes && err ) goto FAIL1;
      
      ob = asBASE tDATA__new( pos, numBytes );
      pos += numBytes;
      break;
    case 0b0101: // string
      //printf("lower:%d\n", lower );
      if( lower == 0b1111 ) numBytes = readSize( &pos, &err );
      else                  numBytes = lower;
      
      if( !numBytes && err ) goto FAIL1;
      
      duprintf("Numbytes:%d\n", numBytes );
      
      duprintf("\"%.*s\"\n", numBytes, pos );
      ob = asBASE tSTR__newl( (char *) pos, numBytes );
      pos += numBytes;
      
      break;
    case 0b0110: // utf-16
      printf("unicode str\n");
      if( lower == 0b1111 ) numBytes = readSize( &pos, &err );
      else                  numBytes = lower;
      
      printf("  len=%d\n", numBytes );
      pos += numBytes * 2 + 1;
      
      break;
    case 0b1000: // uid
      switch( lower + 1 ) {
        case 1: {
          uint8_t uid = *pos;
          duprintf("Uid 1b = %d\n", uid );
          ob = asBASE tARCID__new( uid );
        } break;
        case 2:{
          uint16_t uid = bswap_u16( *( (uint16_t *) pos ) );
          duprintf("Uid 2b = %d\n", uid );
          ob = asBASE tARCID__new( uid );
        } break;
        case 4:{
          uint32_t uid = *( (uint32_t *) pos );
          duprintf("Uid 4b = %d\n", uid );
          ob = asBASE tARCID__new( uid );
        } break;
        default:
          printf("UID length of %d\n", lower+1);
          goto FAIL1;
      }
      pos += lower + 1;
      break;
    case 0b1100: // set
    case 0b1010: // array
      if( lower == 0b1111 ) keyCount = readSize( &pos, &err );
      else                  keyCount = lower;
      
      if( !keyCount && err ) goto FAIL1;
      
      duprintf("Array count=%d\n", lower );
      duprintf("  ");
      if( refSize == 1 ) {
        for( int i=0;i<keyCount;i++ ) {
          uint8_t key = *( pos + i );
          if( key > maxo ) maxo = key;
          duprintf("%d ", key );
        }
      }
      if( refSize == 2 ) {
        for( int i=0;i<keyCount;i++ ) {
          uint16_t key = bswap_u16( *( (uint16_t *) (pos + i*2 ) ) );
          if( key > maxo ) maxo = key;
          duprintf("%d ", key );
        }
      }
      duprintf("\n");
        
      pos += keyCount * refSize;
      ob = asBASE tARR__new();
      break;
    case 0b1101: // dict
      if( lower == 0b1111 ) keyCount = readSize( &pos, &err );
      else                  keyCount = lower;
      
      if( !keyCount && err ) goto FAIL1;
      
      duprintf("Dict pairs=%d\n", keyCount );
      if( refSize == 1 ) {
        for( int i=0;i<keyCount;i++ ) {
          uint8_t key = *( pos + i );
          uint8_t val = *( pos + keyCount + i );
          if( key > maxo ) maxo = key;
          if( val > maxo ) maxo = val;
          duprintf("  %d => %d\n", key, val );
        }
      }
      if( refSize == 2 ) {
        for( int i=0;i<keyCount;i++ ) {
          uint16_t key = bswap_u16( *( (uint16_t *) (pos + i*2 ) ) );
          uint16_t val = bswap_u16( *( (uint16_t *) (pos + keyCount*2 + i*2 ) ) );
          if( key > maxo ) maxo = key;
          if( val > maxo ) maxo = val;
          duprintf("  %d => %d\n", key, val );
        }
      }
      
      pos += keyCount * 2 * refSize;
      ob = asBASE tDICT__new();
      break;
    case 0b0111:
      printf("0111 ??\n");
      continue;
      //break;
    case 0b1111:
      printf("Skipping 1111\n" );
      continue;
    default:
      printf("Upper:%x\n", upper );
      //goto FAIL1;
    }
    //tARR__add( self->obs, ob );
    //self->obsArr[ onum ] = ob;
    ptrArr *newobs = ptrArr__add( obs, (void *) ob );
    if( newobs ) obs = newobs;
    onum++;
  }
  //printf("End pos: %d\n", (int) (pos - data) );
  return 1;
FAIL1:
  
  return 0;
}

void bpList__pass2( bpList *self, uint8_t refSize ) {
  uint8_t *pos = self->data + 8;
  int tStart = self->len - 32;
  uint8_t *end = self->data + tStart;
  int onum = 0;
  
  uint32_t numBytes, numKeys;
  //tBASE *cur = self->obs->head;
  ptrArr *obs = self->obs;
  ptrArr *rootobs = self->obs;
  uint32_t obsPos = 0;
  tBASE *cur = asBASE ptrArr__iter( &obs, &obsPos );
  
  uint8_t *used = (uint8_t *) calloc( 1, self->maxObNum + 1 );
  used[0] = 1;
  uint8_t err;
  
  while( onum < self->maxObNum ) {
    uint8_t b1 = *(pos++);
    uint8_t upper = ( b1 & 0xF0 ) >> 4;
    uint8_t lower = b1 & 0xF;
    switch( upper ) {
      case 0b0001: // int
      case 0b0010: // float
        pos += 1<<lower;
        break;
      case 0b0011: // date
        pos += 8;
        break;
    case 0b0100: // data
      if( lower == 0b1111 ) numBytes = readSize( &pos, &err );
      else                  numBytes = lower;
      
      pos += numBytes;
      break;
    case 0b0101: // string
      if( lower == 0b1111 ) numBytes = readSize( &pos, &err );
      else                  numBytes = lower;
      
      pos += numBytes;
      break;
    case 0b0110: // utf-16
      break;
    case 0b1000: // uid
      pos += lower + 1;
      break;
    case 0b1100: // set
    case 0b1010: // array
      duprintf("pass 2 Onum: %d\n", onum );
      duprintf("  Array\n");
      //printf("  Cur type:%d\n", cur->type );
      if( lower == 0b1111 ) numKeys = readSize( &pos, &err );
      else                  numKeys = lower;
      
      //printf("  Count=%d\n", numKeys );
      for( int i=0;i<numKeys;i++ ) {
        uint16_t key;
        if( refSize == 1 ) key = *( pos + i );
        if( refSize == 2 ) key = bswap_u16( *( (uint16_t *) (pos + i*2 ) ) );
        
        tBASE *ob = (tBASE *) ptrArr__get( rootobs, key );
        if( !ob ) printf("Could not fetch item %d\n", key );
          
        if( used[key] ) {
          //tARR__add( (tARR *) cur, tI32__new( i ) );
          //tARR__add( (tARR *) cur, tI32__new( key ) );
          tARR__add( (tARR *) cur, tREF__new( ob ) );
        } else {
          used[key] = 1;
          //tARR__add( (tARR *) cur, tI32__new( i ) );
          //tARR__add( (tARR *) cur, tI32__new( key ) );
          tARR__add( (tARR *) cur, ob );
        }
      }
            
      pos += numKeys * refSize;
      break;
    case 0b1101: // dict
      duprintf("pass 2 Onum: %d\n", onum );
      duprintf("  Dict\n");
      if( lower == 0b1111 ) numKeys = readSize( &pos, &err );
      else                  numKeys = lower;
      
      //printf("  Count=%d\n", numKeys );
      for( int i=0;i<numKeys;i++ ) {
        uint16_t key;
        uint16_t val;
        if( refSize == 1 ) {
          key = *( pos + i );
          val = *( pos + numKeys + i );
        }
        if( refSize == 2 ) {
          key = bswap_u16( *( (uint16_t *) (pos + i*2 ) ) );
          val = bswap_u16( *( (uint16_t *) (pos + numKeys*2 + i*2 ) ) );
        }
        
        //printf("  %d => %d\n", key, val );
        tBASE *keyT;
        tBASE *valT;
        if( used[key] ) {
          keyT = (tBASE *) ptrArr__get( rootobs, key );
          if( !keyT ) printf("Could not fetch item %d\n", key );
          keyT = asBASE tBASE__dup( keyT );
        } else {
          used[key] = 1;
          keyT = asBASE ptrArr__get( self->obs, key );
        }
        
        if( used[val] ) {
          valT = (tBASE *) ptrArr__get( rootobs, val );
          if( !valT ) printf("Could not fetch item %d\n", val );
          valT = asBASE tREF__new( valT );
        } else {
          used[val] = 1;
          valT = asBASE ptrArr__get( rootobs, val );
        }
        
        #ifdef UDEBUG
        if( keyT->type == xfSTR ) {
          printf("  %s => %d (%d)\n", (( tSTR * ) keyT )->val, val, valT->type );
        }
        else if( keyT->type == xfREF ) {
          tSTR *strT = (tSTR *) ( (tREF *) keyT )->val;
          printf("  %s => %d (%d)\n", strT->val, val, valT->type );
        }
        else {
          tBASE__dump( keyT, 1 );
          printf(" => %d (%d)\n", val, valT->type );
        }
        #endif
        
        tDICT__seto( (tDICT *) cur, keyT, valT );
      }
      pos += 2 * numKeys * refSize;
      
      break;
    }
    
    onum++;
    
    cur = ptrArr__iter( &obs, &obsPos ); 
    if( !cur ) printf("NULL cur!\n");
    //cur = cur->next;
  }
  //printf("End pos: %d\n", (int) (pos - data) );
}