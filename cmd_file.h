// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdint.h>
void run_getfile( ucmd *cmd );
char writeDataToAppFile( void *device, char *bundleid, char *remoteFile, uint8_t *data, uint32_t len );
