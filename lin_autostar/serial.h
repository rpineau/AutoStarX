/* 
	serial port access functions and data
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

#ifndef __BASIC_TYPE__
#define __BASIC_TYPE__
typedef enum { false = 0, true = 1 } bool;
typedef unsigned char Byte;
#endif

// Hold the original termios attributes so we can reset them
struct termios mOriginalTTYAttrs;
int mSerialPortHandle;


bool OpenSerialPort(const char *bsdPath, int speed);
void CloseSerialPort();
bool SetSpeed(int speed);
bool SendData(Byte * dataOut, int length);
bool ReadData(Byte * dataIn, int length);
