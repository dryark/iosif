// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include"uclop.h"
#include"service.h"
//#include"nsutil.h"
#include"cfutil.h"
#include"archiver/archiver.h"
#include"archiver/unarchiver.h"

static ucmd *g_cmd = NULL;
void runBytetest( void *device );
void run_bytetest( ucmd *cmd ) {
  //g_cmd = cmd;
  //waitForConnect( runBytetest );
  runBytetest( 0 );
}

void runBytetest( void *device ) {
  /*{
    CFArrayRef arr = genarr( 2, i8cf( 1 ), str_c2cf( "two" ) );
    int len;
    uint8_t *data = cf2archive( arr, &len, 0 );
    CFTypeRef root = data2plist( data, len );
    cfdump( 1, root );
  }*/
  
  /*{
    //CFArrayRef arr = genarr( 1, kCFBooleanTrue );
    int len;
    //CFDictionaryRef arr = genmap( 2, "a", i8cf( -2 ) );
    
    #define NN 300
    char str[NN];
    memset( str, 'x', NN );
    str[NN-1] = 0x00;
    CFArrayRef arr = genarr( 2, i8cf( 1 ), str_c2cf( str ) );
    uint8_t *data = cf2archive( arr, &len, 0 );
    //printf("%.*s\n", len, data );
    CFTypeRef root = data2plist( data, len );
    cfdump( 1, root );
    
    //bpList *list = bpList__new( data, len );
    //tBASE *rootT = (tBASE *) list->obs->ptr[ 0 ]
    //tBASE__dump( rootT, 1 );
      
    tBASE *clean = dearchive( data, len );
    tBASE__dump( clean, 1 );
  }*/
  
  /*{
    CFDictionaryRef arr = genmap( 2, "a", i8cf( 1 ) );
    int len;
    uint8_t *data = cf2archive( arr, &len, 0 );
    CFTypeRef root = data2plist( data, len );
    cfdump( 1, root );
  */
  
  /*{
    tSTR *str = tSTR__new("test");
    
    uint32_t len;
    char *bytes = (char *) tBASE__archive( (tBASE *) str, &len );
    printf("%s", bytes );
  }
  {
    tARR *arr = tARR__new();
    tARR__addI32( arr, 1 );
    tARR__addSTR( arr, "two" );
    
    uint32_t len;
    char *bytes = (char *) tBASE__archive( (tBASE *) arr, &len );
    printf("%s", bytes );
    
    tBASE__del( arr );
  }
  {
    tDICT *dict = tDICT__newPairs( 4,
      "first", (tBASE *) tI32__new( 1 ),
      "2nd", (tBASE *) tSTR__new( "two" )
    );
    tBASE__dump( (tBASE *) dict, 1 );
    
    uint32_t len;
    char *bytes = (char *) tBASE__archive( (tBASE *) dict, &len );
    printf("%s", bytes );
  }*/
  //dumparchive( data, len );
  
  /*bytearr *msgb = bytearr__new();
  
  uint8_t bytes[] = { 1,2,3,4 };
  bytearr__append( msgb, bytes, 4, 0 );
  
  int msgLen;
  uint8_t *msgBytes = bytearr__bytes( msgb, &msgLen ); 

  uint32_t crc = crc32(0,(const char *) msgBytes,msgLen);
  printf("%08x", crc );
  printf("\n");
  for( int i=0;i<msgLen;i++ ) {
    unsigned char byte = msgBytes[i];
    printf("%02x", byte );
  }
  printf("\n\n");*/
  
  /*bytearr *aux = bytearr__new();
  bytearr__auxi32( aux, 1 );
  bytearr__auxi32( aux, 2 );
  CFDictionaryRef dict = genmap( 2, "a", i32cf(1) );
  //cfdump( 0, dict );
  bytearr__auxcf( aux, dict, 0 );
  
  uint32_t auxLen;
  uint8_t *auxBytes = bytearr__asaux( aux, &auxLen );
  
  uint32_t crc = crc32(0,(const char *) auxBytes,auxLen);
  printf("%08x", crc );
  printf("\n");
  for( int i=0;i<auxLen;i++ ) {
    unsigned char byte = auxBytes[i];
    printf("%02x", byte );
  }
  printf("\n\n");
  
  exit(0);*/
}