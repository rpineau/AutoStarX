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

static EventTypeSpec AboutWindowEventList[] =
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

    // CallBack Functions
    static pascal OSStatus commandHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);
    static pascal OSStatus windowHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);
    static pascal OSStatus aboutWindowHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);

    
private:

// Nib reference
    IBNibRef 		mNibRef;	
// Window
	WindowRef 		mWindow;
	WindowRef 		mAbout;
	
	EventHandlerUPP	mWindowProcHandler;
	EventHandlerUPP	mControlProcHandler;
	EventHandlerUPP	mAboutProcHandler;

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
    SerialPort  *mPorts;
   
    bool    bConnected;
    bool    bFilename;

	// ROM data to be flashed.
	
	ROM *newRom;

// internal data
	FSSpec mFileSpec;
	short mROMFileHandle;
	FSRef mROMfileRef;
	UInt8 *mRomFullPath;
    long mfSize;
	
    // other function
    virtual void InitializeControls();
    virtual void SetSerialPortsControls(ControlRef control);
    virtual void AutoStarConnect();
    virtual void AutoStarDisconnect();
    virtual void ErrorAlert(CFStringRef error);
    virtual void Flash();
    static pascal void FlashThread(void *userData);
    static pascal void MainRunLoopForThreadedApps( EventLoopTimerRef inTimer, void *inUserData );
	
};