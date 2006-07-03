// Asteroid.cpp: implementation of the CAsteroid class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Asteroid.h"
#include <limits.h>
#include <float.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////
//
//	Name		:Default Constructor
//
//	Description :This will initialize the Asteroid data
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
CAsteroid::CAsteroid()
{

// set the body type
	SetBodyType(Asteroid);

// initialize the field Descriptions
	InitFieldDesc();

// initialize the data structure
	m_AsteroidData.Next.offset	= 0;
	m_AsteroidData.Next.page	= 0;
	m_AsteroidData.Active		= (char)0xFF;	// always active
	m_AsteroidData.Name[0]		= 0;
	m_AsteroidData.Epoch_dd		= 1;
	m_AsteroidData.Epoch_mm		= 1;
	m_AsteroidData.Epoch_yy		= 1900;
	m_AsteroidData.Eccentricity = 0;
	m_AsteroidData.SemiMajorAxis = 0;
	m_AsteroidData.Inclination	= 0;
	m_AsteroidData.LongAscNode	= 0;
	m_AsteroidData.ArgPerhelion = 0;
	m_AsteroidData.MeanAnomaly	= 0;
	m_AsteroidData.AbsMag		= 0;
	m_AsteroidData.Slope		= 0;
	m_AsteroidData.Endtag		= 0;
}

/////////////////////////////////////////////
//
//	Name		:InitFieldDesc
//
//	Description :This will Initialize the Field Descriptor table.
//
//  Input		:
//
//	Output		:
//
////////////////////////////////////////////
void CAsteroid::InitFieldDesc()
{

	int i = 0;

// Name
	m_fieldDesc[i].Label	= "Name";
	m_fieldDesc[i].Type		= KEY_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpName]	= i;
	// increment
	i++;

// Epoch Date
	m_fieldDesc[i].Label	= "Epoch Date";
	m_fieldDesc[i].Type		= DATE_TYPE;
	m_fieldDesc[i].Format	= "%02d/%02.1f/%4d";
	m_fieldDesc[i].DatePtr	= (date *)&m_AsteroidData.Epoch_mm;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpDate]	= i;

	// increment
	i++;

// Eccentricity
	m_fieldDesc[i].Label	= "Eccentricity";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_AsteroidData.Eccentricity;
	m_fieldDesc[i].HiLimit	= 1.5;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpEcc]	= i;

	// increment
	i++;

// SemiMajorAxis
	m_fieldDesc[i].Label	= "Semi-Major Axis";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_AsteroidData.SemiMajorAxis;
	m_fieldDesc[i].HiLimit	= FLT_MAX;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpSemi]	= i;

	// increment
	i++;

// Inclination
	m_fieldDesc[i].Label	= "Inclination";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%5.4f";
	m_fieldDesc[i].FloatPtr	= &m_AsteroidData.Inclination;
	m_fieldDesc[i].HiLimit	= 180.0;
	m_fieldDesc[i].LoLimit	= -180.0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpIncl]	= i;

	// increment
	i++;

// LongAscNode
	m_fieldDesc[i].Label	= "Longitudinal Ascending Node";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_AsteroidData.LongAscNode;
	m_fieldDesc[i].HiLimit	= 360;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpLong]	= i;

	// increment
	i++;

// ArgPerhelion
	m_fieldDesc[i].Label	= "Argument of Perihelion";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_AsteroidData.ArgPerhelion;
	m_fieldDesc[i].HiLimit	= 360;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpPeri]	= i;

	// increment
	i++;

// MeanAnomaly
	m_fieldDesc[i].Label	= "Mean Anomaly";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_AsteroidData.MeanAnomaly;
	m_fieldDesc[i].HiLimit	= 360;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpMean]	= i;

	// increment
	i++;

// AbsMag
	m_fieldDesc[i].Label	= "Absolute Magnitude";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%3.2f";
	m_fieldDesc[i].FloatPtr	= &m_AsteroidData.AbsMag;
	m_fieldDesc[i].HiLimit	= 24.0;
	m_fieldDesc[i].LoLimit	= -5.0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpMag]	= i;

	// increment
	i++;

// Slope
	m_fieldDesc[i].Label	= "Slope";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%3.2f";
	m_fieldDesc[i].FloatPtr	= &m_AsteroidData.Slope;
	m_fieldDesc[i].HiLimit	= FLT_MAX;
	m_fieldDesc[i].LoLimit	= -1;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpSlope]	= i;

// put in the skipped fields
	m_importTable[ImpEccEpoch]	= MAX_ASTEROID_FIELDS + 1;
	m_importTable[ImpBlank]		= MAX_ASTEROID_FIELDS + 1;
	m_importTable[ImpLast]		= -1;

}
/////////////////////////////////////////////
//
//	Name		:Copy constructor
//
//	Description :This will copy the data from the Asteroid passed to it.
//
//  Input		:Asteroid to copy
//
//	Output		:None
//
////////////////////////////////////////////
CAsteroid::CAsteroid(CAsteroid &cpy) : CAstroBody(cpy)
{
	SetBodyType(Asteroid);
	m_AsteroidData = cpy.m_AsteroidData;

// initialize the field Descriptions
	InitFieldDesc();

}

CBodyData &CAsteroid::operator=(CBodyData &cpy)
{
	CAstroBody::operator =(cpy);
	if ( &cpy != this)
	{
		SetBodyType(Asteroid);
		m_AsteroidData = dynamic_cast<CAsteroid *>(&cpy)->m_AsteroidData;

	// initialize the field Descriptions
		InitFieldDesc();
	}

	return *this;
}


/////////////////////////////////////////////
//
//	Name		:Default Destructor
//
//	Description :No dynamic data to delete
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
CAsteroid::~CAsteroid()
{

}

/////////////////////////////////////////////
//
//	Name		:GetSizeOf
//
//	Description :This will return the size of the data as it is
//				 stored in the autostar.
//
//  Input		:None
//
//	Output		:size of Data
//
////////////////////////////////////////////
int CAsteroid::GetSizeOf()
{
	return sizeof(m_AsteroidData);
}

/////////////////////////////////////////////
//
//	Name		:Create
//
//	Description :Makes a new empty asteroid.
//				 Used by the factory.
//
//  Input		:None
//
//	Output		:CBodyData pointer to a new CAsteroid
//
////////////////////////////////////////////
CBodyData * CAsteroid::Create()
{
	CBodyData* obj = new CAsteroid;

	return obj;
}

/////////////////////////////////////////////
//
//	Name		:GetActiveFlag
//
//	Description :Returns the state of the active flag;
//
//  Input		:None
//
//	Output		:active flag
//
////////////////////////////////////////////
bool CAsteroid::GetActiveFlag()
{
	bool flag;
	if ((unsigned char)m_AsteroidData.Active == 0xFF)
		flag = true;
	else
		flag = false;

	return flag;
}

/////////////////////////////////////////////
//
//	Name		:SetActiveFlag
//
//	Description :Sets the state of the active flag
//
//  Input		:flag
//
//	Output		:None
//
////////////////////////////////////////////
void CAsteroid::SetActiveFlag(bool flag)
{
	if (flag)
		m_AsteroidData.Active = (char)0xFF;
	else
		m_AsteroidData.Active = 0;
}

/////////////////////////////////////////////
//
//	Name		:GetPosition
//
//	Description :returns the pool position for this asteroid
//
//  Input		:None
//
//	Output		:poolposition of this asteroid
//
////////////////////////////////////////////
const poolposition & CAsteroid::GetPosition()
{
	return m_AsteroidData.Next;
}

/////////////////////////////////////////////
//
//	Name		:SetPosition
//
//	Description :Sets the poolposition for this Asteroid
//
//  Input		:poolposition
//
//	Output		:None
//
////////////////////////////////////////////
void CAsteroid::SetPosition(const poolposition &position)
{
	m_AsteroidData.Next = position;
}

/////////////////////////////////////////////
//
//	Name		:ReadData
//
//	Description :Converts char ptr to Asteroid data and converts the name
//				 to the key. This is used by import of the old Lib files.
//
//  Input		:pointer to memory image
//
//	Output		:None
//
////////////////////////////////////////////
void CAsteroid::ReadData(unsigned char *ptr)
{
// get the structure from the pointer
	m_AsteroidData = *((AsteroidType *)ptr);

// set the name as the key
	char temp[20];
	strncpy(temp, m_AsteroidData.Name, 16);
	temp[16] = 0;
	m_key = temp;

}

/////////////////////////////////////////////
//
//	Name		:GetNumFields
//
//	Description :Returns the total number of data fields.
//
//  Input		:None
//
//	Output		:number of data fields
//
////////////////////////////////////////////
int CAsteroid::GetNumFields()
{
	return MAX_ASTEROID_FIELDS;	// tell them how many fields
}


/////////////////////////////////////////////
//
//	Name		:Copy
//
//	Description :Creates a new copy of this object and returns
//				 as a CBodyData pointer.
//
//  Input		:None
//
//	Output		:CBodyData pointer
//
////////////////////////////////////////////
CBodyData * CAsteroid::Copy()
{
	return new CAsteroid(*this);
}

/////////////////////////////////////////////
//
//	Name		:ReadImageData
//
//	Description :This will take a pointer to an asteroid data image and convert
//				 it to the AsteroidType structure and store it in the m_AsteroidData
//				 member variable.
//
//  Input		:char *ptr - autostar memory image of asteroid data
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CAsteroid::ReadImageData(unsigned char *ptr, int flag)
{
	AsteroidType *Amark = NULL;

// some of it reads ok	
	ReadData(ptr);

// fix the stuff that didn't convert
	m_AsteroidData.Next = ConvertPoolImage(&ptr[(int)(&Amark->Next)]);
	m_AsteroidData.Epoch_yy = ConvertWordImage(&ptr[(int)(&Amark->Epoch_yy)]);
	m_AsteroidData.Eccentricity = ConvertFloatImage(&ptr[(int)(&Amark->Eccentricity)]);
	m_AsteroidData.SemiMajorAxis = ConvertFloatImage(&ptr[(int)(&Amark->SemiMajorAxis)]);
	m_AsteroidData.Inclination = ConvertFloatImage(&ptr[(int)(&Amark->Inclination)]);
	m_AsteroidData.LongAscNode = ConvertFloatImage(&ptr[(int)(&Amark->LongAscNode)]);
	m_AsteroidData.ArgPerhelion = ConvertFloatImage(&ptr[(int)(&Amark->ArgPerhelion)]);
	m_AsteroidData.MeanAnomaly = ConvertFloatImage(&ptr[(int)(&Amark->MeanAnomaly)]);
	m_AsteroidData.AbsMag = ConvertFloatImage(&ptr[(int)(&Amark->AbsMag)]);
	m_AsteroidData.Slope = ConvertFloatImage(&ptr[(int)(&Amark->Slope)]);

	return true;
}

/////////////////////////////////////////////
//
//	Name		:PutImageData
//
//	Description :Puts the body data into the image pointer
//				 formated for the Autostar.
//
//  Input		:Image pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CAsteroid::PutImageData(unsigned char *image, int flag)
{
	AsteroidType *Amark = NULL;

// set the key as the name
	strncpy(m_AsteroidData.Name, GetKey(true).GetBuffer(20), 16);

// some of this works
	*((AsteroidType *)image) = m_AsteroidData;

// take care of the Epoch Date Kluge
	if (flag & KDJ_EPOCH_DATE)
		image[(int)(&Amark->Epoch_mm)] = m_AsteroidData.Epoch_mm & 15;	// strip out the high nibble

// fix the rest
	PutPoolImage(m_AsteroidData.Next, &image[(int)(&Amark->Next)]);
	PutWordImage(m_AsteroidData.Epoch_yy, &image[(int)(&Amark->Epoch_yy)]);
	PutFloatImage(m_AsteroidData.Eccentricity, &image[(int)(&Amark->Eccentricity)]);
	PutFloatImage(m_AsteroidData.SemiMajorAxis, &image[(int)(&Amark->SemiMajorAxis)]);
	PutFloatImage(m_AsteroidData.Inclination, &image[(int)(&Amark->Inclination)]);
	PutFloatImage(m_AsteroidData.LongAscNode, &image[(int)(&Amark->LongAscNode)]);
	PutFloatImage(m_AsteroidData.ArgPerhelion, &image[(int)(&Amark->ArgPerhelion)]);
	PutFloatImage(m_AsteroidData.MeanAnomaly, &image[(int)(&Amark->MeanAnomaly)]);
	PutFloatImage(m_AsteroidData.AbsMag, &image[(int)(&Amark->AbsMag)]);
	PutFloatImage(m_AsteroidData.Slope, &image[(int)(&Amark->Slope)]);


}
/////////////////////////////////////////////
//
//	Name		:GetFieldDesc
//
//	Description :Returns a pointer to the field descriptor.
//
//  Input		:Field Index
//
//	Output		:Field Descriptor pointer
//
////////////////////////////////////////////
sfieldDesc * CAsteroid::GetFieldDesc(int fIndex)
{
	if (fIndex >= 0 && fIndex < MAX_ASTEROID_FIELDS)
		return &m_fieldDesc[fIndex];
	else
		return NULL;
}


/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to asteroids either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CAsteroid::Serialize(CPersist &per)
{

	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + sizeof(m_AsteroidData) + 10));

	// copy the key and data into the space just provided
		per << m_key;
		*((AsteroidType *)per.m_indexPtr) = m_AsteroidData;

	// move the index up
		per.IncrementIndex(sizeof(m_AsteroidData));
	}
	else
	{
	// get the key and the data from the buffer
		per >> m_key;
		m_AsteroidData = *((AsteroidType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_AsteroidData));

	// space fill the key to 16
		while(m_key.GetLength() < 16)
			m_key += " ";
	}

}


int * CAsteroid::GetImportTable()
{
	return m_importTable;
}

