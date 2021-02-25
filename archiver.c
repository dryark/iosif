// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdarg.h>

#include"archiver.h"
#include"bytearr.h"

tI32 *tI32__new( uint32_t val ) {
  tI32 *self = (tI32 *) calloc( 1, sizeof( tI32 ) );
  self->type = xfI32;
  self->val = val;
  return self;
}

tARCID *tARCID__new( uint32_t val ) {
  tARCID *self = (tARCID *) calloc( 1, sizeof( tARCID ) );
  self->type = xfARCID;
  self->val = val;
  return self;
}

tSTR *tSTR__new( char *val ) {
  tSTR *self = (tSTR *) calloc( 1, sizeof( tSTR ) );
  self->type = xfSTR;
  self->val = val;
  return self;
}

tBASE *tREF__new( tBASE *val ) {
  tREF *self = (tREF *) calloc( 1, sizeof( tREF ) );
  self->type = xfREF;
  self->val = val;
  return (tBASE *) self;
}

tARR *tARR__new() {
  tARR *self = (tARR *) calloc( 1, sizeof( tARR ) );
  self->type = xfARR;
  return self;
}

tOBS *_tOBS__new( tBASE *root ) {
  tOBS *self = (tOBS *) calloc( 1, sizeof( tOBS ) );
  self->type = xfOBS;
  tOBS__add( self, root );
  return self;
}

tARR *tARR__newStrs( int count, ... ) {
  va_list va;
  va_start( va, count );
  
  tARR *self = (tARR *) calloc( 1, sizeof( tARR ) );
  self->type = xfARR;
  
  for( int i=0;i<count;i++ ) {
    char *str = va_arg( va, char * );
    tARR__add( self, tSTR__new( str ) );
  }
  
  return self;
}

tARR *tARR__newVals( int count, ... ) {
  va_list va;
  va_start( va, count );
  
  tARR *self = (tARR *) calloc( 1, sizeof( tARR ) );
  self->type = xfARR;
  
  for( int i=0;i<count;i++ ) {
    tBASE *val = va_arg( va, tBASE * );
    tARR__add( self, val );
  }
  
  return self;
}

tDICT *tDICT__new() {
  tDICT *self = (tDICT *) calloc( 1, sizeof( tDICT ) );
  self->type = xfDICT;
  return self;
}

tDICT *tDICT__newPairs( int count, ... ) {
  va_list va;
  va_start( va, count );
  
  tDICT *self = (tDICT *) calloc( 1, sizeof( tDICT ) );
  self->type = xfDICT;
  
  for( int i=0;i<count;i+=2 ) {
    char *key = va_arg( va, char * );
    tBASE *val = va_arg( va, tBASE * );
    tDICT__set( self, key, val );
  }
  
  return self;
}

void _tARR__add( tARR *self, tBASE *ob ) {
  if( !self->head ) {
    self->head = self->tail = ob;
    return;
  }
  if( self->head == self->tail ) {
    self->head->next = ob;
    self->tail = ob;
    return;
  }
  self->tail->next = ob;
  self->tail = ob;
}

uint32_t tOBS__rawadd( tOBS *self, tBASE *ob ) {
  self->num++;
  
  do {
    if( !self->head ) {
      self->head = self->tail = ob;
      break;
    }
    if( self->head == self->tail ) {
      self->head->next = ob;
      self->tail = ob;
      break;
    }
    self->tail->next = ob;
    self->tail = ob;
  } while( 0 );
  
  return self->num;
}

uint32_t tSTR__toobs( tSTR *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tI32__toobs( tI32 *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tBOOL__toobs( tBOOL *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tARR__toobs( tARR *self, tOBS *obs ) {
  tDICT *flat = tDICT__new();
  uint32_t selfId = tOBS__rawadd( obs, (tBASE *) flat );
  
  tDICT *type = tDICT__new();
  tDICT__set( type, "$classname", tSTR__new("NSArray") );
  tDICT__set( type, "$classes", tARR__newStrs( 2, "NSArray", "NSOjbect" ) );
  uint32_t typeId = tOBS__rawadd( obs, (tBASE *) type );
  
  tARR *obsAsIds = tARR__new();
  tBASE *curVal = self->head;
  while( curVal ) {
    uint32_t obId = tOBS__add( obs, curVal );
    tARR__add( obsAsIds, tARCID__new( obId ) );
    curVal = curVal->next;
  }
  
  tDICT__set( flat, "NS.objects", obsAsIds );
  tDICT__set( flat, "$class", tARCID__new( typeId ) );
  
  return selfId;
}

uint32_t tDICT__toobs( tDICT *self, tOBS *obs ) {
  tDICT *flat = tDICT__new();
  uint32_t selfId = tOBS__rawadd( obs, (tBASE *) flat );
  
  tDICT *type = tDICT__new();
  tDICT__set( type, "$classname", tSTR__new("NSDictionary") );
  tDICT__set( type, "$classes", tARR__newStrs(2,"NSDictionary","NSObject") );
  uint32_t typeId = tOBS__rawadd( obs, (tBASE *) type );
  
  tARR *keys = tARR__new();
  tARR *vals = tARR__new();
  
  tBASE *curKey = self->keyHead;
  tBASE *curVal = self->valHead;
  while( curKey ) {
    uint32_t keyId = tOBS__rawadd( obs, tREF__new( (tBASE *) curKey ) );
    uint32_t valId = tOBS__add( obs, curVal );
    tARR__add( keys, tARCID__new( keyId ) );
    tARR__add( vals, tARCID__new( valId ) );
    curKey = curKey->next;
    curVal = curVal->next;
  }
  
  tDICT__set( flat, "NS.keys", keys );
  tDICT__set( flat, "NS.objects", vals );
  tDICT__set( flat, "$class", tARCID__new( typeId ) );
    
  return selfId;
}

void _tDICT__set( tDICT *self, char *rawKey, tBASE *val ) {
  tBASE *key = (tBASE *) tSTR__new( rawKey );
  
  if( !self->keyHead ) {
    self->keyHead = self->keyTail = key;
    self->valHead = self->valTail = val;
    return;
  }
  if( self->keyHead == self->keyTail ) {
    self->keyHead->next = key;
    self->keyTail = key;
    self->valHead->next = val;
    self->valTail = val;
    return;
  }
  self->keyTail->next = key;
  self->keyTail = key;
  self->valTail->next = val;
  self->valTail = val;
}

void tI32__dump( tI32 *self, uint8_t depth ) {
  printf("%d\n", self->val );
}

void tBOOL__dump( tBOOL *self, uint8_t depth ) {
  if( self->val ) printf("true\n" );
  else            printf("false\n" );
}

void tI32__xml( tI32 *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<integer>%d</integer>\n", self->val );
}

void tBOOL__xml( tBOOL *self, bytearr *ba, uint8_t depth ) {
  if( self->val ) {
    ba__print(ba,"<true/>\n" );
  } else {
    ba__print(ba,"<false/>\n" );
  }
}

void tARCID__dump( tARCID *self, uint8_t depth ) {
  printf("oid.%d\n", self->val );
}

void tARCID__xml( tARCID *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<dict><key>CF$UID</key><integer>%d</integer></dict>\n", self->val );
}

void tSTR__dump( tSTR *self, uint8_t depth ) {
  printf("\"%s\"\n", self->val );
}

void tSTR__xml( tSTR *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<string>%s</string>\n", self->val );
}

void tDICT__del( tDICT *self ) {
  tBASE *curKey = self->keyHead;
  tBASE *curVal = self->valHead;
  while( curKey ) {
    tBASE *nextKey = curKey->next;
    tBASE *nextVal = curVal->next;
    tBASE__del( curKey );
    tBASE__del( curVal );
    curKey = nextKey;
    curVal = nextVal;
  }
}

void tDICT__dump( tDICT *self, uint8_t depth ) {
  char sp[20];
  memset( sp, ' ', 20 );
  sp[depth*2] = 0x00;
  if( depth == 1 ) sp[2] = 0x00;
  
  printf("{\n");
  tBASE *curKey = self->keyHead;
  tBASE *curVal = self->valHead;
  while( curKey ) {
    switch( curKey->type ) {
    case xfSTR:
      printf("%s%s:", sp, ( (tSTR*) curKey )->val );
    }
    tBASE__dump( curVal, depth + 1 );
    curKey = curKey->next;
    curVal = curVal->next;
  }
  printf("%s}\n", &sp[2]);
}

void tDICT__xml( tDICT *self, bytearr *ba, uint8_t depth ) {
  char sp[20];
  memset( sp, ' ', 20 );
  sp[depth*2] = 0x00;
  if( depth == 1 ) sp[2] = 0x00;
  
  ba__print(ba,"<dict>\n");
  tBASE *curKey = self->keyHead;
  tBASE *curVal = self->valHead;
  while( curKey ) {
    switch( curKey->type ) {
    case xfSTR:
      ba__print(ba,"%s<key>%s</key>\n%s", sp, ( (tSTR*) curKey )->val, sp );
    }
    tBASE__xml( curVal, ba, depth + 1 );
    curKey = curKey->next;
    curVal = curVal->next;
  }
  ba__print(ba,"%s</dict>\n", &sp[2]);
}

void tARR__del( tARR *self ) {
  tBASE *cur = self->head;
  while( cur ) {
    tBASE *next = cur->next;
    tBASE__del( cur );
    cur = next;
  }
}

void tOBS__del( tOBS *self ) {
  tBASE *cur = self->head;
  while( cur ) {
    tBASE *next = cur->next;
    tBASE__del( cur );
    cur = next;
  }
}

void tARR__dump( tARR *self, uint8_t depth ) {
  char sp[20];
  memset( sp, ' ', 20 );
  sp[depth*2] = 0x00;
  if( depth == 1 ) sp[2] = 0x00;
  
  printf("[\n");
  tBASE *cur = self->head;
  while( cur ) {
    printf( "%s", sp );
    tBASE__dump( cur, depth + 1 );
    cur = cur->next;
  }
  printf("%s]\n", &sp[2]);
}

void tARR__xml( tARR *self, bytearr *ba, uint8_t depth ) {
  char sp[20];
  memset( sp, ' ', 20 );
  sp[depth*2] = 0x00;
  if( depth == 1 ) sp[2] = 0x00;
  
  ba__print(ba,"<array>\n");
  tBASE *cur = self->head;
  while( cur ) {
    ba__print( ba, "%s", sp );
    tBASE__xml( cur, ba, depth + 1 );
    cur = cur->next;
  }
  ba__print(ba,"%s</array>\n", &sp[2]);
}

void tOBS__dump( tOBS *self, uint8_t depth ) {
  char sp[20];
  memset( sp, ' ', 20 );
  sp[depth*2] = 0x00;
  if( depth == 1 ) sp[2] = 0x00;
  
  printf("[\n");
  tBASE *cur = self->head;
  while( cur ) {
    printf( "%s", sp );
    tBASE__dump( cur, depth + 1 );
    cur = cur->next;
  }
  printf("%s]\n", &sp[2]);
}

void tOBS__xml( tOBS *self, bytearr *ba, uint8_t depth ) {
  char sp[20];
  memset( sp, ' ', 20 );
  sp[depth*2] = 0x00;
  if( depth == 1 ) sp[2] = 0x00;
  
  ba__print(ba,"<array>\n");
  ba__print( ba, "%s<key/>\n", sp );
  tBASE *cur = self->head;
  while( cur ) {
    ba__print( ba, "%s", sp );
    tBASE__xml( cur, ba, depth + 1 );
    cur = cur->next;
  }
  ba__print( ba, "%s</array>\n", &sp[2] );
}

void tBASE__dump( tBASE *ob, uint8_t depth ) {
  switch( ob->type ) {
    case xfI32:   return tI32__dump(    (tI32 *)   ob, depth );
    case xfBOOL:  return tBOOL__dump(   (tBOOL *)  ob, depth );
    case xfSTR:   return tSTR__dump(    (tSTR *)   ob, depth );
    case xfARR:   return tARR__dump(    (tARR *)   ob, depth );
    case xfDICT:  return tDICT__dump(   (tDICT *)  ob, depth );
    case xfARCID: return tARCID__dump(  (tARCID *) ob, depth );
    case xfREF:   return tBASE__dump( ( (tREF *) ob )->val, depth );
  }
}

void tBASE__xml( tBASE *ob, bytearr *ba, uint8_t depth ) {
  switch( ob->type ) {
    case xfI32:   return tI32__xml(   (tI32 *)   ob, ba, depth );
    case xfBOOL:  return tBOOL__xml(  (tBOOL *)  ob, ba, depth );
    case xfSTR:   return tSTR__xml(   (tSTR *)   ob, ba, depth );
    case xfARR:   return tARR__xml(   (tARR *)   ob, ba, depth );
    case xfDICT:  return tDICT__xml(  (tDICT *)  ob, ba, depth );
    case xfARCID: return tARCID__xml( (tARCID *) ob, ba, depth );
    case xfREF:   return tBASE__xml( ( (tREF *) ob )->val, ba, depth );
  }
}

uint32_t _tOBS__add( tOBS *self, tBASE *ob ) {
  switch( ob->type ) {
    case xfI32:  return tI32__toobs(  (tI32 *)  ob, self );
    case xfBOOL: return tBOOL__toobs( (tBOOL *) ob, self );
    case xfSTR:  return tSTR__toobs(  (tSTR *)  ob, self );
    case xfARR:  return tARR__toobs(  (tARR *)  ob, self );
    case xfDICT: return tDICT__toobs( (tDICT *) ob, self );
    
  }
  return 0;
}

void _tBASE__del( tBASE *self ) {
  switch( self->type ) {
    case xfARR:  tARR__del(  (tARR *)  self ); break;
    case xfDICT: tDICT__del( (tDICT *) self ); break;
    case xfOBS:  tOBS__del(  (tOBS *)  self ); break;
  }
  free( self );
}

tBOOL *tBOOL__new( uint8_t val ) {
  tBOOL *self = (tBOOL *) calloc( 1, sizeof( tBOOL ) );
  self->type = xfBOOL;
  self->val = val;
  return self;
}

bytearr *tDICT__asaux( tDICT *self ) {
  bytearr *out = bytearr__new();
  tBASE__archiveToAux( (tBASE *) self, out );
  return out;
}

bytearr *tARR__asaux( tARR *self ) {
  bytearr *out = bytearr__new();
  
  tBASE *val = self->head;
  while( val ) {
    tBASE__toaux( val, out );
    val = val->next;
  }
  
  return out;
}

uint8_t *tBASE__archive( tBASE *self, uint32_t *len ) {
  const char prefix[] =
    "<dict>\n"
    "  <key>$objects</key>\n  ";
  const char suffix[] =
    "  <key>$top</key><dict><key>root</key><dict><key>CF$UID</key><integer>1</integer></dict></dict>\n"
    "  <key>$version</key><integer>100000</integer>\n"
  "</dict>\n";
  
  tOBS *obs = tOBS__new( self );
  
  bytearr *ba = bytearr__new();
  bytearr__append( ba, (uint8_t *) prefix, sizeof( prefix ) - 1, 0 );
  tOBS__xml( obs, ba, 2 );
  bytearr__append( ba, (uint8_t *) suffix, sizeof( suffix ) - 1, 0 );
  //tOBS__del( obs );
  //tBASE__del( self );
  
  uint8_t *bytes = bytearr__bytes( ba, len );
  
  #ifdef DEBUG2
  printf("Made archive:%s", bytes );
  #endif
  // bytearr__del( ba )
  
  return bytes;
}

void tBASE__archiveToAux( tBASE *self, bytearr *out ) {
  uint32_t len = 0;
  uint8_t *bytes = tBASE__archive( self, &len );
  
  bytearr__appendi32( out, 10 ); // empty dict
  bytearr__appendi32( out, 2 ); // cfTypeRef
  
  bytearr__appendi32( out, len );
  bytearr__append( out, bytes, len, 1 );
}

void tBASE__toaux( tBASE *self, bytearr *out ) {
  switch( self->type ) {
    case xfSTR:
    case xfARR:
    case xfDICT:
      tBASE__archiveToAux( self, out );
      break;
    case xfI32:
      tI32__toaux( (tI32 *) self, out );
      break;
  }
}

bytearr *tBASE__asaux( tBASE *self ) {
  switch( self->type ) {
    case xfDICT: return tDICT__asaux( (tDICT *) self );
    case xfARR:  return tARR__asaux( (tARR *) self );
    case xfI32:
    case xfSTR: {
      bytearr *out = bytearr__new();
      tBASE__archiveToAux( self, out );
      return out;
    }    
  }
  return 0;
}

void tI32__toaux( tI32 *self, bytearr *out ) {
  bytearr__appendi32( out, 10 ); // empty dict
  bytearr__appendi32( out, 3 ); // i32
  bytearr__appendi32( out, self->val );
}