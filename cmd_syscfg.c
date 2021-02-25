// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdint.h>
#include"service.h"
#include"uclop.h"

static ucmd *g_cmd = NULL;
void runSysCfg( void *device );
void run_syscfg( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runSysCfg ); }

void bswap( unsigned char *in ) {
  char out[4];
  char temp;
  bcopy( in, out, 4 );
  int dest = 3;
  for( int i=0;i<2;i++ ) {
    temp = out[i];
    out[i] = out[dest];
    out[dest--] = temp;
  }
  bcopy( out, in, 4 );
}
//#define bswapul( n ) bswap( (char *) &(n) );
unsigned long bswapul( unsigned long in ) {
  unsigned long out;
  char *outPtr = ( char * ) &out;
  char *inPtr = ( char * ) &in;
  outPtr[3] = inPtr[0];
  outPtr[2] = inPtr[1];
  outPtr[1] = inPtr[2];
  outPtr[0] = inPtr[3];
  return out;
}

typedef struct __attribute__((packed)) {
  char scfg[4]; // 0-3
  unsigned long size; // 4-7
  unsigned long maxSize; // 8-11
  unsigned long version; // 12-15
  unsigned long bigEndian; // 16 - 19
  unsigned long keyCount; // 20-24
} BaseInfo;

char buf[20];
char *hex4( unsigned char *addr ) {
  sprintf(buf,"%02x%02x%02x%02x", addr[0], addr[1], addr[2], addr[3] );
  buf[8] = 0x00;
  return buf;
}

char *hex4ul( unsigned long n ) {
  char *np = (char *) &n;
  sprintf(buf,"%02x%02x%02x%02x", np[0], np[1], np[2], np[3] );
  buf[8] = 0x00;
  return buf;
}

typedef struct {
  char *shortx;
  char *longx;
} NameEntry;

NameEntry *NM( char *a, char *b ) {
  NameEntry *self = (NameEntry *) malloc( sizeof( NameEntry ) );
  self->shortx = a;
  self->longx = b;
  return self;
}

void runSysCfg( void *device ) {
  NameEntry *names[] = {
    NM("AICl","Accelerator Interrupt Calibration"),
NM("ARot","Accelerometer Orientation Calibration"),
NM("ASCi","Accelerometer Sensitivity Inverse Matrix"),
NM("ASCl","Accelerometer Sensitivity Calibration"),
NM("ATGa","Acoustic Trim Gains"),
NM("Batt","Battery Serial Number"),
NM("BCAL","Bluetooth Taurus Calibration"),
NM("BCAR","BackCamera Autofocus Recalibration"),
NM("BCMB","Back Camera Module Board"),
NM("BCMS","Back Camera Module Serialnumber"),
NM("BGMt","Backing Glass Material"),
NM("BLCl","Backlight Calibration"),
NM("BMac","Bluetooth Mac Address"),
NM("BTRx","Bluetooth Reception Calibration"),
NM("BTTx","Bluetooth Transmission Calibration"),
NM("CBAT","Charget input limit calibration"),
NM("CDCC","Compass hilow calibration"),
NM("CGMt","Coverglass Material"),
NM("CGSp","Coverglass type"),
NM("CLBG","Backing Color"),
NM("CLHS","Housing Color"),
NM("CLCG","Coverglass Color"),
NM("CRot","???"),
NM("CSCM","Compass Sensor Calibration"),
NM("CVCC","Compass VBUS Compensation"),
NM("DBCl","Display Backlight Compensation"),
NM("DClr","Device color"),
NM("DPCl","Primary Calibration Matrix"),
NM("DTCl","Display Temperature Calibration"),
NM("EMac","Ethernet Mac Address"),
NM("EnMt","Enclosure Material"),
NM("FCMB","Front Camera Module Board"),
NM("FCMS","Front Camera Module Serialnumber"),
NM("FDAC","Orb Dynamic Accelerator Calibration"),
NM("FG2G","WiFi Calibration Frequency Group 2G"),
NM("GICl","Gyro Interrupt Calibration"),
NM("GLCl","Gamma Tables Calibration"),
NM("GRot","Gyro Orientation Calibration"),
NM("GSCi","Gyro Sensitivity Matrix Inverse"),
NM("GSCl","Gyro Sensitivity Calibration"),
NM("GTCl","Gyro Trim Calibration"),
NM("GYTT","Gyro Temp. Calibration"),
NM("LCM#","Liquid Crystal Monitor Serialnumber (LCD)"),
NM("LSCI","Ambient Lightsensor Calibration"),
NM("LTAO","Low Temperature Accelerometer Offset"),
NM("MLB#","Main Logicboard Serialnumber"),
NM("MdlC","Murata WiFi Configuration"),
NM("MkBH","Marketing Hardware Behavior"),
NM("Mod#","Model number"),
NM("MtCl","Multitouch Calibration"),
NM("MtSN","Multitouch Serialnumber"),
NM("NFCl","Stockholm NFC Calibration"),
NM("NSrN","Touch-ID Serial Number"),
NM("NoCl","---? Calibration"),
NM("NvSn","Apple SandDollar SerialNumber"),
NM("OFCl","Orb Force Calibration"),
NM("OOCl","???"),
NM("OrbC","Orb Calibration"),
NM("OrbG","Orb Gap Calibration"),
NM("OrbM","???"),
NM("PACV","---?"),
NM("PrCL","Pearl calibration"),
NM("PRSq","---?"),
NM("PRTT","Pressure Temperature compensation Table"),
NM("PSCl","Halleffect Calibration"),
NM("PxCl","Proximity Calibration"),
NM("RMd#","Regulatory Model Number"),
NM("RxCL","Rosaline Calibration"),
NM("Regn","Region code"),
NM("SBVr","Software Bundle Version"),
NM("SDAC","??? Sound DAC"),
NM("SFCl","Bt Pico Calibration"),
NM("SIFC","Bt Inverse Filter Calibration"),
NM("SPPO","Pressure Offset Calibration"),
NM("SpCl","Speaker Calibration"),
NM("SrNm","Device Serialnumber"),
NM("STRB","Camera Strobe Color Calibration"),
NM("SwBh","Software Behaviour Bits"),
NM("TCal","Audio Actuator Calibration"),
NM("THPC","Turtle Hot Probe Calibration"),
NM("VBCA","Speaker Configuration"),
NM("VBST","Speaker Configuration"),
NM("VPBR","Speaker Configuration"),
NM("W24R","Wifi Receiver 2.4Ghz Calibration"),
NM("WCAL","Wifi Calibration"),
NM("WMac","Wifi mac address"),
NM("WRxT","Wifi Receiver temp. Calibration")
  };
  int nameCount = sizeof( names ) / sizeof( NameEntry * );
  devUp( device );
  
  CFDataRef data = AMDeviceCopyValue( device, CFSTR("com.apple.mobile.internal"), CFSTR("SysCfgData") );
  //cfdump( 0, val );
  unsigned long len = CFDataGetLength( data );
  CFRange range = CFRangeMake(0,len);
  unsigned char *bytes = malloc( len );
  CFDataGetBytes( data, range, (UInt8 *) bytes );
  
  bswap( bytes );
  printf("%.4s\n", bytes );
  
  for( int i=0;i<25;i++ ) {
    printf("%02i ", i );
  }
  printf("\n");
  for( int i=0;i<25;i++ ) {
    printf("%02x ", *( bytes + i ) );
  }
  
  printf("\n");
  
  //bswap( (char *) bytes + 12 );
  printf("Version: %s\n", hex4( bytes + 12 ) );
  
  //bswap( (char *) bytes + 20 );
  printf("Key count: %s\n", hex4( bytes + 20 ) );
  uint32_t keyCount = * ( (uint32_t *) ( (unsigned char *) bytes + 20 ) );
  printf("Key count: %i\n", keyCount );
  
  uint32_t total = 0x18 + 0x14 * keyCount;
  uint32_t dataStart = total;
  for( int e=0;e<keyCount;e++ ) {
    unsigned char *entry = ( e * 0x14 ) + 0x18 + bytes;
    for( int i=0;i<0x14;i++ ) {
      printf("%02i ", i );
    }
    printf("\n");
    for( int i=0;i<0x14;i++ ) {
      printf("%02x ", *( entry + i ) );
    }
    printf("\n");
    uint32_t entryLen = * ( (uint32_t *) ( (char *) entry + 8 ) );
    uint32_t entryStart = * ( (uint32_t *) ( (char *) entry + 12 ) );
    //if( entryLen == 0 ) entryLen = 4;
    
    //if( entry[0] == 0x42 && entry[1] == 0x54 )
    //if( *(entry + 19) ) 
    //  total += entryLen;
    //else
    //  total -= 4;
    if( !entryStart || ( entryStart >= dataStart && entryStart <= len ) ) total += entryLen;
    
    bswap( entry );
    bswap( entry + 4 );
    
    unsigned char *namepos = entry;
    if( entry[0]=='C' && entry[1] == 'N' ) {
      namepos = entry + 4;
    }
    
    printf("Name:%.4s", namepos );
    char name4[5]="    ";
    sprintf(name4,"%.4s", namepos );
    char found = 0;
    for( int i=0;i<nameCount;i++ ) {
      NameEntry *entry = names[i];
      if( !strcmp( entry->shortx, name4 ) ) {
        printf(" -%s", entry->longx );
        found = 1;
      }
    }
    printf("\n");
    if( !found ) {
      printf("\n\n---- %s ----\n\n", name4);
    }
    
    printf("Length: %u\n", entryLen );
    printf("Start: %u\n", entryStart );
  }
  printf("\n");
  printf("Total: %lu\n", (unsigned long) total );
  printf("Size: %lu\n", (unsigned long) len );
  printf("Diff: %i\n", (int) (total - len) );
  devDown( device );
  
  exit(0);
}