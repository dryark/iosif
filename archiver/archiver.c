// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdarg.h>

#include"archiver.h"
#include"../bytearr.h"
#include"plist.h"

tI8 *tI8__new( uint8_t val ) {
  tI8 *self = (tI8 *) calloc( 1, sizeof( tI8 ) );
  self->type = xfI8;
  self->val = val;
  return self;
}

tI16 *tI16__new( uint16_t val ) {
  tI16 *self = (tI16 *) calloc( 1, sizeof( tI16 ) );
  self->type = xfI16;
  self->val = val;
  return self;
}

tI32 *tI32__new( uint32_t val ) {
  tI32 *self = (tI32 *) calloc( 1, sizeof( tI32 ) );
  self->type = xfI32;
  self->val = val;
  return self;
}

tI64 *tI64__new( int64_t val ) {
  tI64 *self = (tI64 *) calloc( 1, sizeof( tI64 ) );
  self->type = xfI64;
  self->val = val;
  return self;
}

tCAPS *tCAPS__new( tDICT *val ) {
  tCAPS *self = (tCAPS *) calloc( 1, sizeof( tCAPS ) );
  self->type = xfCAPS;
  self->dict = val;
  return self;
}

tTESTCONF *tTESTCONF__new( tDICT *val ) {
  tTESTCONF *self = (tTESTCONF *) calloc( 1, sizeof( tTESTCONF ) );
  self->type = xfTESTCONF;
  self->dict = val;
  return self;
}

tF1 *tF1__new( float val ) {
  tF1 *self = (tF1 *) calloc( 1, sizeof( tF1 ) );
  self->type = xfF1;
  self->val = val;
  return self;
}

tF2 *tF2__new( double val ) {
  tF2 *self = (tF2 *) calloc( 1, sizeof( tF2 ) );
  self->type = xfF2;
  self->val = val;
  return self;
}

tDATA *tDATA__new( uint8_t *val, uint32_t len ) {
  tDATA *self = (tDATA *) calloc( 1, sizeof( tDATA ) );
  self->type = xfDATA;
  self->val = val;
  self->len = len;
  return self;
}

tUUID *tUUID__new( char *str ) {
  tUUID *self = (tUUID *) calloc( 1, sizeof( tUUID ) );
  self->type = xfUUID;
  uint8_t byte;
  int j = 0;
  for( int i=0;i<50;i++ ) {
    char let = str[i];
    if( let == '-' ) continue;
    char let2 = str[++i];
    uint8_t num1;
    if( let >= '0' && let <= '9' ) num1 = let - '0';
    if( let >= 'a' && let <= 'f' ) num1 = let - 'a' + 10;
    if( let >= 'A' && let <= 'F' ) num1 = let - 'A' + 10;
    uint8_t num2;
    if( let2 >= '0' && let2 <= '9' ) num2 = let2 - '0';
    if( let2 >= 'a' && let2 <= 'f' ) num2 = let2 - 'a' + 10;
    if( let2 >= 'A' && let2 <= 'F' ) num2 = let2 - 'A' + 10;
    self->val[j] = num1 * 16 + num2;
    if( j == 15 ) break;
    j++;
  }
  return self;
}

tTIME *tTIME__new( double val ) {
  tTIME *self = (tTIME *) calloc( 1, sizeof( tTIME ) );
  self->type = xfTIME;
  self->val = val;
  return self;
}

tARCID *tARCID__new( uint32_t val ) {
  tARCID *self = (tARCID *) calloc( 1, sizeof( tARCID ) );
  self->type = xfARCID;
  self->val = val;
  return self;
}

tSTR *tSTR__new( const char *val ) {
  tSTR *self = (tSTR *) calloc( 1, sizeof( tSTR ) );
  self->type = xfSTR;
  self->val = val;
  return self;
}

tURL *tURL__new( char *relative ) {
  tURL *self = (tURL *) calloc( 1, sizeof( tURL ) );
  self->type = xfURL;
  self->relative = relative;
  return self;
}

tSTR *tSTR__newl( const char *val, int len ) {
  tSTR *self = (tSTR *) calloc( 1, sizeof( tSTR ) );
  self->type = xfSTR;
  char *buffer = (char *) malloc( len + 1 );
  self->val = buffer;
  sprintf( buffer, "%.*s", len, val );
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

tOFFS *tOFFS__new() {
  tOFFS *self = (tOFFS *) calloc( 1, sizeof( tOFFS ) );
  return self;
}

tOFF *tOFF__new( uint16_t off ) {
  tOFF *self = (tOFF *) calloc( 1, sizeof( tOFF ) );
  self->offset = off;
  return self;
}

uint16_t tOFFS__add( tOFFS *self, uint16_t off ) {
  tOFF *item = tOFF__new( off );
  
  //printf("Call to tOFFS__add\n");
  
  if( !self->head ) {
    self->head = self->tail = item;
    return self->len++;
  }
  self->tail->next = item;
  self->tail = item;
  return self->len++;
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
  self->count++;
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
    //if( self->head == self->tail ) {
    //  self->head->next = ob;
    //  self->tail = ob;
    //  break;
    //}
    self->tail->next = ob;
    self->tail = ob;
  } while( 0 );
  
  return self->num;
}

uint32_t tSTR__toobs( tSTR *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tNULL__toobs( tNULL *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tDATA__toobs( tDATA *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tI8__toobs( tI8 *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tI16__toobs( tI16 *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tI32__toobs( tI32 *self, tOBS *obs ) {
  uint32_t selfId = tOBS__rawadd( obs, tREF__new( (tBASE *) self ) );
  return selfId;
}

uint32_t tI64__toobs( tI64 *self, tOBS *obs ) {
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
  tDICT__set( type, "$classes", tARR__newStrs( 2, "NSArray", "NSOjbect" ) );
  tDICT__set( type, "$classname", tSTR__new("NSArray") );
  uint32_t typeId = tOBS__rawadd( obs, (tBASE *) type );
  
  tARR *obsAsIds = tARR__new();
  tBASE *curVal = self->head;
  while( curVal ) {
    uint32_t obId = tOBS__add( obs, curVal );
    tARR__add( obsAsIds, tARCID__new( obId ) );
    curVal = curVal->next;
  }
  
  tDICT__set( flat, "$class", tARCID__new( typeId ) );
  tDICT__set( flat, "NS.objects", obsAsIds );
    
  return selfId;
}

uint32_t tUUID__toobs( tUUID *self, tOBS *obs ) {
  tDICT *flat = tDICT__new();
  uint32_t selfId = tOBS__rawadd( obs, (tBASE *) flat );
  
  tDICT *type = tDICT__new();
  tDICT__set( type, "$classes", tARR__newStrs(2,"NSUUID","NSObject") );
  tDICT__set( type, "$classname", tSTR__new("NSUUID") );
  uint32_t typeId = tOBS__rawadd( obs, (tBASE *) type );
  
  tDICT__set( flat, "$class", tARCID__new( typeId ) );
  
  //int dataId = tOBS__rawadd( obs, (tBASE *) tDATA__new( self->val, 16 ) );
  //tBASE *dataArc = (tBASE *) tARCID__new( dataId );
  //tDICT__set( flat, "NS.uuidbytes", dataArc );
  tDICT__set( flat, "NS.uuidbytes", tDATA__new( self->val, 16 ) );
      
  return selfId;
}

uint32_t tDICT__toobs( tDICT *self, tOBS *obs ) {
  tDICT *flat = tDICT__new();
  uint32_t selfId = tOBS__rawadd( obs, (tBASE *) flat );
  
  tDICT *type = tDICT__new();
  tDICT__set( type, "$classes", tARR__newStrs(2,"NSDictionary","NSObject") );
  tDICT__set( type, "$classname", tSTR__new("NSDictionary") );
  uint32_t typeId = tOBS__rawadd( obs, (tBASE *) type );
  
  tARR *keys = tARR__new();
  tARR *vals = tARR__new();
  
  tBASE *curKey = self->keyHead;
  tBASE *curVal = self->valHead;
  while( curKey ) {
    
    uint32_t keyId = tOBS__rawadd( obs, tREF__new( (tBASE *) curKey ) );
    tARR__add( keys, tARCID__new( keyId ) );
    //tARR__add( keys, tREF__new( (tBASE *) curKey ) );
    
    char vt = curVal->type;
    //if( vt == xfREF ) vt = ( (tREF*) curVal )->val->type;
    if( vt == xfBOOL && vt == xfI8 ) {
      tARR__add( vals, curVal );
    }
    else {
      uint32_t valId = tOBS__add( obs, curVal );
      tARR__add( vals, tARCID__new( valId ) );
    }
    
    curKey = curKey->next;
    curVal = curVal->next;
  }
  
  tDICT__set( flat, "$class", tARCID__new( typeId ) );
  tDICT__set( flat, "NS.keys", keys );
  tDICT__set( flat, "NS.objects", vals );
      
  return selfId;
}

uint32_t tTESTCONF__toobs( tTESTCONF *self1, tOBS *obs ) {
  tDICT *self = self1->dict;
  
  tDICT *flat = tDICT__new();
  uint32_t selfId = tOBS__rawadd( obs, (tBASE *) flat );
  
  tDICT *type = tDICT__new();
  tDICT__set( type, "$classes", tARR__newStrs(2,"XCTestConfiguration","NSObject") );
  tDICT__set( type, "$classname", tSTR__new("XCTestConfiguration") );
  uint32_t typeId = tOBS__rawadd( obs, (tBASE *) type );
  
  tDICT__set( flat, "$class", tARCID__new( typeId ) );
  
  //tARR *keys = tARR__new();
  //tARR *vals = tARR__new();
  
  char *asarc[] = {
    "aggregateStatisticsBeforeCrash",
    "automationFrameworkPath",
    "productModuleName",
    "sessionIdentifier",
		"targetApplicationBundleID",
		"targetApplicationPath",
		"testBundleURL"
	};
  
  tBASE *curKey = self->keyHead;
  tBASE *curVal = self->valHead;
  while( curKey ) {
    uint32_t keyId = tOBS__rawadd( obs, tREF__new( (tBASE *) curKey ) );
    
    //tARR__add( keys, tARCID__new( keyId ) );
    //tARR__add( vals, tARCID__new( valId ) );
    tSTR *strOb = (tSTR *) curKey;
    
    /*char doarc = 0;
    for( int i=0;i<7;i++ ) {
      if( !strcmp( strOb->val, asarc[i] ) ) {
        doarc = 1;
        break;
      }
    }*/
    
    //if( doarc ) {
    
    char vt = curVal->type;
    if( vt == xfREF ) vt = ( (tREF*) curVal )->val->type;
    //printf("Type:%i\n", vt );
    if( vt == xfBOOL || vt == xfI8 ) {
      //printf("  asis\n");
      tDICT__set( flat, strOb->val, tREF__new( curVal ) );
    }
    else {
      uint32_t valId = tOBS__add( obs, curVal );
      tDICT__set( flat, strOb->val, tARCID__new( valId ) );
    }
    //}
    //else tDICT__set( flat, strOb->val, curVal );
    
    curKey = curKey->next;
    curVal = curVal->next;
  }
  
  //tDICT__set( flat, "NS.keys", keys );
  //tDICT__set( flat, "NS.objects", vals );
      
  return selfId;
}

uint32_t tCAPS__toobs( tCAPS *self, tOBS *obs ) {
  uint32_t dictId = tDICT__toobs( self->dict, obs );
  
  tDICT *flat = tDICT__new();
  uint32_t selfId = tOBS__rawadd( obs, (tBASE *) flat );
  
  tDICT *type = tDICT__new();
  tDICT__set( type, "$classname", tSTR__new("XCTCapabilities") );
  tDICT__set( type, "$classes", tARR__newStrs(2,"XCTCapabilities","NSObject") );
  uint32_t typeId = tOBS__rawadd( obs, (tBASE *) type );
  
  tDICT__set( flat, "capabilities-dictionary", tARCID__new( dictId ) );
  tDICT__set( flat, "$class", tARCID__new( typeId ) );
    
  return selfId;
}

uint32_t tURL__toobs( tURL *self, tOBS *obs ) {
  tDICT *flat = tDICT__new();
  uint32_t selfId = tOBS__rawadd( obs, (tBASE *) flat );
  
  tDICT *type = tDICT__new();
  tDICT__set( type, "$classes", tARR__newStrs(2,"NSURL","NSObject") );
  tDICT__set( type, "$classname", tSTR__new("NSURL") );
  uint32_t typeId = tOBS__rawadd( obs, (tBASE *) type );
  
  tDICT__set( flat, "$class", tARCID__new( typeId ) );
  
  int baseId = tSTR__toobs( tSTR__new("$null"), obs );
  tBASE *baseArc = (tBASE *) tARCID__new( baseId );
  tDICT__set( flat, "NS.base", baseArc );//tNULL__new() );
  
  int relId = tSTR__toobs( tSTR__new( self->relative ), obs );
  tBASE *relArc = (tBASE *) tARCID__new( relId );
  tDICT__set( flat, "NS.relative", relArc );
      
  return selfId;
}

void _tDICT__set( tDICT *self, const char *rawKey, tBASE *val ) {
  tBASE *key = (tBASE *) tSTR__new( rawKey );
  _tDICT__seto( self, key, val );
}

void _tDICT__seto( tDICT *self, tBASE *key, tBASE *val ) {
  if( !self->keyHead ) {
    self->keyHead = self->keyTail = key;
    self->valHead = self->valTail = val;
    self->count++;
    return;
  }
  if( self->keyHead == self->keyTail ) {
    self->keyHead->next = key;
    self->keyTail = key;
    self->valHead->next = val;
    self->valTail = val;
    self->count++;
    return;
  }
  self->keyTail->next = key;
  self->keyTail = key;
  self->valTail->next = val;
  self->valTail = val;
  self->count++;
}

tBASE *tDICT__get( tDICT *self, const char *key ) {
  tSTR *cur = (tSTR *) self->keyHead;
  tBASE *curVal = self->valHead;
  while( cur ) {
    //printf("Find type:%d\n", cur->type );
    if( !strcmp( cur->val, key ) ) {
      //printf("Returning\n");
      return curVal;
    }
    cur = (tSTR *) cur->next;
    curVal = curVal->next;
  }
  //printf("%s not found\n", key );
  return 0;
}

uint32_t tI__val32( tBASE *self ) {
  uint32_t val;
  switch( self->type ) {
  case xfI8: val=( ( (tI8*) self ) )->val; break;
  case xfI16: val=( ( (tI16*) self ) )->val; break;
  case xfI32: val=( ( (tI32*) self ) )->val; break;
  }
  return val;
}

int64_t tI__val64( tBASE *self ) {
  int64_t val;
  switch( self->type ) {
  case xfI8: val=( ( (tI8*) self ) )->val; break;
  case xfI16: val=( ( (tI16*) self ) )->val; break;
  case xfI32: val=( ( (tI32*) self ) )->val; break;
  case xfI64: val=( ( (tI64*) self ) )->val; break;
  }
  return val;
}

char tIsnum( tBASE *self ) {
  switch( self->type ) {
  case xfI8: case xfI16: case xfI32: case xfI64:
    return 1;
  }
  return 0;
}

void tI8__dump( tI8 *self, uint8_t depth ) {
  printf("i8.%u", self->val );
}
void tI16__dump( tI16 *self, uint8_t depth ) {
  printf("i16.%u", self->val );
}
void tI32__dump( tI32 *self, uint8_t depth ) {
  printf("i32.%" PRIu32, self->val );
}
void tI64__dump( tI64 *self, uint8_t depth ) {
  printf("i64.%" PRId64, self->val );
}
void tF1__dump( tF1 *self, uint8_t depth ) {
  printf("%.2f", self->val );
}
void tF2__dump( tF2 *self, uint8_t depth ) {
  printf("%.2f", self->val );
}
void tTIME__dump( tTIME *self, uint8_t depth ) {
  printf("%.2f", self->val );
}
void tDATA__dump( tDATA *self, uint8_t depth ) {
  printf("x.");
  for( int i=0;i<self->len;i++ ) {
    printf("%02x", self->val[i] );
  }
}
void tUUID__dump( tUUID *self, uint8_t depth ) {
  printf("UUID.");
  for( int i=0;i<16;i++ ) {
    printf("%02x", self->val[i] );
  }
}

void tBOOL__dump( tBOOL *self, uint8_t depth ) {
  if( self->val ) printf("true" );
  else            printf("false" );
}

void tI8__xml( tI8 *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<integer>%d</integer>\n", self->val );
}

void tI16__xml( tI16 *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<integer>%d</integer>\n", self->val );
}

void tI32__xml( tI32 *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<integer>%d</integer>\n", self->val );
}

void tI64__xml( tI64 *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<integer>%" PRId64 "</integer>\n", self->val );
}

void tBOOL__xml( tBOOL *self, bytearr *ba, uint8_t depth ) {
  //if( self->val ) {
  //  ba__print(ba,"1\n" );
  //} else {
  //  ba__print(ba,"0\n" );
  //}
  
  if( self->val ) {
    ba__print(ba,"<true/>\n" );
  }
  else {
    ba__print(ba,"<false/>\n" );
  }
}

void tARCID__dump( tARCID *self, uint8_t depth ) {
  printf("oid.%d", self->val );
}

void tARCID__xml( tARCID *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<dict><key>CF$UID</key><integer>%d</integer></dict>\n", self->val );
}

void tSTR__dump( tSTR *self, uint8_t depth ) {
  printf("\"%s\"", self->val );
}

void tNULL__dump( tNULL *self, uint8_t depth ) {
  printf("NULL" );
}

void tSTR__xml( tSTR *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<string>%s</string>\n", self->val );
}

void tNULL__xml( tNULL *self, bytearr *ba, uint8_t depth ) {
  ba__print(ba,"<string>$null</string>\n" );
}

char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};


/** Lookup table that converts a integer to base64 digit. */
static char const bintodigit[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789"
                                 "+/";

/** Get the first base 64 digit of a block of 4.
  * @param a The first byte of the source block of 3.
  * @return A base 64 digit. */
static int get0( int a ) {
    int const index = a >> 2u;
    return bintodigit[ index ];
}

/** Get the second base 64 digit of a block of 4.
  * @param a The first byte of the source block of 3.
  * @param b The second byte of the source block of 3.
  * @return A base 64 digit. */
static int get1( int a, int b ) {
    int const indexA = ( a & 3 ) << 4u;
    int const indexB = b >> 4u;
    int const index  = indexA | indexB;
    return bintodigit[ index ];
}

/** Get the third base 64 digit of a block of 4.
  * @param b The second byte of the source block of 3.
  * @param c The third byte of the source block of 3.
  * @return A base 64 digit. */
static unsigned int get2( unsigned int b, unsigned int c ) {
    int const indexB = ( b & 15 ) << 2u;
    int const indexC = c >> 6u;
    int const index  = indexB | indexC;
    return bintodigit[ index ];
}

/** Get the fourth base 64 digit of a block of 4.
  * @param c The third byte of the source block of 3.
  * @return A base 64 digit. */
static int get3( int c ) {
    int const index = c & 0x3f;
    return bintodigit[ index ];
}

/* Convert a binary memory block in a base64 null-terminated string. */
char* bintob64( char* dest, void const* src, size_t size ) {

    typedef struct { unsigned char a; unsigned char b; unsigned char c; } block_t;
    block_t const* block = (block_t*)src;
    for( ; size >= sizeof( block_t ); size -= sizeof( block_t ), ++block ) {
        *dest++ = get0( block->a );
        *dest++ = get1( block->a, block->b );
        *dest++ = get2( block->b, block->c );
        *dest++ = get3( block->c );
    }

    if ( !size ) goto final;

    *dest++ = get0( block->a );
    if ( !--size ) {
        *dest++ = get1( block->a, 0 );
        *dest++ = '=';
        *dest++ = '=';
        goto final;
    }

    *dest++ = get1( block->a, block->b );
    *dest++ = get2( block->b, 0 );
    *dest++ = '=';

  final:
    *dest = '\0';
    return dest;
}

void tDATA__xml( tDATA *self, bytearr *ba, uint8_t depth ) {
  char dest[50];
  bintob64( dest, &self->val[0], 16 );
  ba__print(ba,"<data>%s</data>\n", dest );
  //free( b64 );
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

void tDICT__dump( tDICT *self, uint8_t depth );

void tCAPS__dump( tCAPS *self, uint8_t depth ) {
  printf("$class:caps - vals:");
  tDICT__dump( self->dict, depth );
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
    tBASE *unrefKey = curKey;
    while( unrefKey->type == xfREF ) unrefKey = ( (tREF *) unrefKey)->val;
    switch( unrefKey->type ) {
      case xfI8:
        printf("%s%d:", sp, ( (tI8*) unrefKey )->val );
        break;
      case xfI16:
        printf("%s%d:", sp, ( (tI16*) unrefKey )->val );
        break;
      case xfI32:
        printf("%s%" PRIu32 ":", sp, ( (tI32*) unrefKey )->val );
        break;
      case xfI64:
        printf("%s%" PRId64 ":", sp, ( (tI64*) unrefKey )->val );
        break;
      case xfSTR:
        printf("%s%s:", sp, ( (tSTR*) unrefKey )->val );
        break;
      case xfARCID:
        printf("%soid.%d:", sp, ( (tARCID*) unrefKey )->val );
        break;
      default:
        printf("Other key type:%d\n", unrefKey->type );
        break;
    }
    tBASE__dump( curVal, depth + 1 );
    curKey = curKey->next;
    curVal = curVal->next;
  }
  printf("%s}", &sp[2]);
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
  self->count--;
}

void **tARR__flatten( tARR *self ) {
  tBASE *cur = self->head;
  uint32_t pos = 0;
  void **res = ( void ** ) malloc( sizeof( void * ) * self->count );
  while( cur ) {
    res[ pos++ ] = (void *) cur;
    cur = cur->next;
  }
  return res;
}

tBASE *tARR__get( tARR *self, uint32_t pos ) {
  tBASE *cur = self->head;
  while( pos ) {
    if( !cur ) return 0;
    cur = cur->next;
    pos--;
  }
  return cur;
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
  
  //printf("Count:%d\n", self->count );
  printf("[\n");
  tBASE *cur = self->head;
  while( cur ) {
    printf( "%s", sp );
    tBASE__dump( cur, depth + 1 );
    cur = cur->next;
  }
  printf("%s]", &sp[2]);
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
  //ba__print( ba, "%s<key/>\n", sp );
  ba__print( ba, "%s<string>$null</string>\n", sp );
  tBASE *cur = self->head;
  while( cur ) {
    ba__print( ba, "%s", sp );
    tBASE__xml( cur, ba, depth + 1 );
    cur = cur->next;
  }
  ba__print( ba, "%s</array>\n", &sp[2] );
}

void tBASE__dump( tBASE *ob, uint8_t depth ) {
  tBASE__dump_nocr( ob, depth );
  printf("\n");
}

void tBASE__dump_nocr( tBASE *ob, uint8_t depth ) {
  switch( ob->type ) {
    case xfI8:    return tI8__dump(     (tI8 *)    ob, depth );
    case xfI16:   return tI16__dump(    (tI16 *)   ob, depth );
    case xfI32:   return tI32__dump(    (tI32 *)   ob, depth );
    case xfI64:   return tI64__dump(    (tI64 *)   ob, depth );
    case xfBOOL:  return tBOOL__dump(   (tBOOL *)  ob, depth );
    case xfSTR:   return tSTR__dump(    (tSTR *)   ob, depth );
    case xfARR:   return tARR__dump(    (tARR *)   ob, depth );
    case xfDICT:  return tDICT__dump(   (tDICT *)  ob, depth );
    case xfARCID: return tARCID__dump(  (tARCID *) ob, depth );
    case xfREF:   return tBASE__dump_nocr( ( (tREF *) ob )->val, depth );
    case xfTIME:  return tTIME__dump(   (tTIME *)  ob, depth );
    case xfF1:    return tF1__dump(     (tF1 *)    ob, depth );
    case xfF2:    return tF2__dump(     (tF2 *)    ob, depth );
    case xfDATA:  return tDATA__dump(   (tDATA *)  ob, depth );
    case xfUUID:  return tUUID__dump(   (tUUID *)  ob, depth );
    case xfNULL:  return tNULL__dump(   (tNULL *)  ob, depth );
    case xfCAPS:  return tCAPS__dump(   (tCAPS *)  ob, depth );
    case xfTESTCONF: return tDICT__dump( ( (tTESTCONF *) ob )->dict, depth );
  }
  printf("Unkown type:%i\n", ob->type);
}

void tBASE__xml( tBASE *ob, bytearr *ba, uint8_t depth ) {
  switch( ob->type ) {
    case xfI8:    return tI8__xml(    (tI8 *)    ob, ba, depth );
    case xfI16:   return tI16__xml(   (tI16 *)   ob, ba, depth );
    case xfI32:   return tI32__xml(   (tI32 *)   ob, ba, depth );
    case xfI64:   return tI64__xml(   (tI64 *)   ob, ba, depth );
    case xfBOOL:  return tBOOL__xml(  (tBOOL *)  ob, ba, depth );
    case xfSTR:   return tSTR__xml(   (tSTR *)   ob, ba, depth );
    case xfARR:   return tARR__xml(   (tARR *)   ob, ba, depth );
    case xfDICT:  return tDICT__xml(  (tDICT *)  ob, ba, depth );
    case xfARCID: return tARCID__xml( (tARCID *) ob, ba, depth );
    case xfREF:   return tBASE__xml( ( (tREF *) ob )->val, ba, depth );
    case xfDATA:  return tDATA__xml(  (tDATA *)  ob, ba, depth );
    case xfNULL:  return tNULL__xml(  (tNULL *)  ob, ba, depth );
  }
}

uint32_t _tOBS__add( tOBS *self, tBASE *ob ) {
  switch( ob->type ) {
    case xfI8:   return tI8__toobs(  (tI8 *)  ob, self );
    case xfI16:  return tI16__toobs(  (tI16 *)  ob, self );
    case xfI32:  return tI32__toobs(  (tI32 *)  ob, self );
    case xfI64:  return tI64__toobs(  (tI64 *)  ob, self );
    case xfBOOL: return tBOOL__toobs( (tBOOL *) ob, self );
    case xfSTR:  return tSTR__toobs(  (tSTR *)  ob, self );
    case xfARR:  return tARR__toobs(  (tARR *)  ob, self );
    case xfDICT: return tDICT__toobs( (tDICT *) ob, self );
    case xfUUID: return tUUID__toobs( (tUUID *) ob, self );
    case xfCAPS: return tCAPS__toobs( (tCAPS *) ob, self );
    case xfTESTCONF:return tTESTCONF__toobs( (tTESTCONF *) ob, self );
    case xfURL:  return tURL__toobs(  (tURL *)  ob, self );
    case xfNULL: return tNULL__toobs( (tNULL *) ob, self );
    //case xfDATA: return tDATA__toobs( (tDATA *) ob, self );
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

tSTR *tSTR__dup( tSTR *self ) {
  return tSTR__new( strdup( self->val ) );
}

tDATA *tDATA__dup( tDATA *self ) {
  if( !self->len ) return tDATA__new( NULL, 0 );
  void *copy = malloc( self->len );
  memcpy( copy, self->val, self->len );
  return tDATA__new( copy, self->len );
}

tARR *tARR__dup( tARR *self ) {
  tARR *dup = tARR__new();
  
  tBASE *cur = self->head;
  while( cur ) {
    tARR__add( dup, tBASE__dup( cur ) );
    cur = cur->next;
  }
  return dup;
}

tDICT *tDICT__dup( tDICT *self ) {
  tDICT *dup = tDICT__new();
  
  printf("Orig size:%d\n", self->count );
  tBASE *curKey = self->keyHead;
  tBASE *curVal = self->valHead;
  while( curKey ) {
    tDICT__seto( dup, tBASE__dup( curKey ), tBASE__dup( curVal ) );
    curKey = curKey->next;
    curVal = curVal->next;
  }
  printf("Dup size:%d\n", dup->count );
  return dup;
}

tREF *tREF__dup( tREF *self ) {
  return (tREF *) tREF__new( self->val );
}

tBASE *_tBASE__dup( tBASE *self ) {
  switch( self->type ) {
    case xfSTR:  return (tBASE *) tSTR__dup(  (tSTR  *) self );
    case xfDATA: return (tBASE *) tDATA__dup( (tDATA *) self );
    case xfARR:  return (tBASE *) tARR__dup(  (tARR  *) self );
    case xfDICT: return (tBASE *) tDICT__dup( (tDICT *) self );
    case xfREF:  return (tBASE *) tREF__dup(  (tREF  *) self );
  }
  uint32_t size;
  switch( self->type ) {
    case xfI8:    size = sizeof( tI8    ); break;
    case xfI16:   size = sizeof( tI16   ); break;
    case xfI32:   size = sizeof( tI32   ); break;
    case xfI64:   size = sizeof( tI64   ); break;
    case xfF1:    size = sizeof( tF1    ); break;
    case xfF2:    size = sizeof( tF2    ); break;
    case xfTIME:  size = sizeof( tTIME  ); break;
    case xfARCID: size = sizeof( tARCID ); break;
    default:
      printf("Cannot duplicate type %d\n", self->type );
      break;
  }
  tBASE *dup = malloc( size );
  memcpy( dup, (void *) self, size );
  dup->next = 0;  
  return (tBASE*) dup;
}

tBOOL *tBOOL__new( uint8_t val ) {
  tBOOL *self = (tBOOL *) calloc( 1, sizeof( tBOOL ) );
  self->type = xfBOOL;
  self->val = val;
  return self;
}

tNULL *tNULL__new() {
  tNULL *self = (tNULL *) calloc( 1, sizeof( tNULL ) );
  self->type = xfNULL;
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
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
    "<plist version=\"1.0\">"
    "<dict>\n"
    "  <key>$objects</key>\n  ";
  const char suffix[] =
    "  <key>$top</key><dict><key>root</key><dict><key>CF$UID</key><integer>1</integer></dict></dict>\n"
    "  <key>$version</key><integer>100000</integer>\n"
    "  <key>$archiver</key><string>NSKeyedArchiver</string>\n"
  "</dict></plist>\n";
  
  tOBS *obs = tOBS__new( self );
  
  bytearr *ba = bytearr__new();
  bytearr__append( ba, (uint8_t *) prefix, sizeof( prefix ) - 1, 0 );
  tOBS__xml( obs, ba, 2 );
  bytearr__append( ba, (uint8_t *) suffix, sizeof( suffix ) - 1, 0 );
  //tOBS__del( obs );
  //tBASE__del( self );
  
  uint8_t *bytes = bytearr__bytes( ba, len );
    
  #ifdef DEBUG2
  printf("Made archive:\n%.*s\n", *len, bytes );
  #endif
  // bytearr__del( ba )
  
  return bytes;
}

void tBASE__dumpxml( tBASE *self ) {
  bytearr *ba = bytearr__new();
  uint32_t len;
  tBASE__xml( self, ba, 1 );
  uint8_t *bytes = bytearr__bytes( ba, &len );
  fwrite( bytes, len, 1, stdout ); 
}

void tBASE__archiveToAux( tBASE *self, bytearr *out ) {
  uint32_t len = 0;
  //uint8_t *bytes = tBASE__archive( self, &len );
  uint8_t *bytes = tBASE__archivebin( self, &len );
  
  bytearr__appendi32( out, 10 ); // empty dict
  bytearr__appendi32( out, 2 ); // cfTypeRef
  
  bytearr__appendi32( out, len );
  bytearr__append( out, bytes, len, 1 );
}

void tBASE__toaux( tBASE *self, bytearr *out ) {
  switch( self->type ) {
    case xfI8:
    case xfSTR:
    case xfARR:
    case xfDICT:
    case xfCAPS:
    case xfTESTCONF:
    case xfURL:
    case xfI16:
    case xfI64:
    case xfNULL:
    case xfUUID:
      tBASE__archiveToAux( self, out );
      break;
    case xfDATA:
      tDATA__toaux( (tDATA*) self, out );
    //case xfI8:
    //  tI8__toaux( (tI8 *) self, out );
    //  break;
    //case xfI16:
    //  tI16__toaux( (tI16 *) self, out );
    //  break;
    case xfI32:
      tI32__toaux( (tI32 *) self, out );
      break;
  }
}

bytearr *tBASE__asaux( tBASE *self ) {
  switch( self->type ) {
    case xfDICT: return tDICT__asaux( (tDICT *) self );
    case xfARR:  return tARR__asaux( (tARR *) self );
    case xfI8:
    case xfI16:
    case xfI32:
    case xfI64:
    case xfUUID:
    case xfCAPS:
    case xfTESTCONF:
    case xfURL:
    case xfNULL:
    case xfDATA:
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

void tI16__toaux( tI16 *self, bytearr *out ) {
  bytearr__appendi32( out, 10 ); // empty dict
  bytearr__appendi32( out, 3 ); // i32
  bytearr__appendi32( out, self->val );
}

void tI8__toaux( tI8 *self, bytearr *out ) {
  bytearr__appendi32( out, 10 ); // empty dict
  bytearr__appendi32( out, 3 ); // i32
  bytearr__appendi32( out, self->val );
}

void tDATA__toaux( tDATA *self, bytearr *out ) {
  bytearr__appendi32( out, 10 ); // empty dict
  bytearr__appendi32( out, 2 ); // data
  bytearr__appendi32( out, self->len );
  if( self->len ) bytearr__appdup( out, self->val, self->len );
}