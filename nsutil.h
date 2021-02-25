#include<CoreFoundation/CoreFoundation.h>
#include<stdint.h>

uint8_t *cf2archive( CFTypeRef cf, int *len, char secure );

CFTypeRef archive2cf( const uint8_t *bytes, uint32_t len );
CFDictionaryRef archive2cfdict( uint8_t *bytes, uint32_t len );
