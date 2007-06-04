/*
 *  AutoStar.h
 *  AutoStar
 *
 *  Created by roro on 11/16/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#include <Carbon/Carbon.h>
#include "Autostar/Autostar.h"

#include "controls.h"
// #define __TEST
#define __COM_DEBUG

#define BLOCKSIZE	0x40
#define PAGESIZE	32768

// constant
static const OSType    kApplicationSignature  = FOUR_CHAR_CODE('Astr');
static const struct HICommand quitCommand={0, kHICommandQuit, {0,0} }; 

#define kHIObjectThreadControllerClassID	CFSTR("com.apple.sample.dts.HIObjectThreadController")

/* ControlKind*/
enum {
	kControlKindHIThreadPane = 'thrp'
};

/* EventClass*/
enum {
	kEventClassHIObjectThreadController = 'thrc'
};

/* Thread Object Events*/
enum {
	kEventUpdateThreadUI		= 'UPDT',
	kEventTerminateThread	= 'TERM'
};


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

static EventTypeSpec kFactoryEvents[] =
    {
        { kEventClassHIObject, kEventHIObjectConstruct },
        { kEventClassHIObject, kEventHIObjectDestruct },
        { kEventClassHIObject, kEventHIObjectInitialize },
        { kEventClassHIObjectThreadController, kEventUpdateThreadUI },
        { kEventClassHIObjectThreadController, kEventTerminateThread }
    };
                    
typedef struct ROM_header {
	// header
	Byte flag;
	Byte origin[3];
	unsigned long checksum; // 4 byte
	Byte version[64];
	Byte filler[4024];
} ROM_header;


typedef CALLBACK_API_C( void * , SetUpProc )(void *);
typedef CALLBACK_API_C( void, TermProc )(void *);

typedef struct
	{
	HIObjectRef hiObject;
	MPTaskID	taskID;
	SetUpProc	setUpProc;
	TaskProc	entryPoint;
	TermProc	termProc;
	void *		parameters;
	HIViewRef	hiThreadPane;
    void *myClass;
	} ThreadControllerData;

typedef struct
	{
	EventTargetRef threadControllerTarget;
	SInt32 progress;
	SInt32 page;
	SInt32 maxPage;
    void *myClass;
	} GeneralTaskWorkParams, *GeneralTaskWorkParamsPtr;

     
// main structure for data storage

class AutoStarX
{

public:
    AutoStarX();
    virtual ~AutoStarX();

    // CallBack Functions
    static pascal OSStatus commandHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);
    static pascal OSStatus windowHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);
    static pascal OSStatus aboutWindowHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);
    static pascal OSStatus ThreadControllerHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon);
    
    static pascal OSStatus Flash(void *userData);
    static pascal void *setupFlash(void *p);
    static pascal void termFlash(void *p);

    virtual void UpdateUI(ThreadControllerData * myData);
    
	Autostar m_autostar;
	
private:
	
    // other function
    virtual void InitializeControls();
    virtual void SetSerialPortsControls(ControlRef control);
    virtual void AutoStarConnect();
    virtual void AutoStarDisconnect();
    virtual void AutoStarReset();
    virtual void ErrorAlert(CFStringRef error);
    virtual void ErrorAlert(CFStringRef error, CFStringRef comment);
    virtual void ActivateControls();
    virtual void DeActivateControls();
 
    
    virtual OSStatus HIObjectThreadControllerCreate(AutoStarX *mySelf,
                                                    SetUpProc inSetUpProc,
                                                    TaskProc inEntryPoint,
                                                    TermProc inTermProc,
                                                    SInt32 inKnownEnd,
                                                    HIViewRef * outHIThreadPane,
                                                    HIObjectRef * outHIObjectThreadController);

    virtual OSStatus HIObjectThreadControllerStartThread(HIObjectRef threadController);
    virtual void HIObjectThreadControllerTermThread(HIObjectRef threadController);
    virtual CFStringRef GetThreadControllerClass();
    virtual void SendEventToUI(UInt32 kind, GeneralTaskWorkParamsPtr params, SInt32 progress, SInt32 page);
	
	virtual int loadROMFile();

	virtual void SetASType();	// set type of Autostar (1 or 2)

// Nib reference
    IBNibRef 		mNibRef;	
// Window
	WindowRef 		mWindow;
	WindowRef 		mAbout;
	
	EventHandlerUPP	mWindowProcHandler;
	EventHandlerUPP	mControlProcHandler;
	EventHandlerUPP	mAboutProcHandler;
    EventHandlerUPP mThreadControlerProcHandler;
    
// dialog control Ref
	ControlRef	mSerialPort;
	ControlRef	mRomFile;
	ControlRef	mAStarVersion;
	ControlRef	mRomVersion;
	ControlRef	mFlashButton;
	ControlRef	mConnectButton;
	ControlRef	mFileRomVersion;
	ControlRef	mQuit;
	ControlRef	mRomOpen;

// HIView controls
    HIViewRef   mProgressPane;
	ControlRef	mFlashStatus;
	ControlRef	mFlashProgress;
    
    
    //Serial port data
    CFMutableArrayRef   mPortArray;
    SerialPort  *mPorts;
    
    bool    bConnected;
    bool    bFilename;
    bool    bFlashing;

	// ROM data to be flashed.
	Byte *newRom;
	ROM_header *romHeader;
	
    int romType;
    int deviceType;
    
// internal data
	FSSpec mFileSpec;
	SInt16 mROMFileHandle;
	FSRef mROMfileRef;
	char *mRomFullPath;
    int mfSize;
	int romSize;
	struct stat f_stat;
	int nbPages;
	int startPage;
	ASType m_ASType;
	
};


