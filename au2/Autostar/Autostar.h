// Autostar.h: interface for the CAutostar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOSTAR_H__FAB5F013_FB17_40BF_83BF_8D5F169786E5__INCLUDED_)
#define AFX_AUTOSTAR_H__FAB5F013_FB17_40BF_83BF_8D5F169786E5__INCLUDED_

enum eAutostarCmnd {MODE, VERSION, SET_DOWNLOAD_MODE, INIT, READ, WRITE_FLASH, ERASE_BANK, PROGRAM_EE, TYPE, TELESCOPECMND, SET_BAUD_RATE};
enum eAutostarMode {UNKNOWN, OPERATIONAL, DOWNLOAD, BUSY};
typedef	enum {TYPE_UNKNOWN = 0, TYPE_AUTOSTAR = 1, TYPE_AUTOSTAR2 = 2, TYPE_RCX = 3} ASType;
enum ePECAxis {AXIS_RA = 2, AXIS_DEC = 3};


#include "BodyDataCollection.h"	// Added by ClassView
#include "SerialPort.h"
#include "UserSettings.h"
#include "autostarStat.h"
#include "Persist.h"	// Added by ClassView
#include "UserInfo.h"


class		CUserInfo;
class		CAutostarModel;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct ReleaseHdType{
		unsigned char flag;
		unsigned char origin[3];
		long int checksum;
		unsigned char version[64];
		unsigned char filler[4024];
	};

 
class CAutostar  
{
	friend class CAutostarModel;
	friend class CModel494_497;
	friend class CModel494;
	friend class CModel497;
	friend class CModelLX;

public:
	eAutostarStat SetPECTable(CBodyData *table, ePECAxis axis);
	eAutostarStat GetPECTable(CBodyData *table, ePECAxis axis);
	bool CheckSiteDelete(bool currentSite);
	int GetMaxSitesAllowed();
	eAutostarStat SetSiteInfo(CBodyDataCollection *siteList);
	eAutostarStat SetUserInfo(CBodyData *data);
	eAutostarStat GetSiteInfo(CBodyDataCollection *siteList);
	eAutostarStat GetUserInfo(CUserInfo *pInfo);
	eAutostarStat TestFunction();
	eAutostarStat CheckDownLoadMode();
	enum eTelescopeCmnd {None, MoveRight, MoveLeft, MoveUp, MoveDown, StopAll, StopHorz, StopVert,
		SetSlewRateGuide, SetSlewRateCenter, SetSlewRateFind, SetSlewRateMax};

	eTelescopeCmnd	m_lastVertCmnd, m_lastHorzCmnd, m_lastSpeedCmnd;
	virtual eAutostarStat SendTelescopeCommand(eTelescopeCmnd cmnd);
	eAutostarStat PowerCycleHandbox();
	eAutostarStat SetMaxBaudRate(CSerialPort::eBaud baud, bool justHandbox = false);
	eAutostarStat ForceGarbageCollection();
	void CloseSerialPort();
	eAutostarStat DeleteCatalog(BodyType bodyType);
	eAutostarStat SendOneObject(CBodyData *body, BodyType bodyType, OUT int &sentBytes);
	eAutostarStat DeleteOneObject(BodyType bodyType, CString objectName);
	bool m_hbxSafeMode;
	bool FindAutostar(bool closeComPort = true);
	eAutostarStat RetrieveType();
	CString m_modelName;
	CString GetModel();
	bool GetVerifyMode();
	void SetVerifyMode(bool mode);
	eAutostarStat ClearPresets(bool closeport=true);
	CString GetVersion();
	eAutostarStat SendProgram(CString fileName, bool spawnThread = true, bool eraseBanks = false);
	int GetAvailableMemory(CBodyDataCollection *data = NULL);
	eAutostarStat RetrieveUserData(bool spawnThread = true);
	eAutostarStat m_lastError;
	CString GetLastError();
	void RestartHandbox();
	void SetStatCallBack(CAutostarStat * stat);
	CAutostarStat * m_stat;
	eAutostarStat SendUserData(CBodyDataCollection *Data = NULL, bool spawnThread = true);
	CBodyDataCollection * GetHandboxData();
	eAutostarStat InitializeConnection(bool CloseComPort = true, bool setDownload = true);
	CAutostar();
	virtual ~CAutostar();
	eAutostarMode			m_mode;
	eAutostarStat SetBaud(CSerialPort::eBaud baud);

protected:
	CSerialPort				m_serialPort;
	

private:
	bool m_verifyMode;
	CPersist m_persist;
	eAutostarStat ConvertDataImage();
	void SetModel(CAutostarModel *model);
	eAutostarStat SendCommand(eAutostarCmnd cmd,unsigned  char * data,unsigned  char * resp, unsigned int &count);
	eAutostarStat SendDownloadMode();
	eAutostarStat RetrieveVersion();
	eAutostarStat InitComPort();

	CUserSettings			m_userSettings;
	CAutostarModel			*m_model;
	CBodyDataCollection		m_handboxData;
	CString					m_version;
};

#endif // !defined(AFX_AUTOSTAR_H__FAB5F013_FB17_40BF_83BF_8D5F169786E5__INCLUDED_)
