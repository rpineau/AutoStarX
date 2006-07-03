// BodyDataMaker.h: interface for the CBodyDataMaker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BODYDATAMAKER_H__00CC5821_0FCA_11D5_A1FC_444553540001__INCLUDED_)
#define AFX_BODYDATAMAKER_H__00CC5821_0FCA_11D5_A1FC_444553540001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BodyDataFactory.h"
#include "Asteroid.h"
#include "UserObjEx.h"

class CBodyDataMaker : public CBodyDataFactory  
{
public:
	virtual CBodyData * Make(BodyType type, CBodyData *copyData = NULL);
	CBodyDataMaker();
	virtual ~CBodyDataMaker();


};

#endif // !defined(AFX_BODYDATAMAKER_H__00CC5821_0FCA_11D5_A1FC_444553540001__INCLUDED_)
