// Model497.h: interface for the CModel497 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODEL497_H__BBAF4F0E_5307_4063_89E7_47541B19F2BC__INCLUDED_)
#define AFX_MODEL497_H__BBAF4F0E_5307_4063_89E7_47541B19F2BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Model494_497.h"
#include "AutostarStat.h"	// Added by ClassView

class CModel497 : public CModel494_497  
{
public:
	void SendUserDataThread();
	CModel497(CAutostar *autostar);
	virtual ~CModel497();
	virtual eAutostarStat SetBaud(CSerialPort::eBaud baud);

private:
	friend class CAutostar;
	CModel497(){}
};

#endif // !defined(AFX_MODEL497_H__BBAF4F0E_5307_4063_89E7_47541B19F2BC__INCLUDED_)
