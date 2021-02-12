// Copyright (c) 2021 David Helkowski
// Anti-Corruption License
#include<CoreFoundation/CoreFoundation.h>
#include"nsutil.h"

CFNumberRef i8cf(  char num    ) { return CFNumberCreate( NULL, kCFNumberCharType,   &num ); }
CFNumberRef i32cf( int32_t num ) { return CFNumberCreate( NULL, kCFNumberSInt32Type, &num ); }
CFNumberRef i64cf( int64_t num ) { return CFNumberCreate( NULL, kCFNumberSInt64Type, &num ); }

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
    printf("{\n");
    for( int i=0;i<keyCnt;i++ ) {
      CFStringRef cfkey = keys[ i ];
      char *key = str_cf2c( cfkey );
      printf("  %s:",key);
      cf2json( CFDictionaryGetValue( node, cfkey ) );
      printf(",\n");
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

char *cftype( CFTypeRef cf ) {
  CFStringRef descr = CFCopyDescription(cf);
  char *type = str_cf2c( descr );
  CFRelease( descr );
  return type;
}

CFArrayRef deserialize( const uint8_t *buf, size_t bufsize) {
  if( bufsize < 16 ) {
    fprintf(stderr, "Error: buffer of size 0x%lx is too small for a serialized array", bufsize);
    return NULL;
  }

  uint64_t size = *((uint64_t *)buf+1);
  if ( size > bufsize ) {
    fprintf(stderr, "size of array object (%llx) is larger than total length of data (%lx)", size, bufsize);
    return NULL;
  }

  CFMutableArrayRef array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

  uint64_t off = sizeof(uint64_t) * 2;
  uint64_t end = off + size;

  while ( off < end ) {
    int32_t length = 0;
    int32_t type = *( (int32_t *) (buf+off) );
    off += 4;

    CFTypeRef ref = NULL;

    switch ( type ) {
      case 2: // object
        length = *( (int32_t *) (buf+off) );
        off += 4;
        ref = archive2cf( (uint8_t *) buf+off, length );
        break;

      case 3:
      case 5: ref = CFNumberCreate(NULL, kCFNumberSInt32Type, buf+off); length = 4; break; // i32

      case 4:
      case 6: ref = CFNumberCreate(NULL, kCFNumberSInt64Type, buf+off); length = 8; break; // i64

      case 10: continue; // dict

      default: break;
    }

    if( !ref ) {
      fprintf(stderr, "invalid object at offset %llx, type: %d\n", off, type);
      return NULL;
    }

    CFArrayAppendValue( array, ref );
    CFRelease(ref);
    off += length;
  }

  return (CFArrayRef) array;
}

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

void dumparchive( uint8_t *data, int len ) {
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