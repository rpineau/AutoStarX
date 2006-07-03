// LandMark.cpp: implementation of the CLandMark class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LandMark.h"

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
//	Description :This will initialize the LandMark data
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
CLandMark::CLandMark()
{

// set the body type
	SetBodyType(LandMark);

// initialize the field Descriptions
	InitFieldDesc();

// Initialize data
	m_LandMarkData.Active	= (char)0xFF;
	m_LandMarkData.Az		= 0;
	m_LandMarkData.El		= 0;
	m_LandMarkData.Endtag	= 0;
	m_LandMarkData.Name[0]	= 0;
	m_LandMarkData.Next.offset = 0;
	m_LandMarkData.Next.page= 0;

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
void CLandMark::InitFieldDesc()
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

// Azmuth
	m_fieldDesc[i].Label	= "Azimuth";
	m_fieldDesc[i].Type		= AZ_TYPE;
	m_fieldDesc[i].Format	= "%+02d°%02d'00\"";
	m_fieldDesc[i].ShortPtr	= (short *)&m_LandMarkData.Az;
	m_fieldDesc[i].HiLimit	= 360 * 60;
	m_fieldDesc[i].LoLimit	= -360 * 60;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpAz]	= i;

	// increment
	i++;

// Altitude
	m_fieldDesc[i].Label	= "Altitude";
	m_fieldDesc[i].Type		= DEC_TYPE;
	m_fieldDesc[i].Format	= "%+02d°%02d'00\"";
	m_fieldDesc[i].ShortPtr	= &m_LandMarkData.El;
	m_fieldDesc[i].HiLimit	= 90 * 60;
	m_fieldDesc[i].LoLimit	= -90 * 60;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpAlt]	= i;

	m_importTable[ImpLast]		= -1;



}
/////////////////////////////////////////////
//
//	Name		:Copy constructor
//
//	Description :This will copy the data from the LandMark passed to it.
//
//  Input		:LandMark to copy
//
//	Output		:None
//
////////////////////////////////////////////
CLandMark::CLandMark(CLandMark &cpy) : CBodyData(cpy)
{
	SetBodyType(LandMark);
	m_LandMarkData = cpy.m_LandMarkData;

// initialize the field Descriptions
	InitFieldDesc();

}

CBodyData &CLandMark::operator=(CBodyData &cpy)
{
	CBodyData::operator =(cpy);
	if ( &cpy != this)
	{
		SetBodyType(LandMark);
		m_LandMarkData = dynamic_cast<CLandMark *>(&cpy)->m_LandMarkData;

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
CLandMark::~CLandMark()
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
int CLandMark::GetSizeOf()
{
	return sizeof(m_LandMarkData);
}

/////////////////////////////////////////////
//
//	Name		:Create
//
//	Description :Makes a new empty Landmark.
//				 Used by the factory.
//
//  Input		:None
//
//	Output		:CBodyData pointer to a new CLandMark
//
////////////////////////////////////////////
CBodyData * CLandMark::Create()
{
	CBodyData* obj = new CLandMark;

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
bool CLandMark::GetActiveFlag()
{
	bool flag;
	if ((unsigned char)m_LandMarkData.Active == 0xFF)
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
void CLandMark::SetActiveFlag(bool flag)
{
	if (flag)
		m_LandMarkData.Active = (char)0xFF;
	else
		m_LandMarkData.Active = 0;
}

/////////////////////////////////////////////
//
//	Name		:GetPosition
//
//	Description :returns the pool position for this Landmark
//
//  Input		:None
//
//	Output		:poolposition of this Landmark
//
////////////////////////////////////////////
const poolposition & CLandMark::GetPosition()
{
	return m_LandMarkData.Next;
}

/////////////////////////////////////////////
//
//	Name		:SetPosition
//
//	Description :Sets the poolposition for this LandMark
//
//  Input		:poolposition
//
//	Output		:None
//
////////////////////////////////////////////
void CLandMark::SetPosition(const poolposition &position)
{
	m_LandMarkData.Next = position;
}

/////////////////////////////////////////////
//
//	Name		:ReadData
//
//	Description :Converts char ptr to 
//				 LandMark data. This is used by import of 
//				 the old Lib files.
//
//  Input		:pointer to memory image
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
void CLandMark::ReadData(unsigned char *ptr)
{
// get the structure from the pointer
	m_LandMarkData = *((LandmarkType *)ptr);

// set the name as the key
	char temp[20];
	strncpy(temp, m_LandMarkData.Name, 16);
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
int CLandMark::GetNumFields()
{
	return MAX_LANDMARK_FIELDS;	// tell them how many fields
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
CBodyData * CLandMark::Copy()
{
	return new CLandMark(*this);
}

/////////////////////////////////////////////
//
//	Name		:ReadImageData
//
//	Description :This will take a pointer to a Landmark data image and convert
//				 it to the LandMarkType structure and store it in the m_LandMarkData
//				 member variable.
//
//  Input		:char *ptr - autostar memory image of User Object data
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CLandMark::ReadImageData(unsigned char *ptr, int flag)
{
	LandmarkType *Amark = NULL;

// some of it reads ok	
	ReadData(ptr);

// fix the stuff that didn't convert
	m_LandMarkData.Next = ConvertPoolImage(ptr);
	m_LandMarkData.Az = ConvertWordImage(&ptr[(int)(&Amark->Az)]);
	m_LandMarkData.El = ConvertWordImage(&ptr[(int)(&Amark->El)]);

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
sfieldDesc * CLandMark::GetFieldDesc(int fIndex)
{
	if (fIndex >= 0 && fIndex < MAX_LANDMARK_FIELDS)
		return &m_fieldDesc[fIndex];
	else
		return NULL;
}

void CLandMark::PutImageData(unsigned char *image, int flag)
{

	LandmarkType *Amark = NULL;

// set the key as the name
	strncpy(m_LandMarkData.Name, GetKey(true).GetBuffer(20), 16);

// some of this works
	*((LandmarkType *)image) = m_LandMarkData;

// fix the rest
	PutPoolImage(m_LandMarkData.Next, &image[(int)(&Amark->Next)]);
	PutWordImage(m_LandMarkData.Az, &image[(int)(&Amark->Az)]);
	PutWordImage(m_LandMarkData.El, &image[(int)(&Amark->El)]);
}

/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to LandMarks either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CLandMark::Serialize(CPersist &per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + sizeof(m_LandMarkData) + 10));

	// copy the key and data into the space just provided
		per << m_key;
		*((LandmarkType *)per.m_indexPtr) = m_LandMarkData;

	// move the index up for the data only
		per.IncrementIndex(sizeof(m_LandMarkData));
	}
	else
	{
	// get the key and the data from the buffer
		per >> m_key;
		m_LandMarkData = *((LandmarkType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_LandMarkData));

	// space fill the key to 16
		while(m_key.GetLength() < 16)
			m_key += " ";
	}


}

/////////////////////////////////////////////
//
//	Name		:ReadTxtData
//
//	Description :This is the overrided version of the this function for user objects.
//				 It will call the base class version and then parse to the end of the line
//
//  Input		:persist reference
//
//	Output		:true - good
//
////////////////////////////////////////////
bool CLandMark::ReadTxtData(CPersist &per)
{
	bool	result;

	// call the base class version
	result = CBodyData::ReadTxtData(per);

	// go to the next line in case the put some comments at the end
	ParseToEOL(per);

	return result;
}



int * CLandMark::GetImportTable()
{
	return m_importTable;
}

