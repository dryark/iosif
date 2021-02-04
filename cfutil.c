// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

CFNumberRef i8cf( char num ) {
  return CFNumberCreate(kCFAllocatorDefault, kCFNumberCharType, &num );
}

char *str_cf2c( CFStringRef cfstr ) {
  if( !cfstr ) return NULL;
  int size = CFStringGetMaximumSizeForEncoding( CFStringGetLength( cfstr ), kCFStringEncodingUTF8 ) + 1;
  char *buffer = ( char * ) malloc( size );
  if( CFStringGetCString( cfstr, buffer, size, kCFStringEncodingUTF8 ) ) return buffer;
  free( buffer );
  return NULL;
}

CFStringRef str_c2cf( char *str ) {
  return CFStringCreateWithCString( NULL, str, kCFStringEncodingUTF8 );
}

void cfdump( int depth, CFTypeRef node ) {
  char sp[20];
  memset( sp, ' ', 20 );
  sp[depth*2] = 0x00;
  CFIndex typeId = CFGetTypeID( node );
    
  int keyCnt;
  CFStringRef valueType;
  char *ctype;
  char *val;
  int64_t inum;
  int len;
  
  switch( typeId ) {
  case 0x07: // string
    val = str_cf2c( node );
    printf("%s\n", val );
    break;
  case 0x12: // dictionary
    printf("\n");
    keyCnt = CFDictionaryGetCount( node );
    CFTypeRef *keysTypeRef = (CFTypeRef *) malloc( keyCnt * sizeof(CFTypeRef) );
    CFDictionaryGetKeysAndValues( node, (const void **) keysTypeRef, NULL);
    const void **keys = (const void **) keysTypeRef;
    for( int i=0;i<keyCnt;i++ ) {
      CFStringRef cfkey = keys[ i ];
      char *key = str_cf2c( cfkey );
      printf("%s%s:",sp,key);
      cfdump( depth+1, CFDictionaryGetValue( node, cfkey ) );
    }
    break;
  case 0x13: // array
    keyCnt = CFArrayGetCount( node );
    printf("[\n");
    for( int i=0;i<keyCnt;i++ ) {
      printf("%s",sp);
      cfdump( depth+1, CFArrayGetValueAtIndex( node, i ) );
    }
    printf("%s]\n",sp);
    break;
  case 0x14: // cfdata
    len = CFDataGetLength( node );
    CFRange range = CFRangeMake(0,len);
    char *bytes = malloc( len );
    CFDataGetBytes( node, range, (UInt8 *) bytes );
    /*for( int i=0;i<len;i++ ) {
      printf("%02x",bytes[i]);
    }*/
    fwrite( bytes, len, 1, stdout );
    free( bytes );
    break;
  case 0x15: // bool
  case 0x16: // number
    CFNumberGetValue( node, kCFNumberSInt64Type, &inum);
    printf("%" PRId64 "\n", inum );
    break;
  default:
    valueType = CFCopyTypeIDDescription( typeId );
    ctype = str_cf2c( valueType );
    printf(" %s[%lx]\n", ctype, typeId );
  }
}

void cf2json( CFTypeRef node ) {
  CFIndex typeId = CFGetTypeID( node );
    
  int keyCnt;
  CFStringRef valueType;
  char *ctype;
  char *val;
  int16_t inum;
  int len;
  
  switch( typeId ) {
  case 0x07: // string
    val = str_cf2c( node );
    printf("\"%s\"", val );
    break;
  case 0x12: // dictionary
    keyCnt = CFDictionaryGetCount( node );
    CFTypeRef *keysTypeRef = (CFTypeRef *) malloc( keyCnt * sizeof(CFTypeRef) );
    CFDictionaryGetKeysAndValues( node, (const void **) keysTypeRef, NULL);
    const void **keys = (const void **) keysTypeRef;
    printf("{");
    for( int i=0;i<keyCnt;i++ ) {
      CFStringRef cfkey = keys[ i ];
      char *key = str_cf2c( cfkey );
      printf("\"%s\":",key);
      cf2json( CFDictionaryGetValue( node, cfkey ) );
    }
    printf("}");
    break;
  case 0x13: // array
    keyCnt = CFArrayGetCount( node );
    printf("[");
    for( int i=0;i<keyCnt;i++ ) {
      cf2json( CFArrayGetValueAtIndex( node, i ) );
      if( i != ( keyCnt - 1 ) ) printf(",");
    }
    printf("]");
    break;
  case 0x14: // cfdata
    len = CFDataGetLength( node );
    CFRange range = CFRangeMake(0,len);
    char *bytes = malloc( len );
    CFDataGetBytes( node, range, (UInt8 *) bytes );
    for( int i=0;i<len;i++ ) {
      printf("%02x",bytes[i]);
    }
    free( bytes );
    break;
  case 0x15: // bool
  case 0x16: // number
    CFNumberGetValue( node, kCFNumberSInt16Type, &inum);
    printf("%i", inum );
    break;
  default:
    valueType = CFCopyTypeIDDescription( typeId );
    ctype = str_cf2c( valueType );
    printf(" %s[%lx]\n", ctype, typeId );
  }
}

CFDictionaryRef genmap( int count, ... ) {
  int keycnt = count / 2;
  va_list va;
  va_start( va, count );
  CFStringRef *keys = malloc( sizeof( CFStringRef ) * keycnt );
  CFTypeRef *vals = malloc( sizeof( CFTypeRef ) * keycnt );
  for( int i=0;i<keycnt;i++ ) {
    keys[ i ] = CFStringCreateWithCString( NULL, va_arg( va, char * ), kCFStringEncodingUTF8 );
    vals[ i ] = va_arg( va, CFTypeRef );
  }
  va_end( va );
  CFDictionaryRef dict = CFDictionaryCreate( NULL, ( const void ** ) keys, ( const void ** ) vals, keycnt,
    &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
  free( keys );
  free( vals );
  return dict;
}

CFArrayRef strsToArr( int count, char **strs ) {
  CFStringRef *vals = malloc( sizeof( CFStringRef ) * count );
  for( int i=0;i<count;i++ ) {
    vals[i] = CFStringCreateWithCString( NULL, strs[i], kCFStringEncodingUTF8 );
  }
  CFArrayRef res = CFArrayCreate( NULL, (void *) vals, count, &kCFTypeArrayCallBacks);
  free( vals );
  return res;
}

CFArrayRef genarr( int count, ... ) {
  va_list va;
  va_start( va, count );
  CFTypeRef *vals = malloc( sizeof( CFTypeRef ) * count );
  for( int i=0;i<count;i++ ) vals[i] = va_arg( va, CFTypeRef );
  CFArrayRef res = CFArrayCreate( NULL, (void *) vals, count, &kCFTypeArrayCallBacks);
  free( vals );
  return res;
}