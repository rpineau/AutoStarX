/*
 *  AutoStar.h
 *  AutoStar
 *
 *  Created by roro on 11/16/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>

#include "controls.h"

// constant
static const OSType    kApplicationSignature  = FOUR_CHAR_CODE('Astr');
static const struct HICommand quitCommand={0, kHICommandQuit, {0,0} }; 
//Data
static EventTypeSpec ControlEventList[] =
		{
		{kEventClassCommand,kEventProcessCommand},
		};			

static EventTypeSpec WindowEventList[] =
		{
		{kEventClassWindow,kEventWindowClosed},
		};			

typedef struct ROM {
	// header
	Byte key[4];
	Byte checksum[4];
	Byte version[4];
	Byte padding[4084];
	// data pages
	Byte pages[32][32768];
	
} ROM;


 
// main structure for data storage

class AutoStarX {

public:
    AutoStarX();
    virtual ~AutoStarX();
    // virtual OSStatus initNIB();
    // virtual void run();

    // CallBack Functions
    static pascal OSStatus buttonHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);
    static pascal OSStatus windowHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);


    
private:
// internal data
	FSSpec mFileSpec;
	short mROMFileHandle;
	FSRef mROMfileRef;
	UInt8 mRomFullPath[255];
    long mfSize;
// Window
	WindowRef 		mWindow;
	EventHandlerUPP	mWindowProcHandler;
	EventHandlerUPP	mControlProcHandler;

// timer & thread
    EventLoopTimerRef mTimer;
    ThreadID mFlashthreadID;
    SInt32  mNumberOfRunningThreads;
    bool    mThreadDone;
    
// dialog control Ref
	ControlRef	mSerialPort;
	ControlRef	mRomFile;
	ControlRef	mAStarVersion;
	ControlRef	mRomVersion;
	ControlRef	mFlashStatus;
	ControlRef	mFlashProgress;
	ControlRef	mFlashButton;
	ControlRef	mConnectButton;
	ControlRef	mFileRomVersion;
	ControlRef	mQuit;
	ControlRef	mRomOpen;
    
    
    //Serial port data
    CFMutableArrayRef   mPortArray;
    SerialPort  mPorts;
   
    bool    bConnected;
    bool    bFilename;

	// ROM data to be flashed.
	
	ROM *newRom;
	
    // other function
    virtual void InitializeControls();
    virtual void SetSerialPortsControls(ControlRef control);
    virtual void AutoStarConnect();
    virtual void AutoStarDisconnect();
    virtual void ErrorAlert(CFStringRef error);
    virtual void Flash();
    static pascal void FlashThread(void *userData);
    static	pascal	void	MainRunLoopForThreadedApps( EventLoopTimerRef inTimer, void *inUserData );
	
};