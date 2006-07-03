// Model494.h: interface for the CModel494 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODEL494_H__0685BEAD_438D_41A9_8000_8422406E5943__INCLUDED_)
#define AFX_MODEL494_H__0685BEAD_438D_41A9_8000_8422406E5943__INCLUDED_

#include "UserSettings.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Model494_497.h"
#include "Autostar.h"	// Added by ClassView
#include "AutostarStat.h"	// Added by ClassView

class CModel494 : public CModel494_497  
{
public:
	virtual eAutostarStat SendProgram(bool spawnThread = true);
	eAutostarStat SendPage7();
	virtual void SendUserDataThread();
	CModel494(CAutostar *autostar);
	virtual ~CModel494();
	virtual eAutostarStat SetBaud(CSerialPort::eBaud baud){ return AUTOSTAR_OK;};


protected:
	friend class CAutostar;
	eAutostarStat RetrievePage7();
	eAutostarStat CheckPage7File();

private:
	CString m_page7File;
	CUserSettings m_userSettings;
	CModel494(){}
};

#endif // !defined(AFX_MODEL494_H__0685BEAD_438D_41A9_8000_8422406E5943__INCLUDED_)
