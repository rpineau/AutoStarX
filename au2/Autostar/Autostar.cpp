// Autostar.cpp: implementation of the CAutostar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Autostar.h"

// used by SendDownloadMode ONLY to create the proper model
#include "Model494.h"
#include "Model497.h"
#include "ModelLX.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAutostar::CAutostar()
{
	m_mode				= UNKNOWN;
	m_model				= NULL;
	m_handboxData.Clear();
	m_version			= "Unknown";
	m_stat				= NULL;
	m_verifyMode		= false;
	m_modelName			= "Unknown";
	m_hbxSafeMode		= FALSE;
	m_lastVertCmnd = m_lastHorzCmnd = m_lastSpeedCmnd = CAutostar::None;
}

CAutostar::~CAutostar()
{
	if (m_model)
		delete m_model;
}

/////////////////////////////////////////////
//
//	Name		:Initialize Connection
//
//	Description :This function will connect to the autostar, retrieve the version
//				 number, and then instanciate the correct AutostarModel and store it in
//               the member variable m_autostarModel.
//				 It will then set the autostar to download mode.
//
//  Input		:None
//
//	Output		:AutostarStatus
//
////////////////////////////////////////////
eAutostarStat CAutostar::InitializeConnection(bool CloseComPort, bool setDownload)
{
// Don't do this if we are busy
	if (m_mode == BUSY)
		return AUTOSTAR_BUSY;

// initialize the com port
	m_lastError = InitComPort();

// if it didn't work try the other baud rate
	if (m_lastError != AUTOSTAR_OK)
	{
		if (m_serialPort.GetBaud() == m_serialPort.GetDefaultBaud())
		{
			TRACE("Switching to Max Baud\n");
			m_serialPort.SetBaud(m_serialPort.GetMaxBaud());
		}
		else
		{
			TRACE("Switching to Default Baud\n");
			m_serialPort.SetBaud(m_serialPort.GetDefaultBaud());
		}

	m_lastError = InitComPort();
	}


// retrieve the version from the autostar and set the autostar model
	if (m_lastError == AUTOSTAR_OK)
		m_lastError = RetrieveVersion();

// Retrieve the type of handbox
	if (m_lastError == AUTOSTAR_OK && setDownload)
		m_lastError = RetrieveType();

	if (CloseComPort || m_lastError != AUTOSTAR_OK)
	// close the port
		m_serialPort.ClosePort();

	if (m_lastError != AUTOSTAR_OK)
	{
		m_modelName = "Unknown";
		m_version = "Unknown";
	}
	return m_lastError;
}


/////////////////////////////////////////////
//
//	Name		:CloseSerialPort
//
//	Description :This function will close the serial port
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CAutostar::CloseSerialPort()
{
	m_serialPort.ClosePort();
}

/////////////////////////////////////////////
//
//	Name		:InitComPort
//
//	Description :This function will get the com port name from CUserSettings
//				:open that com port through CSerialPort
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CAutostar::InitComPort()
{
	CSerialPort::eSerialStat CStat;
	unsigned int count, tOut;
	unsigned char bucket[10];


	// Get the users com port and open the port.
	if ((CStat = m_serialPort.SetConfiguration(m_userSettings.GetComPort())) != CSerialPort::COM_OK)
		return BAD_COMM_PORT;

	// Read any unread bytes
	m_serialPort.SetTimeout(1);			// set timeout to comeback right away
	count = 10;
	m_serialPort.SendData(NULL, 0, bucket, count);
	m_serialPort.SetTimeout(DEFAULT_SER_TIMEOUT);		// set standard timeout

	// sync the autostar
	tOut = 5;				// do a max of 5 times
	bucket[0] = '?';
	while ((bucket[0] == '?' || 
		(bucket[0] != 'A' && bucket[0] != 'P' && bucket[0] != 'L' && bucket[0] != 'D')) &&
		tOut-- > 0)	// if we do this 5 times then it could be an old Safe loader
	{
		count = 10;
		SendCommand(MODE, NULL, bucket, count);
		if (count == 0)
			return NO_AUTOSTAR_RESPONSE;
	}

	// check if an autostar responded
	if (bucket[0] == '?' || 
		bucket[0] == 'A' || bucket[0] == 'P' || bucket[0] == 'L' || bucket[0] == 'D')
		return AUTOSTAR_OK;
	else
		return NO_AUTOSTAR_RESPONSE;
}

/////////////////////////////////////////////
//
//	Name		:RetrieveVersion
//
//	Description :This function will get check the current mode of the autostar
//				 and then request the version. Also the m_mode variable will be set.
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CAutostar::RetrieveVersion()
{
	unsigned int count;
	unsigned char data[50];
	eAutostarStat stat;

	// check the mode
	count = 5;
	if ((stat=SendCommand(MODE, NULL, data, count)) != AUTOSTAR_OK)
		return stat;

	// clear out the buffer in case there was no response
	stat = InitComPort();

	switch (data[0]){

		// Operational mode
	case 'A' :
	case 'P' :
	case 'L' :
		m_mode = OPERATIONAL;			// set the mode member variable
		count = 50;
		if ((stat=SendCommand(VERSION, NULL, data, count)) == AUTOSTAR_OK)
		{
			TRACE("\n2nd SendCommand Response: %s\n", data);
			data[4] = 0;		// insert a null on top of the #
			m_version = data;
		}
		else
			return stat;
		break;

		// Download mode
	case 'D' :
		m_mode = DOWNLOAD;
		count = 4;
		if ((stat=SendCommand(VERSION, NULL, data, count)) != AUTOSTAR_OK)
			return stat;

		// insert a null at the end
		data[count] = 0;
		TRACE("\n2nd SendCommand Response: %s\n", data);

		// this is for version 22Er
		// if the return value is not ETX or the version has not been set before
		if (strcmp((char *)data, "ETX ") || m_version.GetLength() != 4)	
			// set the version
			m_version = data;

		break;

		// old safe loader
	case '?' :
		m_version	= "Safe";
		m_mode		= DOWNLOAD;
		break;

	}

	return	AUTOSTAR_OK;
}

/////////////////////////////////////////////
//
//	Name		:SendDownloadMode
//
//	Description :This will put the Autostar into download mode.
//
//  Input		:None
//
//	Output		:Autostar status
//
////////////////////////////////////////////
eAutostarStat CAutostar::SendDownloadMode()
{
	unsigned char			resp[5];
	eAutostarStat	stat;
	unsigned int	len;

// if we are not already in download then send the command
	if (m_mode != DOWNLOAD)			
	{ 
		len = 1;
		//if ((		//disable this code temporarily because new ASII rev does not send response
		stat = SendCommand(SET_DOWNLOAD_MODE, NULL, resp, len);
		//) != AUTOSTAR_OK)
		//	return stat;

	// it's now in download mode
		m_mode = DOWNLOAD;

	// Set to the max baud
//		m_lastError = SetBaud(CSerialPort::b125k);

//		return m_lastError;

	}

// if we're here then everything worked (we hope)
	return	AUTOSTAR_OK;		
}

/////////////////////////////////////////////
//
//	Name		:RetrieveType
//
//	Description :This will retrieve the type of handbox and set the model name and
//				 the model pointer. If its an original Autostar it will put it in
//				 download mode and leave it there.
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CAutostar::RetrieveType()
{
	unsigned char			resp[20];
	eAutostarStat	stat;
	unsigned int	len;
	bool			doAgain;

	do
	{
		doAgain = false;
	// send the TYPE command 
		len = 20;
		if((stat = SendCommand(TYPE, NULL, resp, len)) != AUTOSTAR_OK)
			return stat;

	// determine the handbox type
		switch (resp[0])
		{
		case 'L':
			SetModel(new CModelLX(this));	// the new LX
			m_modelName = "LX200 GPS";
			break;

		case 'A' :				// its an Autostar in Operational mode
			SendDownloadMode();	// put it in download mode
			doAgain = true;		// and send the TYPE command again
			break;

		case 'R' :
			SetModel(new CModelRCX(this));	// the new RCX
			m_modelName = "RCX400";
			break;

		case 0x06 :
			SetModel(new CModel494(this));
			m_modelName = "494";
			break;

		case 0x07 :		// Starfinder
			SetModel(new CModel494(this));
			m_modelName = "StarFinder";
			break;

			// it's a 495
		case 0x0A :
			m_modelName = "495";
			SetModel(new CModel497(this));	// they're the same to me
			break;

			// it's a 497
		case 0x0F :
			// it's an old safe loader
		case '?' :
			SetModel(new CModel497(this));	// they're the same to me
			m_modelName = "497";
			break;

		case 0x0E :
			SetModel(new CModelLX(this));	// the new LX
			m_modelName = "LX200 GPS";
			break;

		case 0x0D :
			SetModel(new CModelRCX(this));	// the new RCX400
			m_modelName = "RCX400";
			break;

		default :
			return UNKNOWN_AUTOSTAR;

		}
	}
	while(doAgain);

	return stat;

}

eAutostarStat CAutostar::SendTelescopeCommand(CAutostar::eTelescopeCmnd cmnd)
{
	eAutostarStat	stat;
	unsigned char	resp[10];
	unsigned int	count = 0;


	// get connected
	if (m_mode == UNKNOWN)
	{
		stat = InitializeConnection(false);

		if (stat != AUTOSTAR_OK)
			return stat;
	}


	// select the command
	switch (cmnd)
	{
	case CAutostar::MoveLeft :
		if (m_lastHorzCmnd != CAutostar::MoveLeft)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Me#", resp, count);
			m_lastHorzCmnd = CAutostar::MoveLeft;
		}
		break;

	case CAutostar::MoveRight :
		if (m_lastHorzCmnd != CAutostar::MoveRight)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Mw#", resp, count);
			m_lastHorzCmnd = CAutostar::MoveRight;
		}
		break;

	case CAutostar::MoveUp :
		if (m_lastVertCmnd != CAutostar::MoveUp)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Mn#", resp, count);
			m_lastVertCmnd = CAutostar::MoveUp;
		}
		break;

	case CAutostar::MoveDown :
		if (m_lastVertCmnd != CAutostar::MoveDown)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Ms#", resp, count);
			m_lastVertCmnd = CAutostar::MoveDown;
		}
		break;

	case CAutostar::StopAll :
		if (m_lastVertCmnd != CAutostar::StopAll || m_lastHorzCmnd != CAutostar::StopAll)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Q#", resp, count);
			m_lastVertCmnd = m_lastHorzCmnd = CAutostar::StopAll;
		}
		break;

	case CAutostar::StopVert :
		if (m_lastVertCmnd != CAutostar::StopVert)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Qn#", resp, count);
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Qs#", resp, count);
			m_lastVertCmnd = CAutostar::StopVert;
		}
		break;

	case CAutostar::StopHorz :
		if (m_lastHorzCmnd != CAutostar::StopHorz)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Qe#", resp, count);
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":Qw#", resp, count);
			m_lastHorzCmnd = CAutostar::StopHorz;
		}
		break;

	case CAutostar::SetSlewRateGuide :
		if (m_lastSpeedCmnd != CAutostar::SetSlewRateGuide)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":RG#", resp, count);
			m_lastSpeedCmnd = m_lastVertCmnd = m_lastHorzCmnd = CAutostar::SetSlewRateGuide;
		}
		break;

	case CAutostar::SetSlewRateCenter :
		if (m_lastSpeedCmnd != CAutostar::SetSlewRateCenter)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":RC#", resp, count);
			m_lastSpeedCmnd = m_lastVertCmnd = m_lastHorzCmnd = CAutostar::SetSlewRateCenter;
		}
		break;

	case CAutostar::SetSlewRateFind :
		if (m_lastSpeedCmnd != CAutostar::SetSlewRateFind)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":RM#", resp, count);
			m_lastSpeedCmnd = m_lastVertCmnd = m_lastHorzCmnd = CAutostar::SetSlewRateFind;
		}
		break;

	case CAutostar::SetSlewRateMax :
		if (m_lastSpeedCmnd != CAutostar::SetSlewRateMax)
		{
			stat = SendCommand(TELESCOPECMND, (unsigned char *)":RS#", resp, count);
			m_lastSpeedCmnd = m_lastVertCmnd = m_lastHorzCmnd = CAutostar::SetSlewRateMax;
		}
		break;


	}
	return stat;

}

/////////////////////////////////////////////
//
//	Name		:SendCommand
//
//	Description :This will send the enum command passed to it along with the data.
//				 Response to the command will be placed in the resp buffer along with its size in
//				 count. Since no command can illicit a response greater than 64 bytes,
//				 the response buffer is assumed to be at least that large.
//
//  Input		:Command, Data pointer, Responce pointer, count reference
//
//	Output		:Autostar status
//
////////////////////////////////////////////
eAutostarStat CAutostar::SendCommand(eAutostarCmnd cmd,unsigned char *data,unsigned  char *resp, unsigned int &count)
{
	unsigned char cmdStr[10];
	CSerialPort::eSerialStat		CStat;
	unsigned int respCnt;
	unsigned int sndCnt;
	
	switch(cmd)
	{
	case SET_BAUD_RATE :
		if (m_mode != DOWNLOAD)
		{
			return COMMAND_FAILED;
		}
		else
		{
			strcpy ((char *)cmdStr, "F");
			count = 1;
		}
		CStat = m_serialPort.SendData(cmdStr, strlen((char *)cmdStr), resp, count);
		break;

	case VERSION :
		if (m_mode == OPERATIONAL)		// the version command depends on the mode
		{
			strcpy((char *)cmdStr, ":GVN#");	// this will send back the version
			count = 41;
		}
		else
		{
			strcpy((char *)cmdStr, "V");		// this will send back the version
			count = 4;
		}
		CStat = m_serialPort.SendData(cmdStr, strlen((char *)cmdStr), resp, count);
		break;

	case MODE :		// the mode commands works in either mode
		count = 1;
		CStat = m_serialPort.SendData((unsigned char *)"\6", 1, resp, count);
		break;

	case TELESCOPECMND :
	// check for the right mode
		if (m_mode == DOWNLOAD)
			return WRONG_MODE;

	// Build the command
		strcpy((char *)&cmdStr[0], (char *)data);

	// Send the command to the serial port
		CStat = m_serialPort.SendData(cmdStr, strlen((char *)cmdStr), resp, count);
		break;



	case READ :
	// check for the right mode
		if (m_mode == OPERATIONAL)
			return WRONG_MODE;

	// Build the command
		cmdStr[0] = 'R';
		memcpy(&cmdStr[1], data, 4);

	// Send the command to the serial port
		CStat = m_serialPort.SendData(cmdStr, 5, resp, count);
		break;

	case SET_DOWNLOAD_MODE :
	// if its already in download mode then return OK
		if (m_mode == DOWNLOAD)
			return AUTOSTAR_OK;

	// Build the command
		cmdStr[0] = '\4';

	// send the command
		CStat = m_serialPort.SendData(cmdStr, 1, resp, count);
		break;

	case TYPE :
	// check for the right mode
		if (m_mode == OPERATIONAL)
		{
			// build the command
			strcpy((char *)cmdStr, ":GVP#");
			sndCnt = 5;
		}
		else
		{
		// Build the command
			cmdStr[0] = 'T';
			sndCnt = 1;
		}

	// send the command
		CStat = m_serialPort.SendData(cmdStr, sndCnt, resp, count);
		break;

	case ERASE_BANK :
	// check for the right mode
		if (m_mode == OPERATIONAL)
			return WRONG_MODE;

	// Build the command
		cmdStr[0] = 'E';
		cmdStr[1] = data[0];

	// change the timeout value
		m_serialPort.SetTimeout(ERASE_SER_TIMEOUT);

	// send the command
		CStat = m_serialPort.SendData(cmdStr, 2, resp, count);

	// change thetimeout back
		m_serialPort.SetTimeout(DEFAULT_SER_TIMEOUT);

		break;
	case WRITE_FLASH :
	// check for the right mode
		if (m_mode == OPERATIONAL)
			return WRONG_MODE;

	// Build the command
		data[0] = 'W'; // it should already be there
		respCnt = 1;
		sndCnt = count + 5;

	// Send the command to the serial port
		CStat = m_serialPort.SendData(data, sndCnt, resp, respCnt);
		break;

	case PROGRAM_EE :
	// check for the right mode
		if (m_mode == OPERATIONAL)
			return WRONG_MODE;

	// Build the command
		cmdStr[0] = 'P';
		memcpy(&cmdStr[1], data, 3);

	// Send the command to the serial port
		CStat = m_serialPort.SendData(cmdStr, 4, resp, count);
		break;

	case INIT :
	// check for the right mode
		if (m_mode == OPERATIONAL)
			return WRONG_MODE;

	// Build the command
		cmdStr[0] = 'I';
		respCnt = 1;
		sndCnt = 1;

	// change the timeout value
		m_serialPort.SetTimeout(ERASE_SER_TIMEOUT); //

		TRACE("\nCommand: %s\n",cmdStr);

	// Send the command to the serial port
		CStat = m_serialPort.SendData(cmdStr, sndCnt, resp, respCnt);

		TRACE("Response: %s\n", CString(resp).Left(100));

	// change thetimeout back
		m_serialPort.SetTimeout(DEFAULT_SER_TIMEOUT);

		break;
	}


	// figure out the communication status
	switch (CStat)
	{
	case CSerialPort::COM_OK :
		return	AUTOSTAR_OK;

	case CSerialPort::BAD_PORT :
		return BAD_COMM_PORT;

	case CSerialPort::NO_RESPONSE :
		return NO_AUTOSTAR_RESPONSE;
	}
	// if none of the above then, beats me?
	return UNKNOWN_ERROR;
}

/////////////////////////////////////////////
//
//	Name		:SetModel
//
//	Description :This will set the autostar model to the pointer passed to it.
//				 If m_model is not NULL, then it will delete it first and then assign
//				 the model to it.
//
//  Input		:CAutostarModel pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CAutostar::SetModel(CAutostarModel *model)
{
	// if it's not null
	if (m_model)
		// then delete whatever it was pointing at
		delete m_model;

	// assign the new model
	m_model = model;
}


/////////////////////////////////////////////
//
//	Name		:GetHandboxData
//
//	Description :This function will return a pointer to the current data from
//				 the autostar. It is the responsibility of the caller not to 
//				 delete this reference.
//
//  Input		:None
//
//	Output		:pointer to handbox data
//
////////////////////////////////////////////
CBodyDataCollection * CAutostar::GetHandboxData()
{
// send back the pointer
	return &m_handboxData;
}

/////////////////////////////////////////////
//
//	Name		:SendUserData
//
//	Description :This will convert the data collection to a Autostar memory image
//				 and send it to the Autostar. If the Data pointer is Null it will
//				 convert and send m_handboxData.
//
//  Input		:Data to send or Null to send m_handboxData
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CAutostar::SendUserData(CBodyDataCollection *Data, bool spawnThread)
{
// if it's already happening then don't do it again
	if (m_mode == BUSY)
		return AUTOSTAR_BUSY;

// initialize autostar connection
	m_lastError = m_model->InitializeSend(false);

// check data
	if (Data == NULL)
		Data = &m_handboxData;

// check if this data will fit in the handbox
	if (m_lastError == AUTOSTAR_OK && GetAvailableMemory(Data) < 0)
		m_lastError = OUT_OF_MEMORY;

// Send the data
	if (m_lastError == AUTOSTAR_OK)
		m_lastError = m_model->SendUserData(Data, spawnThread);

	if (m_lastError != AUTOSTAR_UPLOADING)
	{
	// report the status to the callback
		if (m_stat)
			m_stat->SendComplete(m_lastError);

	// reset the mode
		m_mode = UNKNOWN;

	// close the port
		m_serialPort.ClosePort();
	}

	return m_lastError;
}

/////////////////////////////////////////////
//
//	Name		:SetStatCallBack
//
//	Description :Set the status callback pointer
//
//  Input		:stat pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CAutostar::SetStatCallBack(CAutostarStat *stat)
{
	m_stat = stat;
}

/////////////////////////////////////////////
//
//	Name		:RestartHandbox
//
//	Description :Sends the init command to the handbox.
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CAutostar::RestartHandbox()
{
	unsigned char resp[5];
	unsigned int cnt = 1;


	// only do this if not busy
	if (m_mode != BUSY)
	{
		// init the com port first
		InitComPort();
		eAutostarStat stat = SendCommand(INIT, NULL, resp, cnt);
//		if (stat == AUTOSTAR_OK)	// took this out 7/25/02 because 494's don't respond
		m_mode = OPERATIONAL;	// added 02/06/02
		if (m_stat)
			m_stat->DoingProcess("Handbox Restarted");
	}

}

/////////////////////////////////////////////
//
//	Name		:PowerCycleHandbox
//
//	Description :Restart the Handbox Anytime, Anywhere, Any Model
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
eAutostarStat CAutostar::PowerCycleHandbox()
{
	eAutostarStat stat;

	// if not in download mode already, put in download mode
	if (m_mode != DOWNLOAD)
	{
		stat = InitializeConnection(false);

		if (stat != AUTOSTAR_OK)
			return stat;

		SendDownloadMode();
	}

	// Send the restart command
	RestartHandbox();

	m_mode = OPERATIONAL;

	// Close the COM port
	CloseSerialPort();

	return AUTOSTAR_OK;
}

/////////////////////////////////////////////
//
//	Name		:GetLastError
//
//	Description :Returns the definition of the last error as a CString for easy use.
//
//  Input		:None
//
//	Output		:Error String
//
////////////////////////////////////////////
CString CAutostar::GetLastError()
{
	switch (m_lastError)
	{
	case AUTOSTAR_OK :
		return "Autostar OK";
		
	case AUTOSTAR_DOWNLOADING :
		return "Downloading";
			
	case AUTOSTAR_UPLOADING :
		return "Uploading";
		
	case AUTOSTAR_BUSY :
		return "Busy";
		
	case BAD_COMM_PORT :
		return "Bad Comm Port";
				
	case NO_AUTOSTAR_RESPONSE :
		return "No Response";
		
	case UNKNOWN_AUTOSTAR :
		return "Unknown Autostar Model";
		
	case OUT_OF_MEMORY :
		return "Out of Memory";
		
	case ERASE_ERROR :
		return "Erase Error";
		
	case WRITE_ERROR :
		return "Write Error";

	case READ_ERROR:
		return "Read Error";
		
	case UNKNOWN_ERROR :
		return "Unknown Error";
		
	case WRONG_MODE :
		return "Wrong Mode";

	case BAD_FILE :
		return "Bad File Data in EEProm hole";

	case BAD_CHECKSUM :
		return "Bad Checksum";

	case NO_PAGE7_FILE :
		return "No Page 7 File found";
		
	case NOT_ALLOWED :
		return "Not Allowed";
		
	case VERIFY_FAILED :
		return "Verify Failed";
		
	case COMMAND_FAILED :
		return "Command Failed";

	case USEROBJEX_RETRIEVE_ERROR :
		return "Failed to Parse Custom User Object Data";

	default :
		return "Unknown Error Code";

	}
}

/////////////////////////////////////////////
//
//	Name		:RetrieveUserData
//
//	Description :This is the first RetrieveUserData function. It will
//				 Initialize connection to the Autostar and set the model.
//				 If all goes well it will call the model specifice version
//				 of RetrieveUserData.
//
//  Input		:None
//
//	Output		:Autostar Status
//
////////////////////////////////////////////
eAutostarStat CAutostar::RetrieveUserData(bool spawnThread)
{
	// if it's already happening then don't do it again
	if (m_mode == BUSY)
		return AUTOSTAR_BUSY;

	// initialize autostar connection
	m_lastError = InitializeConnection(false, false);

	// Get the User Data Memory image from autostar and convert it to CBodyDataCollection
	if (m_lastError == AUTOSTAR_OK)
		m_lastError = m_model->RetrieveUserData(spawnThread);

	// if there was a failure
	if (m_lastError != AUTOSTAR_DOWNLOADING)
	{
	// report the status to the callback
		if (m_stat)
			m_stat->SendComplete(m_lastError);

	// reset the mode
		m_mode = UNKNOWN;

	// close the port
		m_serialPort.ClosePort();
	}


	return m_lastError;

}

/////////////////////////////////////////////
//
//	Name		:AvailableMemory
//
//	Description :Returns the amount of space available if the passed data were loaded
//				 in the Autostar. If data is NULL then m_handboxData is used. If autostar
//				 not initialized then 0 is returned.
//
//  Input		:CBodyDataCollection pointer or NULL
//
//	Output		:Available Memory
//
////////////////////////////////////////////
int CAutostar::GetAvailableMemory(CBodyDataCollection *data)
{
	// if no autostar then return 0
	if (m_model == NULL)
		return 0;

	// if no data then use my data
	if (data == NULL)
		data = &m_handboxData;

	// do the difficult task of counting all the data
	return m_model->GetMaxUserData() - data->GetTotalSizeOf();
}

/////////////////////////////////////////////
//
//	Name		:SendProgram
//
//	Description :This will send the program to the Autostar.
//
//  Input		:filename
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CAutostar::SendProgram(CString fileName, bool spawnThread, bool eraseBanks)
{
// if it's already happening then don't do it again
	if (m_mode == BUSY)
		return AUTOSTAR_BUSY;

// initialize autostar connection
	m_lastError = InitializeConnection(false);

// check data
	if (m_lastError == AUTOSTAR_OK && m_persist.ReadFile(fileName) != CPersist::READCOMPLETE)
		m_lastError = BAD_FILE;

ReleaseHdType *hdr;

	if (m_lastError == AUTOSTAR_OK)
	{
	// get a reference to the header
		hdr = (ReleaseHdType *)m_persist.m_dataPtr;
		
	// increment past the header
		m_persist.IncrementIndex(sizeof(ReleaseHdType));

	// do a checksum
		long int count	= 0L;
		long int chksum = 0L;
		while(m_persist.m_dataIndex < m_persist.m_dataReadCnt)
		{
			chksum += *((unsigned char *)m_persist.m_indexPtr);
			m_persist.IncrementIndex(1);
			count++;
		}
	// reset the index back to the beginning of the file
		m_persist.ResetIndex();

	// compare the checksum.
		if (chksum != hdr->checksum)
			m_lastError = BAD_CHECKSUM;
		else
		
		// turn things over to the worker bees
			m_lastError = m_model->SendProgram(spawnThread, eraseBanks);
	}

	// if there was a failure
	if (m_lastError != AUTOSTAR_UPLOADING)
	{
	// report the error to the callback
		if (m_stat)
			m_stat->SendComplete(m_lastError);

	// reset the mode
		m_mode = UNKNOWN;

	// close the port
		m_serialPort.ClosePort();
	}

	return m_lastError;
}

CString CAutostar::GetVersion()
{
	return m_version;
}

/////////////////////////////////////////////
//
//	Name		:ClearPresets
//
//	Description :This will clear the Autostar Presets.
//
//  Input		:closeport true(default) - Close port when done
//						   false         - Leave port open
//
//	Output		:Autostar status
//
////////////////////////////////////////////
eAutostarStat CAutostar::ClearPresets(bool closeport)
{
	if (m_model)
	{
		m_lastError = m_model->ClearPresets(closeport);
		return m_lastError;
	}
	else
		return COMMAND_FAILED;
}

void CAutostar::SetVerifyMode(bool mode)
{
	m_verifyMode = mode;
}

bool CAutostar::GetVerifyMode()
{
	return m_verifyMode;
}

CString CAutostar::GetModel()
{
	return m_modelName;
}


/////////////////////////////////////////////
//
//	Name		:FindAutostar
//
//	Description :Sets the com port from 1 to 4 and tests for an Autostar.
//				 If one is found then it sets the registry to it and returns
//				 true, else if no autostar is found it returns false.
//
//  Input		:None
//
//	Output		:Found Autostar
//
////////////////////////////////////////////
bool CAutostar::FindAutostar(bool closeComPort)
{
CString		testCom;
CString		OrigComPort = m_userSettings.GetComPort();
char		cTemp[10];

// loop through 255 comm ports
	for (int i = 1; i<= 256; i++)
	{
	// set the com string
		sprintf(cTemp, "COM%d", i);
		testCom = CString(cTemp);
	// make this the com port that InitComPort will use
		m_userSettings.SetComPort(testCom);

		switch (InitComPort())
		{
		case AUTOSTAR_OK :
			TRACE("Trying Com Port: %s at speed: %i\n",testCom,m_serialPort.GetBaud());
			InitializeConnection(closeComPort);
			return true;

		
		case NO_AUTOSTAR_RESPONSE :

			// switch to the other baud rate
			if (m_serialPort.GetBaud() == m_serialPort.GetDefaultBaud())
			{
				TRACE("Switching to Max Baud\n");
				m_serialPort.SetBaud(m_serialPort.GetMaxBaud());
			}
			else
			{
				TRACE("Switching to Default Baud\n");
				m_serialPort.SetBaud(m_serialPort.GetDefaultBaud());
			}

			// repeat the test
			if (InitComPort() == AUTOSTAR_OK)
			{
				TRACE("Trying Com Port: %s at speed: %i\n",testCom,m_serialPort.GetBaud());
				InitializeConnection(closeComPort);
				return true;
			}
			break;

		case BAD_COMM_PORT :
		default :
			break;

		}
	// close the port
		if (closeComPort)
			m_serialPort.ClosePort();
	}

// set com port to original value
	m_userSettings.SetComPort(OrigComPort);

	return false;

}


/////////////////////////////////////////////
//
//	Name		:DeleteOneObject
//
//	Description :Deletes a single object by object type and name
//
//  Input		:Class, CString name
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CAutostar::DeleteOneObject(BodyType bodyType, CString objectName)
{
	return m_model->DeleteOneObject(bodyType, objectName);
}

/////////////////////////////////////////////
//
//	Name		:DeleteCatalog
//
//	Description :Deletes an entire object class by body type
//
//  Input		:Body Type
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CAutostar::DeleteCatalog(BodyType bodyType)
{
	return m_model->DeleteCatalog(bodyType);
}

/////////////////////////////////////////////
//
//	Name		:SendOneObject
//
//	Description :Formats and sends one user data object to the LX.
//
//  Input		:body data, Body Type
//
//	Output		:Stat, int # of Bytes sent
//
////////////////////////////////////////////
eAutostarStat CAutostar::SendOneObject(CBodyData *body, BodyType bodyType, int &sentBytes)
{
	return m_model->SendOneObject(body, bodyType, sentBytes);
}



/////////////////////////////////////////////
//
//	Name		:Force Garbage Collection
//
//	Description :Tells the handbox to garbage collect (if supported)
//
//  Input		:None
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CAutostar::ForceGarbageCollection()
{
	eAutostarStat stat;

	stat = InitializeConnection(false,false);

	if (stat != AUTOSTAR_OK)
		return stat;

	stat = m_model->ForceGarbageCollection();

	if (m_stat && stat == AUTOSTAR_OK)
	{
		m_stat->DoingProcess("Garbage Collection Completed");
	}
	else
	{
		m_stat->DoingProcess(GetLastError());
	}
	MessageBeep(MB_ICONINFORMATION);

	CloseSerialPort();

	return stat;
}


/////////////////////////////////////////////
//
//	Name		:Set the Maximum Baud Rate
//
//	Description :Changes the maximum baud rate of the registry, 
//				 handbox and serial port (if supported)
//
//  Input		:baud rate
//				 justHandbox = do not set registry value
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CAutostar::SetMaxBaudRate(CSerialPort::eBaud baud, bool justHandbox)
{
	if (m_model)	// i.e., handbox is connected
		return m_model->SetMaxBaudRate(baud, justHandbox);

	else	// just set the registry value
	{
		m_serialPort.SetMaxBaud(baud);
		return AUTOSTAR_OK;
	}
}



/////////////////////////////////////////////
//
//	Name		:CheckDownloadMode
//
//	Description :Checks the handbox to see if it is in download mode
//				 if so, sets m_mode to DOWNLOAD
//
//  Input		:none
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CAutostar::CheckDownLoadMode()
{
	if (m_mode == BUSY)
		return AUTOSTAR_BUSY;

	unsigned char			resp[20];
	eAutostarStat	stat;
	unsigned int	len;

	if ((stat = InitializeConnection(false,false)) != AUTOSTAR_OK)
		return stat;

	// send the TYPE command 
	len = 20;
	if((stat = SendCommand(TYPE, NULL, resp, len)) != AUTOSTAR_OK)
		return stat;

// determine the handbox type
	switch (resp[0])
	{
	case 'L':
	case 'A' :	// either of these responses mean its in Operational Mode
	case 'R' :
		m_mode = OPERATIONAL;
		break;
	case 0x0e:	// we know its in Download AND its an AS2
		m_mode = DOWNLOAD;
		m_modelName = "LX200 GPS";
		break;
	default:	// anything else and its in Download mode
		m_mode = DOWNLOAD;
	}

	return stat;
}

eAutostarStat CAutostar::TestFunction()
{
	return m_model->TestFunction();
}

/////////////////////////////////////////////
//
//	Name		:GetUserInfo
//
//	Description :Retrieves the User Info and Personal Info records from the HBX
//
//  Input		:pointer to CUserInfo object
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CAutostar::GetUserInfo(CUserInfo *pInfo)
{
	if (m_model)
		return m_model->GetUserInfo(pInfo);
	else
		return COMMAND_FAILED;
}

/////////////////////////////////////////////
//
//	Name		:GetSiteInfo
//
//	Description :Retrieves the collection of sites from the HBX
//
//  Input		:pointer to a BodyDataCollection of sites
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CAutostar::GetSiteInfo(CBodyDataCollection *siteList)
{
	if (m_model)
		return m_model->GetSiteInfo(siteList);
	else
		return COMMAND_FAILED;
}

/////////////////////////////////////////////
//
//	Name		:SetUserInfo
//
//	Description :Sends the User Info and Personal Info records to the HBX
//
//  Input		:pointer to CUserInfo object
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CAutostar::SetUserInfo(CBodyData *data)
{
	if (m_model)
		return m_model->SetUserInfo(data);
	else
		return COMMAND_FAILED;
}


/////////////////////////////////////////////
//
//	Name		:SetSiteInfo
//
//	Description :Sends the collection of selected sites to the HBX
//
//  Input		:pointer to CBodyDataCollection object containing only sites
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CAutostar::SetSiteInfo(CBodyDataCollection *siteList)
{
	if (m_model)
		return m_model->SetSiteInfo(siteList);
	else
		return COMMAND_FAILED;
}

/////////////////////////////////////////////
//
//	Name		:GetMaxSitesAllowed
//
//	Description :returns the number of sites allowed by the handbox
//
//  Input		:none
//
//	Output		:int
//
////////////////////////////////////////////
int CAutostar::GetMaxSitesAllowed()
{
	if (m_model)
		return m_model->m_maxUserSites;
	else
		return 0;

}

/////////////////////////////////////////////
//
//	Name		:CheckSiteDelete
//
//	Description :Applies the handbox-specific logic to determine if it
//				 is legal to delete all sites
//
//  Input		:bool TRUE = only wish to delete current site
//				     FALSE = wish to delete all sites
//
//	Output		:bool whether to proceed with deletion
//
////////////////////////////////////////////
bool CAutostar::CheckSiteDelete(bool currentSite)
{
	if (m_model)
		return m_model->CheckSiteDelete(currentSite);
	else
		return false;
}

/////////////////////////////////////////////
//
//	Name		:GetPECTable
//
//	Description :Gets the PECTable from the telescope
//
//  Input		:pointer to CBodyData object,
//				 axis - AXIS_RA or AXIS_DEC
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CAutostar::GetPECTable(CBodyData *table, ePECAxis axis)
{
	if (m_model)
		return m_model->GetPECTable(table, axis);
	else
		return COMMAND_FAILED;

}

/////////////////////////////////////////////
//
//	Name		:SetPECTable
//
//	Description :Sends the PECTable to the telescope
//
//  Input		:pointer to CBodyData object,
//				 axis - AXIS_RA or AXIS_DEC
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CAutostar::SetPECTable(CBodyData *table, ePECAxis axis)
{
	if (m_model)
		return m_model->SetPECTable(table, axis);
	else
		return COMMAND_FAILED;

}
/////////////////////////////////////////////
//
//	Name		:SetBaud
//
//	Description :This function will call the specific model
//
//  Input		:baud
//
//	Output		:status
//
////////////////////////////////////////////
eAutostarStat CAutostar::SetBaud(CSerialPort::eBaud baud)
{
	if (m_model)
		return m_model->SetBaud(baud);
	else
		return COMMAND_FAILED;
}

