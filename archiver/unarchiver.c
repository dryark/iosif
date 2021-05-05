// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdint.h>
#include"unarchiver.h"
#include"byteswap.h"

#include"ptrarr.h"

//#define UDEBUG

typedef struct {
  uint32_t obCount;
  tBASE **obs;
} tARCHIVE;

tARCHIVE *tARCHIVE__new( uint32_t obCount, tBASE **obs ) {
  tARCHIVE *self = (tARCHIVE *) malloc( sizeof( tARCHIVE ) );
  self->obCount = obCount;
  self->obs = obs;
  return self;
}

uint8_t willExpand( tBASE *self ) {
  if( self->type == xfREF ) self = ( (tREF *) self )->val;
  switch( self->type ) {
    case xfDICT: return 1;
    case xfARR: return 1;
  }
  return 0;
}

tBASE *tARCHIVE__expand( tARCHIVE *self, tBASE *ob );

tDICT *expandDict( tARCHIVE *self, tDICT *ob ) {
  if( !ob->keyHead ) return ob;
  
  tARR *keys = (tARR *) tDICT__get( (tDICT *) ob, "NS.keys" );
  tARR *vals = (tARR *) tDICT__get( (tDICT *) ob, "NS.objects" );
  tDICT *dict = tDICT__new();
  uint32_t keyCount = keys->count;
  
  duprintf("  keycount=%d\n", keyCount);
  for( int i=0;i<keyCount;i++ ) {
    tARCID *keyRef = (tARCID *) tARR__get( keys, i );
    tARCID *valRef = (tARCID *) tARR__get( vals, i );
    if( keyRef->type == xfREF ) keyRef = (tARCID *) ( (tREF *) keyRef )->val;
    if( valRef->type == xfREF ) valRef = (tARCID *) ( (tREF *) valRef )->val;
    //duprintf("  key otype: %d, val otype: %d\n", keyRef->type, valRef->type );
    tBASE *key = self->obs[ keyRef->val ];
    tBASE *val = self->obs[ valRef->val ];
    //duprintf("  key type: %d, val type: %d\n", key->type, val->type );
    if( willExpand( val ) ) {
      duprintf("Subexpand\n");
      val = tARCHIVE__expand( self, val );
    }
    tDICT__seto( dict, tBASE__dup( key ), tREF__new( val ) );
  }
  return dict;
}

tCAPS *expandCaps( tARCHIVE *self, tDICT *ob ) {
  //if( !ob->keyHead ) return ob;
  
  tBASE *capDict = tDICT__get( (tDICT *) ob, "capabilities-dictionary" );
  if( capDict->type == xfREF ) capDict = (tBASE *) ( (tREF *) capDict )->val;
  if( capDict->type == xfARCID ) capDict = self->obs[ (( tARCID * ) capDict )->val ];
  
  tDICT *capDictE = (tDICT *) expandDict( self, (tDICT *)  capDict );
  tCAPS *caps = tCAPS__new( capDictE );
  
  return caps;
}

tDICT *expandTestConf( tARCHIVE *self, tDICT *ob ) {
  tDICT *dict = tDICT__new();
  
  duprintf("  keycount=%d\n", ob->count);
  
  tBASE *curKey = ob->keyHead;
  for( int i=0; i<ob->count; i++ ) {
    tBASE *key = curKey;
    
    if( key->type == xfREF ) key = (tBASE *) ( (tREF *) key )->val;
   
    //if( key->type == xfARCID ) {
    //   tARCID *keyRef = (tARCID *) ( (tREF *) key )->val;
    //   key = self->obs[ keyRef->val ];
    //}
    
    tSTR *keyStr = (tSTR *) key;
    tBASE *val = tDICT__get( ob, keyStr->val );
    
    if( val->type == xfREF ) val = (tBASE *) ( (tREF *) val )->val;
    
    //printf("val type: %i\n", val->type );
    if( val->type == xfARCID ) {
      tARCID *valRef = (tARCID *) val;
      val = self->obs[ valRef->val ];
      
      //printf("val? %i\n", val ? 1 : 0 );
      //printf("new val type: %i\n", val->type );
    }
            
    if( strcmp( keyStr->val, "$class" ) && willExpand( val ) ) {
      duprintf("Subexpand\n");
      val = tARCHIVE__expand( self, val );
    }
    tDICT__seto( dict, tBASE__dup( key ), tREF__new( val ) );
    
    curKey = curKey->next;
    if( !curKey ) break;
  }
  return dict;
}

tBASE *expandUUID( tARCHIVE *self, tDICT *ob ) {
  tDICT *dict = tDICT__new();
  
  tARCID *classArcId = (tARCID *) tDICT__get( (tDICT *) ob, "$class" );
  if( classArcId->type == xfREF ) classArcId = (tARCID *) ( (tREF *) classArcId )->val;
  tDICT *classDict = (tDICT *) self->obs[ classArcId->val ];
  if( classDict->type == xfREF ) classDict = (tDICT *) ( (tREF *) classDict )->val;
  tSTR *classNameT = (tSTR *) tDICT__get( classDict, "$classname" );
  tDICT__set( dict, "$class", classNameT );
  
  tBASE *base = tDICT__get( ob, "NS.uuidbytes" );
  if( base->type == xfREF ) base = ( (tREF *) base )->val;
  //if( base->type == xfARCID ) base = self->obs[ (( tARCID * ) base )->val ];
  tDICT__set( dict, "NS.uuidbytes", tREF__new( base ) );
      
  return (tBASE *) dict;
}

tBASE *expandURL( tARCHIVE *self, tDICT *ob ) {
  tDICT *dict = tDICT__new();
  
  tARCID *classArcId = (tARCID *) tDICT__get( (tDICT *) ob, "$class" );
  if( classArcId->type == xfREF ) classArcId = (tARCID *) ( (tREF *) classArcId )->val;
  tDICT *classDict = (tDICT *) self->obs[ classArcId->val ];
  if( classDict->type == xfREF ) classDict = (tDICT *) ( (tREF *) classDict )->val;
  tSTR *classNameT = (tSTR *) tDICT__get( classDict, "$classname" );
  tDICT__set( dict, "$class", classNameT );
  
  tBASE *base = tDICT__get( ob, "NS.base" );
  if( base->type == xfREF ) base = ( (tREF *) base )->val;
  if( base->type == xfARCID ) base = self->obs[ (( tARCID * ) base )->val ];
  tDICT__set( dict, "NS.base", tREF__new( base ) );
  
  tBASE *rel = tDICT__get( ob, "NS.relative" );
  if( rel->type == xfREF ) rel = ( (tREF *) rel )->val;
  if( rel->type == xfARCID ) rel = self->obs[ (( tARCID * ) rel )->val ];
  tDICT__set( dict, "NS.relative", tREF__new( rel ) );
    
  return (tBASE *) dict;
}

tBASE *tARCHIVE__expand( tARCHIVE *self, tBASE *ob ) {
  if( ob->type == xfREF ) ob = ( (tREF *) ob )->val;
  
  if( ob->type == xfDICT ) {
    duprintf("Expanding dict/array\n");
    tARCID *classArcId = (tARCID *) tDICT__get( (tDICT *) ob, "$class" );
    //printf("  arcid type:%d\n", classArcId->type );
    if( classArcId->type == xfREF ) classArcId = (tARCID *) ( (tREF *) classArcId )->val;
    //printf("  arcid:%d\n", classArcId->val );
    tDICT *classDict = (tDICT *) self->obs[ classArcId->val ];
    
    if( classDict->type == xfREF ) classDict = (tDICT *) ( (tREF *) classDict )->val;
    
    tSTR *classNameT = (tSTR *) tDICT__get( classDict, "$classname" );
    if( !classNameT ) {
      printf("Could not find classname in dict\n");
      tBASE__dump( (tBASE *) classDict, 1 );
    }
    
    //printf("ClassnameT type:%d\n", classNameT->type );
    const char *class;
    if( classNameT->type == xfREF ) {
      class = ((tSTR *) ( ( (tREF *) classNameT )->val ))->val;
    }
    else {
      class = classNameT->val;
    }
    
    //printf("Class:" );
    //tBASE__dump( (tBASE *) classDict, 1 );
    //printf("Ob:" );
    //tBASE__dump( ob, 1 );
    
    if( !strcmp( class, "NSURL" ) ) {
      return (tBASE *) expandURL( self, (tDICT *) ob );
    }
    
    if( !strcmp( class, "NSUUID" ) ) {
      return (tBASE *) expandUUID( self, (tDICT *) ob );
    }
    
    if( !strcmp( class, "NSDictionary" ) ||
        !strcmp( class, "NSMutableDictionary" )
    ) {
      duprintf("Expanding dict\n");
      //printf("  done\n");
      return (tBASE *) expandDict( self, (tDICT *) ob );
    }
    
    if( !strcmp( class, "XCTestConfiguration" ) ) {
      duprintf("Expanding XCTestConfiguration\n");
      //printf("  done\n");
      return (tBASE *) expandTestConf( self, (tDICT *) ob );
    }
    
    if( !strcmp( class, "XCTCapabilities" ) ) {
      duprintf("Expanding XCTCapabilities\n");
      //printf("  done\n");
      return (tBASE *) expandCaps( self, (tDICT *) ob );
    }
    
    if( !strcmp( class, "NSArray" ) ||
        !strcmp( class, "NSMutableArray" ) ||
        !strcmp( class, "NSSet" ) ||
        !strcmp( class, "NSMutableSet" )
    ) {
      duprintf("Expanding array\n");
      tARR *vals = (tARR *) tDICT__get( (tDICT *) ob, "NS.objects" );
      tARR *arr = tARR__new();
      uint32_t valCount = vals->count;
      
      duprintf("valcount=%d\n", valCount);
      for( int i=0;i<valCount;i++ ) {
        tARCID *valRef = (tARCID *) tARR__get( vals, i );
        if( valRef->type == xfREF ) valRef = (tARCID *) ( (tREF *) valRef )->val;
        duprintf("Adding val at %i\n", valRef->val );
        
        tBASE *val = self->obs[ valRef->val ];
        if( !val ) {
          printf("Could not grab %d from obs\n", valRef->val );
          printf("Ob count %d\n", self->obCount );
        }
        //tBASE__dump( val, 1 );
        
        if( willExpand( val ) ) {
          if( val->type == xfREF ) val = ( (tREF *) val )->val;
          duprintf("Subexpand\n");
          duprintf("  sub type:%d\n", val->type );
          val = tARCHIVE__expand( self, val );
        }
        tARR__add( arr, tREF__new( val ) );
      }
      return (tBASE *) arr;
    }
    if( !strcmp( class, "NSDate" ) ) {
      duprintf("Expanding date\n");
      tF2 *valT = (tF2 *) tDICT__get( (tDICT *) ob, "NS.time" );
      valT->val += 978307200;
      //tBASE__dump( valT, 1 );
      return (tBASE *) valT;
    }
    if( !strcmp( class, "DTSysmonTapMessage" ) ) {
      //printf("Contents of tap message:");
      //tBASE__dump( ob, 1 );
      tARCID *plistRef = (tARCID *) tDICT__get( (tDICT *) ob, "DTTapMessagePlist" );
      tBASE *plist = self->obs[ plistRef->val ];
      //tBASE__dump( plist, 1 );
      return (tBASE *) expandDict( self, (tDICT *) plist );
    }
    if( !strcmp( class, "NSError" ) ) {
      tDICT *obd = (tDICT *) ob;
      
      tDICT *res = tDICT__new();
      tDICT__set( res, "Type", tSTR__new("NSError") );
      tDICT__set( res, "NSCode", tDICT__get( obd, "NSCode" ) );
      tARCID *userInfoRef = (tARCID *) tDICT__get( obd, "NSUserInfo" );
      tBASE *userInfo = self->obs[ userInfoRef->val ];
      printf("user info type:%d\n", userInfo->type );
      tDICT__set( res, "NSUserInfo", expandDict( self, (tDICT *) userInfo ) );
      printf("user info set\n");
      //tARCID *domainRef = (tARCID *) tDICT__get( obd, "NSDomain" );
      //if( !domainRef ) {
      //  printf("Blank domain ref\n");
      //}
      //printf("Domain ref type:%d\n", domainRef->type );
      //tSTR *domain = (tSTR *) self->obs[ domainRef->val ];
      //printf("domain type: %d\n", domain->type );
      //printf("domain: %s\n", domain->val );
      //tDICT__set( res, "NSDomain", domain );
      
      return (tBASE *) res;
    }
    if( !strcmp( class, "NSNull" ) ) {
      return (tBASE *) tNULL__new();
    }
    //printf("Other class:%s\n", class );
  }
  return ob;
}

tBASE *dearchive( uint8_t *data, uint32_t len ) {
  //printf("raw data:%.*s\n", len, data );
  bpList *list = bpList__new( data, len );
  if( !list ) return NULL;
  tDICT *root = (tDICT *) list->obs->ptr[ 0 ];
  
  #ifdef UDEBUG
  tBASE__dump( (tBASE *) root, 1 );
  printf("root type:%d\n", root->type );
  printf("------\n");
  #endif
  
  tARR *obsT = (tARR *) tDICT__get( root, "$objects" );
  duprintf("$objects type %d\n", obsT->type );
  uint32_t count = obsT->count;
  if( obsT ) duprintf("Found $objects; count=%d\n", count );
  
  tBASE **obs = (tBASE **) tARR__flatten( obsT );
  duprintf("Flattened\n");
  
  tARCHIVE *archive = tARCHIVE__new( count, obs );
  
  //tBASE__dump( root, 1 );
  tBASE *expanded = tARCHIVE__expand( archive, obs[1] );
  #ifdef UDEBUG
  tBASE__dump( expanded, 1 );
  #endif
  
  return expanded;
}
