/*
 *  Serial.h
 *  AutoStar
 *
 *  Created by roro on 11/15/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

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
    virtual SInt32 getCount();
    virtual CFStringRef getPortName(int index);
    virtual CFStringRef getPortPath(int index);
    
    //public data
    CFMutableArrayRef   mPortArray;
};
