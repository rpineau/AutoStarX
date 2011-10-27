/*
 *  Serial.c
 *  AutoStar
 *
 *  Created by roro on 11/15/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "SerialPort.h"

//
// Constructor
//
SerialPortIO::SerialPortIO(const char *path)
{
    bsdPath=(char *)(malloc(strlen(path)+1));
    if(bsdPath!=NULL)
        strcpy(bsdPath,path);
    
    mSerialPortHandle=0;
    
}

//
// Destructor free mmemory allocations of the port array
//
SerialPortIO::~SerialPortIO()
{
    if(mSerialPortHandle=!-1)
        CloseSerialPort();
    mSerialPortHandle=-1;
    
}




//
// Given the path to a serial device, open the device and configure it.
// Return true if the device opened ok.
//
bool SerialPortIO::OpenSerialPort(const char *bsdPath, int speed)
{
    int fileDescriptor = -1;
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
		return false;
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
    options.c_cflag &= ~(PARENB);				// clear parity enable
    options.c_cflag &= ~(CSTOPB);				// one stop bit
    options.c_cflag &= ~CSIZE;					// Mask the character size bits
    options.c_cflag |= (CS8 );					// 8bit,

    options.c_cflag &= ~CRTSCTS;				// no hw flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);	// no sw flow control
	
	options.c_cflag |= ( CREAD | CLOCAL);		// enable receiver, local line
    options.c_iflag &= ~(INPCK);				// disable input parity checkinga
    options.c_iflag |= IGNBRK;					// ignore break
    
    // RAW outout and input
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	    
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
void SerialPortIO::CloseSerialPort()
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
    mSerialPortHandle=-1;
}



//
// read "length" byte from the port.
// return true if all data were read, false on error or timeout
// 
bool SerialPortIO::ReadData(Byte * dataIn, int length)
{
    
    int nByte;
    int totalRead;
    int timeout;
    Byte *bufptr;

    bufptr=dataIn;
    totalRead=0;
    timeout=0;
    nByte=0;
    while( (nByte=read(mSerialPortHandle,bufptr,length-totalRead))<length)
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
bool SerialPortIO:: SendData(Byte * dataOut, int length)
{
    
    int nByte;
    int totalWriten;
    int timeout;
    Byte *bufptr;

    bufptr=dataOut;
    totalWriten=0;
    timeout=0;
    nByte=0;
    // send the data
	while( (nByte=write(mSerialPortHandle,bufptr,length-totalWriten))<length)
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

	return true;
}
 
 
bool SerialPortIO::SendData(unsigned char *out, unsigned int outcnt, unsigned char *in, UInt32 &incnt)
{
	bool res;
	
	res=SendData(out,outcnt);
	if(!res)
		return res;
		
	usleep(500000);
	
	res=ReadData(in,incnt);
	
	return res;
	
}

//
// Change the speed of an open connection
//
bool SerialPortIO::SetSpeed(int speed)
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
