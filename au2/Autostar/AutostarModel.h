// AutostarModel.h: interface for the CAutostarModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOSTARMODEL_H__23A6DCFF_EC77_4CAB_BC89_FE699E2DF582__INCLUDED_)
#define AFX_AUTOSTARMODEL_H__23A6DCFF_EC77_4CAB_BC89_FE699E2DF582__INCLUDED_

#include "Autostar.h"	// Added by ClassView
#include "BodyDataFactory.h"
#include "AutostarStat.h"	// Added by ClassView
#include "UserInfo.h"
class CAutostar;		// forward reference for CAutostar

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAutostarModel  
{
public:
	virtual eAutostarStat SetPECTable(CBodyData *table, ePECAxis axis);
	virtual eAutostarStat GetPECTable(CBodyData *table, ePECAxis axis);
	virtual bool CheckSiteDelete(bool currentSite) = 0;
	virtual eAutostarStat ClearPresets(bool closeport);
	virtual eAutostarStat SetSiteInfo(CBodyDataCollection *siteList) = 0;
	virtual eAutostarStat SetUserInfo(CBodyData *data) = 0;
	virtual eAutostarStat GetSiteInfo(CBodyDataCollection *siteList) = 0;
	virtual eAutostarStat GetUserInfo(CUserInfo *pInfo) = 0;
	virtual eAutostarStat TestFunction();
	virtual eAutostarStat SetMaxBaudRate(CSerialPort::eBaud baud, bool justHandbox = false);
	virtual eAutostarStat ForceGarbageCollection();
	virtual eAutostarStat InitializeSend(bool CloseComPort) = 0;
	virtual eAutostarStat DeleteCatalog(BodyType bodyType) = 0;
	virtual eAutostarStat SendOneObject(CBodyData *body, BodyType bodyType, OUT int &sentBytes) = 0;
	virtual eAutostarStat DeleteOneObject(BodyType bodyType, CString objectName) = 0;
	enum eThreadTask{UNASSIGNED, SEND_USER_DATA, RETR_USER_DATA, SEND_PROGRAM};
	virtual	void SendProgramThread();
	virtual eAutostarStat SendProgram(bool spawnThread = true, bool eraseBanks = false);
	virtual eAutostarStat RetrieveMemoryBlock(unsigned int page, unsigned int &addr,unsigned char *data, unsigned int &count, bool jumpOverHole = true);
	virtual eAutostarStat SendMemoryBlock(unsigned int page, unsigned int &addr,unsigned char *data, unsigned int &count);
	virtual eAutostarStat SendProgramBlock();
	virtual void SendUserDataThread() = 0;
	virtual void RetrieveUserDataThread() = 0;
	virtual int GetMaxUserData() = 0;
	virtual eAutostarStat SendUserData(CBodyDataCollection *Data, bool spawnThread = true) = 0;
	virtual eAutostarStat RetrieveUserData(bool spawnThread = true) = 0;
	CAutostarModel(CAutostar *autostar);
	virtual ~CAutostarModel();
	CAutostarModel(){}
	CAutostar * m_autostar;
	eThreadTask m_threadTask;
	virtual eAutostarStat SetBaud(CSerialPort::eBaud baud) = 0;

protected:
	int m_maxUserSites;
	friend class CAutostar;
	CBodyDataFactory *m_factory;
	unsigned int m_totalPages;
	unsigned int m_userPageEnd;
	unsigned int m_userPageStart;
	unsigned int m_pageAddrEnd;
	unsigned int m_pageAddrStart;
	unsigned int m_pageSize;
	unsigned int m_holeSize;
	unsigned int m_pages;
	unsigned int m_writeBlockSize;
	unsigned int m_readBlockSize;
	unsigned int m_maxUserData;
	unsigned int m_eePromEnd;
	unsigned int m_eePromStart;
	bool	firstfail;	// for testing only

private:
};

#endif // !defined(AFX_AUTOSTARMODEL_H__23A6DCFF_EC77_4CAB_BC89_FE699E2DF582__INCLUDED_)
