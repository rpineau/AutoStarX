// Model494.cpp: implementation of the CModel494 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Model494.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ASCII_SOM	0x02


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModel494::CModel494(CAutostar *autostar) : CModel494_497(autostar)
{
	m_userPageEnd	= 7;			// one beyond last user page
    m_writeBlockSize = 16;			// Maximum write block size
	m_pages = m_userPageEnd - m_userPageStart;
	m_holeSize = m_eePromEnd - m_eePromStart;
	m_pageSize = (m_pageAddrEnd - m_pageAddrStart) - m_holeSize;
	m_maxUserData	= m_pages * m_pageSize;		// maximum user data space with hole cut out
	m_totalPages	= 16;
	m_eePromLastValidSiteAddr = 0xb742;
	m_eePromCurrentSiteAddr = 0xb741;
	m_eePromPersonalInfoAddr = 0xb62a;
	m_eePromSNAddr = 0xb6ba;
	m_eePromUserSites = 0xb6ca;
	m_maxUserSites = 5;
}

CModel494::~CModel494()
{

}

/////////////////////////////////////////////
//
//	Name		:CheckPage7File
//
//	Description :This will search for a page7 file for this version of the autostar.
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CModel494::CheckPage7File()
{
	CFileFind	finder;
	CString		foundFile;
	CFile		sendFile;
	CString		fileVer;

	// pattern directory string
	CString pattern = m_userSettings.GetPage7Dir() + CString(_T("Asp7A*.rom"));

	// start the file search
	BOOL bWorking = finder.FindFile(pattern);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		CFile file;
		// skip . and .. files but this should not happen
		if (finder.IsDots())
			continue;

		// Get a file to test
		foundFile = finder.GetFilePath();

		char buff[30];
		file.Open(foundFile, CFile::modeRead);

		// read it all in
		file.Read((void *)buff, 30);

		// if theres no SOM then try the next file
		if(buff[0] != ASCII_SOM)
			continue;

		// convert the version
		fileVer = CString(buff+12, 4);
		
		// compare version
		// either they are exact or file version is 1.0j and handbox is 1.0a - 1.0j
		if (fileVer == m_autostar->m_version || (fileVer == _T("1.0j") && 
			m_autostar->m_version >= _T("1.0a") && m_autostar->m_version <= _T("1.0j") ) )
		{
			m_page7File = foundFile;
			return AUTOSTAR_OK;
		}
	}
	return NO_PAGE7_FILE;
			

}

/////////////////////////////////////////////
//
//	Name		:RetrievePage7
//
//	Description :This will load page 7 from the autostar and save it to a file.
//
//  Input		:None
//
//	Output		:eAutostarStat
//
////////////////////////////////////////////
eAutostarStat CModel494::RetrievePage7()
{
	void	*p7Buff;
	CPersist	per;
	char	fileHeader[] = "\2Autostar|A|1.0j \3";
	CFile	p7File;
	CString	p7FileName;
	unsigned int	thisReadSize;
	eAutostarStat	stat;
	int				percent=101;

	// tell them we're get page 7
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Retrieving Database");

	// allocate a buffer
	p7Buff = malloc(m_pageSize + 100);
	per.SetPointer(p7Buff);

	// fill the header with this version number and copy it to the buffer
	memcpy(&fileHeader[12], m_autostar->GetVersion().GetBuffer(4), 4);
	memcpy(per.m_indexPtr, fileHeader, strlen(fileHeader));
	per.IncrementIndex(strlen(fileHeader));

	unsigned int a = m_pageAddrStart;
	while (a < m_pageAddrEnd)
	{
	// set the read size
		thisReadSize = m_readBlockSize;

	// get a block from the autostar. 'a' will be incremented to the next address 
	// 'thisReadSize' will be modified to how many were actually read
		if((stat = RetrieveMemoryBlock(7, a, (unsigned char *)per.m_indexPtr, thisReadSize)) != AUTOSTAR_OK)
			return stat;

	// increment the place index
		per.IncrementIndex(thisReadSize);

	// Report the status if changed
		if (m_autostar->m_stat && percent != (int)(((float)per.m_dataIndex / (float)m_pageSize) * 100))
		{
			percent = (int)(((float)per.m_dataIndex / (float)m_pageSize) * 100);
			m_autostar->m_stat->PercentComplete(percent);
		}
	}

	// now save the data in a file
	p7FileName = m_userSettings.GetPage7Dir() + CString("Asp7A") + m_autostar->GetVersion() + CString(".rom");

	try
	{
		p7File.Open(p7FileName, CFile::modeReadWrite | CFile::modeCreate);
		p7File.Write(per.m_dataPtr, per.m_dataIndex);
		p7File.Close();
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
		per.FreeBuffer();
		return BAD_FILE;
	}


	per.FreeBuffer();
	m_page7File = p7FileName;

	// all done
	return	AUTOSTAR_OK;
}

/////////////////////////////////////////////
//
//	Name		:SendUserDataThread
//
//	Description :This is the 494 specific version of this function.
//				 It will check for the page 7 file that matches this
//				 version. If the file is not found it will download
//				 page 7 from the Autostar and then call the base class
//				 version of this function which will do the body download
//				 upon return of this it will then upload the proper page7
//				 data to the autostar.
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CModel494::SendUserDataThread()
{
	eAutostarStat	stat;

	// check page 7 file
	if((stat = CheckPage7File()) == NO_PAGE7_FILE)
	{
		// get page 7
		if((stat = RetrievePage7()) != AUTOSTAR_OK)
		{
			if (m_autostar->m_stat)
				m_autostar->m_stat->SendComplete(stat);
			return;
		}
	}

	// make sure it was OK
	else if (stat != AUTOSTAR_OK)
		{
			if (m_autostar->m_stat)
				m_autostar->m_stat->SendComplete(stat);
			return;
		}

	// send the user data
	CModel494_497::SendUserDataThread();

	// send page 7
	stat = SendPage7();

	// restart the handbox
	m_autostar->m_stat->DoingProcess("Restarting Handbox...Please Wait");
	m_autostar->RestartHandbox();

	// close the port
	m_autostar->m_serialPort.ClosePort();

	// send the final status
	if (m_autostar->m_stat)
		m_autostar->m_stat->SendComplete(stat);
}

/////////////////////////////////////////////
//
//	Name		:SendPage7
//
//	Description :This will restore the page7 data back into the 494.
//
//  Input		:None
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModel494::SendPage7()
{
	CPersist	per;
	int			limitcnt;
	eAutostarStat	stat;
	unsigned int	thisWriteSize;
	int				percent = 101;
	unsigned int	a = m_pageAddrStart;



	// read the page 7 file
	if (per.ReadFile(m_page7File) != CPersist::READCOMPLETE)
		return BAD_FILE;

	// index past the header
	limitcnt = 32;
	while(*((char *)per.m_indexPtr) != 0x03 && limitcnt-- > 0)
		per.IncrementIndex(1);
	per.IncrementIndex(1);		// skip past the 0x03

	// check for error
	if (limitcnt <= 0)
		return BAD_FILE;

	// tell them we're Restoring page7
	if (m_autostar->m_stat)
		m_autostar->m_stat->DoingProcess("Restoring Database");

	// Xfer the each block until all done
	while(per.m_dataIndex < per.m_dataReadCnt)
	{
	// set the write size
		thisWriteSize = m_writeBlockSize;

	// send a block to the autostar. 'a' will be incremented to the next address 
	// 'thisWriteSize' will be modified to how many were actually written
		if((stat = SendMemoryBlock(7, a, (unsigned char *)per.m_indexPtr, thisWriteSize)) != AUTOSTAR_OK)
		{
			per.FreeBuffer();
			return stat;
		}

	// increment the index
		per.IncrementIndex(thisWriteSize);

	// Report the status if changed
		if (m_autostar->m_stat && percent != (int)(((float)per.m_dataIndex / (float)per.m_dataReadCnt) * 100))
		{
			percent = (int)(((float)per.m_dataIndex / (float)per.m_dataReadCnt) * 100);
			m_autostar->m_stat->PercentComplete(percent);
		}
	}

	per.FreeBuffer();
	return AUTOSTAR_OK;
}

/////////////////////////////////////////////
//
//	Name		:SendProgram
//
//	Description :This is the 494 specific version of SendProgram. It
//				 will return an error because program update is not
//				 allowed on 494s
//
//  Input		:None
//
//	Output		:Status
//
////////////////////////////////////////////
eAutostarStat CModel494::SendProgram(bool spawnThread)
{

	m_autostar->m_lastError = NOT_ALLOWED;

// Report the status
	if (m_autostar->m_stat)
		m_autostar->m_stat->SendComplete(m_autostar->m_lastError);

	// close the port
	m_autostar->m_serialPort.ClosePort();

	return m_autostar->m_lastError;
}

