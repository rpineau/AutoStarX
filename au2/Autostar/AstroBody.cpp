// AstroBody.cpp: implementation of the CAstroBody class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AstroBody.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAstroBody::CAstroBody()
{

}

CAstroBody::CAstroBody(CAstroBody &cpy) : CBodyData(cpy)
{

}

CAstroBody::~CAstroBody()
{

}

CBodyData &CAstroBody::operator=(CBodyData &cpy)
{
	CBodyData::operator =(cpy);
	return *this;
}


