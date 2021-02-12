#include<CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include<stdint.h>

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

CFTypeRef archive2cf( uint8_t *bytes, int len ) {
  //@autoreleasepool {
    NSData *data = [NSData dataWithBytesNoCopy:(void *)bytes length:len freeWhenDone:false];
    //id ns = [[NSKeyedUnarchiver unarchiveObjectWithData:data] retain];
    id ns = [NSKeyedUnarchiver unarchivedObjectOfClass:[NSObject class] fromData:data error:nil];
    return (__bridge CFTypeRef) ns;
  //}
}

CFDictionaryRef archive2cfdict( uint8_t *bytes, int len ) {
  @autoreleasepool {
    NSData *data = [NSData dataWithBytesNoCopy:(void *)bytes length:len freeWhenDone:false];
    //id ns = [[NSKeyedUnarchiver unarchiveObjectWithData:data] retain];
    id ns = [[NSKeyedUnarchiver unarchivedObjectOfClass:[NSDictionary class] fromData:data error:nil] retain];
    return (__bridge CFTypeRef) ns;
  }
}
