/*
 *  Autostar.cpp
 *  AutoStarX
 *
 *  Created by roro on 6/3/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Autostar.h"

Autostar::Autostar()
{
	m_model = NULL;
	m_modelName = CFSTR("Unknown");
	m_hbxSafeMode = FALSE;
	mPortIO = NULL;   
	m_connected=false;
	m_hbxSafeMode=false;
	
}

Autostar::~Autostar()
{
	if (m_model)
		delete m_model;
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
eAutostarStat Autostar::SendCommand(eAutostarCmnd cmd, Byte *data, Byte *resp, unsigned int &count)
{
	unsigned char cmdStr[10];
	unsigned int respCnt;
	unsigned int sndCnt;
	bool serialStatus;
	
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
		serialStatus = mPortIO->SendData(cmdStr, strlen((char *)cmdStr), resp, count);
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
		serialStatus = mPortIO->SendData(cmdStr, strlen((char *)cmdStr), resp, count);
		break;

	case MODE :		// the mode commands works in either mode
		count = 1;
		serialStatus = mPortIO->SendData((unsigned char *)"\6", 1, resp, count);
		break;

	case TELESCOPECMND :
	// check for the right mode
		if (m_mode == DOWNLOAD)
			return WRONG_MODE;

	// Build the command
		strcpy((char *)&cmdStr[0], (char *)data);

	// Send the command to the serial port
		serialStatus = mPortIO->SendData(cmdStr, strlen((char *)cmdStr), resp, count);
		break;



	case READ :
	// check for the right mode
		if (m_mode == OPERATIONAL)
			return WRONG_MODE;

	// Build the command
		cmdStr[0] = 'R';
		memcpy(&cmdStr[1], data, 4);

	// Send the command to the serial port
		serialStatus = mPortIO->SendData(cmdStr, 5, resp, count);
		break;

	case SET_DOWNLOAD_MODE :
	// if its already in download mode then return OK
		if (m_mode == DOWNLOAD)
			return AUTOSTAR_OK;

	// Build the command
		cmdStr[0] = '\4';

	// send the command
		serialStatus = mPortIO->SendData(cmdStr, 1, resp, count);
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
		serialStatus = mPortIO->SendData(cmdStr, sndCnt, resp, count);
		break;

	case ERASE_BANK :
	// check for the right mode
		if (m_mode == OPERATIONAL)
			return WRONG_MODE;

	// Build the command
		cmdStr[0] = 'E';
		cmdStr[1] = data[0];

	// change the timeout value
		//m_serialPort.SetTimeout(ERASE_SER_TIMEOUT);

	// send the command
		serialStatus = mPortIO->SendData(cmdStr, 2, resp, count);

	// change thetimeout back
		//m_serialPort.SetTimeout(DEFAULT_SER_TIMEOUT);

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
		serialStatus = mPortIO->SendData(data, sndCnt, resp, respCnt);
		break;

	case PROGRAM_EE :
	// check for the right mode
		if (m_mode == OPERATIONAL)
			return WRONG_MODE;

	// Build the command
		cmdStr[0] = 'P';
		memcpy(&cmdStr[1], data, 3);

	// Send the command to the serial port
		serialStatus = mPortIO->SendData(cmdStr, 4, resp, count);
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
		//m_serialPort.SetTimeout(ERASE_SER_TIMEOUT); //

		//TRACE("\nCommand: %s\n",cmdStr);

	// Send the command to the serial port
		serialStatus = mPortIO->SendData(cmdStr, sndCnt, resp, respCnt);

		//TRACE("Response: %s\n", CString(resp).Left(100));

	// change thetimeout back
		//m_serialPort.SetTimeout(DEFAULT_SER_TIMEOUT);

		break;
	}


	// figure out the communication status
	switch (serialStatus)
	{
	case COM_OK :
		return	AUTOSTAR_OK;

	case BAD_PORT :
		return BAD_COMM_PORT;

	case NO_RESPONSE :
		return NO_AUTOSTAR_RESPONSE;
	}
	// if none of the above then, beats me?
	return UNKNOWN_ERROR;
}


eAutostarStat Autostar::ConnectToAutostar(const char *bsdPath)
{
    Byte ioBuffer[64];
    Byte cmd[2];
	int i;

    mPortIO=new SerialPortIO(bsdPath);
    // open port at 9600  
    if(! mPortIO->OpenSerialPort(bsdPath, 9600))
        { // error opening port
        return NO_AUTOSTAR_RESPONSE;
        }

	m_hbxSafeMode=true;
	
	// flush all data before starting
	mPortIO->ReadData(ioBuffer,64);

    for(i=0;i<11;i++)
        {
        // check AutoStarX status : cmd=0x06
        cmd[0]=0x06;    // ^F (1 byte response)
        cmd[1]=0;
        if(!mPortIO->SendData(cmd,1))
            {
            DisconnectFromAutostar();		
            return WRITE_ERROR;
            }
        usleep(100000);
        if(!mPortIO->ReadData(ioBuffer,1))
            {
            DisconnectFromAutostar();
            return READ_ERROR;
            }
 
        if(ioBuffer[0]!='?')
            {
            m_hbxSafeMode=false;
            break;
            }            

        } 

    if(ioBuffer[0]!='D' && !m_hbxSafeMode) // not in download mode
        {
        // switch to download mode
        cmd[0]=0x04;    // ^D (1 byte response)
        cmd[1]=0;
		if(!mPortIO->SendData(cmd,1))
			{
			DisconnectFromAutostar();
            return WRITE_ERROR;
			}
		usleep(500000);	
		if(!mPortIO->ReadData(ioBuffer,1))
			{
			DisconnectFromAutostar();
            return READ_ERROR;
			}

        }
	m_connected=true;

	return AUTOSTAR_OK;
	
}

void Autostar::DisconnectFromAutostar()
{

}

eAutostarMode Autostar::CheckDownLoadMode()
{
	if(m_hbxSafeMode)
		return SAFE_MODE;
	else if (m_connected)
		return DOWNLOAD;
	else
		return UNKNOWN;
}

eAutostarStat Autostar::SendDownloadMode()
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

const char* Autostar::getModelName(char *buffer)
{
	return CFStringGetCString(m_modelName,buffer, sizeof(buffer), kCFStringEncodingASCII);
}

ASType Autostar::GetModel()
{
    Byte ioBuffer[64];
	bool doAgain;
	eAutostarStat serialStatus;
	unsigned int retLen=1;
	
	if (!m_connected)
		return ERROR;
	do
		{
		serialStatus=SendCommand(TYPE,NULL,ioBuffer,retLen);
		if (!serialStatus)
			{
			DisconnectFromAutostar();
			return ERROR;
			}
			
			// set the device type control to the AutoStarX type
		switch(ioBuffer[0])
			{
			case 'L':
				//SetModel(new CModelLX(this));	// the new LX
				m_modelName = CFSTR("LX200 GPS");
				return TYPE_AUTOSTAR2;
				break;

			case 'A' :				// its an Autostar in Operational mode
				SendDownloadMode();	// put it in download mode
				doAgain = true;		// and send the TYPE command again
				break;

			case 'R' :
				//SetModel(new CModelRCX(this));	// the new RCX
				m_modelName = CFSTR("RCX400");
				return TYPE_RCX;
				break;

			case 0x06 :
				//SetModel(new CModel494(this));
				m_modelName = CFSTR("494");
				return TYPE_AUTOSTAR;
				break;

			case 0x07 :		// Starfinder
				//SetModel(new CModel494(this));
				m_modelName = CFSTR("StarFinder");
				break;

				// it's a 495
			case 0x0A :
				m_modelName = CFSTR("495");
				// SetModel(new CModel497(this));	// they're the same to me
				return TYPE_AUTOSTAR;
				break;

				// it's a 497
			case 0x0F :
				// it's an old safe loader
			case '?' :
				//SetModel(new CModel497(this));	// they're the same to me
				m_modelName = CFSTR("497");
				return TYPE_AUTOSTAR;
				break;

			case 0x0E :
				// SetModel(new CModelLX(this));	// the new LX
				m_modelName = CFSTR("LX200 GPS");
				return TYPE_AUTOSTAR2;
				break;

			case 0x0D :
				// SetModel(new CModelRCX(this));	// the new RCX400
				m_modelName = CFSTR("RCX400");
				return TYPE_RCX;
				break;

			default :
				return TYPE_UNKNOWN;

			}

		} while(doAgain);

	return TYPE_UNKNOWN;
	
}
