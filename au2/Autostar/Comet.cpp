// Comet.cpp: implementation of the CComet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Comet.h"
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
//	Description :This will initialize the Comet data
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
CComet::CComet()
{

// set the body type
	SetBodyType(Comet);

// initialize the field Descriptions
	InitFieldDesc();

// initialize data
	m_CometData.AbsMag		= 0;
	m_CometData.Active		= (char)0xFF;
	m_CometData.ArgPerhelion= 0;
	m_CometData.Eccentricity= 0;
	m_CometData.Endtag		= 0;
	m_CometData.Epoch_dd	= 1;
	m_CometData.Epoch_mm	= 1;
	m_CometData.Epoch_yy	= 1900;
	m_CometData.Inclination = 0;
	m_CometData.LongAscNode = 0;
	m_CometData.Name[0]		= 0;
	m_CometData.Next.offset = 0;
	m_CometData.Next.page	= 0;
	m_CometData.Perhelion	= 0;
}

/////////////////////////////////////////////
//
//	Name		:InitFieldDesc
//
//	Description :This will initialize the Field Descriptor table
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CComet::InitFieldDesc()
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
	m_fieldDesc[i].DatePtr	= (date *)&m_CometData.Epoch_mm;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpDate]	= i;

	// increment
	i++;

// Perhelion
	m_fieldDesc[i].Label	= "Perhelion";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_CometData.Perhelion;
	m_fieldDesc[i].HiLimit	= 1000.0;
	m_fieldDesc[i].LoLimit	= -1000.0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpPerDist]	= i;

	// increment
	i++;

// Eccentricity
	m_fieldDesc[i].Label	= "Eccentricity";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_CometData.Eccentricity;
	m_fieldDesc[i].HiLimit	= 1.5;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpEcc]	= i;

	// increment
	i++;

// ArgPerhelion
	m_fieldDesc[i].Label	= "Argument of Perihelion";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_CometData.ArgPerhelion;
	m_fieldDesc[i].HiLimit	= 360;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpArg]	= i;

	// increment
	i++;

// LongAscNode
	m_fieldDesc[i].Label	= "Longitudinal Ascending Node";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_CometData.LongAscNode;
	m_fieldDesc[i].HiLimit	= FLT_MAX;
	m_fieldDesc[i].LoLimit	= -FLT_MAX;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpLong]	= i;

	// increment
	i++;

// Inclination
	m_fieldDesc[i].Label	= "Inclination";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%5.4f";
	m_fieldDesc[i].FloatPtr	= &m_CometData.Inclination;
	m_fieldDesc[i].HiLimit	= 360.0;
	m_fieldDesc[i].LoLimit	= -360.0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpInc]	= i;

	// increment
	i++;

// AbsMag
	m_fieldDesc[i].Label	= "Absolute Magnitude";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%3.2f";
	m_fieldDesc[i].FloatPtr	= &m_CometData.AbsMag;
	m_fieldDesc[i].HiLimit	= 20.0;
	m_fieldDesc[i].LoLimit	= -5.0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpMag]	= i;

// Put in the skipped fields
	m_importTable[ImpEclEpo]	= MAX_COMET_FIELDS + 1;
	m_importTable[ImpK]			= MAX_COMET_FIELDS + 1;
	m_importTable[ImpRem]		= MAX_COMET_FIELDS + 1;
	m_importTable[ImpLast]		= -1;

}
/////////////////////////////////////////////
//
//	Name		:Copy constructor
//
//	Description :This will copy the data from the Comet passed to it.
//
//  Input		:Comet to copy
//
//	Output		:None
//
////////////////////////////////////////////
CComet::CComet(CComet &cpy) : CAstroBody(cpy)
{
	SetBodyType(Comet);
	m_CometData = cpy.m_CometData;

// initialize the field Descriptions
	InitFieldDesc();

}

CBodyData &CComet::operator=(CBodyData &cpy)
{
	CAstroBody::operator =(cpy);

	if ( &cpy != this)
	{
		SetBodyType(Comet);
		m_CometData = dynamic_cast<CComet *>(&cpy)->m_CometData;

	// initialize the field Descriptions
		InitFieldDesc();
	}

	return *this;
}/////////////////////////////////////////////
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
CComet::~CComet()
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
int CComet::GetSizeOf()
{
	return sizeof(m_CometData);
}

/////////////////////////////////////////////
//
//	Name		:Create
//
//	Description :Makes a new empty Comet.
//				 Used by the factory.
//
//  Input		:None
//
//	Output		:CBodyData pointer to a new CComet
//
////////////////////////////////////////////
CBodyData * CComet::Create()
{
	CBodyData* obj = new CComet;

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
bool CComet::GetActiveFlag()
{
	bool flag;
	if ((unsigned char)m_CometData.Active == 0xFF)
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
void CComet::SetActiveFlag(bool flag)
{
	if (flag)
		m_CometData.Active = (char)0xFF;
	else
		m_CometData.Active = 0;
}

/////////////////////////////////////////////
//
//	Name		:GetPosition
//
//	Description :returns the pool position for this Comet
//
//  Input		:None
//
//	Output		:poolposition of this Comet
//
////////////////////////////////////////////
const poolposition & CComet::GetPosition()
{
	return m_CometData.Next;
}

/////////////////////////////////////////////
//
//	Name		:SetPosition
//
//	Description :Sets the poolposition for this Comet
//
//  Input		:poolposition
//
//	Output		:None
//
////////////////////////////////////////////
void CComet::SetPosition(const poolposition &position)
{
	m_CometData.Next = position;
}

/////////////////////////////////////////////
//
//	Name		:ReadData
//
//	Description :Converts char ptr to 
//				 Comet data. This is used by import of 
//				 the old Lib files.
//
//  Input		:pointer to memory image
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
void CComet::ReadData(unsigned char *ptr)
{
// get the structure from the pointer
	m_CometData = *((CometType *)ptr);

// set the name as the key
	char temp[20];
	strncpy(temp, m_CometData.Name, 16);
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
int CComet::GetNumFields()
{
	return MAX_COMET_FIELDS;	// tell them how many fields
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
CBodyData * CComet::Copy()
{
	return new CComet(*this);
}

/////////////////////////////////////////////
//
//	Name		:ReadImageData
//
//	Description :This will take a pointer to an Comet data image and convert
//				 it to the CometType structure and store it in the m_CometData
//				 member variable.
//
//  Input		:char *ptr - autostar memory image of Comet data
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CComet::ReadImageData(unsigned char *ptr, int flag)
{
	CometType *Amark = NULL;

// some of it reads ok	
	ReadData(ptr);

// fix the stuff that didn't convert
	m_CometData.Next = ConvertPoolImage(ptr);
	m_CometData.Epoch_yy = ConvertWordImage(&ptr[(int)(&Amark->Epoch_yy)]);
	m_CometData.Perhelion = ConvertFloatImage(&ptr[(int)(&Amark->Perhelion)]);
	m_CometData.Eccentricity = ConvertFloatImage(&ptr[(int)(&Amark->Eccentricity)]);
	m_CometData.ArgPerhelion = ConvertFloatImage(&ptr[(int)(&Amark->ArgPerhelion)]);
	m_CometData.LongAscNode = ConvertFloatImage(&ptr[(int)(&Amark->LongAscNode)]);
	m_CometData.Inclination = ConvertFloatImage(&ptr[(int)(&Amark->Inclination)]);
	m_CometData.AbsMag = ConvertFloatImage(&ptr[(int)(&Amark->AbsMag)]);

	return true;
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
sfieldDesc * CComet::GetFieldDesc(int fIndex)
{
	if (fIndex >= 0 && fIndex < MAX_COMET_FIELDS)
		return &m_fieldDesc[fIndex];
	else
		return NULL;
}

void CComet::PutImageData(unsigned char *image, int flag)
{

	CometType *Amark = NULL;

// set the key as the name
	strncpy(m_CometData.Name, GetKey(true).GetBuffer(20), 16);

// some of this works
	*((CometType *)image) = m_CometData;

	// take care of the Epoch Date Kluge
	if (flag & KDJ_EPOCH_DATE)
		image[(int)(&Amark->Epoch_mm)] = m_CometData.Epoch_mm & 15;	// strip out the high nibble

// fix the rest
	PutPoolImage(m_CometData.Next, &image[(int)(&Amark->Next)]);
	PutWordImage(m_CometData.Epoch_yy, &image[(int)(&Amark->Epoch_yy)]);
	PutFloatImage(m_CometData.Perhelion, &image[(int)(&Amark->Perhelion)]);
	PutFloatImage(m_CometData.Eccentricity, &image[(int)(&Amark->Eccentricity)]);
	PutFloatImage(m_CometData.ArgPerhelion, &image[(int)(&Amark->ArgPerhelion)]);
	PutFloatImage(m_CometData.LongAscNode, &image[(int)(&Amark->LongAscNode)]);
	PutFloatImage(m_CometData.Inclination, &image[(int)(&Amark->Inclination)]);
	PutFloatImage(m_CometData.AbsMag, &image[(int)(&Amark->AbsMag)]);

}

/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to Comets either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CComet::Serialize(CPersist &per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + sizeof(m_CometData) + 10));

	// copy the key and data into the space just provided
		per << m_key;
		*((CometType *)per.m_indexPtr) = m_CometData;

	// move the index up for the data only
		per.IncrementIndex(sizeof(m_CometData));
	}
	else
	{
	// get the key and the data from the buffer
		per >> m_key;
		m_CometData = *((CometType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_CometData));

	// space fill the key to 16
		while(m_key.GetLength() < 16)
			m_key += " ";
	}


}


int * CComet::GetImportTable()
{
	return m_importTable;
}

