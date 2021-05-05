// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include"archiver.h"
#include"unarchiver.h"
#include"nsutil.h"
#include"../cfutil.h"

uint8_t checkEquals( char *descr, CFTypeRef data );

int main() {
  /*{
    CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    CFDateRef item = CFDateCreate( NULL, now );
    checkEquals( "Time", item );
  }*/
  /*{
    CFTypeRef item = f1cf( 300000.34 );
    checkEquals( "Float", item );
  }*/
  {
    char data[] = { 0x43, 0x87, 0xb1 };
    CFTypeRef item = datacf( data, 3 );
    checkEquals( "Data", item );
  }
  /*{
    CFArrayRef arr = genarr( 1, kCFBooleanTrue );
    checkEquals( arr );
  }*/
  /*{
    CFDictionaryRef arr = genmap( 2, "a", i8cf( -2 ) );
    checkEquals( arr );
  }*/
  /*{
    #define NN 300
    char str[NN];
    memset( str, 'x', NN );
    str[NN-1] = 0x00;
    CFArrayRef arr = genarr( 2, i8cf( 1 ), str_c2cf( str ) );
    checkEquals( arr );
  }*/
  
  
}

uint8_t checkEquals( char *descr, CFTypeRef item ) {
  printf("Checking %s\n", descr );
  cfdump( 1, item );
  
  int len;
  uint8_t *data = cf2archive( item, &len, 0 );
  tBASE *clean = dearchive( data, len );
  //printf("Clean type:%d\n", clean->type );
  tBASE__dump( clean, 1 );
  printf("\n");
  return 0;
}