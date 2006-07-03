// BodyDataFactory.h: interface for the CBodyDataFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BODYDATAFACTORY_H__00CC5820_0FCA_11D5_A1FC_444553540001__INCLUDED_)
#define AFX_BODYDATAFACTORY_H__00CC5820_0FCA_11D5_A1FC_444553540001__INCLUDED_

#include "bodydata.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBodyDataFactory  
{
public:
	CBodyDataFactory();
	virtual ~CBodyDataFactory();
	virtual CBodyData *Make(BodyType type, CBodyData *copyData = NULL) = 0;

};

#endif // !defined(AFX_BODYDATAFACTORY_H__00CC5820_0FCA_11D5_A1FC_444553540001__INCLUDED_)
