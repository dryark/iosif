// Copyright (c) 2021 David Helkowski
// Public Domain
// ( it is headers discovered by dumping the MobileDevice framework )

//extern void AMDSetLogLevel(int level);
extern int AMDeviceConnect(void *device);
extern int AMDeviceDisconnect(void *device);
extern int AMDeviceValidatePairing(void *device);
extern int AMDeviceStartSession(void *device);
extern int AMDeviceStopSession(void *device);
extern int AMDServiceConnectionReceive( void *service, char *buf,int size );
extern int AMDeviceStartService(void *device, CFStringRef service_name, void *handle);
extern int AMDeviceSecureStartService( void *device, CFStringRef serviceName, CFDictionaryRef flags, void *handle);
extern int AMDeviceNotificationSubscribe( void *, int , int , int, void ** );
extern CFStringRef AMDeviceCopyDeviceIdentifier( void * );
extern int AMDServiceConnectionReceiveMessage( void *service, CFPropertyListRef message, CFPropertyListFormat *format );
extern int AMDServiceConnectionSendMessage( void *service, CFPropertyListRef message, CFPropertyListFormat format);
extern int AMDServiceConnectionSend( void *service, const void *message, size_t length);
extern CFTypeRef AMDeviceCopyValue( void *device, CFStringRef domain, CFStringRef cfstring );
//extern int AMDServiceConnectionReceive( void *service, void *buffer, size_t size);

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
extern int USBMuxConnectByPort( int connectionID, int netBytesOrder, int *outHandle );
extern unsigned int USBMuxListenerCreate( muxListenIn *in, muxListenOut **out );
extern unsigned int USBMuxListenerHandleData( void * );
unsigned int AMDeviceGetConnectionID( void *device );

typedef struct AMDeviceNotificationCallbackInformation {
  void     *device;
  uint32_t type; // ADNCI_MSG_
} noticeInfo;
