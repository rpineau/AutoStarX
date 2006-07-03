// Model494_497.cpp: implementation of the CModel494_497 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Model494_497.h"
#include "autoglob.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UINT AutostarThread( LPVOID pParam );



/////////////////////////////////////////////
//
//	Name		:CModel494_497 Constructor
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
CModel494_497::CModel494_497(CAutostar *autostar) : CAutostarModel(autostar)
{
	m_pageAddrStart = 0x8000;		// start address of paged pages
	m_pageAddrEnd	= 0x10000;		// end address of paged pages
	m_eePromStart	= 0xB600;		// start of EEProm hole
	m_eePromEnd		= 0xB800;		// end of EEProm hole
	m_userPageStart	= 6;			// First user data page
	m_readBlockSize	= 64;			// Maximum read block size
	m_mustReload	= false;		// If the link pointers are corrupt then this flag will be set
//  m_writeBlockSize = ?			// Maximum write block size (initialize by sub-class)
//  m_userPageEnd	= ?;			// Last user data page (initialized by sub-class)
	m_currentMaxSites = m_maxUserSites;	// initialize number of sites in handbox to maximum allowed
	firstfail		= true;

}

/////////////////////////////////////////////
//
//	Name		:CModel494_497 Destructor
//
//	Description :Default Destructor No heap memory used.
//
//  Input		:Never
//
//	Output		:What for? The world is about to end.
//
////////////////////////////////////////////
CModel494_497::~CModel494_497()
{

}

/////////////////////////////////////////////
//
//	Name		:RetrieveUserDataThread
//
//	Description :This is the thread that will do all the work. It will call
//				 RetrieveUserdataImage to get the entire user data area and then call 
//				 ConvertDataImage to convert the image into CBodyDataCollection.
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CModel494_497::RetrieveUserDataThread()
{
	eAutostarStat	stat;

	// get the user data image
	stat = RetrieveUserdataImage();

	// if retrieve ok then convert the image to CBodyDataCollection
	if(stat == AUTOSTAR_OK)
		stat = ConvertDataImage();

	m_autostar->m_lastError = stat;

	m_autostar->m_mode = DOWNLOAD;	//ADDED 2/6/02 

	// restart the handbox
	m_autostar->m_stat->DoingProcess("Restarting Handbox...Please Wait");
	m_autostar->RestartHandbox();

	// close the port
	m_autostar->m_serialPort.ClosePort();

	m_autostar->m_mode = OPERATIONAL;

// Report the status
	if (m_autostar->m_stat)
		m_autostar->m_stat->RetrieveComplete(stat);


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
eAutostarStat CModel494_497::RetrieveUserData(bool spawnThread)
{
	// set to busy so it can't be called again
	m_autostar->m_mode = BUSY;

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
//	Name		:RetrieveUserdataImage
//
//	Description :This function will retrieve the user data area of the autostar 
//				 until it finds all 0xFFs followed by zero reading backwards.
//				 It will put this data into m_userdataImage.
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CModel494_497::RetrieveUserdataImage()
{
	eAutostarStat	stat;
	int				place = 0;
	unsigned int	thisReadSize;
	int				percent = 101;
	int				FCnt;
	bool			foundFFs;

// tell them what we are doing
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Retrieving User Data");

	// Start at the first page of user data
	for (unsigned int p = m_userPageStart; p < m_userPageEnd; p++)
	{
		unsigned int a = m_pageAddrStart;
		while (a < m_pageAddrEnd)
		{
		// set the read size
			thisReadSize = m_readBlockSize;

		// get a block from the autostar. 'a' will be incremented to the next address 
		// 'thisReadSize' will be modified to how many were actually read
			if((stat = RetrieveMemoryBlock(p, a, &m_userdataImage[place], thisReadSize)) != AUTOSTAR_OK)
				return stat;

		// increment the place index
			place += thisReadSize;

		// Report the status if changed
			if (m_autostar->m_stat && percent != (int)(((float)place / (float)m_maxUserData) * 100))
			{
				percent = (int)(((float)place / (float)m_maxUserData) * 100);
				m_autostar->m_stat->PercentComplete(percent);
			}

		// look at whole buffer from the end and check for 5 or more FFs followed by a 0 reading backwards
			foundFFs = false;
			FCnt = 5;
			for (int i = place - 1; i >= 0; i--)
			{

			// if its an FF then count it and continue		
				if ( (unsigned char)m_userdataImage[i] == 0xFF )
				{
					if (FCnt-- < 0)
						foundFFs = true;
				}

			// we found a zero after finding FFs then we're done
				else if( m_userdataImage[i] == 0 && foundFFs)
				{
					m_autostar->m_stat->PercentComplete(100);	// set 100 percent
					// set next free (someday)

					// debug stuff
					#ifdef _DEBUG
						CStdioFile outputFile("input.bin",CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
						outputFile.Write(&m_userdataImage,MAX_USER_DATA);
					#endif
					//end debug stuff

					return AUTOSTAR_OK;
				}

			// if it's anything but FF or 0 then we're not at the end
				else if ( m_userdataImage[i] != 0)
					break;
			}
		}
	}

	// debug stuff
	#ifdef _DEBUG
		CStdioFile outputFile("input.bin",CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
		outputFile.Write(&m_userdataImage,MAX_USER_DATA);
	#endif
	//end debug stuff

	return AUTOSTAR_OK;		// this is highly unlikely
}



				


/////////////////////////////////////////////
//
//	Name		:ConvertDataImage
//
//	Description :This will convert the m_userdataImage into a CBodyDataCollection
//				 for the autostar. It will use the m_handboxData pointed to by m_autostar.
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CModel494_497::ConvertDataImage()
{
	BodyType		bodyIndex;
	int				ibody;
	int				dataImageIndex;
	poolposition	poolIndex;
	CBodyData		*body;
	int				flag = KDJ_NONE;

	// set the lo precision flag;
	flag |= KDJ_LO_PREC;

// first clear out the autostars handbox data
	m_autostar->m_handboxData.Clear();

	for(ibody = Asteroid; ibody < UserObj20; ibody++)
	{
	// convert the index
		bodyIndex = (BodyType)ibody;

	// get the offset for this body type	
		poolIndex.offset = getHeadPos(bodyIndex);

	// skip unsupported body types (when offset = 0)
		if (!poolIndex.offset)
			continue;

	// all heads start at page 6
		poolIndex.page = 6;

	// now convert this to a data image index
		dataImageIndex = convertPoolPosition(poolIndex);

	// now loop and create each body
		while((unsigned char)m_userdataImage[dataImageIndex] != 0xFF)
		{
		// get the NEXT index
			poolIndex = ConvertPoolImage(&m_userdataImage[dataImageIndex]);

		// convert it to an index
			dataImageIndex = convertPoolPosition(poolIndex);

		// check the range of the index before using it
			if (dataImageIndex < 0 || dataImageIndex > MAX_USER_DATA - 1)
			{
			// linked list is bad, flag as must reload and go to next body type
				m_mustReload = true;
				break;
			}

		// create a new data object
			if ((body = m_factory->Make(bodyIndex)) == NULL)
			// this type doesn't exist yet go to next body type
				break;											

		// Now initialize it and range check
			if (body->ReadImageData(&m_userdataImage[dataImageIndex], flag))
			{
			// add it to the autostars collection
				m_autostar->m_handboxData.Add(body);
			}
			else
			{
			// failed range check. toss it
				delete body;
			}

		}
	}
	return AUTOSTAR_OK;
}

/////////////////////////////////////////////
//
//	Name		:getHeadPos
//
//	Description :This will return the offset of the head position for the body type.
//
//  Input		:BodyType
//
//	Output		:offset
//
////////////////////////////////////////////
word CModel494_497::getHeadPos(BodyType body)
	{
	// get the head position for the body type		
		switch (body)
		{
		case Asteroid :
			return AsteroidHd;

		case Comet :
			return CometHd;

		case Satellite :
			return SatelliteHd;
			
		case UserObj :
			return UserObjHd;

		case LandMark :
			return LandmarkHd;

		case Tour :
			return TourHd;

	// unknown body type
		default :
			return 0;
		}
}


/////////////////////////////////////////////
//
//	Name		:convertPoolPosition
//
//	Description :This will convert a poolPosition to a image data index.
//
//  Input		:poolposition
//
//	Output		:int index
//
////////////////////////////////////////////
int CModel494_497::convertPoolPosition(poolposition pool)
{
	int index;
	int eePromSize = m_eePromEnd - m_eePromStart;

	// the number of pages times 32K - the hole
	index = (pool.page - m_userPageStart) * m_pageSize;

	// now add the offset minus the page start address
	index += pool.offset - m_pageAddrStart;

	// but check for holes
	if (pool.offset > m_eePromStart)
		index -= m_holeSize;

	return index;
}

/////////////////////////////////////////////
//
//	Name		:ConvertPoolImage
//
//	Description :This will convert a pool position in memory image
//				 to a poolposition struct.
//
//  Input		:image pointer
//
//	Output		:poolposition
//
////////////////////////////////////////////
poolposition CModel494_497::ConvertPoolImage(unsigned char *image)
{
poolposition pool;

	pool.page = *image;		// this is the easy part
	pool.offset = ((unsigned char)image[1] << 8) + (unsigned char)image[2];	// from motorola to intel

	return pool;
}

/////////////////////////////////////////////
//
//	Name		:SendUserDataThread
//
//	Description :This is the device specific version of SendUserData for
//				 the 494/497 models of Autostar. It will take care of converting
//				 the collection to a memory image and then sending that to the Autostar.
//
//  Input		:Data
//
//	Output		:Status
//
////////////////////////////////////////////
void CModel494_497::SendUserDataThread()
{
	eAutostarStat stat;

	stat = ConvertBodyDataColl(m_SendData);
// Convert Data Collection to memory image
	if (stat == AUTOSTAR_OK)
	{
	// Send the image to the autostar
		stat = SendDataImage();
	}

	m_autostar->m_lastError = stat;

	if (stat == AUTOSTAR_OK)
		m_autostar->m_mode = DOWNLOAD;	//changed 2/6/02 WAS: OPERATIONAL

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
eAutostarStat CModel494_497::SendUserData(CBodyDataCollection *Data, bool spawnThread)
{
	// set the data to be sent
	m_SendData = Data;
	m_autostar->m_mode = BUSY;

	// start the thread
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
//	Name		:ConvertBodyDataColl
//
//	Description :Converts a CBodyDataCollection to an Autostar memory image
//				 for downloading.
//
//  Input		:Data
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModel494_497::ConvertBodyDataColl(CBodyDataCollection *Data)
{
	BodyType		bodyIndex;
	int				ibody;
	int				dataImageIndex;
	poolposition	poolIndex, nextFree, pNull;
	CBodyData		*body;
	int				flag = KDJ_NONE;
	bool			atLeastOne;
	int				lastIndex = 0;	
	poolposition	lastNext;
	CString			errOutput = "", bodyList = "";

	errOutput +=   "The following tour(s) could not be sent";
	errOutput += "\nbecause it contains features that are only";
	errOutput += "\nsupported by the Autostar II:\n\n";

// Set any applicable Kluge flags
	if (m_autostar->GetVersion().CompareNoCase("24") < 0)
		flag |= KDJ_EPOCH_DATE;

	flag |= KDJ_LO_PREC;	// always set the lo precision (user objects) flag for this handbox

// init pNull
	pNull.offset = 0xFFFF;
	pNull.page = 0xFF;

// clear the image to FFs
	for (int i = 0; i<MAX_USER_DATA; i++)
		m_userdataImage[i] = 0xFF;

// init the EndHdsTag
	nextFree.page = m_userPageStart;
	nextFree.offset = EndHdsTag;
	dataImageIndex = convertPoolPosition(nextFree);
	m_userdataImage[dataImageIndex] = 0;

// initialize the nextFree poolposition
	nextFree.offset = EndHdsTag + 1;

// go through each body type and put it in its place
	for(ibody = Asteroid; ibody < UserObj20; ibody++)
	{
	// assume there are none of this type
		atLeastOne = false;

	// convert the index
		bodyIndex = (BodyType)ibody;

	// get the offset for this body type	
		poolIndex.offset = getHeadPos(bodyIndex);

	// Start at the first user page
		poolIndex.page = m_userPageStart;

	// now convert this to a data image index
		dataImageIndex = convertPoolPosition(poolIndex);

	// Prepare for the first body data of this type if there are any
		POSITION pos = Data->GetHeadPosition(bodyIndex);
	
		if (pos)// if there is at least one of these
			atLeastOne = true;	// make a note of it

	// now loop through each body adding them to the linked list
		while(pos)
		{
			// get the body data
			body = Data->GetNext(pos, bodyIndex);

			// skip it if it includes Autostar II features
			if (body->IsExtended())
			{
				bodyList += body->GetKey();
				bodyList += "\n";
				// if this is the last one of this type (but not the only) then set the body's NextPool position NULL
				if (!pos & lastIndex)
					putPoolPosition(pNull, &m_userdataImage[lastIndex]);
				else if (lastIndex)
					putPoolPosition(lastNext, &m_userdataImage[lastIndex]);
				continue;
			}

			if (atLeastOne)
			{
				// put the nextFree poolposition in this head pointer 
				putPoolPosition(nextFree, &m_userdataImage[dataImageIndex]);	
			}


			// convert the current nextFree to an index
			dataImageIndex = convertPoolPosition(nextFree);	

			// add the body size to the next free pool position
			nextFree = PoolAdd(nextFree, body->GetSizeOf());

			// if something went wrong with nextFree and this is not the last body then report an error
			if (pos && nextFree.page == 0)
				return OUT_OF_MEMORY;

			// if this is not the last one of this type then set the body's NextPool position to the next free
			if (pos)
				body->SetPosition(nextFree);	
			else
				body->SetPosition(pNull);

			// put the body in the image
			body->PutImageData(&m_userdataImage[dataImageIndex], flag);
			lastIndex = dataImageIndex;
			lastNext = nextFree;
		}	// do the next body
	}// do the next body type

	// debug stuff
	#ifdef _DEBUG
		CStdioFile outputFile("output.bin",CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
		outputFile.Write(&m_userdataImage,MAX_USER_DATA);
	#endif
	//end debug stuff

	// Display warning message if necessary
	if (bodyList != "")
	{
		errOutput += bodyList;
		MessageBoxEx(NULL,errOutput,"Warning", MB_ICONEXCLAMATION | MB_TOPMOST, LANG_ENGLISH);
	}

	// set the member variable nextFree
	m_nextFree = nextFree;
	return AUTOSTAR_OK;
}
			
/////////////////////////////////////////////
//
//	Name		:putPoolPosition
//
//	Description :This will put the passed pool in the m_userdataImage at Img
//				 and convert it to and Autostar image.
//
//  Input		:pool position, index
//
//	Output		:None
//
////////////////////////////////////////////
void CModel494_497::putPoolPosition(poolposition pool, unsigned char *Img)
{
	// first just put it in place
	*((poolposition *)Img) = pool;

	// now reverse the offset
	Img[1] = (pool.offset & 0xFF00) >> 8;
	Img[2] = pool.offset & 0xFF;
}

/////////////////////////////////////////////
//
//	Name		:PoolAdd
//
//	Description :This will add val to pool and will skip over
//				:the EE hole and step pages. It will also set
//				 page to zero if it overflows memory
//
//  Input		:pool, val
//
//	Output		:pool + val
//
////////////////////////////////////////////
poolposition CModel494_497::PoolAdd(poolposition pool,unsigned int val)
{
poolposition retVal;

	// make a zero based offset
	unsigned int offset = (pool.offset + val) - m_pageAddrStart;	
																	
	// these are the pages to add
	unsigned int pages = offset / (m_pageSize + m_holeSize);		
																	
	// get the left over offset
	offset %= (m_pageSize + m_holeSize);							
																	
	// offset it by the start address
	offset += m_pageAddrStart;			
	
	/* There are 4 cases where the "next" offset needs to be adjusted for holes:
		1) item starts before hole and finishes on same page after the hole
		2) item starts before page 1 hole and finishes on 2nd page before page 2 hole
		3) item starts before page 1 hole and finishes after page 2 hole (ADD 2 HOLES)
		4) item starts on page 1 after hole and finishes after page 2 hole
	*/															
	if (pool.offset <= m_eePromStart && pages > 0)				// Case 2 & Case 3: if start before the hole AND a page boundary is crossed			
	{
		offset += m_holeSize;								
		TRACE("\nJump Over A Hole & Off The Page\n");
	}																

	if (pool.offset <= m_eePromStart && offset >= m_eePromStart)	// Case 1 & Case 3: if beginning & ending positions span a hole(s)
	{
		offset += m_holeSize;											
		TRACE("\nJump Over A Hole\n");
	}																

	if (pool.offset > m_eePromStart && offset >= m_eePromStart && pages > 0)	// Case 4: is start and finish after the hole BUT a page was crossed
	{
		offset += m_holeSize;							
		TRACE("\nJump Off The Page & Over A Hole\n");
	}																

	// set the return value
	retVal.offset = offset;
	retVal.page = pool.page + pages;

	// if we're beyond the end then we are out of memory
	if (retVal.page >= m_userPageEnd)
		retVal.page = 0;

	return retVal;
}

/////////////////////////////////////////////
//
//	Name		:SendDataImage
//
//	Description :This function will send m_userdataImage to the autostar
//				 until it see's all FFs
//
//  Input		:
//
//	Output		:
//
////////////////////////////////////////////
eAutostarStat CModel494_497::SendDataImage()
{
	eAutostarStat	stat;
	int				place = 0;
	int				endPlace;
	unsigned int	thisWriteSize;
	unsigned char	data[5];
	unsigned char	resp[5];
	unsigned int	count;
	int				percent = 101;

// tell them what we're going to do
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Sending User Data");

	// set the ending index to the first unused memory space
	endPlace = convertPoolPosition(m_nextFree);


	// Start at the first page of user data
	for (unsigned int p = m_userPageStart; p < m_userPageEnd; p++)
	{
		if ((p % 2) == 0)	// every even page erase the bank
		{
		// erase the bank
		data[0] = m_userPageStart / 2;
		count = 1;
		if ((stat = m_autostar->SendCommand(ERASE_BANK, data, resp, count)) != AUTOSTAR_OK)
			return stat;

		if (resp[0] != 'Y')
			return ERASE_ERROR;
		}

		unsigned int a = m_pageAddrStart;
		while (a < m_pageAddrEnd)
		{
		// set the write size
			if ((unsigned int)(endPlace - place) > m_writeBlockSize)
				thisWriteSize = m_writeBlockSize;
			else
				thisWriteSize = endPlace - place;

		// send a block to the autostar. 'a' will be incremented to the next address 
		// 'thisWriteSize' will be modified to how many were actually read
			if((stat = SendMemoryBlock(p, a, &m_userdataImage[place], thisWriteSize)) != AUTOSTAR_OK)
			{
				// check for verify failure
				if (stat == VERIFY_FAILED)
				{
					// reset the page and place index
					if ((p % 2) == 0)
						p -= 1;
					else
						p -= 2;

					place = ((p + 1) - m_userPageStart) * m_pageSize;

					// break out of the while loop
					break;
				}
				else
				// return on any other error
					return stat;
			}

		// increment the place index
			place += thisWriteSize;

		// Report the status if changed
			if (m_autostar->m_stat && percent != (int)(((float)place / (float)endPlace) * 100))
			{
				percent = (int)(((float)place / (float)endPlace) * 100);
				m_autostar->m_stat->PercentComplete(percent);
			}

		// quit when we reach the nextFree position
			if (place >= endPlace)
				return AUTOSTAR_OK;
		}
	}
	return AUTOSTAR_OK;		// this is highly unlikely
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
int CModel494_497::GetMaxUserData()
{
	return (int)m_maxUserData;
}


/////////////////////////////////////////////
//
//	Name		:DeleteOneObject
//
//	Description :Deletes a single object by object type and name
//				NOTE: not supported for #494/497 yet
//
//  Input		:Body Type, CString name
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CModel494_497::DeleteOneObject(BodyType bodyType, CString objectName)
{
	return NOT_ALLOWED;
}

/////////////////////////////////////////////
//
//	Name		:SendOneObject
//
//	Description :Formats and sends one user data object to the LX.
//				NOTE: not supported for #494/497 yet
//
//  Input		:body data, Body Type
//
//	Output		:Stat, int # of Bytes sent
//
////////////////////////////////////////////
eAutostarStat CModel494_497::SendOneObject(CBodyData *body, BodyType bodyType, int &sentBytes)
{
	return NOT_ALLOWED;
}

/////////////////////////////////////////////
//
//	Name		:DeleteCatalog
//
//	Description :Deletes an entire catalog of objects from the handbox
//				NOTE: not supported for #494/497 yet
//
//  Input		:Body Type
//
//	Output		:Stat
//
////////////////////////////////////////////
eAutostarStat CModel494_497::DeleteCatalog(BodyType bodyType)
{
	return NOT_ALLOWED;
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
void CModel494_497::SendProgramThread()
{
	// Call the base class
	CAutostarModel::SendProgramThread();

	eAutostarStat stat;

	// restart the handbox
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Initializing Handbox...Please Wait");
	stat = m_autostar->InitializeConnection(true,false);
		
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
eAutostarStat CModel494_497::InitializeSend(bool closeComPort)
{
	// note: 2nd param must be TRUE to force handbox into download mode
	return m_autostar->InitializeConnection(closeComPort, true);
}

eAutostarStat CModel494_497::TestFunction()
{
	return AUTOSTAR_OK;
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
eAutostarStat CModel494_497::ClearPresets(bool closePort)
{
	unsigned char resp[5];
	unsigned int cnt = 1;
	unsigned char data[] = {0xB7, 0xFF, 0xFF};
	eAutostarStat stat;

	// only do this if not busy
	if (m_autostar->m_mode != BUSY)
	{
		// init the com port first
		stat = m_autostar->InitializeConnection(false);
		// now send the command
		if (stat == AUTOSTAR_OK)
			stat = m_autostar->SendCommand(PROGRAM_EE, data, resp, cnt);
	}

	if (closePort)
	// close the port
		m_autostar->m_serialPort.ClosePort();

	return stat;
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
eAutostarStat CModel494_497::GetUserInfo(CUserInfo *pInfo)
{
	eAutostarStat stat;
	unsigned int page = 0;
	unsigned int addr;
	unsigned int count;
	CPersist per;

	UserInfoType *Amark = NULL;
	PersonalInfoType *Amark2 = NULL;

	// read the current & max site
	count	= 2;
	addr	= m_eePromCurrentSiteAddr;	// the address of the current site byte
	per.SetPointer(malloc(sizeof(UserInfoType) + sizeof(PersonalInfoType) + 10));

	// fill the memory image with "default" data
	pInfo->PutImageData((unsigned char *) per.m_dataPtr, KDJ_ZERO_BASED);

	// now overwrite only the current and max site data (all that we care about)

	per.IncrementIndex((int)(&Amark->CurrentSite));	// skip to the current site
	
	stat = RetrieveMemoryBlock(page, addr, (unsigned char *) per.m_indexPtr, count, false);

	if (stat != AUTOSTAR_OK)
	{
		per.FreeBuffer();
		return stat;
	}

	// convert two chars to two shorts
	unsigned char *data = (unsigned char *) per.m_indexPtr;
	data[3] = data[1];
	data[2] = 0;
	data[1] = data[0];
	data[0] = 0;

	per.ResetIndex();
	per.IncrementIndex((int) (&Amark->EndTag) + 1);	// skip to Personal Info Record

	// read the Personal Information Class

	count = sizeof(pInfo->m_personalInfo) - 4;
	addr = m_eePromPersonalInfoAddr;

	per.IncrementIndex(4);	// to skip over the poolposition
	
	stat = RetrieveMemoryBlock(page, addr, (unsigned char *) per.m_indexPtr, count, false);

	if (stat != AUTOSTAR_OK)
	{
		per.FreeBuffer();
		return stat;
	}

	//need to fix the serial number before proceeding
	char *tempSN = (char *)per.m_indexPtr + (int)(&Amark2->SerialNum) - 4;

	char copySN[16];
	strncpy(copySN, (tempSN - 2), 16);

	// "move" the string over 2 bytes
	strncpy(tempSN, copySN, 16);

	if (!pInfo->ReadImageData((unsigned char *)(per.m_dataPtr)))
		stat = READ_ERROR;

	// convert site info from zero-based to one-based
	pInfo->m_userInfo.CurrentSite++;
	pInfo->m_userInfo.MaxSites++;

	//remember the number of sites
	m_currentMaxSites = pInfo->m_userInfo.MaxSites;

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
eAutostarStat CModel494_497::GetSiteInfo(CBodyDataCollection *siteList)
{
	eAutostarStat stat;
	unsigned int page = 0;
	unsigned int addr;
	unsigned int count;
	CPersist per;
	CBodyData *body;

	// read a data length equal to the record size times the # of records
	count = sizeof(SiteType) * m_currentMaxSites;

	addr = m_eePromUserSites;	// the address of the start of the data
	
	per.SetPointer(malloc(count + 10));

	stat = RetrieveMemoryBlock(page, addr, (unsigned char *) per.m_indexPtr, count, false);

	if (stat != AUTOSTAR_OK)
	{
		per.FreeBuffer();
		return stat;
	}

	// reset the pointer
	per.SetIndex(0);

	// loop and create each site object
	for (int i  = 1; i <= m_currentMaxSites; i++)
	{
		// create a new data object
		if ((body = m_factory->Make(SiteInfo)) == NULL)
			break;

		// add it to the collection
		if (body->ReadImageData((unsigned char *) per.m_indexPtr, KDJ_LO_PREC) && body->GetKey() != "<AN_UNUSED_SITE>")
			siteList->Add(body);
		else
			delete body;

		per.IncrementIndex(sizeof(SiteType));

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
eAutostarStat CModel494_497::SetUserInfo(CBodyData *data)
{	
//	return AUTOSTAR_OK;


	eAutostarStat		stat = AUTOSTAR_OK;
	UserInfoType		*Amark = NULL;
	PersonalInfoType	*Amark2 = NULL;
	CPersist			image;
	unsigned int		dataLen = 10;
	int					flag = KDJ_ZERO_BASED;

	// get the data
	dataLen = data->GetSizeOf();
	image.SetPointer(malloc(dataLen + 10));
	data->PutImageData((unsigned char *)image.m_dataPtr, flag);

	// set the pointer to the current site number
	image.IncrementIndex((int)(&Amark->CurrentSite) + 1);

	// write the current site number
	stat = WriteEepromData(m_eePromCurrentSiteAddr, 1, (unsigned char *)image.m_indexPtr);

	if (stat != AUTOSTAR_OK)
	{
		image.FreeBuffer();
		return stat;
	}

	// set the pointer to the max site
	image.IncrementIndex(2);

	// write the current site number
	stat = WriteEepromData(m_eePromLastValidSiteAddr, 1, (unsigned char *)image.m_indexPtr);

	if (stat != AUTOSTAR_OK)
	{
		image.FreeBuffer();
		return stat;
	}

	// send the personal information data
	// set the pointer to the FirstName variable
	image.ResetIndex();
	image.IncrementIndex((int)(&Amark->EndTag) + 1 + (int)(&Amark2->FirstName));

	dataLen = sizeof(PersonalInfoType) - 4 - 21;  // subtract poolposition and everything after postcode

	stat = WriteEepromData(m_eePromPersonalInfoAddr, dataLen, (unsigned char *)image.m_indexPtr);

	if (stat != AUTOSTAR_OK)
	{
		image.FreeBuffer();
		return stat;
	}

	// send the Serial Number
	image.ResetIndex();
	image.IncrementIndex((int)(&Amark->EndTag) + 1 + (int)(&Amark2->SerialNum));

	dataLen = 16;

	WriteEepromData(m_eePromSNAddr, dataLen, (unsigned char *)image.m_indexPtr);

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
eAutostarStat CModel494_497::SetSiteInfo(CBodyDataCollection *siteList)
{
	eAutostarStat		stat = AUTOSTAR_OK;
	CPersist			image;
	unsigned int		dataLen;
	int					flag = KDJ_LO_PREC;
	CBodyData			*data;

	// format the data
	dataLen = sizeof(SiteType) * siteList->GetCount(SiteInfo);
	image.SetPointer(malloc(dataLen + 10));

	POSITION pos = siteList->GetHeadPosition(SiteInfo);

	while (pos)
	{
		data = siteList->GetNext(pos, SiteInfo);
		data->PutImageData((unsigned char *)image.m_indexPtr, flag);

		image.IncrementIndex(sizeof(SiteType));
	}

	// reset the index
	image.ResetIndex();

	// send the data to the handbox
	stat = WriteEepromData(m_eePromUserSites, dataLen, (unsigned char *)image.m_indexPtr);

	if (stat != AUTOSTAR_OK)
	{
		image.FreeBuffer();
		return stat;
	}

	image.FreeBuffer();
	
	// assumming that this is the last transmission to the handbox, restart it
	stat = m_autostar->PowerCycleHandbox();

	return stat;
}

/////////////////////////////////////////////
//
//	Name		:WriteEepromData
//
//	Description :Writes data to the Eeprom at the address indicated
//
//  Input		:addr, len, data
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModel494_497::WriteEepromData(int addr, int len, unsigned char *data)
{
	eAutostarStat	stat = AUTOSTAR_OK;
	unsigned char	resp[10], cmd[3];
	unsigned int	cnt = 1;
	int				index = 0;


	while (index < len && stat == AUTOSTAR_OK)
	{
		cmd[0] = (char) ((addr & 0xFF00) >> 8);
		cmd[1] = (char) (addr & 0xFF);
		cmd[2] = data[index++];

		// send one byte of data
		stat = m_autostar->SendCommand(PROGRAM_EE, cmd, resp, cnt);

		// check the response
		if (resp[0] != 'Y')	
			return COMMAND_FAILED;

		//increment the address
		addr++;

	}

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
bool CModel494_497::CheckSiteDelete(bool currentSite)
{
	// prompt if its ok to delete the current site
	if (currentSite)
	{
			CString warning = "You are about to delete the current site.\n";
				   warning += "Do you wish to proceed?";
			if (MessageBoxEx(NULL, warning, "Warning", MB_YESNO | MB_TOPMOST, LANG_ENGLISH) == IDNO)
				return false;
	}
	else
	// prompt if its ok to delete all user data
	{
		CString warning = "If you delete the last site from the handbox, the entire user\n";
		       warning += "data area will be deleted, including motor calibrations and\n";
			   warning += "Owner Info.  Do you wish to proceed?";
		if (MessageBoxEx(NULL, warning, "Warning", MB_YESNO | MB_TOPMOST, LANG_ENGLISH) == IDNO)
			return false;
	}

	return true;
}
