// UserSettings.cpp: implementation of the CUserSettings class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserSettings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserSettings::CUserSettings()
{
	CFileFind	finder;
	CString		foundFile;
	CFile		sendFile;
	CString		fileVer;
	BOOL		bWorking;

	if (m_regKey.Open(HKEY_USERS,".DEFAULT\\Software\\Meade Inst\\ASU") == ERROR_SUCCESS)
	{
		// Check the install directory
		CString pattern = GetInstallDirectory() + "au2.exe";

		// start the file search
		bWorking = finder.FindFile(pattern);
		
		if (bWorking)
			return;
	}


	// Check the install directory
	CString pattern = "au2.exe";

	// start the file search
	bWorking = finder.FindFile(pattern);
	
	// if not executing from the default directory try the autostar suite directory
	if (!bWorking)
	{
		pattern = CString("updater\\") + pattern;
		bWorking = finder.FindFile(pattern);
	}

	if (bWorking)
	{
		bWorking = finder.FindNextFile();

		// Get a file to test
		foundFile = finder.GetRoot() + "\\";

	}
	else
	{
		return;
	}
	
	SetDefaults(foundFile);

}

void CUserSettings::SetDefaults(CString foundFile)
{
		// set all the default values
		m_regKey.Create(HKEY_USERS, ".DEFAULT\\Software\\Meade Inst\\ASU");
		DWORD	val;
		m_regKey.SetValue("INSTDIR", REG_SZ, (void *)foundFile.GetBuffer(5), foundFile.GetLength() + 1);
		foundFile += "Ephemerides\\";
		m_regKey.SetValue("EPHEM", REG_SZ, (void *)foundFile.GetBuffer(5), foundFile.GetLength() + 1);
		m_regKey.SetValue("COMPORT", REG_SZ, "COM1", 5);
		val = 0x3c;
		m_regKey.SetValue("READWAIT", REG_DWORD, &val, sizeof(val));
		val = 0x7d3;
		m_regKey.SetValue("EPOCH", REG_DWORD, &val, sizeof(val));
		m_regKey.SetValue("OPT_BACKGROUND", REG_SZ, "rosette.jpg", 12);
		m_regKey.SetValue("SATURL", REG_SZ, "http://celestrak.com/NORAD/elements/", 37);
		m_regKey.SetValue("TOURURL", REG_SZ, "http://www.meade.com/support/auto/AutostarTourFiles/", 53);
		m_regKey.SetValue("ASTRURL", REG_SZ, "http://cfa-www.harvard.edu/iau/Ephemerides/", 44);
		m_regKey.SetValue("CMTURL", REG_SZ, "http://cfa-www.harvard.edu/iau/Ephemerides/Comets/", 51);
		m_regKey.SetValue("MEADEURL", REG_SZ, "http://www.meade.com/support/auto/", 35);
		m_regKey.SetValue("SUPPURL", REG_SZ, "http://www.meade.com/support/auto/", 35);
		val = 1;
		m_regKey.SetValue("OPT_ADVANCED", REG_DWORD, &val, sizeof(val));
		val = 0;
		m_regKey.SetValue("OPT_CONNECT", REG_DWORD, &val, sizeof(val));
		m_regKey.SetValue("OPT_RECENT", REG_DWORD, &val, sizeof(val));
		m_regKey.SetValue("OPT_RETRIEVE", REG_DWORD, &val, sizeof(val));
		m_regKey.SetValue("OPT_VERIFY", REG_DWORD, &val, sizeof(val));

}

CUserSettings::~CUserSettings()
{

}

/////////////////////////////////////////////
//
//	Name		:GetComPort
//
//	Description :returns the current com port value from the registry.
//
//  Input		:None
//
//	Output		:Com port
//
////////////////////////////////////////////
CString CUserSettings::GetComPort()
{
	char		cp[10];
	DWORD		cpSize = 10;

	if (m_regKey.QueryValue(cp, "COMPORT", &cpSize) == ERROR_SUCCESS)
		return CString(cp, cpSize);
	else
		return "";
}

/////////////////////////////////////////////
//
//	Name		:GetPage7Dir
//
//	Description :Returns the Ephemeride directory from the registry.
//
//  Input		:None
//
//	Output		:Ephemeride directory
//
////////////////////////////////////////////
CString CUserSettings::GetPage7Dir()
{
	return GetDirectory("EPHEM");
}

/////////////////////////////////////////////
//
//	Name		:SetComPort
//
//	Description :Set the com port name in the registry.
//
//  Input		:Port name
//
//	Output		:None
//
////////////////////////////////////////////
void CUserSettings::SetComPort(CString port)
{
	m_regKey.SetValue(port.GetBuffer(5), "COMPORT");
}

/////////////////////////////////////////////
//
//	Name		:SetOptions
//
//	Description :Set the program operation options registry values.
//
//  Input		:option = CONNECT, ADVANCED, VERIFY, RECENT, RETRIEVE
//				 state = 0 (Off), 1(On)
//
//	Output		:None
//
//////////////////////////////////////////// 
void CUserSettings::SetOptions(optionName option, int state)
{
	switch (option)
	{
	case CONNECT:
		m_regKey.SetValue(state,"OPT_CONNECT");
		break;
	case RETRIEVE: 
		m_regKey.SetValue(state,"OPT_RETRIEVE");
		break;
	case ADVANCED:
		m_regKey.SetValue(state,"OPT_ADVANCED");
		break;
	case VERIFY:
		m_regKey.SetValue(state,"OPT_VERIFY");
		break;
	case RECENT:
		m_regKey.SetValue(state,"OPT_RECENT");
		break;
	}
}

/////////////////////////////////////////////
//
//	Name		:GetOptions
//
//	Description :Get the program operation options registry values.
//
//  Input		:option = CONNECT, ADVANCED, VERIFY, RECENT, RETRIEVE
//
//	Output		:boolean
//
//////////////////////////////////////////// 
bool CUserSettings::GetOptions(CUserSettings::optionName option)
{
	DWORD value;
	switch (option)
	{
	case CONNECT:
		if ((m_regKey.QueryValue(value,"OPT_CONNECT") != ERROR_SUCCESS) || value == 0)
			return FALSE;
		else
			return TRUE;	
		break;
	case RETRIEVE:
		if ((m_regKey.QueryValue(value,"OPT_RETRIEVE") != ERROR_SUCCESS) || value == 0)
			return FALSE;	
		else
			return TRUE;
	case ADVANCED:
		if ((m_regKey.QueryValue(value,"OPT_ADVANCED") != ERROR_SUCCESS) || value == 0)
			return FALSE;
		else
			return TRUE;
		break;
	case VERIFY:
		if ((m_regKey.QueryValue(value,"OPT_VERIFY") != ERROR_SUCCESS) || value == 0)
			return FALSE;
		else
			return TRUE;
		break;
	case RECENT:
		if ((m_regKey.QueryValue(value,"OPT_RECENT") != ERROR_SUCCESS) || value == 0)
			return FALSE;
		else
			return TRUE;
		break;
	default:
		return FALSE;
		break;
	}
}

/////////////////////////////////////////////
//
//	Name		:GetInstallDirectory
//
//	Description :Get the directory where Autostar Update was installed
//
//  Input		:None
//
//	Output		:CString path
//
//////////////////////////////////////////// 
CString CUserSettings::GetInstallDirectory()
{
	return GetDirectory("INSTDIR");
}

/////////////////////////////////////////////
//
//	Name		:GetEphemDirectory
//
//	Description :Get the directory where ephemeride data is stored
//
//  Input		:None
//
//	Output		:CString path
//
//////////////////////////////////////////// 
CString CUserSettings::GetEphemDirectory()
{
	return GetDirectory("EPHEM");
}

/////////////////////////////////////////////
//
//	Name		:GetSupportURL
//
//	Description :Get the URL where the BUILD.ROM file is stored
//
//  Input		:None
//
//	Output		:CString url
//
//////////////////////////////////////////// 
CString CUserSettings::GetSupportURL()
{
	return GetURL("SUPPURL");
}

/////////////////////////////////////////////
//
//	Name		:GetAsteroidURL
//
//	Description :Get the URL where asteroid ephemerides are stored
//
//  Input		:None
//
//	Output		:CString url
//
//////////////////////////////////////////// 
CString CUserSettings::GetAsteroidURL()
{
	return GetURL("ASTRURL");
}

/////////////////////////////////////////////
//
//	Name		:GetTourURL
//
//	Description :Get the URL where tour ephemerides are stored
//
//  Input		:None
//
//	Output		:CString url
//
//////////////////////////////////////////// 
CString CUserSettings::GetTourURL()
{
	return GetURL("TOURURL");
}

/////////////////////////////////////////////
//
//	Name		:GetCometURL
//
//	Description :Get the URL where comet ephemerides are stored
//
//  Input		:None
//
//	Output		:CString url
//
//////////////////////////////////////////// 
CString CUserSettings::GetCometURL()
{
	return GetURL("CMTURL");
}

/////////////////////////////////////////////
//
//	Name		:GetSatelliteURL
//
//	Description :Get the URL where asteroid ephemerides are stored
//
//  Input		:None
//
//	Output		:CString url
//
//////////////////////////////////////////// 
CString CUserSettings::GetSatelliteURL()
{
	return GetURL("SATURL");
}

/////////////////////////////////////////////
//
//	Name		:GetURL
//
//	Description :Return the data from the registry for "key"
//
//  Input		:CString key
//
//	Output		:CString url
//
//////////////////////////////////////////// 
CString CUserSettings::GetURL(CString key)
{
	char		cp[150];
	// initialize with the size of the string buffer
	DWORD		cpSize = 150;	

	// get the directory in the buffer
	m_regKey.QueryValue(cp, key, &cpSize); 
	// null terminate the directory
	cp[cpSize] = 0;

	// return it as a CString
	return CString(cp);
}

/////////////////////////////////////////////
//
//	Name		:GetDirectory
//
//	Description :Return the data from the registry for "key"
//
//  Input		:CString key
//
//	Output		:CString url
//
//////////////////////////////////////////// 
CString CUserSettings::GetDirectory(CString key)
{
	char		cp[100];
	// initialize with the size of the string buffer
	DWORD		cpSize = 100;	

	// get the directory in the buffer
	if (m_regKey.QueryValue(cp, key, &cpSize) == ERROR_SUCCESS)
	{
		// null terminate the directory
		cp[cpSize] = 0;

		// return it as a CString
		return CString(cp);
	}
	else
		return CString("c:\\");
}

/////////////////////////////////////////////
//
//	Name		:SetBackground
//
//	Description :Set the registry entry for the background image file
//
//  Input		:CString path
//
//	Output		:none
//
//////////////////////////////////////////// 
void CUserSettings::SetBackground(CString path)
{
	m_regKey.SetValue(path,"OPT_BACKGROUND");
}


/////////////////////////////////////////////
//
//	Name		:GetBackground
//
//	Description :Get the registry entry for the background image file
//
//  Input		:none
//
//	Output		:CString full path of background file
//
//////////////////////////////////////////// 
CString CUserSettings::GetBackground()
{
	return GetDirectory("OPT_BACKGROUND");
}


/////////////////////////////////////////////
//
//	Name		:SetLastModel
//
//	Description :Set the registry entry for the last handbox model connected
//
//  Input		:CString model
//
//	Output		:none
//
//////////////////////////////////////////// 
void CUserSettings::SetLastModel(CString model)
{
	m_regKey.SetValue(model,"LASTMODEL");
}


/////////////////////////////////////////////
//
//	Name		:GetLastModel
//
//	Description :Get the registry entry for the last handbox model connected
//
//  Input		:none
//
//	Output		:CString model
//
//////////////////////////////////////////// 
CString CUserSettings::GetLastModel()
{
	return GetDirectory("LASTMODEL");
}


void CUserSettings::SetBaud(CSerialPort::eBaud baud)
{
	DWORD value = (DWORD) baud;
	m_regKey.SetValue(value,"OPT_BAUD");

}

CSerialPort::eBaud CUserSettings::GetBaud()
{
	DWORD value;

	if ((m_regKey.QueryValue(value,"OPT_BAUD") != ERROR_SUCCESS))
		return CSerialPort::b56k;	//maximum baud rate
	else
		return (CSerialPort::eBaud) value;
}
