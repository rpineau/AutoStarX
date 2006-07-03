// BodyDataMaker.cpp: implementation of the CBodyDataMaker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BodyDataMaker.h"
#include "asteroid.h"
#include "satellite.h"
#include "comet.h"
#include "userobj.h"
#include "userobjex.h"
#include "landmark.h"
#include "tour.h"
#include "site.h"
#include "pectable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBodyDataMaker::CBodyDataMaker()
{

}

CBodyDataMaker::~CBodyDataMaker()
{

}

CBodyData * CBodyDataMaker::Make(BodyType type, CBodyData *copyData)
{
	switch (type){

	case Asteroid :
		return new CAsteroid;
		break;

	case Satellite :
		return new CSatellite;
		break;

	case Comet :
		return new CComet;
		break;

	case UserObj :
		return new CUserObj;
		break;

	case LandMark :
		return new CLandMark;
		break;

	case Tour :
		return new CTour;
		break;

	case UserObj20:
	case UserObj21:
	case UserObj22:
	case UserObj23:
	case UserObj24:
	case UserObj25:
	case UserObj26:
	case UserObj27:
	case UserObj28:
	case UserObj29:
	case UserObj30:
	case UserObj31:
	case UserObj32:
	case UserObj33:
	case UserObj34:
	case UserObj35:
	case UserObj36:
	case UserObj37:
	case UserObj38:
	case UserObj39:
		if (copyData == NULL)	// if an object to copy was not passed,
			return new CUserObjEx(type);	// return a new, blank object
		else
			return ((CUserObjEx *) copyData)->CopyTemplate();
		break;

	case SiteInfo:
		return new CSite;
		break;

	case UserInfo:
		return new CUserInfo;
		break;

	case PECTable:
		return new CPECTable;
		break;

	default :
		return NULL;
	}

}
