/*
 *  Serial.c
 *  AutoStar
 *
 *  Created by roro on 11/15/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "Serial.h"

//
// Constructor
//
SerialPort::SerialPort()
{
}

//
// Destructor free mmemory allocations of the port array
//
SerialPort::~SerialPort()
{
    int i;
    int numItems;
    
    // free the PortArray data alocation
    
    numItems=getCount();
    for(i=0;i<numItems;i++)
        {
        CFRelease(getPortName(i));
        CFRelease(getPortPath(i));
        delete ( ASSerialPort *)CFArrayGetValueAtIndex(mPortArray,i);

        }
}


//
// Returns an iterator across all known Ports. Caller is responsible for
// releasing the iterator when iteration is complete.
//
OSErr SerialPort::FindPorts(io_iterator_t *matchingServices)
{
    kern_return_t		kernResult; 
    mach_port_t			masterPort;
    CFMutableDictionaryRef	classesToMatch;

    // function IOMasterPort
    // Returns the mach port used to initiate communication with IOKit.

    kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (KERN_SUCCESS != kernResult)
        {
        printf("IOMasterPort returned %d\n", kernResult);
        return kernResult;
        }
        
    // function IOServiceMatching
    // Create a matching dictionary that specifies an IOService class match.

    // Serial devices are instances of class IOSerialBSDClient
    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classesToMatch == NULL)
        {
        printf("IOServiceMatching returned a NULL dictionary.\n");
        }
    else {
    //function CFDictionarySetValue
	// Sets the value of the key in the dictionary.
        CFDictionarySetValue(classesToMatch,
                             CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDRS232Type));
        }
    
        // function IOServiceGetMatchingServices
        // Look up registered IOService objects that match a matching dictionary.

    kernResult = IOServiceGetMatchingServices(masterPort, classesToMatch, matchingServices);    
    if (KERN_SUCCESS != kernResult)
        {
        printf("IOServiceGetMatchingServices returned %d\n", kernResult);
        }
	return kernResult;
}


//    
// Given an iterator across a set of modems, return the BSD path to the first one.
// If no modems are found the path name is set to an empty string.
//
CFMutableArrayRef SerialPort::GetPortList(io_iterator_t serialPortIterator)
{
    io_object_t		portService;
    
    CFStringRef	portNameAsCFString;
    CFStringRef	bsdPathAsCFString;
    
    ASSerialPort *asSerialPort;
    
    
    //create an empty array
    mPortArray=CFArrayCreateMutable(kCFAllocatorDefault,0,NULL);
    
    
    // Iterate across all ports found.
    
    while (portService = IOIteratorNext(serialPortIterator))
        {

        // Get the callout device's path (/dev/cu.xxxxx). The callout device should almost always be
        // used: the dialin device (/dev/tty.xxxxx) would be used when monitoring a serial port for
        // incoming calls, e.g. a fax listener.
        
        portNameAsCFString = (CFStringRef)IORegistryEntryCreateCFProperty(portService,
                                                              CFSTR(kIOTTYDeviceKey),
                                                              kCFAllocatorDefault,
                                                              0);
        bsdPathAsCFString = (CFStringRef)IORegistryEntryCreateCFProperty(portService,
                                                            CFSTR(kIODialinDeviceKey),		//kIOCalloutDeviceKey
                                                            kCFAllocatorDefault,
                                                            0);
		// create a MutableArray with the names of the port (System and bsd path)
        if (portNameAsCFString && bsdPathAsCFString)
            {
            asSerialPort=new ASSerialPort;  // Don't forget to delete these in the destructor
        
			asSerialPort->PortName=CFStringCreateCopy(kCFAllocatorDefault, portNameAsCFString);
            asSerialPort->PortPath=CFStringCreateCopy(kCFAllocatorDefault, bsdPathAsCFString);
            
            CFArrayAppendValue(mPortArray,asSerialPort);
            }

		// Release the stringRef
        if (portNameAsCFString)
            CFRelease(portNameAsCFString);
            
        if (bsdPathAsCFString)
            CFRelease(bsdPathAsCFString);
        
		// Release the io_service_t now that we are done with it.
        IOObjectRelease(portService);
        }
        
    return mPortArray;
}




//
// Get the number of port in the PortArray (so on the machine)
//
SInt32 SerialPort::getCount()
{
    return CFArrayGetCount(mPortArray);
}

//
// Get the system port name of the device at index
//
CFStringRef SerialPort::getPortName(int index)
{
    return (((ASSerialPort *)CFArrayGetValueAtIndex(mPortArray,index))->PortName);
}

//
// get the BSD path of the device at index
//
CFStringRef SerialPort::getPortPath(int index)
{
    return (((ASSerialPort *)CFArrayGetValueAtIndex(mPortArray,index))->PortPath);
}


