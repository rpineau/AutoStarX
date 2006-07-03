// AutostarModel.cpp: implementation of the CAutostarModel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AutostarModel.h"
#include "BodyDataMaker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////
//
//	Name		:AutostarThread
//
//	Description :This is the Thread function that will do SendUserData or
//				 RetrieveUserData depending on m_threadTask
//
//  Input		:void pointer to CAutostarModel
//
//	Output		:Return stat
//
////////////////////////////////////////////
UINT AutostarThread( LPVOID pParam )
{
	CAutostarModel* pAutostar = (CAutostarModel*)pParam;

	if(pParam == NULL)
		return 1;

	switch (pAutostar->m_threadTask)
	{
	case CAutostarModel::UNASSIGNED :
		return 1;

	case CAutostarModel::SEND_USER_DATA :
		pAutostar->SendUserDataThread();
		return 0;

	case CAutostarModel::RETR_USER_DATA :
		pAutostar->RetrieveUserDataThread();
		return 0;

	case CAutostarModel::SEND_PROGRAM :
		pAutostar->SendProgramThread();
		return 0;

	default :
		return 1;
	}


}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAutostarModel::CAutostarModel(CAutostar *autostar)
{
	m_autostar = autostar;
	m_factory = new CBodyDataMaker;
}

CAutostarModel::~CAutostarModel()
{
	delete m_factory;
}

/////////////////////////////////////////////
//
//	Name		:SendProgramBlock
//
//	Description :This will send the program to the Autostar.
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CAutostarModel::SendProgramBlock()
{
	eAutostarStat	stat;
	int				place = 0;
	DWORD			startPlace, endPlace;
	unsigned int	thisWriteSize;
	unsigned char	data[5];
	unsigned char	resp[5];
	unsigned int	count;
	int				percent = 101;

ReleaseHdType *hdr;

// tell them what we're going to do
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Sending Program");

// get a reference to the header
	hdr = (ReleaseHdType *)m_autostar->m_persist.m_dataPtr;

// increment past the header
	m_autostar->m_persist.IncrementIndex(sizeof(ReleaseHdType));

// figure out the highest index
	endPlace = m_autostar->m_persist.m_dataReadCnt;

// set the start place for percent status
	startPlace = sizeof(ReleaseHdType);

	// Start at the first page of user data
	for (unsigned int p = hdr->origin[0]; p < m_totalPages; p++)
	{

	// check for user data pages
		while (p >= m_userPageStart && p < m_userPageEnd)
		{
		// always skip 2 pages at a time
			m_autostar->m_persist.IncrementIndex((m_pageSize + m_holeSize) * 2);
			p += 2;
		}

	// every even page erase the bank
		if ((p % 2) == 0)	
		{
		// erase the bank
			data[0] = p / 2;
			count = 1;
			if ((stat = m_autostar->SendCommand(ERASE_BANK, data, resp, count)) != AUTOSTAR_OK)
				return stat;

		// check the response
			if (resp[0] != 'Y')
			{	
			// check if we tried to erase page 0
				if (p != 0)			
					return ERASE_ERROR;
				else
				{
				// skip past the Safe loader
					m_autostar->m_persist.IncrementIndex((m_pageSize + m_holeSize) * 2);
					p += 1;		// skip the next page also
					startPlace += (m_pageSize + m_holeSize) * 2;	// so we don't start at 6%
					continue;	// force the loop again
				}
			}
		}

		unsigned int a = m_pageAddrStart;
		while (a < m_pageAddrEnd)
		{

		// look for non FF's
			while(*((unsigned char *)m_autostar->m_persist.m_indexPtr) == 0xFF)
			{
				m_autostar->m_persist.IncrementIndex(1);

			// check for the end of file
				if (m_autostar->m_persist.m_dataIndex >= m_autostar->m_persist.m_dataReadCnt)
				{
					m_autostar->m_stat->PercentComplete(100);
					return AUTOSTAR_OK;
				}

			// increment the address	
				a++;
				if (a == m_pageAddrEnd)
					break;
			}

			// trying to write in the hole
			if (a > m_eePromStart && a < m_eePromEnd)
				return BAD_FILE;

		// if we didn't end up at the end of the page 
			if (a < m_pageAddrEnd)
			{
			// figure out a block that we can send
				thisWriteSize = 0;
				// find the next FF or we reached the write block size or we reach the end of the page
				while (thisWriteSize < m_writeBlockSize && (a + thisWriteSize) < m_pageAddrEnd)
				{
					thisWriteSize++;
					if (m_autostar->m_persist.m_dataIndex + thisWriteSize >= m_autostar->m_persist.m_dataReadCnt - 1) // ||
//						*((unsigned char *)(m_autostar->m_persist.m_indexPtr) + thisWriteSize) == 0xFF)
						break;
				}

			// send a block to the autostar. 'a' will be incremented to the next address 
			// 'thisWriteSize' will be modified to how many were actually read
				if((stat = SendMemoryBlock(p, a, (unsigned char *)m_autostar->m_persist.m_indexPtr, thisWriteSize)) != AUTOSTAR_OK)
				{
				// check for verify failure
					if (stat == VERIFY_FAILED)
					{
						// reset the page and place index
						if ((p % 2) == 0)
							p -= 1;
						else
							p -= 2;
						m_autostar->m_persist.SetIndex(((p + 1) * (m_pageSize + m_holeSize)) + sizeof(ReleaseHdType));

						// break out of the while loop
						break;
					}
					else
					// return on any other error
						return stat;
				}

			// increment the index
				m_autostar->m_persist.IncrementIndex(thisWriteSize);

			// check for sendmemoryblock changeing address on me
				if (a == m_eePromEnd)
					m_autostar->m_persist.IncrementIndex(m_holeSize);

			// check for the end of file
				if (m_autostar->m_persist.m_dataIndex >= m_autostar->m_persist.m_dataReadCnt)
				{
					m_autostar->m_stat->PercentComplete(100);
					return AUTOSTAR_OK;
				}

			// Report the status if changed
				if (m_autostar->m_stat && percent != (int)(((float)(m_autostar->m_persist.m_dataIndex - startPlace) / (float)endPlace) * 100))
				{
					percent = (int)(((float)(m_autostar->m_persist.m_dataIndex - startPlace)/ (float)endPlace) * 100);
					m_autostar->m_stat->PercentComplete(percent);
				}
			}
		}
	}
	return AUTOSTAR_OK;	// all done


}

/////////////////////////////////////////////
//
//	Name		:SendMemoryBlock
//
//	Description :This will send the data to the page and address passed to it to the autostar
//				 in sequencial locations starting at the data pointer.
//				 Before sending to the autostar it will first check the
//				 address and count and modify it to skip over the EEProm hole.
//				 It will also leave the address at the next available memory location
//				 and set the count to how many bytes were sent.
//
//  Input		:Page, Address, Data pointer, count
//
//	Output		:Address of next available memory
//				 count of bytes written
//               eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CAutostarModel::SendMemoryBlock(unsigned int page, unsigned int &addr,unsigned char *data, unsigned int &count)
{
	unsigned int toAddr;
	eAutostarStat stat;
	unsigned char resp[5];

		
// figure out if we are going to hit the EEProm hole 
// this will never happen with even block sizes.
	toAddr = addr + count;
	if (toAddr > m_eePromStart && toAddr < m_eePromEnd)
		count = m_eePromStart - addr;
	else
		if(toAddr > m_pageAddrEnd)
			count = m_pageAddrEnd - addr;

	if (count == 0)
		return WRITE_ERROR;

// get some memory for this
	unsigned char *writeCmd = (unsigned char *)malloc(count + 10);

// create the data for the command
	writeCmd[0] = (unsigned char)'W';
	writeCmd[1] = (unsigned char)page;
	writeCmd[2] = (unsigned char)((addr >> 8) & 0xFF);
	writeCmd[3] = (unsigned char)(addr & 0xFF);
	writeCmd[4] = (unsigned char)count;
	memcpy(&writeCmd[5], data, count);

// now send the command to the Autostar
	stat = m_autostar->SendCommand(WRITE_FLASH, writeCmd, resp, count);

// free the memory block	
	free(writeCmd);

// check the return status
	if (stat != AUTOSTAR_OK)
	{
		m_autostar->m_mode = UNKNOWN;
		return stat;
	}

// check the response
	if (resp[0] != (unsigned char)'Y')
		return WRITE_ERROR;

// do a verify
	if (m_autostar->m_verifyMode)
	{
		// make my own data area
		void *cmpData			= malloc(count + 5);
		unsigned int cmpCount	= count;
		unsigned int cmpAddr	= addr;

		// read back what was just written
		stat = RetrieveMemoryBlock(page, cmpAddr, (unsigned char *)cmpData, cmpCount);
		if (stat != AUTOSTAR_OK)
			return stat;

		// check that the returned data size is the same
		if (cmpCount != count)
		{
			// if they don't compare then try it one more time
			cmpCount	= count;
			cmpAddr	= addr;

			// read back what was just written
			stat = RetrieveMemoryBlock(page, cmpAddr, (unsigned char *)cmpData, cmpCount);
			if (stat != AUTOSTAR_OK)
				return stat;

			// check that the returned data size is the same.
			if (cmpCount != count)
			{
				free (cmpData);
				return VERIFY_FAILED;
			}
		}

		// compare it to the original
		for (unsigned int i=0; i<cmpCount; i++)
		{
			if (*((unsigned char *)cmpData+i) != data[i])
			{
				free (cmpData);
				return VERIFY_FAILED;
			}

			// for testing
/*			if (page == 6 && addr > 0x9040 && firstfail)
			{
				free (cmpData);
				firstfail = false;
				return VERIFY_FAILED;
			}	*/
		}
		free (cmpData);
	}

// now set addr to the next available address
	addr += count;

// fix it if we just read up to the hole
	if (addr == m_eePromStart)
		addr = m_eePromEnd;

	return AUTOSTAR_OK;
}

/////////////////////////////////////////////
//
//	Name		:RetrieveMemoryBlock
//
//	Description :This will read the page and address passed to it from the autostar
//				 put the data in sequencial locations starting at the data pointer.
//				 Before getting from the autostar it will first check the
//				 address and count and modify it to skip over the EEProm hole.
//				 It will also leave the address at the next available memory location
//				 and set the count to how many bytes were read.
//
//  Input		:Page, Address, Data pointer, count
//
//	Output		:Address of next available memory
//				 count of bytes read
//               eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CAutostarModel::RetrieveMemoryBlock(unsigned int page, unsigned int &addr,unsigned char *data, unsigned int &count, bool jumpOverHole)
{
	unsigned int toAddr;
	unsigned char cmdData[5];
	eAutostarStat stat;
		
// figure out if we are going to hit the EEProm hole 
// this will never happen with even block sizes.
	toAddr = addr + count;
	if (toAddr > m_eePromStart && toAddr < m_eePromEnd && jumpOverHole)
		count = m_eePromStart - addr;
	else
		if(toAddr > m_pageAddrEnd)
			count = m_pageAddrEnd - addr;

// create the data for the command
	cmdData[0] = (char)page;
	cmdData[1] = (char)((addr >> 8) & 0xFF);
	cmdData[2] = (char)(addr & 0xFF);
	cmdData[3] = (char)count;

// now send the command to the Autostar
	if ((stat = m_autostar->SendCommand(READ, cmdData, data, count)) != AUTOSTAR_OK)
		return stat;

// now set addr to the next available address
	addr += count;

// fix it if we just read up to the hole
	if (addr == m_eePromStart && jumpOverHole)
		addr = m_eePromEnd;

	return AUTOSTAR_OK;
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
eAutostarStat CAutostarModel::SendProgram(bool spawnThread, bool eraseBanks)
{
	// set the data to be sent
	m_autostar->m_mode = BUSY;

	// start the thread
	m_threadTask = SEND_PROGRAM;
	// Start the thread if spawnThread = true (default)
	if (spawnThread)
		AfxBeginThread(AutostarThread, this);
	else	// else call the function in the current thread
		AutostarThread(this);

	return AUTOSTAR_UPLOADING;
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
void CAutostarModel::SendProgramThread()
{
	unsigned char resp[5];
	unsigned int cnt = 1;
	eAutostarStat stat;

	stat = SendProgramBlock();

	m_autostar->m_lastError = stat;

// send a restart if all went ok
	if (m_autostar->m_lastError == AUTOSTAR_OK)
	{
		m_autostar->m_stat->DoingProcess("Restarting Handbox...Please Wait");
		m_autostar->SendCommand(INIT, NULL, resp, cnt);
//		m_autostar->RetrieveVersion();	
		m_autostar->m_mode = OPERATIONAL;
	}
	else
		m_autostar->m_mode = DOWNLOAD;

// close the port
	m_autostar->m_serialPort.ClosePort();

// free up the file block
	m_autostar->m_persist.FreeBuffer();



}


/////////////////////////////////////////////
//
//	Name		:ForceGarbageCollection
//
//	Description :Overidden function in ModelLX to send a garbage collection message
//
//  Input		:None
//
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CAutostarModel::ForceGarbageCollection()
{	
	return NOT_ALLOWED;
}



/////////////////////////////////////////////
//
//	Name		:Set the Maximum Baud Rate
//
//	Description :Changes the maximum baud rate of the registry, 
//				 handbox and serial port (if supported)
//
//  Input		:baud rate
///
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CAutostarModel::SetMaxBaudRate(CSerialPort::eBaud baud, bool justHandbox)
{
	return NOT_ALLOWED;
}

eAutostarStat CAutostarModel::TestFunction()
{
	return AUTOSTAR_OK;
}


/////////////////////////////////////////////
//
//	Name		:ClearPresets
//
//	Description :Clear the user presets in the handbox
//				 (not supported on Autostar II)
//
//  Input		:bool to close serial port when completed
///
//	Output		:stat
//
////////////////////////////////////////////
eAutostarStat CAutostarModel::ClearPresets(bool closeport)
{
	return NOT_ALLOWED;
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
eAutostarStat CAutostarModel::GetPECTable(CBodyData *table, ePECAxis axis)
{

	// if this function is not overridden by a handbox type that
	// supports PEC, the table will be returned as a default
	// table with an inactive flag

	// set the flag to inactive
	table->SetActiveFlag(false);

	// give it a unique key based on the axis enum and date
	COleDateTime time = COleDateTime::GetCurrentTime();
	CString key = time.Format("%y%m%d%H%M%S");
	key += CPersist::ToString((int) axis);
	table->SetKey(key);

	return AUTOSTAR_OK;
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
eAutostarStat CAutostarModel::SetPECTable(CBodyData *table, ePECAxis axis)
{
	// if this function is not overridden by a handbox type that
	// supports PEC, this function will do nothing

	return AUTOSTAR_OK;
}
