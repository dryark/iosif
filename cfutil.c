// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<CoreFoundation/CoreFoundation.h>
//#include"archiver/nsutil.h"
#include"archiver/archiver.h"
#include"archiver/unarchiver.h"

CFNumberRef i8cf(  int8_t num  ) { return CFNumberCreate( NULL, kCFNumberSInt8Type,  &num ); }
CFNumberRef i16cf( int16_t num ) { return CFNumberCreate( NULL, kCFNumberSInt16Type, &num ); }
CFNumberRef i32cf( int32_t num ) { return CFNumberCreate( NULL, kCFNumberSInt32Type, &num ); }
CFNumberRef i64cf( int64_t num ) { return CFNumberCreate( NULL, kCFNumberSInt64Type, &num ); }
CFNumberRef f1cf( float num ) { return CFNumberCreate( NULL, kCFNumberFloat32Type, &num ); }
CFNumberRef f2cf( double num ) { return CFNumberCreate( NULL, kCFNumberFloat64Type, &num ); }
CFDataRef datacf( const void *data, int len ) {
  return CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, ( const uint8_t * ) data, len, NULL );
}

CFBooleanRef boolcf( char val ) { return val ? kCFBooleanTrue : kCFBooleanFalse; }
uint64_t cfi64( CFNumberRef num ) {
  uint64_t res;
  CFNumberGetValue( num, kCFNumberSInt64Type, &res);
  return res;
}
uint16_t cfi16( CFNumberRef num ) {
  uint64_t res;
  CFNumberGetValue( num, kCFNumberSInt16Type, &res);
  return res;
}
uint8_t cfi8( CFNumberRef num ) {
  uint64_t res;
  CFNumberGetValue( num, kCFNumberSInt8Type, &res);
  return res;
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

char cfstreq( CFStringRef cf, const char *to ) {
    CFStringRef toCf = str_c2cf( (char *) to );
    return ( CFStringCompare( cf, toCf, 0 ) == kCFCompareEqualTo ) ? 1 : 0;
}

char iscfstr( CFTypeRef cf ) {
    return ( CFGetTypeID( cf ) == 0x07 ) ? 1 : 0;
}
char iscfdict( CFTypeRef cf ) {
    return ( CFGetTypeID( cf ) == 0x12 ) ? 1 : 0;
}
char iscfarr( CFTypeRef cf ) {
    return ( CFGetTypeID( cf ) == 0x13 ) ? 1 : 0;
}
char iscfnum( CFTypeRef cf ) {
    return ( CFGetTypeID( cf ) == 0x16 ) ? 1 : 0;
}

CFTypeRef cfmapget( CFDictionaryRef node, char *key ) {
  CFTypeRef val = CFDictionaryGetValue( node, str_c2cf( key ) );
  return val;
}

void cfdump( int depth, CFTypeRef node ) {
  char sp[20];
  memset( sp, ' ', 20 );
  sp[depth*2] = 0x00;
  sp[depth*2+2] = 0x00;
  CFIndex typeId = CFGetTypeID( node );
    
  int keyCnt;
  CFStringRef valueType;
  char *ctype;
  char *val;
  int64_t inum;
  int32_t inum32;
  int len;
  
  switch( typeId ) {
  case 0x07: // string
    val = str_cf2c( node );
    printf("\"%s\"\n", val );
    break;
  case 0x11: // set
    printf("[\n");
    keyCnt = CFSetGetCount( node );
    CFTypeRef **values = CFAllocatorAllocate( NULL, sizeof (void *) * keyCnt, 0); 
    CFSetGetValues( node, (const void **) values );
    for( int i=0;i<keyCnt;i++ ) {
      printf( "%s", sp );
      cfdump( depth+1, values[ i ] );
    }
    printf("%s]\n",&sp[2]);
    break;
  case 0x12: // dictionary
    printf("{\n");
    keyCnt = CFDictionaryGetCount( node );
    CFTypeRef *keysTypeRef = (CFTypeRef *) malloc( keyCnt * sizeof(CFTypeRef) );
    CFDictionaryGetKeysAndValues( node, (const void **) keysTypeRef, NULL);
    const void **keys = (const void **) keysTypeRef;
    for( int i=0;i<keyCnt;i++ ) {
      CFStringRef cfkey = keys[ i ];
      CFIndex keyType = CFGetTypeID( cfkey );
      switch( keyType ) {
        case 0x07: {
          char *key = str_cf2c( cfkey );
          int len = strlen( key );
          uint8_t needQuotes = 0;
          for( int i=0;i<len;i++ ) {
            char let = key[i];
            if( ! (
              ( let >= 'a' && let <= 'z' ) ||
              ( let >= 'A' && let <= 'Z' ) ||
              ( let >= '0' && let <= '9' ) 
            ) ) {
              needQuotes = 1;
              break;
            }
          }
          if( needQuotes ) {
            printf("%s\"%s\":",sp,key);
          } else { 
            printf("%s%s:",sp,key);
          }
          break;
        }
        case 0x16: { // number
          CFNumberGetValue( (CFNumberRef) cfkey, kCFNumberSInt32Type, &inum32);
          printf("%s%d:",sp,inum32);
          break;
        }
        default:
          printf("%s?:",sp);
      }
      cfdump( depth+1, CFDictionaryGetValue( node, cfkey ) );
    }
    printf("%s}\n",&sp[2]);
    break;
  case 0x13: // array
    keyCnt = CFArrayGetCount( node );
    printf("[\n");
    for( int i=0;i<keyCnt;i++ ) {
      printf("%s",sp);
      cfdump( depth+1, CFArrayGetValueAtIndex( node, i ) );
    }
    printf("%s]\n",&sp[2]);
    break;
  case 0x14: // cfdata
    len = CFDataGetLength( node );
    CFRange range = CFRangeMake(0,len);
    char *bytes = malloc( len );
    CFDataGetBytes( node, range, (UInt8 *) bytes );
    if( depth > 0 ) {
      printf("x.");
      for( int i=0;i<len;i++ ) {
        printf("%02x",(unsigned char) bytes[i]);
      }
      printf("\n");
    } else {
      fwrite( bytes, len, 1, stdout );
    }
    free( bytes );
    break;
  case 0x15: // bool
    CFNumberGetValue( node, kCFNumberSInt32Type, &inum32);
    if( inum32 ) printf("true\n");
    else         printf("false\n");
    break;
  case 0x16: // number
    CFNumberGetValue( node, kCFNumberSInt64Type, &inum);
    printf("%" PRId64 "\n", inum );
    break;
  case 0x1b: // CFError
    printf("%sCFError\n", sp );
    CFStringRef domain = CFErrorGetDomain( (CFErrorRef) node );
    printf("%s  Domain:%s\n", sp, str_cf2c( domain ) );
    CFStringRef descr = CFErrorCopyDescription( (CFErrorRef) node );
    printf("%s  Descr:%s\n", sp, str_cf2c( descr ) );
    CFStringRef reason = CFErrorCopyFailureReason( (CFErrorRef) node );
    printf("%s  Reason:%s\n", sp, str_cf2c( reason ) );
    CFStringRef rec = CFErrorCopyRecoverySuggestion( (CFErrorRef) node );
    printf("%s  Recovery:%s\n", sp, str_cf2c( rec ) );
    int code = CFErrorGetCode( (CFErrorRef) node );
    printf("%s  Code:%i\n", sp, code );
    CFDictionaryRef userInfo = CFErrorCopyUserInfo( (CFErrorRef) node );
    cfdump( depth+1, userInfo );
    break;
  case 0x29: {// archiver UID
      void *uid = (void*) node;
      uint32_t *valuePtr = uid+16;
      printf("oid.%d\n", *valuePtr );
    }
    break;
  case 0x2a: {
      double unix = (double) CFDateGetAbsoluteTime( node ) + (double) kCFAbsoluteTimeIntervalSince1970;
      printf("%.2f\n", unix );
    }
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

CFArrayRef genstrarr( int count, ... ) {
  va_list va;
  va_start( va, count );
  CFTypeRef *vals = malloc( sizeof( CFTypeRef ) * count );
  for( int i=0;i<count;i++ ) {
    CFStringRef strCf = str_c2cf( va_arg( va, char * ) );
    vals[i] = strCf;
  }
  CFArrayRef res = CFArrayCreate( NULL, (void *) vals, count, &kCFTypeArrayCallBacks);
  free( vals );
  return res;
}

char *cftype( CFTypeRef cf ) {
  CFStringRef descr = CFCopyDescription(cf);
  char *type = str_cf2c( descr );
  CFRelease( descr );
  return type;
}

typedef struct {
  uint32_t bufferSize;
  uint32_t x1; // 4
  uint32_t auxSize; // 8
  uint32_t x2; // 12
} dtxAuxHeader;

tARR *deserialize2t( const uint8_t *buf, size_t bufsize ) {
  if( bufsize < 16 ) {
    fprintf(stderr, "Error: buffer of size 0x%lx is too small for a serialized array", bufsize);
    return NULL;
  }
  
  tARR *array = tARR__new();

  const uint8_t *pos = buf + 16; // skip header
  uint64_t size = *((uint64_t *)buf+1);
  const uint8_t *end = pos + size; // header->bufferSize;

  while( pos < end ) {
    int32_t length = 0;
    int32_t type = *( (int32_t *) pos );
    pos += 4;

    tBASE *ref = NULL;

    //printf("Primitive type %d\n", type );
    switch( type ) {
      case 1: // string
      case 2: // object / bytearray
        length = *( (int32_t *) pos );
        pos += 4;
        ref = dearchive( (uint8_t *) pos, length );
        break;

      case 3:
      case 5: ref = (tBASE *) tI32__new( *( (int32_t *) pos ) ); length = 4; break; // i32

      case 4:
      case 6: ref = (tBASE *) tI64__new( *( (int64_t *) pos ) ); length = 4; break; // i32

      case 10: continue; //ref = kCFNull; length = 0; break; // null

      default:
        printf("Unknown primitive type %d\n", type );
        break;
    }

    if( !ref ) {
      fprintf(stderr, "invalid object at offset %lx, type: %d\n", pos - buf, type);
      return NULL;
    }

    tARR__add( array, ref );
    pos += length;
  }

  return array;
}

/*CFArrayRef deserialize2cf( const uint8_t *buf, size_t bufsize) {
  if( bufsize < 16 ) {
    fprintf(stderr, "Error: buffer of size 0x%lx is too small for a serialized array", bufsize);
    return NULL;
  }

  //dtxAuxHeader *header = (dtxAuxHeader*) buf;
  //uint32_t size = header.bufferSize;
  
  CFMutableArrayRef array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

  const uint8_t *pos = buf + 16; // skip header
  //uint64_t off = 16;//sizeof(uint64_t) * 2;
  //uint64_t end = off + size;
  
  uint64_t size = *((uint64_t *)buf+1);
  //printf("primitive size: %d, %d\n", header->bufferSize, (uint32_t) size );
  const uint8_t *end = pos + size; // header->bufferSize;

  while( pos < end ) {
    int32_t length = 0;
    int32_t type = *( (int32_t *) pos );
    pos += 4;

    CFTypeRef ref = NULL;

    //printf("Primitive type %d\n", type );
    switch( type ) {
      case 1: // string
        
      case 2: // object / bytearray
        length = *( (int32_t *) pos );
        pos += 4;
        ref = archive2cf( pos, length );
        break;

      case 3:
      case 5: ref = CFNumberCreate(NULL, kCFNumberSInt32Type, pos); length = 4; break; // i32

      case 4:
      case 6: ref = CFNumberCreate(NULL, kCFNumberSInt64Type, pos); length = 8; break; // i64

      case 10: continue; //ref = kCFNull; length = 0; break; // null

      default:
        printf("Unknown primitive type %d\n", type );
        break;
    }

    if( !ref ) {
      fprintf(stderr, "invalid object at offset %lx, type: %d\n", pos - buf, type);
      return NULL;
    }

    CFArrayAppendValue( array, ref );
    CFRelease(ref);
    pos += length;
  }

  return (CFArrayRef) array;
}*/

uint32_t crc32( uint32_t crc, const char *buf, size_t len ) {
    static uint32_t table[256];
    static int have_table = 0;
    uint32_t rem;
    uint8_t octet;
    int i, j;
    const char *p, *q;
 
    /* This check is not thread safe; there is no mutex. */
    if (have_table == 0) {
        /* Calculate CRC table. */
        for (i = 0; i < 256; i++) {
            rem = i;  /* remainder from polynomial division */
            for (j = 0; j < 8; j++) {
                if (rem & 1) {
                    rem >>= 1;
                    rem ^= 0xedb88320;
                } else
                    rem >>= 1;
            }
            table[i] = rem;
        }
        have_table = 1;
    }
 
    crc = ~crc;
    q = buf + len;
    for (p = buf; p < q; p++) {
        octet = *p;  /* Cast to unsigned octet. */
        crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}

// Lazy mans NSKeyedArchiver
char *strArchive( const char *str, int *strLen ) {
  // Initial <?xml...?>, DOCTYPE, and surrounding <plist> can all be dropped
  const char prefix[] =
  "<dict>"
  //"<key>$archiver</key><string>NSKeyedArchiver</string>" // this is not needed
  "<key>$objects</key><array><key/><key>"; // <string>$null</string> replaced with <key/> :D
  const char suffix[] = "</key></array>"
  "  <key>$top</key><dict><key>root</key><dict><key>CF$UID</key><integer>1</integer></dict></dict>"
  "  <key>$version</key><integer>100000</integer>"
  //"  <integer>0x186a0</integer>" // You can write integers in hex format. For lulz.
  "</dict>";
  
  int totLen = sizeof( prefix ) + strlen(str) + sizeof( suffix ) - 2;
  char *res = malloc( totLen + 1 );
  *strLen = totLen;
  sprintf( res, "%s%s%s", prefix, str, suffix );
  return res;
}

/*char *strArchive2( const char *str, int *strLen ) {
  const char prefix[] = PLIST_HEAD
  "<dict>"
  "<key>$archiver</key>"
  "<string>NSKeyedArchiver</string>"
  "<key>$objects</key>"
  "<array>"
  "<string>$null</string>"
  "<string>";
  const char suffix[] = "</string>"
  "</array>"
  "<key>$top</key>"
  "<dict>"
  "<key>root</key>"
  "<dict>"
  "<key>CF$UID</key>"
  "<integer>1</integer>"
  "</dict>"
  "</dict>"
  "<key>$version</key>"
  "<integer>100000</integer>"
  "</dict>"
  "</plist>";
  int totLen = sizeof( prefix ) + strlen(str) + sizeof( suffix ) - 2;
  char *res = malloc( totLen + 1 );
  *strLen = totLen;
  sprintf( res, "%s%s%s", prefix, str, suffix );
  return res;
}*/

CFPropertyListRef xml2plist( const char *data ) {
  CFDataRef cfd = CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, ( const uint8_t *) data, strlen(data), NULL );
  CFErrorRef errorCode;
  CFPropertyListRef pList = CFPropertyListCreateWithData(
    kCFAllocatorDefault,
    cfd,
    kCFPropertyListImmutable,
    NULL,
    &errorCode
  );
  return pList;
}

CFPropertyListRef data2plist( const uint8_t *data, int len ) {
  CFDataRef cfd = CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, ( const uint8_t *) data, len, NULL );
  CFErrorRef errorCode;
  CFPropertyListRef pList = CFPropertyListCreateWithData(
    kCFAllocatorDefault,
    cfd,
    kCFPropertyListImmutable,
    NULL,
    &errorCode
  );
  return pList;
}

void dumparchive( const uint8_t *data, uint32_t len ) {
  if( data[0] != 'b' || data[1] != 'p' ) {
    /*int i = 0;
    for( i=0;i<len;i++ ) {
      if( data[i] == 'b' && data[i+1] == 'p' ) break;
    }
    printf("Skipping %d bytes\n", i );
    data += i; len -= i;*/
    printf("Unknown format\n");
    printf("Sample:\n");
    uint32_t end = 50;
    if( len < 50 ) end = len;
    for( int i=0;i<end;i++ ) {
      printf("%02x", data[i] );
    }
    return;
  }
  
  CFDataRef cfd = CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, data, len, NULL );
  CFErrorRef errorCode;
  CFPropertyListRef pList = CFPropertyListCreateWithData(
    kCFAllocatorDefault,
    cfd,
    kCFPropertyListImmutable,
    NULL,
    &errorCode
  );
  CFDataRef xfd = CFPropertyListCreateData(
    kCFAllocatorDefault,
    pList,
    kCFPropertyListXMLFormat_v1_0,
    0,
    NULL
  );
  int xmlLen = CFDataGetLength( xfd );
  const uint8_t *xml = CFDataGetBytePtr( xfd );
  printf("xml:%.*s\n", xmlLen, xml );
}