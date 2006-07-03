// Model497.cpp: implementation of the CModel497 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Model497.h"
#include "SerialPort.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModel497::CModel497(CAutostar *autostar) : CModel494_497(autostar)
{
	m_userPageEnd	= 8;			// One beyond the last user page
    m_writeBlockSize = 64;			// Maximum write block size
	m_pages = m_userPageEnd - m_userPageStart;
	m_holeSize = m_eePromEnd - m_eePromStart;
	m_pageSize = (m_pageAddrEnd - m_pageAddrStart) - m_holeSize;
	m_maxUserData	= m_pages * m_pageSize;		// maximum user data space with hole cut out
	m_totalPages	= 32;
	m_eePromLastValidSiteAddr = 0xb742;
	m_eePromCurrentSiteAddr = 0xb741;
	m_eePromPersonalInfoAddr = 0xb62a;
	m_eePromSNAddr = 0xb6ba;
	m_eePromUserSites = 0xb6ca;
	m_maxUserSites = 5;
}

CModel497::~CModel497()
{

}

/////////////////////////////////////////////
//
//	Name		:SendUserDataThread
//
//	Description :This is the 497 specific version of this functions.
//				 All this needs to do is call the base class version
//				 and then call the status class to report complete.
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CModel497::SendUserDataThread()
{
// call the base class version
	CModel494_497::SendUserDataThread();

// restart the handbox
	if (m_autostar->m_lastError == AUTOSTAR_OK)
	{
		m_autostar->m_stat->DoingProcess("Restarting Handbox...Please Wait");
		m_autostar->RestartHandbox();
	}

// close the port
	m_autostar->m_serialPort.ClosePort();

// Report the status
	if (m_autostar->m_stat)
		m_autostar->m_stat->SendComplete(m_autostar->m_lastError);

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
eAutostarStat CModel497::SetBaud(CSerialPort::eBaud baud)
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
		case CSerialPort::b56k :
			strcpy((char *)cmd, "F");
			break;

		default :
			return AUTOSTAR_OK;	// unknown baud
		}

	// response count
		cnt = 1;

	// send the command
		stat = m_autostar->SendCommand(SET_BAUD_RATE, cmd, resp, cnt);

	// check the command status and the response 
		if (stat == AUTOSTAR_OK && (resp[0] == '1' || resp[0] == 'Y'))
		// set the port baud rate
		m_autostar->m_serialPort.SetBaud(CSerialPort::b56k);
		else
			stat = COMMAND_FAILED;

		return stat;

	}

// already set to this baud rate
	return AUTOSTAR_OK;
}

