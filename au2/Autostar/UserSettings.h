// UserSettings.h: interface for the CUserSettings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERSETTINGS_H__C612754E_2BD0_415E_B1C7_65F3A8E3E0ED__INCLUDED_)
#define AFX_USERSETTINGS_H__C612754E_2BD0_415E_B1C7_65F3A8E3E0ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlbase.h>

#include "SerialPort.h"


class CUserSettings  
{
public:
	CSerialPort::eBaud GetBaud();
	void SetDefaults(CString foundFile);
	void SetBaud(CSerialPort::eBaud baud);
	CString GetLastModel();
	VOID SetLastModel(CString model);
	void SetBackground(CString path);
	CString GetBackground();
	CString GetAsteroidURL();
	CString GetTourURL();
	CString GetCometURL();
	CString GetSatelliteURL();
	CString GetSupportURL();
	CString GetEphemDirectory();
	CString GetInstallDirectory();
	enum optionName {CONNECT, ADVANCED, VERIFY, RECENT, RETRIEVE};
	bool GetOptions(optionName option);
	void SetOptions(optionName option, int state);
	void SetComPort(CString port);
	CString GetPage7Dir();
	CString GetComPort();
	CUserSettings();
	virtual ~CUserSettings();

private:
	CString GetURL(CString key);
	CString GetDirectory(CString key);
	CRegKey m_regKey;
};

#endif // !defined(AFX_USERSETTINGS_H__C612754E_2BD0_415E_B1C7_65F3A8E3E0ED__INCLUDED_)
