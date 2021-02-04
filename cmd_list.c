// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

void runList( void *device );
void run_list( ucmd *cmd ) {
  g_cmd = cmd;
  setupTimeout(1);
  waitForConnect( runList );
}

void runList( void *device ) {
  char g_json = ucmd__get( g_cmd, "-json" ) ? 1 : 0;
  
  CFStringRef devId = AMDeviceCopyDeviceIdentifier( device );
  char *udid = str_cf2c( devId );
  if( g_json ) {
    printf("[udid:\"%s\"],\n", udid );
  } else {
    fprintf(stderr,"udid:%s\n", udid );
  }
}