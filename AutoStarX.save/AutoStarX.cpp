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
    
    // Control Events Handler
	mControlProcHandler=NewEventHandlerUPP(&buttonHandler);
	
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
                    return -1;
                    }
                // get file size    
                err=GetEOF(self->mROMFileHandle,&self->mfSize);
				if(err)
					{
                    return -1;
                    }
				
				if(self->newRom)
					delete self->newRom;
				self->newRom= new ROM;
				
                // set the progress bar max length.. need to be set to the actual data size to be flashed
                SetControl32BitMaximum(self->mFlashProgress, (SInt32)self->mfSize);
                // just for testing... should bet set to 0 latter                
                SetControl32BitValue(self->mFlashProgress, (SInt32)self->mfSize/2);
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
				break;
				}
				
		case 'Port':
				result=0;
				break;

		case 'Flsh':  // start the flashing of the AutoStarX
				result=0;
				break;

		case 'Cnct': // connect to the AutoStarX on the selected serial port
                if(self->bConnected)
                    self->AutoStarDisconnect();
                else
                    self->AutoStarConnect();
				result=0;
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
        result=ProcessHICommand(&quitCommand);
    
    return result;
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
	//				err=ActivateControl(mFlashButton);

	// Connect button
	ctrlID.id = kDlgConnectID;
	ctrlID.signature = kDlgConnect;
	err=GetControlByID(mWindow, &ctrlID, &mConnectButton);
    if(mPorts.getCount()<1)
        DeactivateControl(mConnectButton);

	// File ROM version
	ctrlID.id = kDlgFileRomVersiontID;
	ctrlID.signature = kDlgFileRomVersion;
	err=GetControlByID(mWindow, &ctrlID, &mFileRomVersion);
	SetControlData (mFileRomVersion, kControlEditTextPart, kControlEditTextTextTag, strlen (""), "");


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
    
    // find available serial ports
	err=mPorts.FindPorts(&matchingServices);
	if(err != noErr)
        {
        DeactivateControl(control);
        return;
        }

    mPorts.GetPortList(matchingServices);

	//create menu from available serial devices
    numItems=mPorts.getCount();
	CreateNewMenu(kDlgSerialPortID,0,&SerialPopup);
    for(i=0;i<numItems;i++)
        {
            AppendMenuItemTextWithCFString (SerialPopup,
											mPorts.getPortName(i),
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
	SInt16	alertOut;
	
    char bsdPath[255];
    char ioBuffer[255];
    char cmd[2];
	
    // Get current selected port index
    index=GetControl32BitValue(mSerialPort);

    //get the bsd path at index -1 (index are going from 1 to n in the popup but from 0 to n-1 in the array)
    CFStringGetCString(mPorts.getPortPath(index-1),bsdPath,255,kCFStringEncodingASCII);

    // open port at 9600  
    if(! mPorts.OpenSerialPort(bsdPath, 9600))
        { // error opening port
        return;
        }
        
    bConnected=true;

    // check AutoStarX status : cmd=0x06
    cmd[0]=0x06;    // ^F (1 byte response)
    cmd[1]=0;
    if(!mPorts.SendData(cmd,1))
		{
		AutoStarDisconnect();		
		ErrorAlert(CFSTR("Write error !"));
        return;
		}

	if(!mPorts.ReadData(ioBuffer,1))
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
		if(!mPorts.SendData(cmd,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Write error !"));
            return;
			}
			
		if(!mPorts.ReadData(ioBuffer,1))
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
	if(mPorts.SetSpeed(125000))
		{
		mPorts.SetSpeed(9600);  //switch back to 9600 to send the F command
        
		cmd[0]=0x46;    // F (1 byte response)
		cmd[1]=0;
		if(!mPorts.SendData(cmd,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Write error !"));
            return;
			}
			
		if(!mPorts.ReadData(ioBuffer,1))
			{
			AutoStarDisconnect();
			ErrorAlert(CFSTR("Read error !"));
            return;
			}
			
		// check if answer is "Y"
		if(ioBuffer[0]=='Y') // ok to switch to 125K
			{
			if(!mPorts.SetSpeed(125000))
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
    if(!mPorts.SendData(cmd,1))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Write error !"));
        return;
		}
	
	if(!mPorts.ReadData(ioBuffer,1))
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
    if(!mPorts.SendData(cmd,1))
		{
		AutoStarDisconnect();
		ErrorAlert(CFSTR("Write error !"));
        return;
		}
		
	if(!mPorts.ReadData(ioBuffer,4))
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
    char cmd[2];
    
    cmd[0]='I'; // Initialize .. proper way of exiting download mode (0 byte response)
    cmd[1]=0;
    mPorts.SendData(cmd,1);
    mPorts.CloseSerialPort();
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
