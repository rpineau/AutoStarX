/* 
	function prototype and data
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curses.h>
#include <signal.h>


#ifndef __BASIC_TYPE__
#define __BASIC_TYPE__
typedef enum { false = 0, true = 1 } bool;
typedef unsigned char Byte;
#endif

#define BLOCKSIZE 64

typedef struct ROM {
	// header
	unsigned long key;  // 4 byte
	unsigned long checksum; // 4 byte
	Byte version[4];
	Byte padding[4084];
	// data pages
	Byte pages[96][32768];  // 32 pages for 495 & 495 , 96 pages for autostar II
} ROM;

struct stat Stat;

ROM *newRom;
int romType;
int deviceType;
bool bConnected;
int mROMFileHandle;

int main(int argc, char **argv);
void AutoStarConnect(char *serialPort);
void AutoStarDisconnect();
void AutoStarReset();
void Flash();
static void finish(int sig);

