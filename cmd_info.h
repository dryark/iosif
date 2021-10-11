#ifndef __INFO_H
#define __INFO_H
void run_info( ucmd *cmd );
void run_ios_version( ucmd *cmd );

typedef struct {
  int major;
  int medium;
  int minor;
} iosVersion;

iosVersion getIosVersion( void *device );
#endif