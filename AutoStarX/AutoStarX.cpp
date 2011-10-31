//
//  main.c
//  AutoStarX
//
//  Created by roro on 11/15/04.
//  Copyright __MyCompanyName__ 2004. All rights reserved.
//

#include <Carbon/Carbon.h>

#include "FileSelector.h"
#include "Serial.h"

#include "AutoStarX.h"



int main(int argc, char* argv[])
{

    AutoStarX AutoStar;
    
}


AutoStarX::AutoStarX()
{
    
    OSStatus		err;
    
    // init data
    bConnected=false;
    bFilename=false;
    newRom=NULL;
    mRomFullPath=NULL;    
    bFlashing=false;    
    InitCursor();
    
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &mNibRef);
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(mNibRef, CFSTR("MenuBar"));
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(mNibRef, CFSTR("MainWindow"), &mWindow);
	
	
    
	// Init controls in window
	InitializeControls();
    	
	// Installing the event handlers
    
    // Window events handler
	mWindowProcHandler=NewEventHandlerUPP( &windowHandler);

	err=InstallEventHandler(GetWindowEventTarget(mWindow),
							mWindowProcHandler,
							GetEventTypeCount(WindowEventList),
							WindowEventList,
							this,
							NULL);

    
    // Control Events Handler
	mControlProcHandler=NewEventHandlerUPP(&commandHandler);
	
	err=InstallEventHandler(GetWindowEventTarget(mWindow),
							mControlProcHandler,
							GetEventTypeCount(ControlEventList),
							ControlEventList,
							this,
							NULL);

     

    // The window was created hidden so show it.
    ShowWindow(mWindow);

    // Call the event loop
    RunApplicationEventLoop();

	return;
}

AutoStarX::~AutoStarX()
{        
    if(bConnected)
        AutoStarDisconnect();
	if(newRom)
		delete newRom;
	if(mRomFullPath)
		delete mRomFullPath;
	
	delete mPorts;
}


pascal OSStatus AutoStarX::commandHandler(EventHandlerCallRef myHandler, EventRef event, void *userData)
{
	HICommandExtended commandStruct;
	UInt32	CommandID;
	OSErr err;
    bool wrong;
	OSStatus result=eventNotHandledErr;
	
	// get a pointer to the object itself to be able to access private member variable and functions
    AutoStarX* self = static_cast<AutoStarX*>(userData);
    
	// Get the event command
	GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand),NULL,&commandStruct);
	
	CommandID=commandStruct.commandID;

#ifdef __TEST
	printf("command ID %ul\n",CommandID);
#endif

	// execute the command
	switch(CommandID)
		{
		case 'open':  // get the file name and its size
				{
				self->loadAutostarROMFile();
				break;
				}
				
		case 'Port':
				result=noErr;
				break;

		case 'Flsh':  // start the flashing of the autostar
                if(self->deviceType!=0xFFFF && self->romType!=0xFFFF)
                    {
                    if(self->deviceType==self->romType)
                        {
                        wrong=false;
                        switch(self->deviceType)
                            {
                            case 1: // autostar 495 & 497
                                result=self->HIObjectThreadControllerCreate(self, setupFlash, Flash, termFlash, 967680, NULL, NULL);
                                if(result==noErr)
                                    {
                                    // set the progress bar max length and initial value 
                                    // page 0 and 1 aren't writen
                                    // the top 512 bytes of each page aren't writen
                                    // pages 30 and 31 are for garbage collection (pages $1E and $1F)
                                    // if page is all $FF, do not write
                                    // so we have 30 pages x 32K (minus the 512 bytes)
                                    // so 30x(32768-512) = 967680 byte to write
                                    SetControl32BitMaximum(self->mFlashProgress, 967680);
                                    self->DeActivateControls();
                                    }
                                break;
                            
                            case 2: // autostar II
                                wrong=false;
                                break;

                            default:
                                wrong=true;
                                break;
                            }
                        }
					else
						wrong=true;
                    }
                else
                    wrong=true;
                    
                if(wrong)
                    {
                    self->ErrorAlert(CFSTR("Wrong device or ROM file!"),CFSTR("The ROM file doesn't correspond to the autostar you're trying to flash. Please verify your ROM file."));
                    }
                    
                result=noErr;
				break;

		case 'Cnct': // connect to the AutoStarX on the selected serial port
                if(self->bConnected)
                    self->AutoStarDisconnect();
                else
                    self->AutoStarConnect();
				result=noErr;
				break;
        case 'quit' :
                if ( self->bFlashing )
                    {
                    result=noErr; //don't proces quit while flashing
                    }
                break;
		case 'abou':
                DisableMenuCommand( NULL, kHICommandAbout );
                err = CreateWindowFromNib(self->mNibRef, CFSTR("About"), &self->mAbout);
                self->mAboutProcHandler=NewEventHandlerUPP( &self->aboutWindowHandler);
                err=InstallEventHandler(GetWindowEventTarget(self->mAbout),
                                        self->mAboutProcHandler,
                                        GetEventTypeCount(AboutWindowEventList),
                                        AboutWindowEventList,
                                        self,
                                        NULL);

				ShowWindow (self->mAbout);
				SelectWindow (self->mAbout);    
				result=noErr;
				break;

		}
	return (result);

}


// window event hadler

pascal OSStatus AutoStarX::windowHandler(EventHandlerCallRef myHandler, EventRef event, void *userData)
{
    OSStatus result=eventNotHandledErr;
	// get a pointer to the object itself to be able to access private member variable and functions
    AutoStarX* self = static_cast<AutoStarX*>(userData);

	// if the window is closed, quit the application
    if(GetEventKind(event) == kEventWindowClosed)
        {
        if ( self->bFlashing )
            result=noErr; //don't proces quit while flashing
        result=ProcessHICommand(&quitCommand);
        }
    return result;
}

pascal OSStatus AutoStarX::aboutWindowHandler(EventHandlerCallRef myHandler, EventRef event, void *userData)
{
    OSStatus result=eventNotHandledErr;
	
	// get a pointer to the object itself to be able to access private member variable and functions
    AutoStarX* self = static_cast<AutoStarX*>(userData);

	// if the window is closed, quit the application
    if(GetEventKind(event) == kEventWindowClosed)
        {
        HideWindow(self->mAbout);
        EnableMenuCommand( NULL, kHICommandAbout );
        result=noErr;
        }
    return result;
}


// Creating our thread controller object
// This is mostly setting up the Initialization event with the parameters
OSStatus AutoStarX::HIObjectThreadControllerCreate(
	AutoStarX *myClass,
	SetUpProc inSetUpProc,
	TaskProc inEntryPoint,
	TermProc inTermProc,
	SInt32 inKnownEnd,
	HIViewRef * outHIThreadPane,
	HIObjectRef * outHIObjectThreadController)
{
    OSStatus status = noErr;
    EventRef theInitializeEvent = NULL;
    ThreadControllerData * myData = NULL;
    HIObjectRef hiObject;

    status = CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(), kEventAttributeUserEvent, &theInitializeEvent);
    status = SetEventParameter(theInitializeEvent, 'TCCL', typeVoidPtr, sizeof(myClass), &myClass);
    status = SetEventParameter(theInitializeEvent, 'TCSU', typeVoidPtr, sizeof(inSetUpProc), &inSetUpProc);
    status = SetEventParameter(theInitializeEvent, 'TCEP', typeVoidPtr, sizeof(inEntryPoint), &inEntryPoint);
    status = SetEventParameter(theInitializeEvent, 'TCTP', typeVoidPtr, sizeof(inTermProc), &inTermProc);
    status = SetEventParameter(theInitializeEvent, 'TCKE', typeSInt32, sizeof(inKnownEnd), &inKnownEnd);

    status = HIObjectCreate(GetThreadControllerClass(), theInitializeEvent, &hiObject);

    myData = (ThreadControllerData *) HIObjectDynamicCast(hiObject, kHIObjectThreadControllerClassID);

    if (theInitializeEvent != NULL)
        ReleaseEvent(theInitializeEvent);
    if (outHIThreadPane != NULL)
        *outHIThreadPane = myData->hiThreadPane;
    if (outHIObjectThreadController != NULL)
        *outHIObjectThreadController = hiObject;
    
    return status;
}



pascal OSStatus AutoStarX::ThreadControllerHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* userData)
{
    OSStatus	status = eventNotHandledErr;
    AutoStarX   *self;
    ThreadControllerData* myData = (ThreadControllerData*) userData;

    // get a pointer to the object itself to be able to access private member variable and functions
    if(myData)
        self = static_cast<AutoStarX*>(myData->myClass);


	switch (GetEventClass(inEvent))
		{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
				{
				case kEventHIObjectConstruct:
					{
					myData = (ThreadControllerData*) calloc(1, sizeof(ThreadControllerData));
					require_string((myData != NULL), exitHandler, "ThreadControllerHandler--kEventHIObjectConstruct--calloc ");

					// get our superclass instance
					HIObjectRef hiObject;
					status = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(hiObject), NULL, &hiObject);
					require_noerr_string(status, exitHandler, "ThreadControllerHandler--kEventHIObjectConstruct--GetEventParameter ");
                    GetEventParameter(inEvent, 'TCCL', typeVoidPtr, NULL, sizeof(myData->myClass), NULL, &myData->myClass);
					myData->hiObject = hiObject;
					myData->taskID = NULL;
					myData->hiThreadPane = NULL;
					myData->parameters = NULL;
					// store our instance data into the event
					status = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					require_noerr_string(status, exitHandler, "ThreadControllerHandler--kEventHIObjectConstruct--SetEventParameter ");
					}
					break;

				case kEventHIObjectInitialize:
					{
					// always begin kEventHIObjectInitialize by calling through to the previous handler
					status = CallNextEventHandler(inCaller, inEvent);
					// if that succeeded, do our own initialization
					if (status == noErr)
						{
                        GetEventParameter(inEvent, 'TCCL', typeVoidPtr, NULL, sizeof(myData->myClass), NULL, &myData->myClass);
						GetEventParameter(inEvent, 'TCSU', typeVoidPtr, NULL, sizeof(myData->setUpProc), NULL, &myData->setUpProc);
						GetEventParameter(inEvent, 'TCEP', typeVoidPtr, NULL, sizeof(myData->entryPoint), NULL, &myData->entryPoint);
						GetEventParameter(inEvent, 'TCTP', typeVoidPtr, NULL, sizeof(myData->termProc), NULL, &myData->termProc);
						SInt32 knownEnd;
						GetEventParameter(inEvent, 'TCKE', typeSInt32, NULL, sizeof(knownEnd), NULL, &knownEnd);
                        self = static_cast<AutoStarX*>(myData->myClass);
                        myData->hiThreadPane=self->mProgressPane;
						// associating the pane and the thread controller
						SetControlReference(myData->hiThreadPane, (SInt32) myData->hiObject);
						
						self->HIObjectThreadControllerStartThread(myData->hiObject);
						}
					}
					break;

				case kEventHIObjectDestruct:
					{
					// HIObjectThreadControllerStopThread(myData->hiObject);
					if (myData->hiThreadPane != NULL)
						{
						DisposeControl(myData->hiThreadPane);
						myData->hiThreadPane = NULL;
						}
					free(myData);
					}
					break;
				
				default:
					break;
				}
			break;
			
		case kEventClassHIObjectThreadController:
			switch (GetEventKind(inEvent))
				{
				case kEventUpdateThreadUI:
					self->UpdateUI(myData);
					break;
				
				case kEventTerminateThread:
					self->HIObjectThreadControllerTermThread(myData->hiObject);
					break;
				
				default:
					break;
				}
			break;
		
		default:
			break;
		}

exitHandler:
	return status;
}




// Registering our class and setting the handlers
CFStringRef AutoStarX::GetThreadControllerClass()
	{
	static HIObjectClassRef	theClass;
	
	if (theClass == NULL)
		{

		// mThreadControlerProcHandler=NewEventHandlerUPP( &ThreadControllerHandler);
		HIObjectRegisterSubclass(kHIObjectThreadControllerClassID, NULL, 0,
                                ThreadControllerHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, NULL, &theClass);
		}
	
	return kHIObjectThreadControllerClassID;
	}


// Starting a thread by calling its setup routine, the Multiprocessing Services API
// to actually start it, and updating the User Interface.
OSStatus AutoStarX::HIObjectThreadControllerStartThread(HIObjectRef threadController)
	{
	OSStatus status = noErr;
	ThreadControllerData * myData = (ThreadControllerData *) HIObjectDynamicCast(threadController, kHIObjectThreadControllerClassID);
	EventTargetRef theTarget = HIObjectGetEventTarget(myData->hiObject);
	myData->parameters = myData->setUpProc(this);
	((GeneralTaskWorkParamsPtr)myData->parameters)->threadControllerTarget = theTarget;
	status = MPCreateTask(myData->entryPoint, myData->parameters, 0, NULL, NULL, NULL, 0, &myData->taskID);
	UpdateUI(myData);
	return status;
	}

void AutoStarX::HIObjectThreadControllerTermThread(HIObjectRef threadController)
	{
	ThreadControllerData * myData = (ThreadControllerData *) HIObjectDynamicCast(threadController, kHIObjectThreadControllerClassID);
	if (myData->taskID != NULL)
		{
		myData->taskID = NULL;
		UpdateUI(myData);
		myData->termProc(myData->parameters);
		}
	}


void  AutoStarX::UpdateUI(ThreadControllerData * myData)
{
    CFStringRef theCFString;

    GeneralTaskWorkParamsPtr params = (GeneralTaskWorkParamsPtr) myData->parameters;

    SetControl32BitValue(mFlashProgress, params->progress);
    HIViewSetNeedsDisplay(mFlashProgress, true);
    switch(params->page)
        {
        case 0:
                theCFString = CFStringCreateWithFormat(NULL, NULL, CFSTR("Idle"), NULL);
                break;
        case -1:
                theCFString = CFStringCreateWithFormat(NULL, NULL, CFSTR("Error"), NULL);
                ErrorAlert(CFSTR("Communication error !"),CFSTR("The autostar isn't responding to the flashing commands. Check all connections and restart the autostar in safe load mode (press Enter end down close to ? and power on the autostar)"));
                // SendEventToUI(kEventTerminateThread, (GeneralTaskWorkParamsPtr)params, 0, -1);
                break;
        case 33:
                theCFString = CFStringCreateWithFormat(NULL, NULL, CFSTR("Upgrade done"), NULL);
                break;
            
        default:
            theCFString = CFStringCreateWithFormat(NULL, NULL, CFSTR("writing page %u/30"), params->page-1);
            break;
        }
        
    SetControlData(mFlashStatus, kControlEntireControl, kControlStaticTextCFStringTag, sizeof(CFStringRef), &theCFString);
    HIViewSetNeedsDisplay(mFlashStatus, true);
    
    CFRelease(theCFString);
}
/*****************************************************
*
* SendEventToUI(kind, params, iterator, result) 
*
* Purpose:  Posts an event to the main thread which is handling the User Interface
*
* Inputs:   kind      - the kind of event (in this sample code: kEventUpdateThreadUI or kEventTerminateThread)
*           params    - a pointer to the task params
*           iterator  - the current value of the iteration index
*           result    - the current value of the variable being calculated, in this sample code: pi
*
*/
void AutoStarX::SendEventToUI(UInt32 kind, GeneralTaskWorkParamsPtr params, SInt32 progress, SInt32 page)
	{
    
    params->progress = progress;
	params->page = page;
    params->myClass=this;
    
	EventRef theEvent;
	CreateEvent(NULL, kEventClassHIObjectThreadController, kind, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
	SetEventParameter(theEvent, kEventParamPostTarget, typeEventTargetRef, sizeof(params->threadControllerTarget), &params->threadControllerTarget);
	PostEventToQueue(GetMainEventQueue(), theEvent, kEventPriorityStandard);

	ReleaseEvent(theEvent);
	}





//
// Initialize the controls and lookup the serial ports
//
void AutoStarX::InitializeControls()
{
	ControlID ctrlID;
	OSErr err;
	
	// serial port popup
	ctrlID.id = kDlgSerialPortID;
	ctrlID.signature = kDlgSerialPort;
	err=GetControlByID(mWindow, &ctrlID, &mSerialPort);
	
	SetSerialPortsControls(mSerialPort);
	
	// ROM filename
	ctrlID.id = kDlgRomFileID;
	ctrlID.signature = kDlgRomFile;
	err=GetControlByID(mWindow, &ctrlID, &mRomFile);
	SetControlData (mRomFile, kControlEditTextPart, kControlEditTextTextTag, strlen (""), "");
	
	// Unit version
	ctrlID.id = kDlgUnitVersionID;
	ctrlID.signature = kDlgUnitVersion;
	err=GetControlByID(mWindow, &ctrlID, &mAStarVersion);
	SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen (""), "");
	
	// ROM version
	ctrlID.id = kDlgRomVersionID;
	ctrlID.signature = kDlgRomVersion;
	err=GetControlByID(mWindow, &ctrlID, &mRomVersion);
	SetControlData (mRomVersion, kControlEditTextPart, kControlEditTextTextTag, strlen (""), "");
	
	// Flash button
	ctrlID.id = kDlgFlashID;
	ctrlID.signature = kDlgFlash;
	err=GetControlByID(mWindow, &ctrlID, &mFlashButton);
	DeactivateControl(mFlashButton);

	// Connect button
	ctrlID.id = kDlgConnectID;
	ctrlID.signature = kDlgConnect;
	err=GetControlByID(mWindow, &ctrlID, &mConnectButton);
    if(mPorts->getCount()<1)
        DeactivateControl(mConnectButton);

	// File ROM version
	ctrlID.id = kDlgFileRomVersiontID;
	ctrlID.signature = kDlgFileRomVersion;
	err=GetControlByID(mWindow, &ctrlID, &mFileRomVersion);
	SetControlData (mFileRomVersion, kControlEditTextPart, kControlEditTextTextTag, strlen (""), "");

	// Browse button
	ctrlID.id = kDlgFileRomOpenID;
	ctrlID.signature = kDlgFileRomOpen;
	err=GetControlByID(mWindow, &ctrlID, &mRomOpen);

	// Quit button
	ctrlID.id = kDlgQuitID;
	ctrlID.signature = kDlgQuit;
	err=GetControlByID(mWindow, &ctrlID, &mQuit);

    
    
    /// Progress Pane controls
    
	
    // Flashing Progress Pane
	ctrlID.id = kDlgProgressPanID;
	ctrlID.signature = kDlgProgressPan;
    HIViewFindByID(HIViewGetRoot(mWindow), ctrlID, &mProgressPane);
    
	// Flashing progress bar
	ctrlID.id = kDlgFlashPogressID;
	ctrlID.signature = kDlgFlashPogress;
    HIViewFindByID(mProgressPane,ctrlID,&mFlashProgress);
	SetControl32BitValue(mFlashProgress, (SInt32)0);

	// Status of the firmware flashing
	ctrlID.id = kDlgFlashStatusID;
	ctrlID.signature = kDlgFlashStatus;
    HIViewFindByID(mProgressPane,ctrlID,&mFlashStatus);
	SetControlData (mFlashStatus, kControlEditTextPart, kControlEditTextTextTag, strlen ("Idle"), "Idle");


}


//
// Fill the serial port selection list with available serial ports
//
void AutoStarX::SetSerialPortsControls(ControlRef control)
{
    OSErr err;
    SInt32 numItems=0;
    int i;
	io_iterator_t matchingServices;
    MenuRef SerialPopup;
    
	mPorts= new SerialPort;
	
    // find available serial ports
	err=mPorts->FindPorts(&matchingServices);
	if(err != noErr)
        {
        DeactivateControl(control);
        return;
        }

    mPorts->GetPortList(matchingServices);

	//create menu from available serial devices
    numItems=mPorts->getCount();
	CreateNewMenu(kDlgSerialPortID,0,&SerialPopup);
    for(i=0;i<numItems;i++)
        {
            AppendMenuItemTextWithCFString (SerialPopup,
											mPorts->getPortName(i),
											0,
											0,
											NULL);
        }

	// attach menu to pop-up
	SetControlData(control,kControlEntireControl,kControlPopupButtonMenuRefTag,sizeof(MenuRef),&SerialPopup);
	
    if(!numItems)
        DeactivateControl(control);
    else
        {
        // select item #1 and set max items to the num of available serial ports
        SetControl32BitValue(control, (SInt32)1);
        SetControl32BitMaximum(control, numItems);
        }
	
}

//
// Load the ROM file, check fr what device it is
//

void AutoStarX::loadAutostarROMFile()
{
	OSErr err;
	FileSelector Fselector;
	Byte indexes[16];

	memset(indexes,0,16);

	mFileSpec=Fselector.FileSelect();
	FSpMakeFSRef(&mFileSpec,&mROMfileRef);
	if(mRomFullPath)
		delete mRomFullPath;
	mRomFullPath=new UInt8[255];
	FSRefMakePath(&mROMfileRef, mRomFullPath,255);
	// set the rom file path control to the full file path
	SetControlData (mRomFile, 
					kControlEditTextPart, 
					kControlEditTextTextTag, 
					strlen ((const char *)(mRomFullPath)), 
					mRomFullPath);
	// we need to open the file to get its size
	err=FSpOpenDF(&mFileSpec, fsRdPerm ,&mROMFileHandle);
	if(err)
		return;
	// get file size    
	err=GetEOF(mROMFileHandle,&mfSize);
	if(err)
		return;
	if(newRom)
		delete newRom;
	newRom= new ROM;

	// we need to load the file in memory here
	FSRead(mROMFileHandle,&mfSize,(void *)(newRom));
	//set the version in the control
	SetControlData (mFileRomVersion, 
					kControlEditTextPart, 
					kControlEditTextTextTag, 
					4, 
					newRom->version);
	// what autostar this rom is for ?
	switch(newRom->key)
		{
			case 0x00000000:    // autostar 495 & 497, 497EP, Audiostar
					
					// check the verion of the rom to make sure it's for a 497 and not a 497EP
					if (strncmp("4",(const char *)newRom->version,1) == 0)
						romType=1; // 495, 497
				if (strncmp("5",(const char *)newRom->version,1) == 0)
						romType=3; // 497EP
					break;
			// need to be fixed for big endian v little endian : intel : 0x800200
			case 0x00028000:    // autostar II PPC
			case 0x00800200:    // autostar II Intel
					romType=2;
					break;
			default :           // unknown rom file
					romType=0xffff;
					break;
		}
	
	// close the file after reading it
	err=FSClose(mROMFileHandle);
	if(bConnected)
		ActivateControl(mFlashButton);
	bFilename=true;

}

// 
// connect to the AutoStarX at 9600 bauds and get device type and ROM version
//
void AutoStarX::AutoStarConnect()
{
    SInt32 index;
	bool bSafeLoad;
    char bsdPath[255];
    Byte ioBuffer[64];
    Byte cmd[2];
	int i;
    // Get current selected port index
    index=GetControl32BitValue(mSerialPort);

    //get the bsd path at index -1 (index are going from 1 to n in the popup but from 0 to n-1 in the array)
    CFStringGetCString(mPorts->getPortPath(index-1),bsdPath,255,kCFStringEncodingASCII);

    // open port at 9600  
    if(! mPorts->OpenSerialPort(bsdPath, 9600))
        { // error opening port
        ErrorAlert(CFSTR("Communication error !"),CFSTR("The serial port is used by another application"));
        return;
        }
        
    bConnected=true;
    bSafeLoad=true;
	// flush all data before starting
	mPorts->ReadData(ioBuffer,64);

    for(i=0;i<11;i++)
        {
        // check AutoStarX status : cmd=0x06
        cmd[0]=0x06;    // ^F (1 byte response)
        cmd[1]=0;
        if(!mPorts->SendData(cmd,1))
            {
            AutoStarDisconnect();		
            ErrorAlert(CFSTR("Write error !"));
            return;
            }
        usleep(100000);
        if(!mPorts->ReadData(ioBuffer,1))
            {
            AutoStarDisconnect();
            ErrorAlert(CFSTR("Read error !"));
            return;
            }
 
        if(ioBuffer[0]!='?')
            {
            bSafeLoad=false;
            break;
            }            

        } 

    if(ioBuffer[0]!='D' && ioBuffer[0]!='B' && !bSafeLoad) // not in download mode (D = download or safe mode, B = 497EP in interactive mode)
        {
		// get autostart string 
        // switch to download mode
        cmd[0]=0x04;    // ^D (1 byte response)
        cmd[1]=0;
		if(!mPorts->SendData(cmd,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Write error !"));
            return;
			}
		usleep(500000);	
		if(!mPorts->ReadData(ioBuffer,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Read error !"));
            return;
			}

        }
    // otherwize we are already in download mode !!!!  most probably in safe load
    
    // get device type (495, 497, ....)
    cmd[0]='T';    // ask for AutoStarX type 0x0F=497 0x0A=495 0x05=??? (1 byte response)
    cmd[1]=0;
    if(!mPorts->SendData(cmd,1))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Write error !"));
        return;
		}
	usleep(500000);
	if(!mPorts->ReadData(ioBuffer,1))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Read error !"));
        return;
		}
	
	deviceType = ioBuffer[0];

    if(!bSafeLoad)
        {
        // get ROM version
        cmd[0]='V';    // ask for ROM version (4 bytes response)
        cmd[1]=0;
        if(!mPorts->SendData(cmd,1))
            {
            AutoStarDisconnect();
            ErrorAlert(CFSTR("Write error !"));
            return;
            }
        usleep(500000);    
        if(!mPorts->ReadData(ioBuffer,4))
            {
            AutoStarDisconnect();
            ErrorAlert(CFSTR("Read error !"));
            return;
            }

    // set the device type control to the AutoStarX type
	printf("ioBuffer[0] = %c", ioBuffer[0]);
    switch(deviceType)
        {
        case 0x07:
            SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("494"), "494");
            deviceType=0;
            break;
        case 0x0A:
            SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("495"), "495");
            deviceType=1;
            break;
        case 0x0f:
            SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("497"), "497");
            deviceType=1;
            break;
        case 0x0e:
            SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("Autostar II (not supported)"), "Autostar II (not supported)");
            deviceType=2;
            break;
        case 0x1f:
			if (ioBuffer[0] == '5') {
				SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("497EP (not supported)"), "497EP (not supported)");
				deviceType=3;
			}
			else if (ioBuffer[0] == 'A') {
				SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("Audiostar (not supported)"), "Audiostar (not supported)");
				deviceType=4;
			}
			else {
				SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("Other"), "Other");
			}
            break;
        default:
            SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("Other"), "Other");
            deviceType=0xFFFF;
            break;
        }

        // set the rom version control to the AutoStarX current version
        SetControlData (mRomVersion, kControlEditTextPart, kControlEditTextTextTag, 4, ioBuffer);
        }
    else
        SetControlData (mRomVersion, kControlEditTextPart, kControlEditTextTextTag, strlen("Safe Load"), "Safe Load");
    
    DeactivateControl(mSerialPort);
    if(bFilename)
        ActivateControl(mFlashButton);
    
    SetControlTitleWithCFString(mConnectButton,CFSTR("Disconnect"));
 
    
}

void AutoStarX::AutoStarDisconnect()
{
    Byte cmd[2];
    
    cmd[0]='I'; // Initialize .. proper way of exiting download mode (0 byte response)
    cmd[1]=0;
    mPorts->SendData(cmd,1);
    mPorts->CloseSerialPort();
    bConnected=false;
    DeactivateControl(mFlashButton);
    ActivateControl(mSerialPort);
    SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen (""), "");
    SetControlData (mRomVersion, kControlEditTextPart, kControlEditTextTextTag, strlen (""), "");
    SetControlTitleWithCFString(mConnectButton,CFSTR("Connect"));

}


void AutoStarX::AutoStarReset()
{
    Byte cmd[16];
    int timeout;
    Byte ioBuffer[64];
    
    cmd[0]='I'; // Initialize .. proper way of exiting download mode (0 byte response)
    cmd[1]=0;
    mPorts->SendData(cmd,1);

    // wait for the "X" from the autostar boot (~ 10 secondes)
	timeout=0;
    while(!mPorts->ReadData(ioBuffer,1))
		{
        timeout++;
        if(timeout==15) // 15 secondes just in case
            {
            ErrorAlert(CFSTR("Communication error !"),CFSTR("The autostar doesn't respond to the reset command"));
            AutoStarDisconnect();
            return;
            }
		}


    // we flush the port to be sure
    mPorts->ReadData(ioBuffer,16);
    
    // get ROM version :GVN#
    cmd[0]=':';    // ask for ROM version (5 bytes response)
    cmd[1]='G';
    cmd[2]='V';
    cmd[3]='N';
    cmd[4]='#';
    
    if(!mPorts->SendData(cmd,5))
		{
		AutoStarDisconnect();
        ErrorAlert(CFSTR("Write error !"));
        return;
		}
		
	if(!mPorts->ReadData(ioBuffer,5))
		{
		AutoStarDisconnect();
        ErrorAlert(CFSTR("Read error !"));
        return;
		}
		
    // set the rom version control to the AutoStarX current version
    SetControlData (mRomVersion, kControlEditTextPart, kControlEditTextTextTag, 4, ioBuffer);

}



void AutoStarX::ErrorAlert(CFStringRef error)
{

	ErrorAlert(error,NULL);
}

void AutoStarX::ErrorAlert(CFStringRef error, CFStringRef comment)
{
	DialogRef	alert;

    if(!comment)
        CreateStandardAlert(kAlertStopAlert,error,CFSTR("The AutoStarX isn't responding. Check connections and be sure to select the correct serial port."),NULL, &alert);
    else
        CreateStandardAlert(kAlertStopAlert,error,comment,NULL, &alert);
        
	RunStandardAlert(alert, NULL, NULL);
}



void *AutoStarX::setupFlash(void *p)
{
    AutoStarX* self = static_cast<AutoStarX*>(p);

    GeneralTaskWorkParamsPtr params=new GeneralTaskWorkParams;
    params->page=0;
    params->progress=0;
    params->myClass=self;
    return params;
}


pascal OSStatus AutoStarX::Flash(void *userData)
{
	Byte doublepages;
    Byte page;
    int i,j;
    SInt32 progress;
	Byte ioBuffer[64];
	Byte cmd[70];	// 5 byte command + 64 byte of data maximum
    Byte *ff_data;
	unsigned short addr;
	int erase_dbl_page;
    
    GeneralTaskWorkParamsPtr params=(GeneralTaskWorkParamsPtr)userData;
    
     // get a pointer to the object itself to be able to access private member variable and functions
    AutoStarX* self = static_cast<AutoStarX*>(((GeneralTaskWorkParamsPtr)userData)->myClass);

    self->bFlashing=true;
	
    ff_data=new Byte[BLOCKSIZE];
	memset(ff_data,0xff,BLOCKSIZE);
    progress=0;

#ifndef __TEST

    // we start on page 2 so it's double page 1 and write 2 pages as we need to erase a double page each time
    for( doublepages=1;doublepages<16;doublepages++)
        {
        // update the progress bar and status
        self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, progress, page);	

		// we need to test if the page is full set to $FF 
		// and if yes not erase it a go to the next double page
		erase_dbl_page=0;
		for(i=0;i<32768;i+=BLOCKSIZE)
			{
			erase_dbl_page+=memcmp(&(self->newRom->pages[doublepages*2][i]),ff_data,BLOCKSIZE);
			erase_dbl_page+=memcmp(&(self->newRom->pages[doublepages*2+1][i]),ff_data,BLOCKSIZE);
			if(erase_dbl_page)
				break;
			}
			
		if(!erase_dbl_page)
            {
            progress+=(32768*2);
			continue;
            }

#ifdef __COM_DEBUG
                printf("command = E doublepage =%X\n",doublepages);
#endif

        // erase double page
		cmd[0]=0x45;    // E (1 byte response)
		cmd[1]=doublepages;
		cmd[2]=0;
		if(!self->mPorts->SendData(cmd,2))
			{
			self->AutoStarDisconnect();
			//
            self->ActivateControls();
            // quiting the trhread
            self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, 0, -1);
            self->bFlashing=false;
            return kNSLSchedulerError;
			}
#ifdef __COM_DEBUG
                printf("command = E doublepage =%X command sent\n",doublepages);
#endif
        usleep(1000000); // a second
        
		if(!self->mPorts->ReadData(ioBuffer,1))
			{
			self->AutoStarDisconnect();
            self->ActivateControls();
            // quiting the trhread
            self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, 0, -1);
            self->bFlashing=false;
            delete ff_data;
            return kNSLSchedulerError;
			}

#ifdef __COM_DEBUG
                printf("command = E doublepage =%X result= %c\n",doublepages,ioBuffer[0]);
#endif
			
		// check if answer is "Y"
		if(ioBuffer[0]!='Y') // page has been erased
			{
			// if not we don't try to write
			self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, progress, page);
			continue;
			}

#ifdef __COM_DEBUG
                printf("command = E doublepage =%X   ERASE OK\n",doublepages);
#endif

		
		for(j=0;j<2;j++)
			{
			// start write page
			addr=0x8000;
			page=doublepages*2+j;
			// we write "BLOCKSIZE" byte each time
			for(i=0;i<32768;i+=BLOCKSIZE)		
				{
                // update the progress bar and status								
                self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, progress, page);

				// we need to avoid the 512 byte of eeprom at B600-B7FF
				// it should be mark by all FF in the file but testing for it is safer
				if( (addr>0xB5FF) && (addr<0xB800) )
					{
					// set addr after the eeprom, increment i and progress by 512
                    addr=0xB800;
                    i+=512-BLOCKSIZE;   // new value for i
                    progress+=512;
					continue;
					}

#ifdef __COM_DEBUG
                printf("command = %W page =%X addresse=%02X%02X size = %02X\n",page,(Byte)((addr&0xFF00)>>8),(Byte)(addr&0xFF),BLOCKSIZE);
#endif

				progress+=BLOCKSIZE;

				// we don't write block that are all $FF
				if( ! memcmp(&(self->newRom->pages[page][i]),ff_data,BLOCKSIZE))
					{
					// increment addr
					addr+=BLOCKSIZE;
#ifdef __COM_DEBUG
                    printf("all $FF\n");
#endif				
                    continue;
					}
				// write data
				cmd[0]=0x57;    // W (1 byte response)
				cmd[1]=page;
				cmd[2]=(Byte)((addr&0xFF00)>>8);		// HI part address
				cmd[3]=(Byte)(addr&0xFF);		// LOW part address
				cmd[4]=BLOCKSIZE;			// nb byte
				memcpy(&cmd[5],&(self->newRom->pages[page][i]),BLOCKSIZE);
				if(!self->mPorts->SendData(cmd,69))
					{
					self->AutoStarDisconnect();
                    self->ActivateControls();
                    // quiting the trhread
                    self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, 0, -1);
                    self->bFlashing=false;
                    delete ff_data;
					return kNSLSchedulerError;
					}
					
				if(!self->mPorts->ReadData(ioBuffer,1))
					{
					self->AutoStarDisconnect();
                    self->ActivateControls();
                    // quiting the trhread
                    self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, 0, -1);
                    self->bFlashing=false;
                    delete ff_data;
                    return kNSLSchedulerError;
					}
					
				// check if answer is "Y"
				if(ioBuffer[0]!='Y') // Data have been writen ?
					{
					// if not we have a problem .. we stop it all
					self->AutoStarDisconnect();
					//reactivate controls
                    self->AutoStarDisconnect();
                    self->ActivateControls();
					// quiting the trhread
                    self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, 0, -1);
                    self->bFlashing=false;
                    delete ff_data;
					return kNSLSchedulerError;
					}
				// increment addr
				addr+=BLOCKSIZE;
				
				}
			}
			
        }

#endif				

    
    // quiting the trhread
    self->SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, 0, 33);
    self->bFlashing=false;
    delete ff_data;

    //reactivate controls
    self->ActivateControls();

	// iteration is finished, we send the appropriate event to the main thread.
	self->SendEventToUI(kEventTerminateThread, (GeneralTaskWorkParamsPtr)params, 0, 33);
    
    return noErr;
}


void AutoStarX::termFlash(void *p)
{
         // get a pointer to the object itself to be able to access private member variable and functions
    AutoStarX* self = static_cast<AutoStarX*>(((GeneralTaskWorkParamsPtr)p)->myClass);
    self->AutoStarReset();
    
    delete (GeneralTaskWorkParamsPtr)p;
}


void AutoStarX::ActivateControls()
{
    ActivateControl(mFlashButton);
    ActivateControl(mConnectButton);
    ActivateControl(mRomOpen);
    ActivateControl(mQuit);
}

void AutoStarX::DeActivateControls()
{

    DeactivateControl(mFlashButton);
    DeactivateControl(mConnectButton);
    DeactivateControl(mRomOpen);
    DeactivateControl(mQuit);
}
