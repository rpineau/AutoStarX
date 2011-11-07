/*
 *  Serial.h
 *  AutoStar
 *
 *  Created by roro on 11/15/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>


#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>


typedef struct ASSerialPort {
    CFStringRef   PortName;
    CFStringRef   PortPath;
} ASSerialPort;

class SerialPort
{

public:
    SerialPort();
    virtual ~SerialPort();
    
    virtual OSErr FindPorts(io_iterator_t *matchingServices);
    virtual CFMutableArrayRef GetPortList(io_iterator_t serialPortIterator);
    virtual bool OpenSerialPort(const char *bsdPath, int speed);
    virtual void CloseSerialPort();
    virtual SInt32 getCount();
    virtual CFStringRef getPortName(int index);
    virtual CFStringRef getPortPath(int index);
    virtual bool SetSpeed(int speed);
    virtual bool SendData(Byte * dataOut, int length);
    virtual int ReadData(Byte * dataIn, int length);
    
    //public data
    CFMutableArrayRef   mPortArray;
    
private:
    // Hold the original termios attributes so we can reset them
    struct termios mOriginalTTYAttrs;
    int mSerialPortHandle;
};
