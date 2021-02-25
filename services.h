// Copyright (c) 2021 David Helkowski
// Anti-Corruption License
#ifndef __SERVICES_H
#define __SERVICES_H
#include"service.h"

void *activateService( void *device, CFTypeRef serviceName, CFDictionaryRef options );

void *activateSyslog(            void *device );
void *activateScreenshotService( void *device );
void *activateDiagService(       void *device );

serviceT *service__new_instruments( void *device );
serviceT *service__new_testmanagerd( void *device );
serviceT *service__new_remoteautomation( void *device );
#endif