// Model494_497.h: interface for the CModel494_497 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODEL494_497_H__7B78555E_CC41_45BE_9077_69DBA1FC2082__INCLUDED_)
#define AFX_MODEL494_497_H__7B78555E_CC41_45BE_9077_69DBA1FC2082__INCLUDED_

#define MAX_USER_DATA 65536


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "AutostarModel.h"
#include "Autostar.h"	// Added by ClassView
#include "AutostarStat.h"	// Added by ClassView
#include "UserInfo.h"

class CModel494_497 : public CAutostarModel  
{
	friend class CAutostar;
public:
	bool CheckSiteDelete(bool currentSite);
	eAutostarStat WriteEepromData(int addr, int len, unsigned char *data);
	eAutostarStat ClearPresets(bool ClosePort);
	virtual eAutostarStat SetSiteInfo(CBodyDataCollection *siteList);
	virtual eAutostarStat SetUserInfo(CBodyData *data);
	virtual eAutostarStat GetSiteInfo(CBodyDataCollection *siteList);
	virtual eAutostarStat GetUserInfo(CUserInfo *pInfo);
	eAutostarStat TestFunction();
	eAutostarStat InitializeSend(bool CloseComPort = false);
	void SendProgramThread();
	eAutostarStat DeleteCatalog(BodyType bodyType);
	eAutostarStat SendOneObject(CBodyData *body, BodyType bodyType, OUT int &sentBytes);
	eAutostarStat DeleteOneObject(BodyType bodyType, CString objectName);
	virtual int GetMaxUserData();
	CBodyDataCollection * m_SendData;
	virtual void SendUserDataThread();
	virtual void RetrieveUserDataThread();
	poolposition PoolAdd(poolposition pool,unsigned int val);
	virtual eAutostarStat ConvertBodyDataColl(CBodyDataCollection *Data);
	virtual eAutostarStat SendUserData(CBodyDataCollection *Data, bool spawnThread = true);
	virtual poolposition ConvertPoolImage(unsigned char *image);
	int convertPoolPosition(poolposition pool);
	virtual eAutostarStat RetrieveUserData(bool spawnThread = true);
	virtual ~CModel494_497();
	CModel494_497(CAutostar *autostar);
	CModel494_497(){}

protected:
	unsigned int m_eePromUserSites;
	unsigned int m_eePromSNAddr;
	unsigned int m_eePromLastValidSiteAddr;
	unsigned int m_eePromCurrentSiteAddr;
	unsigned int m_eePromPersonalInfoAddr;
	eAutostarStat SendDataImage();
	poolposition m_nextFree;
	bool m_mustReload;

	virtual eAutostarStat ConvertDataImage();
	virtual eAutostarStat RetrieveUserdataImage();
	word getHeadPos(BodyType body);

	unsigned char m_userdataImage[MAX_USER_DATA];

private:
	int m_currentMaxSites;
	void putPoolPosition(poolposition pool, unsigned char *Img);
};

#endif // !defined(AFX_MODEL494_497_H__7B78555E_CC41_45BE_9077_69DBA1FC2082__INCLUDED_)
