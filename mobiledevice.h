// Copyright (c) 2021 David Helkowski
// Public Domain
// ( it is headers discovered by dumping the MobileDevice framework )

#ifndef __MOBILEDEVICE_H
#define __MOBILEDEVICE_H
//extern void AMDSetLogLevel(int level);
extern int         AMDeviceConnect(              void *device );
extern int         AMDeviceDisconnect(           void *device );
extern int         AMDeviceValidatePairing(      void *device );
extern int         AMDeviceStartSession(         void *device );
extern int         AMDeviceStopSession(          void *device );
unsigned int       AMDeviceGetConnectionID(      void *device );
extern CFStringRef AMDeviceCopyDeviceIdentifier( void *device );

extern int AMDeviceStartService(          void *device, CFStringRef service_name, void *handle);
extern int AMDeviceSecureStartService(    void *device, CFStringRef serviceName, CFDictionaryRef flags, void *handle);
extern int AMDeviceNotificationSubscribe( void *device, int , int , int, void ** );
extern CFTypeRef AMDeviceCopyValue(       void *device, CFStringRef domain, CFStringRef cfstring );

extern int AMDServiceConnectionReceiveMessage( void *service, CFPropertyListRef message, CFPropertyListFormat *format );
extern int AMDServiceConnectionReceive(        void *service, char *buf, size_t size );
extern int AMDServiceConnectionSendMessage(    void *service, CFPropertyListRef message, CFPropertyListFormat format);
extern int AMDServiceConnectionSend(           void *service, const void *message, size_t length);
extern int AMDServiceConnectionGetSocket(      void *service );
extern int AMDServiceConnectionInvalidate(     void *service );

extern int AMDeviceSecureTransferPath(       int, void *device, CFURLRef, CFDictionaryRef, void *, int);
extern int AMDeviceSecureInstallApplication( int, void *device, CFURLRef, CFDictionaryRef, void *, int);
extern int AMDeviceSecureUninstallApplication( void *service, void * dunno, CFStringRef bundleIdentifier, CFDictionaryRef params, void (*installCallback)(CFDictionaryRef, void *));
//extern int AMDeviceSecureInstallApplicationBundle(void *device, CFURLRef, CFDictionaryRef params, void (*installCallback)(CFDictionaryRef, void *));

typedef struct muxListenIn_s muxListenIn;
typedef struct muxListenOut_s muxListenOut;
struct muxListenIn_s {
    unsigned int x0;
    unsigned char *x1;
    void *callback; // _AMDDeviceAttached
    unsigned int x3;                  
    unsigned int x4;                  
    unsigned int x5;                 
};
struct muxListenOut_s {
    unsigned char x0[4144];
};
extern int          USBMuxConnectByPort( int connectionID, int netBytesOrder, int *outHandle );
extern unsigned int USBMuxListenerCreate( muxListenIn *in, muxListenOut **out );
extern unsigned int USBMuxListenerHandleData( void * );

typedef struct AMDeviceNotificationCallbackInformation {
  void     *device;
  uint32_t type; // ADNCI_MSG_
} noticeInfo;
#endif