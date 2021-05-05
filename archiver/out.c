// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdio.h>
#include"plist.h"
#include"archiver.h"

int main( int argc, char *argv[] ) {
  tDICT *dict = tDICT__newPairs( 12,
    "i8", tI8__new( 4 ),
    "i16", tI16__new( 5 ),
    "i32", tI32__new( 6 ),
    "i64", tI64__new( 7 ),
    "bool", tBOOL__new( 1 ),
    "arcid", tARCID__new( 1000 ),
    //"null", tNULL__new(),
		"key", tSTR__new( "str" ),
		"arr", tARR__newVals( 2, tI16__new( 2 ), tI16__new( 3 ) )
  );
  tI16 *num = tI16__new(2);
  
  uint32_t len;
  //uint8_t *data = tBASE__tobin( (tBASE *) dict, &len );
  //uint8_t *data = tBASE__tobin( (tBASE *) num, &len );
  //uint8_t *data = tBASE__archivebin( (tBASE *) tARR__newVals( 1, (tBASE *) num ), &len );
  
  //uint8_t *data = tBASE__archivebin( (tBASE *) num, &len );
  uint8_t *data = tBASE__archivebin( (tBASE *) tSTR__new("/private/var/mobile/Containers/Data/Application/BE9D64A9-B78A-4E2C-8C5E-3E9BCFD56473/tmp/5f33ca05-b98c-40ab-b18a-e414e2d486e0.xctestconfiguration"), &len );
  
  //uint8_t *data = tBASE__archive( (tBASE *) tARR__newVals( 1, (tBASE *) num ), &len );
  
  FILE *fh = fopen( "data", "wb" );
  if( !fh ) {
    fprintf(stderr,"Could not open %s for writing\n", data );
    return 1;
  }
  fwrite( data, len, 1, fh ); 
  fclose( fh );
  return 0;
}