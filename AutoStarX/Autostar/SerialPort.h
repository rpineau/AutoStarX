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

enum eSerialStat {COM_OK, BAD_PORT, NO_RESPONSE};

class SerialPortIO
{

public:
    SerialPortIO(const char *bsdPath);
    virtual ~SerialPortIO();
    
    virtual bool OpenSerialPort(const char *bsdPath, int speed);
    virtual void CloseSerialPort();
    virtual bool SetSpeed(int speed);
    virtual bool SendData(Byte * dataOut, int length);
    virtual bool ReadData(Byte * dataIn, int length);
    virtual bool SendData(unsigned char *out, unsigned int outcnt, unsigned char *in, UInt32 &incnt);
    
private:
    // Hold the original termios attributes so we can reset them
    struct termios mOriginalTTYAttrs;
    int mSerialPortHandle;
    char *bsdPath;
};
