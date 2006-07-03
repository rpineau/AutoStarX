// AutostarStat.h: interface for the CAutostarStat class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOSTARSTAT_H__89CB5072_9B68_4210_8B6D_154ACA0735CF__INCLUDED_)
#define AFX_AUTOSTARSTAT_H__89CB5072_9B68_4210_8B6D_154ACA0735CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum eAutostarStat {AUTOSTAR_OK, AUTOSTAR_DOWNLOADING, AUTOSTAR_UPLOADING, AUTOSTAR_BUSY, 
					BAD_COMM_PORT, NO_AUTOSTAR_RESPONSE, UNKNOWN_AUTOSTAR, OUT_OF_MEMORY, 
					ERASE_ERROR, WRITE_ERROR, READ_ERROR, UNKNOWN_ERROR, WRONG_MODE, BAD_FILE, 
					BAD_CHECKSUM, NO_PAGE7_FILE, NOT_ALLOWED, VERIFY_FAILED, COMMAND_FAILED,
					USEROBJEX_RETRIEVE_ERROR, OLD_FIRMWARE};

class CAutostarStat  
{
public:
	virtual void DoingProcess(CString stat);
	virtual void SendComplete(eAutostarStat stat, bool noPopup = false) = 0;
	virtual void RetrieveComplete(eAutostarStat stat, bool noPopup = false) = 0;
	virtual void PercentComplete(int val) = 0;
	CAutostarStat();
	virtual ~CAutostarStat();

};

#endif // !defined(AFX_AUTOSTARSTAT_H__89CB5072_9B68_4210_8B6D_154ACA0735CF__INCLUDED_)
