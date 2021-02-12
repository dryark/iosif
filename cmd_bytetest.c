// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

void runBytetest( void *device );
void run_bytetest( ucmd *cmd ) {
  g_cmd = cmd;
  waitForConnect( runBytetest );
}

void runBytetest( void *device ) {
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
  
  bytearr *aux = bytearr__new();
  bytearr__auxi32( aux, 1 );
  bytearr__auxi32( aux, 2 );
  CFDictionaryRef dict = genmap( 2, "a", i32cf(1) );
  //cfdump( 0, dict );
  bytearr__auxcf( aux, dict, 0 );
  
  int auxLen;
  uint8_t *auxBytes = bytearr__asaux( aux, &auxLen );
  
  uint32_t crc = crc32(0,(const char *) auxBytes,auxLen);
  printf("%08x", crc );
  printf("\n");
  for( int i=0;i<auxLen;i++ ) {
    unsigned char byte = auxBytes[i];
    printf("%02x", byte );
  }
  printf("\n\n");
  
  exit(0);
}