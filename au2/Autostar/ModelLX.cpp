// ModelLX.cpp: implementation of the CModelLX class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ModelLX.h"
#include "UserObjEx.h"
#include "UserInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define		WRITE_CMD_LEN 19
#define		BACKUP_10B_FILENAME "backup10b.bak"
#define		BACKUP2_10B_FILENAME "backupOriginal.bak"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
UINT AutostarThread( LPVOID pParam );

CModelLX::~CModelLX()
{

}

/////////////////////////////////////////////
//
//	Name		:CModelLX Constructor
//
//	Description :This is the only constructor that should be used for this class.
//				 It passes the autostar pointer to it's parent and then
//				 sets the model specific variables.
//
//  Input		:CAutostar pointer
//
//	Output		:None
//
////////////////////////////////////////////
CModelLX::CModelLX(CAutostar *autostar) : CAutostarModel(autostar)
{
	m_pageAddrStart = 0x8000;		// start address of paged pages
	m_pageAddrEnd	= 0x10000;		// end address of paged pages
	m_eePromStart	= 0x0000;		// start of EEProm hole
	m_eePromEnd		= 0x0000;		// end of EEProm hole
	m_userPageStart	= 0;			// First user data page
	m_userPageEnd	= 0;			// Last user data page
	m_readBlockSize	= 64;			// Maximum binary read block size
	m_writeBlockSize =64;			// Maximum write block size
	firstfail		= true;			// for testing only

	m_pages = m_userPageEnd - m_userPageStart;
	m_holeSize = m_eePromEnd - m_eePromStart;
	m_pageSize = (m_pageAddrEnd - m_pageAddrStart) - m_holeSize;
	m_maxUserData	= 307200; //m_pages * m_pageSize;		// maximum user data space with hole cut out
	m_totalPages	= 112;
	m_freemem		= 0;
	m_from10B		= false;

	m_eraseBanks	= false;
	m_maxUserSites	= SHRT_MAX;
	m_userBankFrom	= 0x30;
	m_userBankTo	= 0x37;
}
/////////////////////////////////////////////
//
//	Name		:GetMaxUserData
//
//	Description :Returns the maximum user data space in bytes.
//
//  Input		:None
//
//	Output		:Max user data space
//
////////////////////////////////////////////
int CModelLX::GetMaxUserData()
{
/*	Don't worry about the dynamic amount of memory ACTUALLY available anymore.
	Just use an conservative number for the maximum memory
// if freemem has not been set & autostar is not busy
	if (m_freemem == 0 && (m_autostar->m_mode != BUSY))
	{
		RetrieveFreeMemory(false);		// then get it from autostar and close the serial port
	}

	return m_freemem; 
*/
	return (int)m_maxUserData;

}



/////////////////////////////////////////////
//
//	Name		:SendUserDataThread
//
//	Description :This is the device specific version of SendUserData for
//				 the LX model of Autostar. It will take care of erasing
//				 all the current user data in the LX and then force a 
//				 garbage collection and then send the new user data.
//
//  Input		:Data
//
//	Output		:Status
//
////////////////////////////////////////////
void CModelLX::SendUserDataThread()
{
	eAutostarStat stat;
	CString statString;

// tell them what we're going to do
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Erasing old Data");

// Erase all user data
	for(int i=LandmarkClass; i < MaxClass;//UserObjectClass+1;
		i++)
	{
		// compensate for the fact that there are no classes numbered between
		// 13 and 19
		if (i == 13) i = UserObjectClass;
		if (m_autostar->m_stat)
		{
			statString.Format("Deleting Catalog %i of %i: %s",
				i < UserObjectClass ? i-LandmarkClass+1 : i-13,
				MaxClass-LandmarkClass-6,   
				GetCatalogString((RecordClass)i));
			m_autostar->m_stat->DoingProcess(statString);
		}
		stat = DeleteCatalog((RecordClass)i);
		if (stat != AUTOSTAR_OK)
			break;
	}


// Send garbage collection command
	if (stat == AUTOSTAR_OK)
	{
	// see if we are getting low on memory, if so, force garbage collection
		if (RetrieveFreeMemory() - m_SendData->GetTotalSizeOf(All) < 30000)
		{
		// tell them what we're going to do
			if (m_autostar->m_stat)
				m_autostar->m_stat->DoingProcess("Garbage Collecting");
			TRACE("Pre GC: Total Data Size = %i, FreeMemory = %i",
				m_SendData->GetTotalSizeOf(All), RetrieveFreeMemory());
			stat = ForceGarbageCollection();
		}
	}

// before we go any further, make sure enough memory is available
	if (m_SendData->GetTotalSizeOf(All) > RetrieveFreeMemory())
	{
		stat = OUT_OF_MEMORY;
		TRACE("Post GC Out of Memory: Total Data Size = %i, FreeMemory = %i",
			m_SendData->GetTotalSizeOf(All), RetrieveFreeMemory());
	}

// Send user Data
	if (stat == AUTOSTAR_OK)
	{
	// tell them what we're going to do
		if (m_autostar->m_stat)
			m_autostar->m_stat->DoingProcess("Sending User Data");

		stat = SendAllUserdata();
	}


	m_autostar->m_lastError = stat;

	m_autostar->m_mode = OPERATIONAL;

// close the port
	m_autostar->m_serialPort.ClosePort();

	// send the final status
	if (m_autostar->m_stat)
		m_autostar->m_stat->SendComplete(stat,(m_from10B) ? true : false);
}

/////////////////////////////////////////////
//
//	Name		:SendUserData
//
//	Description :This will start the thread which will call SendUserDataThread
//
//  Input		:Data
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModelLX::SendUserData(CBodyDataCollection *Data, bool spawnThread)
{
	// set the data to be sent
	m_SendData = Data;
	m_autostar->m_mode = BUSY;

	// change baud rate
	SetBaud(CSerialPort::bMax);

	// tell the thread routine what to do
	m_threadTask = CAutostarModel::SEND_USER_DATA;
	// Start the thread if spawnThread = true (default)
	if (spawnThread)
		AfxBeginThread(AutostarThread, this);
	else	// else call the function in the current thread
		AutostarThread(this);

	return AUTOSTAR_UPLOADING;
}

/////////////////////////////////////////////
//
//	Name		:RetrieveUserDataThread
//
//	Description :This is the thread that will do all the work. 
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CModelLX::RetrieveUserDataThread()
{
	eAutostarStat	stat;

// get the user data image
	stat = RetrieveAllUserdata();

// close the port
	m_autostar->m_serialPort.ClosePort();

// Report the status
	if (m_autostar->m_stat)
		m_autostar->m_stat->RetrieveComplete(stat,(m_from10B) ? true : false);

	m_autostar->m_mode = OPERATIONAL;

}

/////////////////////////////////////////////
//
//	Name		:RetrieveUserData
//
//	Description :This is the concrete version of RetrieveUserData for the 494 and 497 models
//				 of Autostar. Who knows what the future may bring? This will start a thread
//				 which will do all the work while this routines will return with an 
//				 AUTOSTAR_DOWNLOADING status.
//				 status
//				
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CModelLX::RetrieveUserData(bool spawnThread)
{
	// set to busy so it can't be called again
	m_autostar->m_mode = BUSY;

	// change baud rate
	SetBaud(CSerialPort::bMax);

	// tell the thread routine what to do
	m_threadTask = CAutostarModel::RETR_USER_DATA;
	// Start the thread if spawnThread = true (default)
	if (spawnThread)
		AfxBeginThread(AutostarThread, this);
	else	// else call the function in the current thread
		AutostarThread(this);

	return AUTOSTAR_DOWNLOADING;
}

/////////////////////////////////////////////
//
//	Name		:SendProgram
//
//	Description :This will start the thread which will call SendProgramThread
//
//  Input		:Data
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModelLX::SendProgram(bool spawnThread, bool eraseBanks)
{
	eAutostarStat stat;

	m_eraseBanks = eraseBanks;

	// change baud rate
	stat = SetBaud(CSerialPort::bMax);

	if (stat != AUTOSTAR_OK)
		return stat;

	// check if upgrading from 10b
	CString temp = m_autostar->GetVersion();
	if (m_autostar->GetVersion() == "1.0b")			
		stat = UpgradeFrom10B();

	// put in download mode
	stat = m_autostar->SendDownloadMode();
	if (stat != AUTOSTAR_OK)
		return stat;

	// erase the user banks if the flag has been set
	if (m_eraseBanks)
		stat = EraseUserBanks();

	if (stat != AUTOSTAR_OK)
		return stat;

	// set the data to be sent
	m_autostar->m_mode = BUSY;


	if (stat != AUTOSTAR_OK)
		return stat;
	// start the thread
	m_threadTask = CAutostarModel::SEND_PROGRAM;
	// Start the thread if spawnThread = true (default)
	if (spawnThread)
		AfxBeginThread(AutostarThread, this);
	else	// else call the function in the current thread
		AutostarThread(this);

	return AUTOSTAR_UPLOADING;
}


/////////////////////////////////////////////
//
//	Name		:DeleteAllRecords
//
//	Description :This will erase all the records of the specified class.
//
//  Input		:Class
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::DeleteAllRecords(RecordClass LXclass)
{
	eAutostarStat	stat;
	int count;
// while the count is not zero keep deleteing index 1
	while((count = RetrieveCount(LXclass)) > 0)
	{
		stat = DeleteDynamicObject(LXclass, 1);
		if (stat != AUTOSTAR_OK)
			return stat;
	}

	if (count < 0)
		return UNKNOWN_ERROR;
	else
		return AUTOSTAR_OK;

}


/////////////////////////////////////////////
//
//	Name		:RetrieveCount
//
//	Description :Sends the command to the LX to retrieve the number of objects of 
//				 the specified class. It the returns the count received. returns
//				 -1 if there was an error
//
//  Input		:Record class
//
//	Output		:count
//
////////////////////////////////////////////
int CModelLX::RetrieveCount(CModelLX::RecordClass LXclass)
{
	eAutostarStat	stat;
	CString			data;
	char			resp[10];
	unsigned int	len = 10;
	int				count;

	// turn the data into ascii
	data.Format("%02d", LXclass);

	// send the command
	stat = SendCommand(QUERY_COUNT, (unsigned char *)data.GetBuffer(5), (unsigned char *)resp, len);

	// check if it sent OK
	if (stat != AUTOSTAR_OK)
		return -1;
	
	// turn the returned count into an int
	if (sscanf(resp, "%d", &count) == 1)
		return count;
	else
		return -1;
}


/////////////////////////////////////////////
//
//	Name		:SendCommand
//
//	Description :This will send the LX command passed to it along with the data.
//				 Response to the command will be placed in the resp buffer along with its size in
//				 count. 
//
//  Input		:Command, Data pointer, Responce pointer, count reference
//
//	Output		:Autostar status
//
////////////////////////////////////////////
eAutostarStat CModelLX::SendCommand(eLXCmnd cmd,unsigned char *data,unsigned  char *resp, unsigned int &count)
{
	unsigned char cmdStr[150];		// no command will be longer than 64 binary bytes
	CSerialPort::eSerialStat		CStat;
	
	switch(cmd)
	{
	case QUERY_COUNT :
		strcpy((char *)cmdStr, ":uC");	// put in the command
		count = 6;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case DELETE_OBJECT :
		strcpy ((char *)cmdStr, ":uD");	// put in the command
		count = 5;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case DELETE_CATALOG :
		strcpy ((char *)cmdStr, ":uE"); // put in the command
		count = 5;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(DELETE_CATALOG_TIMEOUT);
		goto	INSERT_DATA;

	case GARBAGE_COLLECT:
		strcpy ((char *)cmdStr, ":uG");	// put in the command
		count = 5;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	CLOSE_COMMAND;

	case ALLOCATE_OBJECT :
		strcpy ((char *)cmdStr, ":uA");
		count = 6;
	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	COPY_MEM;

	case WRITE_DATA :
		strcpy ((char *)cmdStr, ":uW");
		count = 2;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case REPLACE_DATA :
		strcpy ((char *)cmdStr, ":uO");
		count = 2;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case QUERY_CATALOG_STRUCT:
		strcpy ((char *)cmdStr, ":um");
		count = 256 + 16 + 5;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case MAKE_CATALOG:
		strcpy ((char *)cmdStr, ":uM");
		count = 2;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case RETRIEVE_OBJ_BY_INDEX :
		strcpy ((char *)cmdStr, ":uR");
	// count set by caller

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case RETRIEVE_ALL_OBJ :
		strcpy ((char *)cmdStr, ":uX");
	// count set by caller

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case RETRIEVE_INDEX_BY_NAME :
		strcpy ((char *)cmdStr, ":un"); // put in the command
		count = 6;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;

	case SET_BAUD_RATE :
		if (m_autostar->m_mode != DOWNLOAD)
		{
			strcpy ((char *)cmdStr, ":SB");
			count = 2;
		}
		else
		{
			strcpy ((char *)cmdStr, "F");
			count = 1;
		}
	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	INSERT_DATA;


	case QUERY_FREE_MEMORY :
		strcpy ((char *)cmdStr, ":uF");
		count = 8;

	// change the timeout value
		m_autostar->m_serialPort.SetTimeout(GARBAGE_COL_TIMEOUT);
		goto	CLOSE_COMMAND;

	default :
		return UNKNOWN_ERROR;

COPY_MEM:
		memcpy(&cmdStr[3], data, 25);	// data is 2 + , + 5 + , + 16 = 25 characters long ALWAYS

		cmdStr[28] = '#';	// finish it with the #

		TRACE("\nCommand: %s\n",cmdStr);

		CStat = m_autostar->m_serialPort.SendData(cmdStr, 29, resp, count);	// command is ALWAYS 29 characters

		TRACE("Response: %s\n", CString(resp).Left(100));

		m_autostar->m_serialPort.SetTimeout(DEFAULT_SER_TIMEOUT);

		goto	END;
		
INSERT_DATA:
		strcat((char *)cmdStr, (char *)data);	// add the data

CLOSE_COMMAND:
		strcat((char *)cmdStr, "#");	// finish it with the #

		TRACE("\nCommand: %s\n",cmdStr);

		CStat = m_autostar->m_serialPort.SendData(cmdStr, strlen((char *)cmdStr), resp, count);

		TRACE("Response: %s\n", CString(resp).Left(100));

		m_autostar->m_serialPort.SetTimeout(DEFAULT_SER_TIMEOUT);

	}

END:
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
//	Name		:ForceGarbageCollection
//
//	Description :Sends the force garbage collection command to the LX.
//
//  Input		:None
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::ForceGarbageCollection()
{
	eAutostarStat	stat;
	unsigned char	resp[20];
	unsigned int	count;

	// send the command
	stat = SendCommand(GARBAGE_COLLECT, 0, resp, count);

	// check if it went out ok
	if (stat != AUTOSTAR_OK)
		return stat;

	// check the response
	if (resp[0] != '1')
		return COMMAND_FAILED;
	else
		return AUTOSTAR_OK;
}


/////////////////////////////////////////////
//
//	Name		:SendAllUserdata
//
//	Description :Formats and sends all the user data in m_SendData to the LX.
//
//  Input		:None
//
//	Output		:Stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::SendAllUserdata()
{
	eAutostarStat	stat = AUTOSTAR_OK;
	CString			name;
	int				ibody;
	CBodyData		*body;
	int				percent=1000, nPercent, sentBytes = 0, totalBytes = m_SendData->GetTotalSizeOf();
	RecordClass		LxBodyType;

// go through each body type and put it in its place except user objects
	for(ibody = Asteroid; ibody < BodyTypeMax && stat == AUTOSTAR_OK; ibody++)
	{

	// Prepare for the first body data of this type if there are any
		POSITION pos = m_SendData->GetHeadPosition((BodyType)ibody);
	
	// get the new body type and check if it's supported
		LxBodyType = ConvertBodytype((BodyType)ibody);
		if (LxBodyType == Error)
			continue;		// if not supported then skip it


	// now loop through each body adding them to the linked list
		while(pos && stat == AUTOSTAR_OK)
		{
		// Report the status if changed
			if (m_autostar->m_stat && percent != (nPercent = (int)(((float)sentBytes / (float)totalBytes) * 100)))
			{
				percent = nPercent;
				m_autostar->m_stat->PercentComplete(percent);
			}

			// get the body data
			body = m_SendData->GetNext(pos, (BodyType)ibody);

			// send the object to the LX
			stat = SendOneObject(body,LxBodyType, sentBytes);



		}	// do the next body
	}// do the next body type

	return stat;
}

/////////////////////////////////////////////
//
//	Name		:SendOneObject
//
//	Description :Formats and sends one user data object to the LX.
//
//  Input		:body data, LX body type
//
//	Output		:Stat, int # of Bytes sent
//
////////////////////////////////////////////
eAutostarStat CModelLX::SendOneObject(CBodyData *body, RecordClass LxBodyType, int &sentBytes, bool site)
{
	eAutostarStat	stat = AUTOSTAR_OK;
	CPersist		image;
	int				offset;
	int				wlen;
	unsigned int	dataLen = 10;
	int				bodyIndex;
	int				flag = KDJ_NONE;

	// Set any applicable Kluge flags
	// for versions earlier than 11, suppress extra precision of epoch date
	if (m_autostar->GetVersion().CompareNoCase("1.1") < 0)
		flag |= KDJ_EPOCH_DATE;

	// for custom objects, see if the catalog has been defined already
	if (body->IsCustom())
	{
		// query the body type to see if the catalog is defined
		CString response = "#";
		if (QueryCatalog(body->GetBodyType(), response) == AUTOSTAR_OK)
			// a response of "#" means it is not defined
			if (response[0] == '#')
				// so go ahead and define it
				DefineCatalog(body);
	}

	// get the data
	dataLen = body->GetSizeOf();
	sentBytes += dataLen;
	image.SetPointer(malloc(dataLen + 10));
	body->PutImageData((unsigned char *)image.m_dataPtr, flag);

	// figure out the length
	// subtract out the position and active flag
	dataLen -= 4;
	image.IncrementIndex(4);

	// allocate space for this object
	bodyIndex = AllocateObject(LxBodyType, dataLen, body->GetKey(true));

	// check for error
	if (bodyIndex < 0)
		return OUT_OF_MEMORY;

	// subtract out the name
	dataLen -= 16;
	image.IncrementIndex(16);

	offset = 16;

	// account for the fact that site names are 17 characters
	if (site)
	{
		dataLen -= 1;
		image.IncrementIndex(1);
		offset++;
		// write the 16th character as null
		unsigned char space = 0x00;
		stat = WriteData(SiteInformationClass, bodyIndex, 16, 1, &space);
	}

	// check if the data is too long for one block
	if (dataLen * 2 + WRITE_CMD_LEN > m_writeBlockSize)
	{
		// break up the data into BlockSize byte chunks
		while(dataLen > 0 && stat == AUTOSTAR_OK)
		{
			// set the write length to the block size or whats left
			wlen = dataLen * 2 + WRITE_CMD_LEN > m_writeBlockSize ? (m_writeBlockSize - WRITE_CMD_LEN)/2 : dataLen;

			// Write the body data to the LX
			stat = WriteData(LxBodyType, bodyIndex, offset, wlen, (unsigned char *)image.m_indexPtr);

			// increment the pointer
			image.IncrementIndex(wlen);
			dataLen -= wlen;
			offset += wlen;
		}
	}
	// just write a single block
	else
	{
		stat = WriteData(LxBodyType, bodyIndex, offset, dataLen, (unsigned char *)image.m_indexPtr);
	}

	// free up the image data
	image.FreeBuffer();

	return stat;

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
eAutostarStat CModelLX::SendOneObject(CBodyData *body, BodyType bodyType, int &sentBytes)
{
	return SendOneObject(body, ConvertBodytype(bodyType), sentBytes);
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
eAutostarStat CModelLX::DeleteOneObject(RecordClass LXclass, CString name)
{
	eAutostarStat	stat;
	CString			data;
	char			resp[10];
	unsigned int	len = 10;
	int				index;


	// this is all wrong, first you have to find the index number,
	//   THEN you can send the delete command

	// turn the data into ascii
	data.Format("%02d,%s", LXclass, name);

	// send the find command
	stat = SendCommand(RETRIEVE_INDEX_BY_NAME,(unsigned char *)data.GetBuffer(5),(unsigned char *)resp, len);

	// check if it sent OK
	if (stat != AUTOSTAR_OK)
		return stat;

	// check the response
	if (resp[0] == '?')
		return COMMAND_FAILED;

	//check if the name was found
	index=atoi(resp);
	if (!index)
		return ERASE_ERROR;

	// format the delete string
	data.Format("%02d,%s",LXclass, resp);
	
	// send the delete command
	stat = SendCommand(DELETE_OBJECT, (unsigned char *)data.GetBuffer(5), (unsigned char *)resp, len);

	// check if it sent OK
	if (stat != AUTOSTAR_OK)
		return stat;
	
	// check the response
	if (resp[0] != '1')
		return COMMAND_FAILED;
	else
		return AUTOSTAR_OK;
}


/////////////////////////////////////////////
//
//	Name		:DeleteOneObject
//
//	Description :Deletes a single object by object type and name
//
//  Input		:Body Type, CString name
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::DeleteOneObject(BodyType bodyType, CString objectName)
{
	return DeleteOneObject(ConvertBodytype(bodyType),objectName);
}

/////////////////////////////////////////////
//
//	Name		:DeleteCatalog
//
//	Description :Delete an entire class of objects and check
//				 the response. (Use this function for external calls)
//
//  Input		:BodyType
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::DeleteCatalog(BodyType bodyType)
{
  	return DeleteCatalog(ConvertBodytype(bodyType));

}

/////////////////////////////////////////////
//
//	Name		:DeleteCatalog
//
//	Description :Delete an entire class of objects and check
//				 the response.
//
//  Input		:Class
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::DeleteCatalog(RecordClass LXclass)
{
	eAutostarStat	stat;
	CString			data;
	char			resp[10];
	unsigned int	len = 10;
	RecordClass startClass, endClass;

	if (LXclass == AllClasses)	// if all, iterate through all user classes
	{
		startClass = LandmarkClass;
		endClass = MaxClass;
	}
	else	// else just start and finish with the desired class
	{
		startClass = LXclass;
		endClass = LXclass;
	}
		
	// iterate through each class, sending the appropriate delete command
	for (int currentClass = startClass; currentClass <= endClass; currentClass++)
	{
		// turn the data into ascii
		data.Format("%02d", LXclass);

		// send the command
		stat = SendCommand(DELETE_CATALOG, (unsigned char *)data.GetBuffer(5), (unsigned char *)resp, len);

		// check if it sent OK
		if (stat != AUTOSTAR_OK)
			return stat;
		
		// check the response
		if (resp[0] != '1')
			return COMMAND_FAILED;
	}
	
	// if we got here everything is OK
	return AUTOSTAR_OK;


}

/////////////////////////////////////////////
//
//	Name		:DeleteDynamicObject
//
//	Description :Send the delete dynamic object command to the LX and check
//				 the response.
//
//  Input		:Class, index
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::DeleteDynamicObject(RecordClass LXclass, int index)
{
	eAutostarStat	stat;
	CString			data;
	char			resp[10];
	unsigned int	len = 10;

	// turn the data into ascii
	data.Format("%02d,%d", LXclass, index);

	// send the command
	stat = SendCommand(DELETE_OBJECT, (unsigned char *)data.GetBuffer(5), (unsigned char *)resp, len);

	// check if it sent OK
	if (stat != AUTOSTAR_OK)
		return stat;
	
	// check the response
	if (resp[0] != '1')
		return COMMAND_FAILED;
	else
		return AUTOSTAR_OK;
}

/////////////////////////////////////////////
//
//	Name		:AllocateObject
//
//	Description :Sends an Allocate Dynamic Object command to the LX to
//				 allocate space for an object.
//
//  Input		:Class, Length, Name
//
//	Output		:Index
//				 -1 if failed
//
////////////////////////////////////////////
int CModelLX::AllocateObject(int aClass, int len, CString Name)
{
	eAutostarStat	stat;
	CString			data;
	char			resp[10];
	int				index;
	unsigned int	ulen;


	// convert the name into a 16-char array filled to the right with zeros
	char name[16];
	Name.TrimRight();
	int textLength = Name.GetLength();

	// copy 16 characters of the name string
	strncpy(name, Name.GetBuffer(20), 16);

	// convert the command data to ascii
	data.Format("%02d,%05d,", aClass, len);
	char* cmd = (char *) malloc(30);
	strcpy(cmd, data.GetBuffer(5));
	memcpy(&cmd[9], name, 16);

	// send the command
	stat = SendCommand(ALLOCATE_OBJECT, (unsigned char *) cmd, (unsigned char *)resp, ulen);

	free(cmd);

	// check if it sent OK
	if (stat == AUTOSTAR_OK)
	{
		// check the response
		resp[5] = 0;	// insert a null
		if (sscanf(resp, "%d", &index) == 1 && index != 0)	// convert the ascii response to an int
			return index;
	}

	return -1;
}

/////////////////////////////////////////////
//
//	Name		:WriteData
//
//	Description :Writes record data of the specified class in the specified index 
//				 at the specified offset for the specified length with the specified
//				 data. It will turn all this into an ascii formated command to send
//				 to the LX
//
//  Input		:aClass, index, offset, len, data
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModelLX::WriteData(int aClass, int index, int offset, int len, unsigned char *data)
{

	eAutostarStat	stat;
	CString			cmd, sTmp;
	char			resp[10];
	int				chksum;
	unsigned int	ulen;

	// turn the parameters into ascii
	cmd.Format("%d,%d,%d,%d,", aClass, index, offset, len);

	// turn the data into ascii
	chksum = 0;
	for (int i=0; i<len; i++)
	{
		sTmp.Format("%02X", data[i]);
		cmd += sTmp;
		chksum += data[i];
	}
	// add the checksum
	chksum %= 100;
	sTmp.Format("%02d", chksum);
	cmd += sTmp;

	ulen = (unsigned int)len;
	// send the command
	stat = SendCommand(WRITE_DATA, (unsigned char *)cmd.GetBuffer(5), (unsigned char *)resp, ulen);

	// check if it sent OK
	if (stat != AUTOSTAR_OK)
		return stat;
	
	// check the response
	if (resp[0] != '1')
		return COMMAND_FAILED;
	else
		return AUTOSTAR_OK;
}

/////////////////////////////////////////////
//
//	Name		:RetrieveAllUserdata
//
//	Description :This function will retrieve all the user data of the autostar II
//				 and convert them to objects and put them in m_handboxData.
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CModelLX::RetrieveAllUserdata()
{
	int				ibody;
	CPersist		per;
	CBodyData		*body, *bodyTemplate = NULL;
	eAutostarStat	stat;
	int				bodyCount;
	bool			thereWereErrors = false;
	char			*bodyString[] = {"Asteroids","Comets","Satellites","UserObjs","LandMarks","Tours",
									"Undefined20s","Undefined21s","Undefined22s","Undefined23s","Undefined24s",
									"Undefined25s","Undefined26s","Undefined27s","Undefined28s","Undefined29s",
									"Undefined30s","Undefined31s","Undefined32s","Undefined33s","Undefined34s",
									"Undefined35s","Undefined36s","Undefined37s","Undefined38s","Undefined39s"};



// clear current handbox data
	m_autostar->m_handboxData.Clear();

// iterate through all the body types except user objects
	for( ibody = Asteroid; ibody < BodyTypeMax; ibody++)
	{
	// check for undone body types (development only)
		if (ConvertBodytype((BodyType)ibody) == Error)
			continue;	// undone so skip it

	// find out how many of these		
		bodyCount = RetrieveCount(ConvertBodytype((BodyType)ibody));
		
	// skip empty classes
		if (bodyCount == 0)
			continue;

	// retrieve all the data if the bodytype is not a tour
		if (ibody != Tour)
		{
			// if this is a custom user object
			if (ibody >= UserObj20 && ibody <= UserObj39)	
			{
				// we need to query the catalog to define the custom fields
				CString response = "#";
				
				stat = QueryCatalog((BodyType) ibody,response);

				if (stat == AUTOSTAR_OK && response[0] != '#')
				{
					// make a default custom user object
					if ((bodyTemplate = m_factory->Make((BodyType)ibody)) == NULL)
					// this type doesn't exist yet go to next body type
						break;		
					
					if (!((CUserObjEx *)bodyTemplate)->SetFieldDataFromString(response))
						return USEROBJEX_RETRIEVE_ERROR;
				}
			}
		// get a test body
			if ((body = m_factory->Make((BodyType)ibody, bodyTemplate)) == NULL)
			// this type doesn't exist yet go to next body type
				break;

			// allocate space
			per.SetPointer(malloc((body->GetSizeOf() - 3) * bodyCount + 10));

			// adjust pointer for the lack of position and active flag
			per.IncrementIndex(4);

		// tell them what we are doing
			if (m_autostar->m_stat)
			{
				CString		sStatus,
					name = (ibody < UserObj20) ? "" : ((CUserObjEx *)body)->GetCatalogName();
				name.TrimRight();
				sStatus.Format("Retrieving %s",	
					(ibody < UserObj20) ? bodyString[ibody] : name);
				m_autostar->m_stat->DoingProcess(sStatus);
			}

			// get all the data
			stat = RetrieveUserDataImage(ConvertBodytype((BodyType)ibody), (body->GetSizeOf() - 4) * bodyCount, bodyCount, per, body->GetSizeOf() - 4);
			if (stat != AUTOSTAR_OK)
				return stat;

			// get rid of the test body
			delete body;

		// reset the index pointer
			per.SetIndex(0);
		}


	// now loop and create each body
		for (int i=1; i< bodyCount + 1; i++)
		{
		// create a new data object
			if ((body = m_factory->Make((BodyType)ibody, bodyTemplate)) == NULL)
			// this type doesn't exist yet go to next body type
				break;											
	
		// if this is a custom user obj, skip the first object
			if ((i == 1) && (body->IsCustom()))
			{
				per.IncrementIndex(body->GetSizeOf());
				delete body;
				continue;
			}


		// get body data for tours
			if (ibody == Tour)
			{
			// get buffer space	
				per.SetPointer(malloc(body->GetSizeOf() + 20));

			// adjust pointer for lack of position and active flag
				per.IncrementIndex(4);

			// get body data
				stat = RetrieveUserDataImage(ConvertBodytype((BodyType)ibody), body->GetSizeOf() - 4, i, per);
				if (stat != AUTOSTAR_OK)
					return stat;

			// read the data to get the real size
				body->ReadData((unsigned char *)per.m_dataPtr);

			// tell them what we are doing
				if (m_autostar->m_stat)
				{
					CString sStatus;
					sStatus.Format("Retrieving \"%s\" Tour", body->GetKey()); 
					m_autostar->m_stat->DoingProcess(sStatus);
				}

			// trash the old buffer and allocate a new one
				per.FreeBuffer();
				per.SetPointer(malloc(body->GetSizeOf() + 20));
				per.IncrementIndex(4);

			// read it again with the real size
				stat = RetrieveUserDataImage(ConvertBodytype((BodyType)ibody), body->GetSizeOf() - 4, i, per);
				if (stat != AUTOSTAR_OK && stat != READ_ERROR) // Allow READ_ERRORs through
					return stat;

			// reset the index pointer
				per.SetIndex(0);
			}

		// Read all the data 
			bool	imageStat;
			imageStat = body->ReadImageData((unsigned char *)per.m_indexPtr);

		// increment index for non tour bodies
			if (ibody != Tour)
				per.IncrementIndex(body->GetSizeOf() - 4);	// size - page/active data

		// range check all the data into the object. Don't add objects with READ_ERROR
			if (imageStat && !body->CheckFieldRanges() && stat != READ_ERROR)
			{
			// add it to the autostars collection
				m_autostar->m_handboxData.Add(body);
			}
			else
			{
			// failed range check, toss it
				delete body;
				thereWereErrors = true;
			}

			if (ibody == Tour)
			// done with buffer
				per.FreeBuffer();
		}
	if (ibody != Tour)
		per.FreeBuffer();

	if (bodyTemplate)
		delete bodyTemplate;

	}
	if (m_autostar->m_stat)
	{
		m_autostar->m_stat->PercentComplete(100);
	}
	if (thereWereErrors)
		MessageBoxEx(NULL,"Some objects were not retrieved","Serial Communications Error",MB_OK | MB_TOPMOST, LANG_ENGLISH);

	return AUTOSTAR_OK;
}



/////////////////////////////////////////////
//
//	Name		:ConvertBodytype
//
//	Description :This function will convert the passed bodyType and return
//				 the LX RecordClass equivalant.
//
//  Input		:bodyType
//
//	Output		:RecordClass
//
////////////////////////////////////////////
CModelLX::RecordClass CModelLX::ConvertBodytype(BodyType type)
{
// convert ibody to LX RecordClass
		switch (type)
		{
		case Asteroid :
			return AsteroidClass;

		case Comet :
			return CometClass;

		case Satellite :
			return SatelliteClass;

		case UserObj:
			return UserObjectClass;

		case LandMark :
			return LandmarkClass;

		case Tour :
			return TourClass;

		case All :
			return AllClasses;

		case UserObj20 :
			return UserObject20Class;

		case UserObj21 :
			return UserObject21Class;

		case UserObj22 :
			return UserObject22Class;

		case UserObj23 :
			return UserObject23Class;

		case UserObj24 :
			return UserObject24Class;

		case UserObj25 :
			return UserObject25Class;

		case UserObj26 :
			return UserObject26Class;

		case UserObj27 :
			return UserObject27Class;

		case UserObj28 :
			return UserObject28Class;

		case UserObj29 :
			return UserObject29Class;

		case UserObj30 :
			return UserObject30Class;

		case UserObj31 :
			return UserObject31Class;

		case UserObj32 :
			return UserObject32Class;

		case UserObj33 :
			return UserObject33Class;

		case UserObj34 :
			return UserObject34Class;

		case UserObj35 :
			return UserObject35Class;

		case UserObj36 :
			return UserObject36Class;

		case UserObj37 :
			return UserObject37Class;

		case UserObj38 :
			return UserObject38Class;

		case UserObj39 :
			return UserObject39Class;

		case UserInfo :
			return UserInfoClass;

		case SiteInfo :
			return SiteInformationClass;

		default :
			return Error;
		}

}

/////////////////////////////////////////////
//
//	Name		:RetrieveUserDataImage
//
//	Description :This function will send a Retreive Object By Name command to
//				 the Autostar II and put the binary converted response in the
//				 persist object at the persist object index. The passed type,
//				 size and index are sent with the command.
//
//  Input		:bodyType, size, index, persist object
//
//	Output		:status, persist object
//
////////////////////////////////////////////
eAutostarStat CModelLX::RetrieveUserDataImage(RecordClass type, int size, int index, CPersist &per, int cSize)
{
	CString			cmd;
	unsigned char	*resp = (unsigned char *)malloc(size * 2 + index + 10);		// allocate the response buffer
	eAutostarStat	stat;
	unsigned int	tmp, cntRec;
	unsigned int	recSize = size * 2 + 1;
	int retryCount = 1;

	while (retryCount <= 3)
	{
		if (type == TourClass || 
			type == UserInfoClass || 
			type == PersonalInformationClass ||
			type == RA_PEC_Class || type == DEC_PEC_Class)
		{
			// figure out how many to receive
			recSize = size * 2 + 1; // number of bytes * characters per byte + #

			// turn the parameters into ascii
			cmd.Format("%d,%d,%d", (int)type, size, index);

			// send the command
			tmp = recSize < MAX_SER_BUF ? recSize : MAX_SER_BUF;
			stat = SendCommand(RETRIEVE_OBJ_BY_INDEX, (unsigned char *)cmd.GetBuffer(5), resp, tmp);
			cntRec = tmp;
		}
		else
		{
			// figure out how many to receive
			recSize = size * 2 + index; // number of bytes * characters per byte + commas 

			// turn the parameters into ascii
			cmd.Format("%d,%d", (int)type, cSize);

			// send the command
			tmp = recSize < MAX_SER_BUF ? recSize : MAX_SER_BUF;
			stat = SendCommand(RETRIEVE_ALL_OBJ, (unsigned char *)cmd.GetBuffer(5), resp, tmp);
			cntRec = tmp;
		}		

		// if the object length was bigger than MAX_SER_BUF then this "while loop" 
		// will dice it up into MAX_SER_BUF size chunks
		// look for a # in either the last or 2nd to last char
		while (stat == AUTOSTAR_OK && *(resp + (cntRec - 1)) != '#' && cntRec < recSize && tmp != 0)
		{
			tmp = (recSize - cntRec) < MAX_SER_BUF ? (recSize - cntRec) : MAX_SER_BUF;

			m_autostar->m_serialPort.SendData(NULL, 0, resp + cntRec, tmp);
			cntRec += tmp;
		// Report the status
			if (m_autostar->m_stat)
			{
				m_autostar->m_stat->PercentComplete((int)(((float)cntRec / (float)recSize) * 100));
			}

		}
		if (type == TourClass)
			TRACE("\nindex: %i, tmp=%i, cntRec=%i, recSize=%i\n",index, tmp, cntRec, recSize);

		if (cntRec < recSize)	// indicates that the expected number of characters didn't arrive
		{
			stat = READ_ERROR;
			retryCount++;
//			MessageBoxEx(NULL,"Retry","Error",MB_OK | MB_TOPMOST, LANG_ENGLISH);
		}
		else
			break;

	}

		// process the response
	if (stat == AUTOSTAR_OK)
	{
		unsigned char *pResp = resp;
		while (*pResp != '#')
		{
			// convert every 2 ascii characters to a byte
			sscanf((char *)pResp, "%02X", &tmp);
			per << (char)tmp;

			//increment the pointer
			pResp += 2;

			// skip over commas
			if (*pResp == ',')
				pResp++;

		}
	}

	free(resp);
	return stat;

}

/////////////////////////////////////////////
//
//	Name		:SetBaud
//
//	Description :This function will compare the passed baud rate
//				 to the current baud rate and if different will
//				 send the set baud rate command to the the Autostar II.
//				 If the Autostar II returns a valid response the 
//				 serial port will be set the the new baud rate.
//
//  Input		:baud
//
//	Output		:status
//
////////////////////////////////////////////
eAutostarStat CModelLX::SetBaud(CSerialPort::eBaud baud)
{

	unsigned char	cmd[5];
	unsigned char	resp[5];
	unsigned int	cnt;
	eAutostarStat	stat;

	// if bMax or bDefault was passed, get the actual value
	if (baud == CSerialPort::bMax)
		baud = m_autostar->m_serialPort.GetMaxBaud();

	if (baud == CSerialPort::bDefault)
		baud = m_autostar->m_serialPort.GetDefaultBaud();
	

// compare the passed baud rate to the current baud rate
	if (m_autostar->m_serialPort.GetBaud() != baud)
	{
	// set the command data for the baud rate passed
		switch (baud)
		{
		case CSerialPort::b9600 :
			strcpy((char *)cmd, "6");
			break;

		case CSerialPort::b14k :
			strcpy((char *)cmd, "5");
			break;

		case CSerialPort::b19k :
			strcpy((char *)cmd, "4");
			break;

		case CSerialPort::b28k :
			strcpy((char *)cmd, "3");
			break;

		case CSerialPort::b38k :
			strcpy((char *)cmd, "2");
			break;

		case CSerialPort::b56k :
			strcpy((char *)cmd, "1");
			break;

		case CSerialPort::b115k :
			strcpy((char *)cmd, "0");
			break;

		default :
			return COMMAND_FAILED;	// unknown baud
		}

	// response count
		cnt = 1;

	// send the command
		stat = SendCommand(SET_BAUD_RATE, cmd, resp, cnt);

	// check the command status and the response 
		if (stat == AUTOSTAR_OK && (resp[0] == '1' || resp[0] == 'Y'))
		// set the port baud rate
			m_autostar->m_serialPort.SetBaud(baud);
		else
			stat = COMMAND_FAILED;

		return stat;

	}

// already set to this baud rate
	return AUTOSTAR_OK;
}

int CModelLX::RetrieveFreeMemory(bool closeport)
{
	unsigned char	cmd[5];
	unsigned char	resp[10];
	unsigned int	cnt;
	int				freeMem;
	eAutostarStat	stat;

// check the port
	stat = m_autostar->InitComPort();

// send the command
	if (stat == AUTOSTAR_OK)
		stat = SendCommand(QUERY_FREE_MEMORY, cmd, resp, cnt);

// close the port
	if (closeport)
		m_autostar->m_serialPort.ClosePort();

// check the command status and the response 
	if (stat == AUTOSTAR_OK)
	{
		// check the response
		resp[8] = 0;	// insert a null
		if (sscanf((char *)resp, "%d", &freeMem) == 1)	// convert the ascii response to and int
		{
			m_freemem = freeMem;
			return freeMem;
		}
	}

	return -1;

}


/////////////////////////////////////////////
//
//	Name		:GetCatalogString
//
//	Description :returns a CString to describe the catalog enumerated in RecordClass
//
//  Input		:RecordClass
//
//	Output		:CString
//
////////////////////////////////////////////
CString CModelLX::GetCatalogString(RecordClass LXClass)
{
		switch (LXClass)
		{
		case Error:
			return "Error";
		case UserInfoClass:
			return "User Information";
		case ScopeInformationClass:
			return "Telescope Information";
		case RA_PEC_Class:
			return "RA PEC Table";
		case DEC_PEC_Class:
			return "DEC PEC Table";
		case SmartMountClass:
			return "Smart Mount Table";
		case PersonalInformationClass:
			return "Personal Information";
		case SiteMapClass:
			return "Site Map Table";
		case SiteInformationClass:
			return "Site Information";
		case LandmarkClass:
			return "Landmarks";
		case TourClass:
			return "Tours";
		case AsteroidClass:
			return "Asteroids";
		case CometClass:
			return "Comets";
		case SatelliteClass:
			return "Satellites";
		case UserObjectClass:
			return "User Objects";
		case UserObject20Class :
		case UserObject21Class :
		case UserObject22Class :
		case UserObject23Class :
		case UserObject24Class :
		case UserObject25Class :
		case UserObject26Class :
		case UserObject27Class :
		case UserObject28Class :
		case UserObject29Class :
		case UserObject30Class :
		case UserObject31Class :
		case UserObject32Class :
		case UserObject33Class :
		case UserObject34Class :
		case UserObject35Class :
		case UserObject36Class :
		case UserObject37Class :
		case UserObject38Class :
		case UserObject39Class :
			return "Custom User Objects";
		default:
			return "unknown";
		}

}

/////////////////////////////////////////////
//
//	Name		:UpgradeFrom10B
//
//	Description :When going from version 1.0B to anything else, it is necessary
//				 to reset the User Record Class.  This function steps through
//				 the linked list and sets all entries in that record to inactive.
//
//  Input		:none
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::UpgradeFrom10B()
{
	eAutostarStat stat;
	CUserSettings user;

	// set the member variable to note that we are upgrading from ver. 1.0b
	m_from10B = true;

	// Send a status bar message
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Backing up user objects...");

	// save the current contents of the list view
	if (m_autostar->GetHandboxData()->SaveToFile(user.GetInstallDirectory() + BACKUP2_10B_FILENAME,All) != READCOMPLETE)
		return COMMAND_FAILED;

	// retrieve user objects, and save to disk
	m_autostar->GetHandboxData()->Clear();
	if ((stat = m_autostar->RetrieveUserData(false)) != AUTOSTAR_DOWNLOADING)
		MessageBoxEx(NULL, m_autostar->GetLastError(), _T("Autostar Error"), MB_OK | MB_TOPMOST, LANG_ENGLISH);

	if (m_autostar->GetHandboxData()->SaveToFile(user.GetInstallDirectory() + BACKUP_10B_FILENAME,All) != READCOMPLETE)
		return COMMAND_FAILED;

	// re-open serial port
	if ((stat = m_autostar->InitializeConnection(false,false)) != AUTOSTAR_OK)
		return stat;

	m_eraseBanks = true;	// set flag to erase user banks

	return stat;
}




/////////////////////////////////////////////
//
//	Name		:SendProgramThread
//
//	Description :This will send the program to the Autostar.
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CModelLX::SendProgramThread()
{
	// Call the base class
	CAutostarModel::SendProgramThread();

	eAutostarStat stat = m_autostar->m_lastError;

	// make sure the program was sent without any errors
	if (stat != AUTOSTAR_OK)
	{
		MessageBoxEx(NULL, m_autostar->GetLastError(), "Error",
			MB_ICONEXCLAMATION | MB_TOPMOST, LANG_ENGLISH);
		return;
	}

	CUserSettings user;

	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Initializing Handbox...Please Wait");

	// since it was just restarted, it must be 9600 baud
	m_autostar->m_serialPort.SetBaud(CSerialPort::bDefault);

	// restart the handbox
	int count = 1;
	while (count <= 6)	// try up to 6 times, because the LX200GPS may be initializing smart drive
	{					// and not communicating with the PC.  This is so GetAvailableMemory() works.
		if ((stat = m_autostar->InitializeConnection(false,false)) == AUTOSTAR_OK)
			count = 7;
		else
			count++;
	}

	// if handbox is being upgraded from ver. 1.0b, restore the handbox data now
	if (m_from10B && stat == AUTOSTAR_OK)
	{
		// restore the data going into the handbox
		m_autostar->GetHandboxData()->Clear();
		if (m_autostar->m_handboxData.LoadFromFile(user.GetInstallDirectory() + BACKUP_10B_FILENAME) != READCOMPLETE)
		{
			MessageBox(NULL,"Could not open backup object file","Error",MB_OK);
		}
		
		if (m_autostar->m_stat)
			m_autostar->m_stat->DoingProcess("Restoring objects to handbox");
		m_autostar->SendUserData(NULL,false);

		// restore the data going into the listview
		m_autostar->GetHandboxData()->Clear();
		if (m_autostar->m_handboxData.LoadFromFile(user.GetInstallDirectory() + BACKUP2_10B_FILENAME) != READCOMPLETE)
		{
			MessageBox(NULL,"Could not open backup object file","Error",MB_OK);
		}

		m_from10B = false;
	}

	// Report the status
	if (m_autostar->m_stat)
		m_autostar->m_stat->SendComplete(stat);
}



/////////////////////////////////////////////
//
//	Name		:InitializeSend
//
//	Description :Calls InitializeConnection just prior to sending data
//
//  Input		:bool CloseComPort (default = false)
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::InitializeSend(bool closeComPort)
{
	// note: 2nd param must be FALSE (or rev. 1.0b update will
	// try to access a newly created but empty collection
	return m_autostar->InitializeConnection(closeComPort, false);
}

/////////////////////////////////////////////
//
//	Name		:EraseUserBanks
//
//	Description :Erases user record banks prior to sending new firmware
//
//  Input		:none
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::EraseUserBanks()
{
	unsigned char data[5], resp[5];
	unsigned int count;
	eAutostarStat stat;

	// Erase the banks which contain the "old" structures
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Erasing User Banks...");
	for (unsigned int b = m_userBankFrom; b <= m_userBankTo; b++)
	{
		data[0] = b;
		count = 1;
		if ((stat = m_autostar->SendCommand(ERASE_BANK, data, resp, count)) != AUTOSTAR_OK)
			return stat;
	}
	return stat;
}


/////////////////////////////////////////////
//
//	Name		:Set the Maximum Baud Rate
//
//	Description :Changes the maximum baud rate of the registry, 
//				 handbox and serial port
//
//  Input		:baud rate
///
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModelLX::SetMaxBaudRate(CSerialPort::eBaud baud, bool justHandbox)
{
	// update the registry & serial port
	if (!justHandbox)
		m_autostar->m_serialPort.SetMaxBaud(baud);

	eAutostarStat stat;

	// open the com port
	stat = m_autostar->InitComPort();

	if (stat != AUTOSTAR_OK)
		return stat;

	// tell the handbox
	stat = SetBaud(baud);

	if (stat != AUTOSTAR_OK)
		return stat;

	// close the com port
	m_autostar->CloseSerialPort();

	return AUTOSTAR_OK;
}


/////////////////////////////////////////////
//
//	Name		:Query Catalog
//
//	Description :Gets the field names and sizes for a Custom User Catalog
//
//  Input		:BodyType
///
//	Output		:CString containing catalog header info
//
////////////////////////////////////////////
eAutostarStat CModelLX::QueryCatalog(BodyType bodyType, CString &queryResp)
{
	CString			cmd;
	unsigned char	resp[300];
	unsigned int	cnt;
	eAutostarStat	stat;

	// format the command data
	cmd.Format("%d",(int) ConvertBodytype(bodyType));

	// check the port
	stat = m_autostar->InitComPort();

	// send the command
	if (stat == AUTOSTAR_OK)
		stat = SendCommand(QUERY_CATALOG_STRUCT, (unsigned char*) cmd.GetBuffer(5), (unsigned char*) resp, cnt);

	queryResp = CString(resp);
	return stat;
}

/////////////////////////////////////////////
//
//	Name		:Define Catalog
//
//	Description :Send the command to create a Custom User Catalog
//
//  Input		:BodyType
///
//	Output		:CString containing catalog header info
//
////////////////////////////////////////////
eAutostarStat CModelLX::DefineCatalog(CBodyData *data)
{
	// if this is not a custom object, return immediately
	if (!data->IsCustom())
		return NOT_ALLOWED;

	CString cmdString = "";
	unsigned char	resp[10];
	unsigned int	cnt;

	// check the port
	eAutostarStat stat;
	stat = m_autostar->InitComPort();
	if (stat != AUTOSTAR_OK)
		return stat;

	// format for command string is: class#, Name[16], fieldName:fieldLength, fieldName:fieldLength...

	// add the class number
	cmdString += CPersist::ToString((int) ConvertBodytype(data->GetBodyType()));
	cmdString += ",";

	// add the class description
	cmdString += ((CUserObjEx *) data)->GetCatalogName();

	// add the field descriptors
	for (int i = REQD_USEROBJEX_FIELDS; i < ((CUserObjEx *) data)->GetNumFields(); i++)
	{
		cmdString += ",";

		cmdString += ((CUserObjEx *) data)->GetFieldName(i);
		cmdString += " :";					// note the extra space in the field name

		cmdString += CPersist::ToString(((CUserObjEx *) data)->GetFieldSize(i));
	}

	stat = SendCommand(MAKE_CATALOG, (unsigned char*) cmdString.GetBuffer(cmdString.GetLength()), resp, cnt);

	// check the response to see if it was created successfully
	if (resp[0] = '1' && stat == AUTOSTAR_OK)
		return stat;
	else
		return COMMAND_FAILED;

}

/////////////////////////////////////////////
//
//	Name		:TestFunction
//
//	Description :Temporary function called by pressing CTRL-t
//
//  Input		:none
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModelLX::TestFunction()
{

//	return AUTOSTAR_OK;

	// nice little function to erase the user banks:
	eAutostarStat stat;
	
	if (MessageBoxEx(NULL, "Erase User Banks?", "Erase, Erase?", MB_YESNO | MB_TOPMOST, LANG_ENGLISH) == IDYES)
	{
		m_autostar->SendDownloadMode();
		stat = EraseUserBanks();
		stat = m_autostar->PowerCycleHandbox();
		MessageBoxEx(NULL, "Press OK to continue", "Pause", MB_OK | MB_TOPMOST, LANG_ENGLISH);
		m_autostar->InitializeConnection(false, false);
	}

	return stat;

}

/////////////////////////////////////////////
//
//	Name		:ReplaceData
//
//	Description :Replaces record data of the specified class (single record classes only) 
//				 at the specified offset for the specified length with the specified
//				 data. It will turn all this into an ascii formated command to send
//				 to the LX
//
//  Input		:aClass, offset, len, data
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModelLX::ReplaceData(int aClass, int offset, int len, unsigned char *data)
{
	eAutostarStat	stat;
	CString			cmd, sTmp;
	char			resp[10];
	int				chksum;
	unsigned int	ulen;

	// turn the parameters into ascii
	cmd.Format("%d,%d,%d,", aClass, offset, len);
//	cmd.Format("%d,%d,", aClass, len);

	// turn the data into ascii
	chksum = 0;
	for (int i=0; i<len; i++)
	{
		sTmp.Format("%02X", data[i]);
		cmd += sTmp;
		chksum += data[i];
	}
	// add the checksum
	chksum %= 100;
	sTmp.Format("%02d", chksum);
	cmd += sTmp;

	ulen = (unsigned int)len;
	// send the command
	stat = SendCommand(REPLACE_DATA, (unsigned char *)cmd.GetBuffer(5), (unsigned char *)resp, ulen);

	// check if it sent OK
	if (stat != AUTOSTAR_OK)
		return stat;
	
	// check the response
	if (resp[0] != '1')
		return COMMAND_FAILED;
	else
		return AUTOSTAR_OK;
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
eAutostarStat CModelLX::GetUserInfo(CUserInfo *pInfo)
{
	eAutostarStat stat;
	CPersist per;
	int size = sizeof(UserInfoType) * 2 + 1;  // number of bytes * characters per byte + #

	// allocate space
	per.SetPointer(malloc(sizeof(UserInfoType) + sizeof(PersonalInfoType) + 10));
	per.IncrementIndex(4);

	stat = RetrieveUserDataImage(UserInfoClass, sizeof(UserInfoType) - 4, 1, per);

	if (stat != AUTOSTAR_OK)
		return stat;

	per.IncrementIndex(4);
	size = sizeof(PersonalInfoType) * 2 + 1;


	stat = RetrieveUserDataImage(PersonalInformationClass, sizeof(PersonalInfoType) - 4, 1, per);

	if (stat != AUTOSTAR_OK)
		return stat;


	if (!pInfo->ReadImageData((unsigned char *)(per.m_dataPtr)))
		return READ_ERROR;

	per.FreeBuffer();

	return stat;
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
eAutostarStat CModelLX::GetSiteInfo(CBodyDataCollection *siteList)
{
	eAutostarStat stat;
	CBodyData *body;
	int siteCount;
	CPersist per;

	siteCount = RetrieveCount(SiteInformationClass);

	body = m_factory->Make(SiteInfo);

	// allocate space
	per.SetPointer(malloc(body->GetSizeOf() * siteCount + 10));
	per.IncrementIndex(4);

	stat = RetrieveUserDataImage(SiteInformationClass, (body->GetSizeOf() - 4) * siteCount, siteCount, per, body->GetSizeOf() - 4);

	delete body;

	if (stat != AUTOSTAR_OK)
		return stat;

	// reset the pointer
	per.SetIndex(0);

	// loop and create each site object
	for (int i  = 1; i < siteCount + 1; i++)
	{
		// create a new data object
		if ((body = m_factory->Make(SiteInfo)) == NULL)
			break;

		bool imageStat = body->ReadImageData((unsigned char *) per.m_indexPtr);

		per.IncrementIndex(body->GetSizeOf() - 4);

		// add it to the collection
		if (imageStat)
			siteList->Add(body);

	}


	per.FreeBuffer();

	return stat;
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
eAutostarStat CModelLX::SetUserInfo(CBodyData *data)
{
	eAutostarStat	stat = AUTOSTAR_OK;
	CPersist		image;
	int				offset, wlen;
	unsigned int	dataLen = 10;
	int				flag = KDJ_NONE;

	// get the data
	dataLen = data->GetSizeOf();
	image.SetPointer(malloc(dataLen + 10));
	data->PutImageData((unsigned char *)image.m_dataPtr, flag);

	// figure out the length
	// subtract out the position and active flag
	dataLen = sizeof(UserInfoType);
	dataLen -= 4;
	image.IncrementIndex(4);

	offset = 4;

	// write the user info data
	stat = 
		ReplaceData(UserInfoClass, offset, dataLen, (unsigned char *)image.m_indexPtr);

	if (stat != AUTOSTAR_OK)
		return stat;

	// write the personal info data
	dataLen = sizeof(PersonalInfoType);
	dataLen -= 4;
	image.IncrementIndex(sizeof(UserInfoType));

	offset = 4;	// note: offset must be 4 to skip the poolposition & active flag

	// break up the data into BlockSize byte chunks
	while(dataLen > 0 && stat == AUTOSTAR_OK)
	{
		// set the write length to the block size or whats left
		wlen = dataLen * 2 + WRITE_CMD_LEN > m_writeBlockSize ? (m_writeBlockSize - WRITE_CMD_LEN)/2 : dataLen;

		// Write the body data to the LX
		stat = ReplaceData(PersonalInformationClass, offset, wlen, (unsigned char *)image.m_indexPtr);

		// increment the pointer
		image.IncrementIndex(wlen);
		dataLen -= wlen;
		offset += wlen;
	}

	if (stat != AUTOSTAR_OK)
		return stat;

	// free up the image data
	image.FreeBuffer();

	return stat;

}


/////////////////////////////////////////////
//
//	Name		:SetSiteInfo
//
//	Description :Sends the site collection to the HBX
//
//  Input		:pointer to CBodyDataCollection object that contains only sites
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModelLX::SetSiteInfo(CBodyDataCollection *siteList)
{
	eAutostarStat stat = AUTOSTAR_OK;
	CBodyData *body;
	int sentBytes;

// Delete the existing sites
	stat = DeleteCatalog(SiteInformationClass);

// Prepare for the first body data of this type if there are any
	POSITION pos = siteList->GetHeadPosition(SiteInfo);

// now loop through each body adding them to the linked list
	while(pos && stat == AUTOSTAR_OK)
	{
		// get the site data
		body = siteList->GetNext(pos, SiteInfo);

		// send the object to the LX
		stat = SendOneObject(body, SiteInformationClass, sentBytes, true);

	}	// do the next site

	return stat;
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
bool CModelLX::CheckSiteDelete(bool currentSite)
{
	// the current site must always remain as the last site, and cannot be deleted,
	// therefore this function always returns false

	CString warning = "The current site cannot be deleted.\n";

	MessageBoxEx(NULL, warning, "Warning", MB_OK | MB_TOPMOST, LANG_ENGLISH);

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
eAutostarStat CModelLX::GetPECTable(CBodyData *table, ePECAxis axis)
{
	eAutostarStat stat;
	CPersist per;

	int size = table->GetSizeOf() * 2 + 1;  // number of bytes * characters per byte + #

	// allocate space
	per.SetPointer(malloc(table->GetSizeOf() + 10));
	per.IncrementIndex(4);

	stat = RetrieveUserDataImage((CModelLX::RecordClass) axis, table->GetSizeOf() - 4, 1, per);

	if (stat != AUTOSTAR_OK)
		return stat;

	if (!table->ReadImageData((unsigned char *)(per.m_dataPtr)))
		return READ_ERROR;

	// since this is an LX-200, this record must be active
	table->SetActiveFlag(true);

	// give it a unique key based on the axis enum and date
	COleDateTime time = COleDateTime::GetCurrentTime();
	CString key = time.Format("%y%m%d%H%M%S");
	key += CPersist::ToString((int) axis);
	table->SetKey(key);

	per.FreeBuffer();

	return stat;
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
eAutostarStat CModelLX::SetPECTable(CBodyData *data, ePECAxis axis)
{
	eAutostarStat	stat = AUTOSTAR_OK;
	CPersist		image;
	int				offset, wlen;
	unsigned int	dataLen = 10;
	int				flag = KDJ_NONE;

	// get the data
	dataLen = data->GetSizeOf();
	image.SetPointer(malloc(dataLen + 10));
	data->PutImageData((unsigned char *)image.m_dataPtr, flag);

	// figure out the length
	// subtract out the position and active flag
	dataLen = data->GetSizeOf();
	dataLen -= 4;
	image.IncrementIndex(4);

	offset = 4;

	// write the data

	// break up the data into BlockSize byte chunks
	while(dataLen > 0 && stat == AUTOSTAR_OK)
	{
		// set the write length to the block size or whats left
		wlen = dataLen * 2 + WRITE_CMD_LEN > m_writeBlockSize ? (m_writeBlockSize - WRITE_CMD_LEN)/2 : dataLen;

		// Write the body data to the LX
		stat = ReplaceData(axis, offset, wlen, (unsigned char *)image.m_indexPtr);

		// increment the pointer
		image.IncrementIndex(wlen);
		dataLen -= wlen;
		offset += wlen;
	}

	if (stat != AUTOSTAR_OK)
		return stat;

	// free up the image data
	image.FreeBuffer();

	return stat;
}
/////////////////////////////////////////////
//
//	Name		:CModelRCX Constructor
//
//	Description :This is the only constructor that should be used for this class.
//				 It passes the autostar pointer to it's parent and then
//				 sets the model specific variables.
//
//  Input		:CAutostar pointer
//
//	Output		:None
//
////////////////////////////////////////////
CModelRCX::CModelRCX(CAutostar *autostar) : CModelLX(autostar)
{
	m_userBankFrom	= 0x38;
	m_userBankTo	= 0x3F;
}

CModelRCX::~CModelRCX()
{

}
