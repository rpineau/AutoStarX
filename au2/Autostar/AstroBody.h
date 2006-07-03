// AstroBody.h: interface for the CAstroBody class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASTROBODY_H__13521840_0FA7_11D5_A1FC_00E098887D2E__INCLUDED_)
#define AFX_ASTROBODY_H__13521840_0FA7_11D5_A1FC_00E098887D2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BodyData.h"

class CAstroBody : public CBodyData  
{
public:
	CAstroBody();
	CAstroBody(CAstroBody &cpy);
	virtual ~CAstroBody();
	virtual CBodyData &operator=(CBodyData &cpy);
};

#endif // !defined(AFX_ASTROBODY_H__13521840_0FA7_11D5_A1FC_00E098887D2E__INCLUDED_)
