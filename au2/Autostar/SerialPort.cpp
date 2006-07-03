// SerialPort.cpp: implementation of the CSerialPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerialPort.h"
#include "UserSettings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerialPort::CSerialPort()
{
	m_portHandle	= INVALID_HANDLE_VALUE;
	m_Baud			= b9600;
	m_defaultBaud	= b9600;
	m_maxBaud		= b56k;

}

CSerialPort::~CSerialPort()
{
	ClosePort();
}

/////////////////////////////////////////////
//
//	Name		:SetConfiguration
//
//	Description :This function will open the port sent to it and set the
//				 baud rate and number of bits.
//
//  Input		:Port, baud, number of bits
//
//	Output		:eSerialStat
//
////////////////////////////////////////////
CSerialPort::eSerialStat CSerialPort::SetConfiguration(CString Port)
{
	DCB				commParams;

// if it's not invalid then close it first	
	if (m_portHandle != INVALID_HANDLE_VALUE)
		ClosePort();


	CString	nPort = "\\\\?\\" + Port;
	// open the port
	m_portHandle = CreateFile(	nPort, 
								GENERIC_READ | GENERIC_WRITE,
								0,
								0,
								OPEN_EXISTING,
								0,
								0);

	// check the handle
	if (m_portHandle == INVALID_HANDLE_VALUE)
	{
		DWORD er = GetLastError();
		return BAD_PORT;
	}
	// set the length
	commParams.DCBlength		= sizeof(DCB);

	// Get the default port setting information
	GetCommState(m_portHandle, &commParams);

	// Set comm port parameters
	switch(m_Baud)
	{
	case b9600 :
		commParams.BaudRate          = CBR_9600; // Autostar requires 9600!  
		break;

	case b14k :
		commParams.BaudRate          = CBR_14400; // Autostar II can go this rate  
		break;

	case b19k :
		commParams.BaudRate          = CBR_19200; // Autostar II can go this rate 
		break;

	case b28k :
		commParams.BaudRate          = 28800; // Autostar II can go this rate  
		break;

	case b38k :
		commParams.BaudRate          = CBR_38400; // Autostar II can go this rate  
		break;

	case b56k :
		commParams.BaudRate          = CBR_57600; // Autostar II can go this rate  
		break;

	case b115k :
		commParams.BaudRate          = CBR_115200; // Autostar II can go this rate  
		break;

	default :
		commParams.BaudRate          = CBR_9600; // Autostar requires 9600!
	}

	commParams.fBinary           = TRUE; // Binary transfers
	commParams.fParity           = FALSE;
	commParams.fOutxCtsFlow      = FALSE;
	commParams.fOutxDsrFlow      = FALSE;
	commParams.fDtrControl       = DTR_CONTROL_DISABLE;
	commParams.fDsrSensitivity   = FALSE;
	commParams.fTXContinueOnXoff = TRUE;
	commParams.fOutX             = FALSE;
	commParams.fInX              = FALSE;
	commParams.fErrorChar        = FALSE;
	commParams.fNull             = FALSE;
	commParams.fRtsControl       = RTS_CONTROL_DISABLE;
	commParams.fAbortOnError     = FALSE;
	commParams.XonLim            = 0;
	commParams.XoffLim           = 0;
	commParams.ByteSize          = (BYTE) 8;
	commParams.Parity            = NOPARITY;
	commParams.StopBits          = ONESTOPBIT;

	if ( !SetCommState ( m_portHandle, &commParams ) )
		return BAD_PORT; 

	return(SetTimeout(DEFAULT_SER_TIMEOUT));
}

/////////////////////////////////////////////
//
//	Name		:SendData
//
//	Description :This will send the data sent in *out for the number of bytes
//				 in outcnt. It will then wait for a response and put that in
//				 *in. The number of bytes expected or greater should be in incnt.
//				 incnt will then return with the number of bytes actually received.
//				 If a failure should happen a message box will be sent to the user.
//				 This should be removed in the final version.
//
//  Input		:*out - pointer to buffer to send
//				 outcnt		- number of characters to send
//				 *in		- buffer to put response into
//				 incnt		- number of bytes expected back
//
//	Output		:eSerialStat - COMM_OK or failure mode
//				 incnt		- number of bytes actually received
//
////////////////////////////////////////////
CSerialPort::eSerialStat CSerialPort::SendData(unsigned char *out, unsigned int outcnt, unsigned char *in, unsigned int &incnt)
{
	unsigned long written, readen;

//	TRACE("Current Baud Rate: %i\n",m_Baud);

	if (m_portHandle != INVALID_HANDLE_VALUE)			// if the port handle is OK
	{
		if(!outcnt || WriteFile(m_portHandle, out, outcnt, &written, NULL))	// if it wrote OK
			if(ReadFile(m_portHandle, in, incnt, &readen, NULL))	// if it read OK
			{														// then it must be OK
				incnt = (unsigned int)readen; // tell the caller how many were read
				if (incnt == 0)
					return NO_RESPONSE;
				else
					return COM_OK;
			}

		// otherwise check the error
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
		// Display the string.
		MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
		// Free the buffer.
		LocalFree( lpMsgBuf );
		// return the error
		return NO_RESPONSE;
	}
	return BAD_PORT;

}

/////////////////////////////////////////////
//
//	Name		:SetTimeout
//
//	Description :Sets the comm timeout to the timeout in milliseconds.
//
//  Input		:timeout
//
//	Output		:Status
//
////////////////////////////////////////////
CSerialPort::eSerialStat CSerialPort::SetTimeout(DWORD timeout)
{
	COMMTIMEOUTS	timeOuts;

	GetCommTimeouts(m_portHandle, &timeOuts);

	// set comm port timeouts
	timeOuts.ReadIntervalTimeout = 100;			// milliseconds between characters
	timeOuts.ReadTotalTimeoutMultiplier = 100;	// milliseconds X characters
	timeOuts.ReadTotalTimeoutConstant = timeout;	// milliseconds + above product for total timeout
	timeOuts.WriteTotalTimeoutConstant = 0;		// no write timeouts
	timeOuts.WriteTotalTimeoutMultiplier = 0;	// no write timeouts

	if (SetCommTimeouts(m_portHandle, &timeOuts))
		return COM_OK;
	else
	{

		// otherwise check the error
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
		// Display the string.
		MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
		// Free the buffer.
		LocalFree( lpMsgBuf );
		// return the error
		return NO_RESPONSE;
	}

}

void CSerialPort::ClosePort()
{
	if (m_portHandle != INVALID_HANDLE_VALUE)
		CloseHandle(m_portHandle);

	m_portHandle = INVALID_HANDLE_VALUE;
}

void CSerialPort::SetBaud(eBaud baud)
{
	DCB				commParams;
	bool			stat;

	if (baud == bMax)	// replace with an actual value
		baud = m_maxBaud;

	if (baud == bDefault)
		baud = m_defaultBaud;

	m_Baud = baud;

	if (m_portHandle != INVALID_HANDLE_VALUE)
	{
		// Get the default port setting information
		GetCommState(m_portHandle, &commParams);

		// Set comm port baud rate
		switch(m_Baud)
		{
		case b9600 :
			commParams.BaudRate          = CBR_9600; // Autostar requires 9600!  
			break;

		case b14k :
			commParams.BaudRate          = CBR_14400; // Autostar II can go this rate  
			break;

		case b19k :
			commParams.BaudRate          = CBR_19200; // Autostar II can go this rate 
			break;

		case b28k :
			commParams.BaudRate          = 28800; // Autostar II can go this rate  
			break;

		case b38k :
			commParams.BaudRate          = CBR_38400; // Autostar II can go this rate  
			break;

		case b56k :
			commParams.BaudRate          = CBR_57600; // Autostar II can go this rate  
			break;

		case b115k :
			commParams.BaudRate          = CBR_115200; // Autostar II can go this rate  
			break;

		}

		stat = SetCommState ( m_portHandle, &commParams );
	}
}

CSerialPort::eBaud CSerialPort::GetBaud()
{
	return m_Baud;
}

CSerialPort::eBaud CSerialPort::GetDefaultBaud()
{
	return m_defaultBaud;
}

CSerialPort::eBaud CSerialPort::GetMaxBaud()
{
	return m_maxBaud;
}

void CSerialPort::SetDefaultBaud(eBaud baud)
{
	if (baud == bDefault || baud == bMax)	// must pass an actual value
		m_defaultBaud = b9600;
	else
		m_defaultBaud = baud;
}


void CSerialPort::SetMaxBaud(eBaud baud)
{
	if (baud == bDefault || baud == bMax)	// must pass an actual value
		m_maxBaud = b56k;
	else
		m_maxBaud = baud;

	TRACE("Max Baud Set to: %i\n",m_maxBaud);

	// keep the registry up to date
	CUserSettings userSettings;
	userSettings.SetBaud(m_maxBaud);
}	
