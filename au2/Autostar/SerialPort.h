// SerialPort.h: interface for the CSerialPort class.
//
//////////////////////////////////////////////////////////////////////
//enum eBaud {b125k = 1, b56k, b38k, b28k, b19k, b14k, b9600, bDefault, bMax};

#if !defined(AFX_SERIALPORT_H__55D9A877_87AE_41A5_802D_02802255CCBB__INCLUDED_)
#define AFX_SERIALPORT_H__55D9A877_87AE_41A5_802D_02802255CCBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define		ERASE_SER_TIMEOUT		16000
#define		DEFAULT_SER_TIMEOUT		2000
#define		GARBAGE_COL_TIMEOUT		180000
#define		DELETE_CATALOG_TIMEOUT	600000
#define		MAX_SER_BUF				1000


class CSerialPort  
{
public:
	enum eBaud {b115k = 1, b56k, b38k, b28k, b19k, b14k, b9600, bDefault, bMax};
	//enum eBaud {b56k = 1, b38k, b28k, b19k, b14k, b9600, bDefault, bMax};
	enum eSerialStat {COM_OK, BAD_PORT, NO_RESPONSE};
	void SetMaxBaud(eBaud baud);
	void SetDefaultBaud(eBaud baud);
	eBaud GetMaxBaud();
	eBaud GetDefaultBaud();

	eBaud GetBaud();
	void SetBaud(eBaud baud);
	void ClosePort();
	eSerialStat SetTimeout(DWORD timeout);
	eSerialStat SendData(unsigned char * out,unsigned int outcnt,unsigned  char * in, unsigned int &incnt);
	eSerialStat SetConfiguration(CString Port);
	CSerialPort();
	virtual ~CSerialPort();

private:
	eBaud m_maxBaud;
	eBaud m_defaultBaud;
	HANDLE	m_portHandle;
	eBaud	m_Baud;
};

#endif // !defined(AFX_SERIALPORT_H__55D9A877_87AE_41A5_802D_02802255CCBB__INCLUDED_)
