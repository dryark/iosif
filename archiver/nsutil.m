#include<CoreFoundation/CoreFoundation.h>
#include<Foundation/Foundation.h>
#include<stdint.h>
//#include"cfutil.h"

uint8_t *cf2archive( CFTypeRef cf, int *len, char secure ) {
  //@autoreleasepool {
    id object = (__bridge id)cf;
    //NSData *data = [NSKeyedArchiver archivedDataWithRootObject:object];
      
    NSData *data;
    if( 1 ) {//secure ) {
        data = [NSKeyedArchiver archivedDataWithRootObject:( (__bridge id) cf ) requiringSecureCoding:true error:nil];
    }
    else {
        data = [NSKeyedArchiver archivedDataWithRootObject:( (__bridge id) cf ) requiringSecureCoding:false error:nil];
    }
    if( !data ) {
        fprintf(stderr,"Could not code with NSKeyedArchiver\n");
        exit(1);
    }
    *len = [data length];
    //printf("Archived length: %i\n", *len );
    
    return (uint8_t *) [data bytes];
  //}
}

CFTypeRef archive2cf( const uint8_t *bytes, uint32_t len ) {
  if( bytes[0] != 'b' || bytes[1] != 'p' ) {
    int i = 0;
    for( i=0;i<len;i++ ) {
      if( bytes[i] == 'b' && bytes[i+1] == 'p' ) break;
    }
    //printf("Skipping %d bytes\n", i );
    //bytes += i;
    //len -= i;
    //bytes += i; len -= i;
    //uint16_t bsize = *( (uint16_t *) bytes + 14 );
    //printf("bsize: %d\n", bsize );
    
    //dumparchive( bytes + i, len - i );
    return NULL;
    /*printf("Not bplist\n");
    for( int i=0;i<len;i++ ) {
      if( bytes[i] == 'b' && bytes[i+1] == 'p' && bytes[i+2] == 'l' ) {
        printf("bpl at offset %d\n", i );
      }
    }
    return NULL;*/
  }
  
  //@autoreleasepool {
    NSData *data = [NSData dataWithBytesNoCopy:(void *)bytes length:len freeWhenDone:false];
    //id ns = [[NSKeyedUnarchiver unarchiveObjectWithData:data] retain];
    id ns = [NSKeyedUnarchiver unarchivedObjectOfClass:[NSObject class] fromData:data error:nil];
    return (__bridge CFTypeRef) ns;
  //}
}

CFDictionaryRef archive2cfdict( uint8_t *bytes, uint32_t len ) {
  @autoreleasepool {
    NSData *data = [NSData dataWithBytesNoCopy:(void *)bytes length:len freeWhenDone:false];
    //id ns = [[NSKeyedUnarchiver unarchiveObjectWithData:data] retain];
    id ns = [[NSKeyedUnarchiver unarchivedObjectOfClass:[NSDictionary class] fromData:data error:nil] retain];
    return (__bridge CFTypeRef) ns;
  }
}
