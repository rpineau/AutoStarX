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
    IBNibRef 		nibRef;	
    OSStatus		err;
    
    // init data
    bConnected=false;
    bFilename=false;
    newRom=NULL;
	mThreadDone=false;
    
    InitCursor();
    
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &mWindow);
    require_noerr( err, CantCreateWindow );
	
	err = CreateWindowFromNib(nibRef, CFSTR("About"), &mAbout);
    require_noerr( err, CantCreateWindow );
	
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
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

	mWindowProcHandler=NewEventHandlerUPP( &aboutWindowHandler);

	err=InstallEventHandler(GetWindowEventTarget(mAbout),
							mWindowProcHandler,
							GetEventTypeCount(WindowEventList),
							WindowEventList,
							this,
							NULL);
    
    // Control Events Handler
	mControlProcHandler=NewEventHandlerUPP(&buttonHandler);
	
	err=InstallEventHandler(GetWindowEventTarget(mWindow),
							mControlProcHandler,
							GetEventTypeCount(ControlEventList),
							ControlEventList,
							this,
							NULL);

     
    // Event Loop Timer to idle controls 5 times a second.
    InstallEventLoopTimer( GetCurrentEventLoop(), 0, 0, NewEventLoopTimerUPP( MainRunLoopForThreadedApps ), this, &mTimer );


    // The window was created hidden so show it.
    ShowWindow(mWindow);

    // Call the event loop
    RunApplicationEventLoop();


CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:

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


pascal OSStatus AutoStarX::buttonHandler(EventHandlerCallRef myHandler, EventRef event, void *userData)
{
	HICommand commandStruct;
	UInt32	CommandID;
	OSStatus result=eventNotHandledErr;
	OSErr err;

	FileSelector Fselector;

	// get a pointer to the object itself to be able to access private member variable and functions
    AutoStarX* self = static_cast<AutoStarX*>(userData);
    
	// Get the event command
	GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand),NULL,&commandStruct);
	
	CommandID=commandStruct.commandID;
	
	// execute the command
	switch(CommandID)
		{
		case 'open':  // get the file name and its size
				{
				self->mFileSpec=Fselector.FileSelect();
				FSpMakeFSRef(&self->mFileSpec,&self->mROMfileRef);
				if(self->mRomFullPath)
					delete self->mRomFullPath;
				self->mRomFullPath=new UInt8[255];
				FSRefMakePath(&self->mROMfileRef, self->mRomFullPath,255);
				// set the rom file path control to the full file path
				SetControlData (self->mRomFile, 
								kControlEditTextPart, 
								kControlEditTextTextTag, 
								strlen ((const char *)(self->mRomFullPath)), 
								self->mRomFullPath);
				// we need to open the file to get its size
				err=FSpOpenDF(&self->mFileSpec, fsRdPerm ,&self->mROMFileHandle);
				if(err)
					{
                    break;
                    }
                // get file size    
                err=GetEOF(self->mROMFileHandle,&self->mfSize);
				if(err)
					{
                   break;
                    }
				
				if(self->newRom)
					delete self->newRom;
				self->newRom= new ROM;
                
                // we need to load the file in memory here
                FSRead(self->mROMFileHandle,&self->mfSize,(void *)(self->newRom));
				//set the version in the control
				SetControlData (self->mFileRomVersion, 
								kControlEditTextPart, 
								kControlEditTextTextTag, 
								4, 
								self->newRom->version);
                // close the file after reading it
                err=FSClose(self->mROMFileHandle);
                if(self->bConnected)
                    ActivateControl(self->mFlashButton);
                self->bFilename=true;
				break;
				}
				
		case 'Port':
				result=noErr;
				break;

		case 'Flsh':  // start the flashing of the AutoStarX
                self->Flash();
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
                if ( self->mNumberOfRunningThreads > 0 )
                    {
                    result=noErr; //don't proces quit while flashing
                    }
                else
                    self->mThreadDone=true;
                break;
		case 'abou':
				ShowWindow (self->mAbout);
				SelectWindow (self->mAbout);    
				result=noErr;
				break;
				
		}
    // printf("control events\n");
		
	return (result);

}


// window event hadler

pascal OSStatus AutoStarX::windowHandler(EventHandlerCallRef myHandler, EventRef event, void *userData)
{
    OSStatus result=eventNotHandledErr;
	// if the window is closed, quit the application
    if(GetEventKind(event) == kEventWindowClosed)
        {
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
        }
    return result;
}


//
// Main Event Loop for threaded Apps
// taken from developer sample
// 

pascal void AutoStarX::MainRunLoopForThreadedApps( EventLoopTimerRef inTimer, void *userData )
{
	OSStatus		err;
	EventRef		theEvent;
	EventTimeout	timeToWaitForEvent;
	EventTargetRef	theTarget			= GetEventDispatcherTarget();

	// get a pointer to the object itself to be able to access private member variable and functions
    AutoStarX* self = static_cast<AutoStarX*>(userData);
    	
	do
	{
	    if (self->mNumberOfRunningThreads == 0 )
	        timeToWaitForEvent	= kEventDurationForever;
	    else
	        timeToWaitForEvent	= kEventDurationNoWait;
	    
	    err = ReceiveNextEvent( 0, NULL, timeToWaitForEvent, true, &theEvent );
	    
	    if ( err == noErr )
	    {
	        (void) SendEventToEventTarget( theEvent, theTarget );
	        ReleaseEvent( theEvent );
	    }
	    else if ( err == eventLoopTimedOutErr )
	    {
	        err = noErr;
	    }
	    if ( self->mNumberOfRunningThreads > 0 )
	        (void) YieldToAnyThread();
												//	eventLoopQuitErr may be sent to wake up the queue and does not necessarily
	} while ( self->mThreadDone == false );				//	mean 'quit', we handle the Quit case from our AppleEvent handler
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
	
	// Status of the firmware flashing
	ctrlID.id = kDlgFlashStatusID;
	ctrlID.signature = kDlgFlashStatus;
	err=GetControlByID(mWindow, &ctrlID, &mFlashStatus);
	SetControlData (mFlashStatus, kControlEditTextPart, kControlEditTextTextTag, strlen ("Idle"), "Idle");
	
	// Flashing progress bar
	ctrlID.id = kDlgFlashPogressID;
	ctrlID.signature = kDlgFlashPogress;
	err=GetControlByID(mWindow, &ctrlID, &mFlashProgress);
	SetControl32BitValue(mFlashProgress, (SInt32)0);

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
// connect to the AutoStarX at 9600 bauds and get device type and ROM version
//
void AutoStarX::AutoStarConnect()
{
    SInt32 index;
	
    char bsdPath[255];
    Byte ioBuffer[64];
    Byte cmd[2];
	
    // Get current selected port index
    index=GetControl32BitValue(mSerialPort);

    //get the bsd path at index -1 (index are going from 1 to n in the popup but from 0 to n-1 in the array)
    CFStringGetCString(mPorts->getPortPath(index-1),bsdPath,255,kCFStringEncodingASCII);

    // open port at 9600  
    if(! mPorts->OpenSerialPort(bsdPath, 9600))
        { // error opening port
        return;
        }
        
    bConnected=true;
	// flush all data before starting
	mPorts->ReadData(ioBuffer,64);
	
    // check AutoStarX status : cmd=0x06
    cmd[0]=0x06;    // ^F (1 byte response)
    cmd[1]=0;
    if(!mPorts->SendData(cmd,1))
		{
		AutoStarDisconnect();		
		ErrorAlert(CFSTR("Write error !"));
        return;
		}

	if(!mPorts->ReadData(ioBuffer,1))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Read error !"));
        return;
		}
		
    if(ioBuffer[0]!='D') // not in download mode
        {
        // switch to download mode
        cmd[0]=0x04;    // ^D (1 byte response)
        cmd[1]=0;
		if(!mPorts->SendData(cmd,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Write error !"));
            return;
			}
			
		if(!mPorts->ReadData(ioBuffer,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Read error !"));
            return;
			}

        }
    // otherwize we are already in download mode !!!!  most probably in safe load

	//
	// switch to 125Kbaud ---> apparently not a standar speed .. will probably never work on mac
	// but we can test just in case..
	// does serial port support 125Kbauds?
	if(mPorts->SetSpeed(125000))
		{
		mPorts->SetSpeed(9600);  //switch back to 9600 to send the F command
        
		cmd[0]=0x46;    // F (1 byte response)
		cmd[1]=0;
		if(!mPorts->SendData(cmd,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Write error !"));
            return;
			}
			
		if(!mPorts->ReadData(ioBuffer,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Read error !"));
            return;
			}
			
		// check if answer is "Y"
		if(ioBuffer[0]=='Y') // ok to switch to 125K
			{
			if(!mPorts->SetSpeed(125000))
				{
				AutoStarDisconnect();
				ErrorAlert(CFSTR("Error switching to high speed !"));
                return;
				}
			}
		}
    
    // get device type (495, 497, ....)
    cmd[0]='T';    // ask for AutoStarX type 0x0F=497 0x0A=495 0x05=??? (1 byte response)
    cmd[1]=0;
    if(!mPorts->SendData(cmd,1))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Write error !"));
        return;
		}
	
	if(!mPorts->ReadData(ioBuffer,1))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Read error !"));
        return;
		}
		
    // set the device type control to the AutoStarX type
    switch(ioBuffer[0])
        {
        case 0x0f:
            SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("497"), "497");
            break;
            
        case 0x0A:
            SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("495"), "495");
            break;
        default:
            SetControlData (mAStarVersion, kControlEditTextPart, kControlEditTextTextTag, strlen ("Other"), "Other");
            break;
        }
        
    // get ROM version
    cmd[0]='V';    // ask for ROM version (4 bytes response)
    cmd[1]=0;
    if(!mPorts->SendData(cmd,1))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Write error !"));
        return;
		}
		
	if(!mPorts->ReadData(ioBuffer,4))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Read error !"));
        return;
		}
		
    // set the rom version control to the AutoStarX current version
    SetControlData (mRomVersion, kControlEditTextPart, kControlEditTextTextTag, 4, ioBuffer);
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

void AutoStarX::ErrorAlert(CFStringRef error)
{
	DialogRef	alert;

	CreateStandardAlert(kAlertStopAlert,error,CFSTR("The AutoStarX isn't responding. Check connections and be sure to select the correct serial port."),NULL, &alert);
	RunStandardAlert(alert, NULL, NULL);
}


void AutoStarX::Flash()
{
    OSErr result;
    
    // set the progress bar max length and initial value 
    // page 0 and 1 aren't writen
    // the top 512 bytes of each page aren't writen
    // pages 30 and 31 are for garbage collection (pages $1E and $1F)
    // if page is all $FF, do not write
    // so we have 30 pages x 32K (minus the 512 bytes)
    // so 30x(32768-512) = 967680 byte to write
    SetControl32BitMaximum(mFlashProgress, 967680);

    // create the falshing thread
    result = NewThread( kCooperativeThread, NewThreadEntryUPP( (ThreadEntryProcPtr) &FlashThread ), this, 0, kCreateIfNeeded, nil, &mFlashthreadID );

    if(result == noErr) // thread created ok
        {
        mNumberOfRunningThreads++;
        // disable the controls until we're done with the flashing
        DeactivateControl(mFlashButton);
        DeactivateControl(mConnectButton);
        DeactivateControl(mRomOpen);
        DeactivateControl(mQuit);
        }
    YieldToAnyThread();
}


pascal void AutoStarX::FlashThread(void *userData)
{
	Byte doublepages;
    int page;
    int i,j;
    SInt32 progress;
    char status[64];
	Byte ioBuffer[64];
	Byte cmd[69];	// 5 byte command + 64 byte of data maximum
    int blockSize;
    Byte *ff_data;
	unsigned short addr;
	int erase_dbl_page;
	
     // get a pointer to the object itself to be able to access private member variable and functions
    AutoStarX* self = static_cast<AutoStarX*>(userData);

    // recomended block size is 64 byte (0x40)
	blockSize=64;
	
    SetThemeCursor( kThemeWatchCursor );
    ff_data=new Byte[blockSize];
	memset(ff_data,0xff,blockSize);
    progress=0;
    SetControl32BitValue(self->mFlashProgress, progress);
    Draw1Control(self->mFlashProgress);
    
    // we start on page 2 so it's double page 1 and write 2 pages as we need to erase a double page each time
    for( doublepages=1;doublepages<16;doublepages++)
        {

		// we need to test if the page is full set to $FF 
		// and if yes not erase it a go to the next double page
		erase_dbl_page=0;
		for(i=0;i<32768;i++)
			{
			erase_dbl_page+=memcmp(&(self->newRom->pages[doublepages*2][i]),ff_data,blockSize);
			erase_dbl_page+=memcmp(&(self->newRom->pages[doublepages*2+1][i]),ff_data,blockSize);
			if(erase_dbl_page)
				break;
			}
			
		if(!erase_dbl_page)
			{
			YieldToAnyThread();
			continue;
			}
        // erase double page
		
		cmd[0]=0x45;    // E (1 byte response)
		cmd[1]=doublepages;
		cmd[2]=0;
		if(!self->mPorts->SendData(cmd,2))
			{
			self->AutoStarDisconnect();
			self->ErrorAlert(CFSTR("Write error !"));
            return;
			}
			
		if(!self->mPorts->ReadData(ioBuffer,1))
			{
			self->AutoStarDisconnect();
			self->ErrorAlert(CFSTR("Read error !"));
            return;
			}
			
		// check if answer is "Y"
		if(ioBuffer[0]!='Y') // page has been erased
			{
			// if not we don't try to write
			YieldToAnyThread();
			continue;
			}
		
		for(j=0;j<2;j++)
			{
			// start write page
			addr=0x8000;
			page=doublepages*2+j;
			sprintf(status,"writing page : %u/32\n", (int)(page+1));
			SetControlData (self->mFlashStatus, kControlEditTextPart, kControlEditTextTextTag, strlen (status), status);
			// we write "blocksize" byte each time
			for(i=0;i<32768;i+=blockSize)		
				{								
				progress+=blockSize;
				// we need to avoid the 512 byte of eeprom at B600-B7FF
				// it should be mark by all FF in the file but testing for it is safer
				if( (addr>0xB5FF) && (addr<0xB800) )
					{
					// increment addr
					addr+=blockSize;
					YieldToAnyThread();
					continue;
					}
				// we don't write block that are all $FF
				if( ! memcmp(&(self->newRom->pages[page][i]),ff_data,blockSize))
					{
					// increment addr
					addr+=blockSize;
					YieldToAnyThread();
					continue;
					}
				// write data
				cmd[0]=0x57;    // W (1 byte response)
				cmd[1]=(char)(page);
				cmd[2]=(char)((addr&0xFF00)>>8);		// HI part address
				cmd[3]=(char)(addr&0xFF);		// LOW part address
				cmd[4]=(char)blockSize;			// nb byte
				memcpy(&cmd[5],&(self->newRom->pages[page][i]),blockSize);
				if(!self->mPorts->SendData(cmd,69))
					{
					self->AutoStarDisconnect();
					self->ErrorAlert(CFSTR("Write error !"));
					return;
					}
					
				if(!self->mPorts->ReadData(ioBuffer,1))
					{
					self->AutoStarDisconnect();
					self->ErrorAlert(CFSTR("Read error !"));
					return;
					}
					
				// check if answer is "Y"
				if(ioBuffer[0]!='Y') // Data have been writen ?
					{
					// if not we have a problem .. we stop it all
					self->AutoStarDisconnect();
					self->ErrorAlert(CFSTR("Error writing data to flash!"));
					
					//reactivate controls
					ActivateControl(self->mFlashButton);
					ActivateControl(self->mConnectButton);
					ActivateControl(self->mRomOpen);
					ActivateControl(self->mQuit);
					// quiting the trhread
					SetThemeCursor( kThemeArrowCursor );
					self->mNumberOfRunningThreads--;
					SetControlData (self->mFlashStatus, kControlEditTextPart, kControlEditTextTextTag, strlen ("Upgrade done"), "Upgrade done");
					delete ff_data;

					return;
					}
				
				// increment addr
				addr+=blockSize;
				
				SetControl32BitValue(self->mFlashProgress, progress);
				Draw1Control(self->mFlashProgress);
				YieldToAnyThread();	
				}
			}
			
        }
    //reactivate controls
    ActivateControl(self->mFlashButton);
    ActivateControl(self->mConnectButton);
    ActivateControl(self->mRomOpen);
    ActivateControl(self->mQuit);
    // quiting the trhread
    SetThemeCursor( kThemeArrowCursor );
    self->mNumberOfRunningThreads--;
    SetControlData (self->mFlashStatus, kControlEditTextPart, kControlEditTextTextTag, strlen ("Upgrade done"), "Upgrade done");
	SetControl32BitValue(self->mFlashProgress, 0);
    Draw1Control(self->mFlashProgress);

	delete ff_data;
}

