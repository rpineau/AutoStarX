// ModelLX.h: interface for the CModelLX class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODELLX_H__8D1A637D_AEF6_485C_B733_37FBCD4C1DCC__INCLUDED_)
#define AFX_MODELLX_H__8D1A637D_AEF6_485C_B733_37FBCD4C1DCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AutostarModel.h"
#include "AutostarStat.h"	// Added by ClassView

class CModelLX : public CAutostarModel  
{
public:
	eAutostarStat SetPECTable(CBodyData *table, ePECAxis axis);
	eAutostarStat GetPECTable(CBodyData *table, ePECAxis axis);
	bool CheckSiteDelete(bool currentSite);
	eAutostarStat SetSiteInfo(CBodyDataCollection *siteList);
	eAutostarStat SetUserInfo(CBodyData *data);
	eAutostarStat GetSiteInfo(CBodyDataCollection *siteList);
	eAutostarStat GetUserInfo(CUserInfo *pInfo);
	eAutostarStat TestFunction();
	eAutostarStat QueryCatalog(BodyType bodyType, CString &queryResp);
	eAutostarStat SetMaxBaudRate(CSerialPort::eBaud baud, bool justHandbox = false);
	eAutostarStat InitializeSend(bool CloseComPort = false);
	void SendProgramThread();
	eAutostarStat DeleteCatalog(BodyType bodyType);
	enum RecordClass{Error = -1,
					 UserInfoClass = 0,
					 ScopeInformationClass,
					 RA_PEC_Class,
					 DEC_PEC_Class,
					 SmartMountClass,
					 PersonalInformationClass,
					 SiteMapClass,
					 SiteInformationClass,
					 LandmarkClass,
					 TourClass,
					 AsteroidClass,
					 CometClass,
					 SatelliteClass,
					 UserObjectClass = 19,
					 UserObject20Class,
					 UserObject21Class,
					 UserObject22Class,
					 UserObject23Class,
					 UserObject24Class,
					 UserObject25Class,
					 UserObject26Class,
					 UserObject27Class,
					 UserObject28Class,
					 UserObject29Class,
					 UserObject30Class,
					 UserObject31Class,
					 UserObject32Class,
					 UserObject33Class,
					 UserObject34Class,
					 UserObject35Class,
					 UserObject36Class,
					 UserObject37Class,
					 UserObject38Class,
					 UserObject39Class,
					 MaxClass = 40,
					 AllClasses};
	eAutostarStat DeleteOneObject(BodyType bodyType, CString objectName);
	eAutostarStat DeleteOneObject(RecordClass LXclass, CString name);
	eAutostarStat DeleteCatalog(RecordClass LXclass);
	eAutostarStat SendOneObject(CBodyData *body, RecordClass LxBodyType, OUT int &sentBytes, bool site = false);
	eAutostarStat SendOneObject(CBodyData *body, BodyType bodyType, OUT int &sentBytes);
	int RetrieveFreeMemory(bool closeport = false);
	eAutostarStat SetBaud(CSerialPort::eBaud baud);
	RecordClass ConvertBodytype(BodyType type);
	eAutostarStat RetrieveAllUserdata();
	eAutostarStat WriteData(int aClass, int index, int offset, int len, unsigned char *data);
	int AllocateObject(int Class, int len, CString Name);
	eAutostarStat DeleteDynamicObject(RecordClass LXclass, int index);
	eAutostarStat SendAllUserdata();
	eAutostarStat ForceGarbageCollection();
	enum eLXCmnd{QUERY_COUNT, 
				 DELETE_OBJECT, 
				 DELETE_CATALOG,
				 GARBAGE_COLLECT, 
				 ALLOCATE_OBJECT, 
				 WRITE_DATA,
				 ERASE_CATALOG,
				 QUERY_MEMORY,
				 QUERY_CATALOG_STRUCT,
				 MAKE_CATALOG,
				 RETRIEVE_OBJ_BY_NAME,
				 RETRIEVE_OBJ_BY_INDEX,
				 RETRIEVE_INDEX_BY_NAME,
				 REPLACE_OBJECT,
				 RETRIEVE_OBJ_BY_CLASS,
				 SET_BAUD_RATE,
				 QUERY_FREE_MEMORY,
				 RETRIEVE_ALL_OBJ,
				 REPLACE_DATA
	};

	eAutostarStat SendCommand(eLXCmnd cmd,unsigned char *data,unsigned  char *resp, unsigned int &count);
	int RetrieveCount(CModelLX::RecordClass LXclass);
	eAutostarStat DeleteAllRecords(RecordClass LXclass);
	enum eThreadTask{UNASSIGNED, SEND_USER_DATA, RETR_USER_DATA, SEND_PROGRAM};
	CModelLX(CAutostar *autostar);
	virtual ~CModelLX();
	CModelLX(){}
	virtual eAutostarStat SendProgram(bool spawnThread = true, bool eraseBanks = false);
	virtual int GetMaxUserData();
	virtual eAutostarStat SendUserData(CBodyDataCollection *Data, bool spawnThread = true);
	virtual eAutostarStat RetrieveUserData(bool spawnThread = true);
	virtual void SendUserDataThread();
	virtual void RetrieveUserDataThread();
	CBodyDataCollection * m_SendData;

protected:
	friend class CAutostar;
	int		m_userBankFrom;
	int		m_userBankTo;

	eAutostarStat ReplaceData(int aClass, int offset, int len, unsigned char *data);
	eAutostarStat DefineCatalog(CBodyData *data);
	bool m_eraseBanks;
	eAutostarStat EraseUserBanks();
	bool m_from10B;
	eAutostarStat UpgradeFrom10B();
	CString GetCatalogString(RecordClass LXClass);
	eAutostarStat RetrieveUserDataImage(RecordClass type, int size, int index, CPersist &per, int cSize = 0);
	int m_freemem;
};

class CModelRCX	:	public	CModelLX
{
public:
	CModelRCX(CAutostar *autostar);
	virtual ~CModelRCX();

private:
	CModelRCX(){}
};



#endif // !defined(AFX_MODELLX_H__8D1A637D_AEF6_485C_B733_37FBCD4C1DCC__INCLUDED_)
