// HTTPDownloadStat.cpp: implementation of the CHTTPDownloadStat class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HTTPDownload.h"
#include "HTTPDownloadStat.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTTPDownloadStat::CHTTPDownloadStat()
{

}

CHTTPDownloadStat::~CHTTPDownloadStat()
{

}

VOID CHTTPDownloadStat::UpdateStatus(CString statString, int percent)
{

}

VOID CHTTPDownloadStat::EnableButtons(bool state)
{

}


bool CHTTPDownloadStat::SaveFileComplete(CHTTPDownloadParams info)
{
	return FALSE;
}
