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
        
exit:
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
                                                            CFSTR(kIOCalloutDeviceKey),
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
// Given the path to a serial device, open the device and configure it.
// Return true if the device opened ok.
//
bool SerialPort::OpenSerialPort(const char *bsdPath, int speed)
{
    int 		fileDescriptor = -1;
    int 		handshake;
    struct termios	options;
    
    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    // See open(2) ("man 2 open") for details.
    
    fileDescriptor = open(bsdPath, O_RDWR | O_NOCTTY | O_NDELAY );
    if (fileDescriptor == -1)
		{
        printf("Error opening serial port %s - %s(%d).\n",
               bsdPath, strerror(errno), errno);
		if (fileDescriptor != -1)
			close(fileDescriptor);    
		return -1;
		}

    // Note that open() follows POSIX semantics: multiple open() calls to the same file will succeed
    // unless the TIOCEXCL ioctl is issued. This will prevent additional opens except by root-owned
    // processes.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
    
    if (ioctl(fileDescriptor, TIOCEXCL) == -1)
		{
        printf("Error setting TIOCEXCL on %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
		if (fileDescriptor != -1)
			close(fileDescriptor);    
		return false;
		}
    
    
    // Get the current options and save them so we can restore the default settings later.
    if (tcgetattr(fileDescriptor, &mOriginalTTYAttrs) == -1)
		{
        printf("Error getting tty attributes %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
		if (fileDescriptor != -1)
			close(fileDescriptor);    
		return false;
		}

    // The serial port attributes such as timeouts and baud rate are set by modifying the termios
    // structure and then calling tcsetattr() to cause the changes to take effect. Note that the
    // changes will not become effective without the tcsetattr() call.
    // See tcsetattr(4) ("man 4 tcsetattr") for details.
    
    options = mOriginalTTYAttrs;
        
    // Set raw input (non-canonical) mode, with reads blocking until either a single character 
    // has been received or a one second timeout expires.
    // See tcsetattr(4) ("man 4 tcsetattr") and termios(4) ("man 4 termios") for details.
    
    cfmakeraw(&options);
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;       // timeout is 10 x 1/10th of a second .. ie 1 seconde
        
    // The baud rate, word length, and handshake
    
    cfsetspeed(&options, speed); 	// Set speed  
    
	// Use 8N1
    options.c_cflag &= ~CSIZE;					// Mask the character size bits
    options.c_cflag &= ~(PARENB);				// clear parity enable
    options.c_cflag &= ~(CSTOPB);				// one stop bit
    options.c_cflag |= (CS8 | CREAD | CLOCAL);	// 8bit, enable receiver, local line
    options.c_iflag &= ~(IXON | IXOFF | IXANY);			// no sw flow control
    options.c_iflag &= ~(INPCK);				// disable input parity checking
    options.c_iflag |= IGNBRK;					// ignore break
    options.c_cflag &= ~CRTSCTS;				// no hw flow control
	    
    // Cause the new options to take effect immediately.
    if (tcsetattr(fileDescriptor, TCSANOW, &options) == -1)
		{
        printf("Error setting tty attributes %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
		if (fileDescriptor != -1)
			close(fileDescriptor);    
		return false;
		}

    // Set the opened device to non blocking mode  so subsequent I/O will no block.
    // See fcntl(2) ("man 2 fcntl") for details.
    
    if (fcntl(fileDescriptor, F_SETFL, FNDELAY) == -1)
		{
        printf("Error setting FNDELAY %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
		if (fileDescriptor != -1)
			close(fileDescriptor);    
		return -1;
		}
    
        // Success
    mSerialPortHandle=fileDescriptor;
    return true;
}


//
// Given the file descriptor for a serial device, close that device.
//
void SerialPort::CloseSerialPort()
{
    // Block until all written output has been sent from the device.
    // Note that this call is simply passed on to the serial device driver.
    // See tcsendbreak(3) ("man 3 tcsendbreak") for details.
    if (tcdrain(mSerialPortHandle) == -1)
    {
        printf("Error waiting for drain - %s(%d).\n",
            strerror(errno), errno);
    }
    
    // Traditionally it is good practice to reset a serial port back to
    // the state in which you found it. This is why the original termios struct
    // was saved.
    if (tcsetattr(mSerialPortHandle, TCSANOW, &mOriginalTTYAttrs) == -1)
    {
        printf("Error resetting tty attributes - %s(%d).\n",
            strerror(errno), errno);
    }

    close(mSerialPortHandle);
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


//
// read "length" byte from the port.
// return true if all data were read, false on error or timeout
// 
bool SerialPort::ReadData(char * dataIn, int length)
{
    
    int nByte;
    int totalRead;
    int timeout;
    char *bufptr;

    bufptr=dataIn;
    totalRead=0;
    timeout=0;
    nByte=0;
    while( (nByte=read(mSerialPortHandle,bufptr,length-nByte))<=length)
        {
        bufptr+=nByte;
        totalRead+=nByte;
        if(length == totalRead)
            break;
        if(!nByte)
            {
            usleep(100000); // 1/10th of a seconde
            timeout++;
            }
        else
            timeout=0;
        
        if(timeout==10)
            return false;
        }

    return true;
}


//
// Send "length" byte data present in dataOut 
// return true if data was correctly send
// 
bool SerialPort:: SendData(char * dataOut, int length)
{
    
    int nByte;
    int totalWriten;
    int timeout;
    char *bufptr;

    bufptr=dataOut;
    totalWriten=0;
    timeout=0;
    nByte=0;
    // send the data
	while( (nByte=write(mSerialPortHandle,bufptr,length-nByte))<=length)
		{
		if (nByte<0)
			{
			printf("Error writing data %s(%d).\n", strerror(errno), errno);
			if (mSerialPortHandle != -1)
				close(mSerialPortHandle);    
			return false;
			}
        bufptr+=nByte;
        totalWriten+=nByte;
        if(length == totalWriten)
            break;
        if(!nByte)
            {
            usleep(100000); // 1/10th of a seconde
            timeout++;
            }
        else
            timeout=0;
        
        if(timeout==10)
            return false;
		
		}
    tcdrain(mSerialPortHandle); 

	return true;
}
 
 
 
//
// Change the speed of an open connection
//
bool SerialPort::SetSpeed(int speed)
{
    struct termios	options;
    
    
    // Get curent configuration
    if (tcgetattr(mSerialPortHandle, &options) == -1)
		{
        printf("Error getting tty attributes %s(%d).\n",strerror(errno), errno);
		return false;
		}

    cfsetspeed(&options, speed); 	// Set speed  to new value

    // Cause the new options to take effect immediately.
    if (tcsetattr(mSerialPortHandle, TCSANOW, &options) == -1)
		{
        printf("Error setting tty attributes for new speed %s(%d).\n", strerror(errno), errno);
		return false;
		}

    return true;
    
}
