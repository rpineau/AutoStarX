// HTTPDownloadStat.h: interface for the CHTTPDownloadStat class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPDOWNLOADSTAT_H__EC6AE9B3_DD3F_11D5_8655_0060081FFE37__INCLUDED_)
#define AFX_HTTPDOWNLOADSTAT_H__EC6AE9B3_DD3F_11D5_8655_0060081FFE37__INCLUDED_

#include "HTTPDownload.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CHTTPDownloadStat  
{
public:
	virtual bool SaveFileComplete(CHTTPDownloadParams info);
	virtual VOID EnableButtons(bool state = TRUE);
	virtual VOID UpdateStatus(CString statString, int percent = 0);
	CHTTPDownloadStat();
	virtual ~CHTTPDownloadStat();

};

#endif // !defined(AFX_HTTPDOWNLOADSTAT_H__EC6AE9B3_DD3F_11D5_8655_0060081FFE37__INCLUDED_)
