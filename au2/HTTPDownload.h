// HTTPDownload.h: interface for the CHTTPDownload class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPDOWNLOAD_H__8AA56BB1_D9EB_11D5_8653_0060081FFE37__INCLUDED_)
#define AFX_HTTPDOWNLOAD_H__8AA56BB1_D9EB_11D5_8653_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxinet.h>
#include <afxtempl.h>


/*struct downloadParams {
		CString serverName;
		CString URIName;
		CString saveFileName;
		CString saveFilePath;
};*/

class CHTTPDownloadParams  
{
public:
	CString URIName;
	CString serverName;
	CString saveFilePath;
	CString saveFileName;
	CHTTPDownloadParams();
	virtual ~CHTTPDownloadParams();

};

#include "HTTPDownloadStat.h"





class CHTTPDownload : public CInternetSession  
{
public:
	enum TransferType {TT_CONNECT, TT_DOWNLOAD, TT_SAVE, TT_NONE, TT_FINAL};
	VOID AddTransferLogEntry(CString text, TransferType type = TT_NONE);
	VOID CloseAll();
	CHttpFile* GetHttpFilePtr(CString server, CString uri);
	enum ParseInfo {PI_SERVER, PI_OBJECT};
	static CString ParseURL(CString url, ParseInfo info);
	bool GetCancelFlag();
	VOID SetCancelFlag();
	int DownloadFile(CString server, CString URI, CString fileName = "", CString filePath = "");
	VOID EnableFileCheck(bool state = TRUE);
	CString GetTransferLog();
	static CString GetURINoPath(CString URI);
	VOID Cancel();
	const CHTTPDownload& CHTTPDownload::operator=(const CHTTPDownload &right);
	CHTTPDownload(const CHTTPDownload &cpy);
	CHTTPDownload(CString server, CString URI, CString fileName = "", CString pathName = "");
	VOID SetCallbackPointer(CHTTPDownloadStat* window);
	char* DownloadFile(IN OUT unsigned int &nBytes);
	VOID ClearDownloadParams();
	CString GetLastStatusText();
	DWORD GetLastStatusCode();
	int DownloadFile();
	VOID GetDownloadParams(CArray<CHTTPDownloadParams,CHTTPDownloadParams&> &dpArray);
	CString GetSaveFilePath(int index = 0);
	VOID SetSaveFilePath(CString path);
	CString GetSaveFileName(int index = 0);
	VOID SetSaveFileName(CString name);
	int SetDownloadParams(CArray<CHTTPDownloadParams, CHTTPDownloadParams&> &dpArray);
	int GetParamSize();
	CString GetURIName(int index = 0);
	VOID SetURIName(CString name);
	CString GetServerName(int index = 0);
	VOID SetServerName(CString name);
	CHTTPDownload();
	virtual ~CHTTPDownload();
private:
	bool m_cancel;
	CString* m_fullSavePath;
	CString* m_statString;
	CFile* m_pSaveFile;
	CHttpFile* m_pHttpFile;
	bool m_fileCheck;
	VOID CheckFileName();
	bool SaveRemoteFile(int index = 0);
	VOID InitializeMembers();
	VOID ClearTransferLog();
	CString m_lastTransferLog;
	CHTTPDownloadStat* m_ptrCallback;
	BOOL Connect(int index);
	CHttpConnection* m_pConnection;
	VOID GetHttpFilePtr(int index);
	CString m_lastStatusText;
	DWORD m_lastStatusCode;
	CArray<CHTTPDownloadParams, CHTTPDownloadParams&> m_downloadParams;
};



#endif // !defined(AFX_HTTPDOWNLOAD_H__8AA56BB1_D9EB_11D5_8653_0060081FFE37__INCLUDED_)
