// UserObj.cpp: implementation of the CUserObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserObj.h"

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
//	Description :This will initialize the UserObj data
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
CUserObj::CUserObj()
{

// set the body type
	SetBodyType(UserObj);

// initialize the field Descriptions
	InitFieldDesc();

// set initial values
	m_UserObjData.Active	= (char)0xFF;
	m_UserObjData.Dec		= 0;
	m_UserObjData.Endtag	= 0;
	m_UserObjData.Mag		= 0;
	m_UserObjData.Name[0]	= 0;
	m_UserObjData.Next.offset = 0;
	m_UserObjData.Next.page	= 0;
	m_UserObjData.RA		= 0;
	m_UserObjData.Size		= 0;

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
void CUserObj::InitFieldDesc()
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

// Right Ascention
	m_fieldDesc[i].Label	= "Right Asc.";
	m_fieldDesc[i].Type		= LONG_RA_TYPE;
	m_fieldDesc[i].Format	= "%02d:%02d:%02d";
	m_fieldDesc[i].LongPtr	= (long *)&m_UserObjData.RA;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].HiLimit	= 360 * 3600;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpRA]	= i;

	// increment
	i++;

// Declination
	m_fieldDesc[i].Label	= "Declination";
	m_fieldDesc[i].Type		= LONG_DEC_TYPE;
	m_fieldDesc[i].Format	= "%+03d°%02d'%02d\"";
	m_fieldDesc[i].LongPtr	= (long *)&m_UserObjData.Dec;
	m_fieldDesc[i].LoLimit	= -90 * 3600;
	m_fieldDesc[i].HiLimit	= 90 * 3600;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpDec]	= i;

	// increment
	i++;

// Size
	m_fieldDesc[i].Label	= "Size";
	m_fieldDesc[i].Type		= SHORT_TYPE;
	m_fieldDesc[i].Format	= "%+d";
	m_fieldDesc[i].ShortPtr	= &m_UserObjData.Size;
	m_fieldDesc[i].HiLimit	= 32000;
	m_fieldDesc[i].LoLimit	= -32000;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpSize]	= i;

	// increment
	i++;

// Magnitude
	m_fieldDesc[i].Label	= "Magnitude";
	m_fieldDesc[i].Type		= SHORT_TYPE;
	m_fieldDesc[i].Format	= "%+d";
	m_fieldDesc[i].ShortPtr	= &m_UserObjData.Mag;
	m_fieldDesc[i].HiLimit	= 20;
	m_fieldDesc[i].LoLimit	= -5;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpMag]	= i;

	m_importTable[ImpLast]		= -1;


}
/////////////////////////////////////////////
//
//	Name		:Copy constructor
//
//	Description :This will copy the data from the UserObj passed to it.
//
//  Input		:UserObj to copy
//
//	Output		:None
//
////////////////////////////////////////////
CUserObj::CUserObj(CUserObj &cpy) : CAstroBody(cpy)
{
	SetBodyType(UserObj);
	m_UserObjData = cpy.m_UserObjData;

// initialize the field Descriptions
	InitFieldDesc();

}

CBodyData &CUserObj::operator=(CBodyData &cpy)
{
	CAstroBody::operator =(cpy);
	if ( &cpy != this)
	{
		SetBodyType(UserObj);
		m_UserObjData = dynamic_cast<CUserObj *>(&cpy)->m_UserObjData;

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
CUserObj::~CUserObj()
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
int CUserObj::GetSizeOf()
{
	return sizeof(m_UserObjData);
}

/////////////////////////////////////////////
//
//	Name		:Create
//
//	Description :Makes a new empty UserObj.
//				 Used by the factory.
//
//  Input		:None
//
//	Output		:CBodyData pointer to a new CUserObj
//
////////////////////////////////////////////
CBodyData * CUserObj::Create()
{
	CBodyData* obj = new CUserObj;

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
bool CUserObj::GetActiveFlag()
{
	bool flag;
	if ((unsigned char)m_UserObjData.Active == 0xFF)
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
void CUserObj::SetActiveFlag(bool flag)
{
	if (flag)
		m_UserObjData.Active = (char)0xFF;
	else
		m_UserObjData.Active = 0;
}

/////////////////////////////////////////////
//
//	Name		:GetPosition
//
//	Description :returns the pool position for this UserObj
//
//  Input		:None
//
//	Output		:poolposition of this UserObj
//
////////////////////////////////////////////
const poolposition & CUserObj::GetPosition()
{
	return m_UserObjData.Next;
}

/////////////////////////////////////////////
//
//	Name		:SetPosition
//
//	Description :Sets the poolposition for this UserObj
//
//  Input		:poolposition
//
//	Output		:None
//
////////////////////////////////////////////
void CUserObj::SetPosition(const poolposition &position)
{
	m_UserObjData.Next = position;
}

/////////////////////////////////////////////
//
//	Name		:ReadData
//
//	Description :Converts char ptr to 
//				 UserObj data. This is used by import of 
//				 the old Lib files.
//
//  Input		:pointer to memory image
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
void CUserObj::ReadData(unsigned char *ptr)
{
// get the structure from the pointer
	m_UserObjData = *((UserObjTypePrec *)ptr);

// set the name as the key
	char temp[20];
	strncpy(temp, m_UserObjData.Name, 16);
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
int CUserObj::GetNumFields()
{
	return MAX_USEROBJ_FIELDS;	// tell them how many fields
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
CBodyData * CUserObj::Copy()
{
	return new CUserObj(*this);
}

/////////////////////////////////////////////
//
//	Name		:ReadImageData
//
//	Description :This will take a pointer to an UserObj data image and convert
//				 it to the UserObjType structure and store it in the m_UserObjData
//				 member variable.
//
//  Input		:char *ptr - autostar memory image of User Object data
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CUserObj::ReadImageData(unsigned char *ptr, int flag)
{
	// if flag is set, use the Lo Precision Conversion
	if (flag & KDJ_LO_PREC)
		return ReadImageDataLoPrec(ptr);


	UserObjTypePrec *Amark = NULL;

// some of it reads ok	
	ReadData(ptr);

// fix the stuff that didn't convert
	m_UserObjData.Next = ConvertPoolImage(ptr);
	m_UserObjData.RA = ConvertLongImage(&ptr[(int)(&Amark->RA)]);
	m_UserObjData.Dec = ConvertLongImage(&ptr[(int)(&Amark->Dec)]);
	m_UserObjData.Size = ConvertWordImage(&ptr[(int)(&Amark->Size)]);
	m_UserObjData.Mag = ConvertWordImage(&ptr[(int)(&Amark->Mag)]);

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
sfieldDesc * CUserObj::GetFieldDesc(int fIndex)
{
	if (fIndex >= 0 && fIndex < MAX_USEROBJ_FIELDS)
		return &m_fieldDesc[fIndex];
	else
		return NULL;
}

/////////////////////////////////////////////
//
//	Name		:PutImageData
//
//	Description :Formats the User Object prior to sending to the handbox
//
//  Input		:flag - to accept unique requirements, such as:
//					KDJ_LOPREC - format for lo precision hbx (494/495/497)
//
//	Output		:image - formatted data image
//
////////////////////////////////////////////
void CUserObj::PutImageData(unsigned char *image, int flag)
{
	if (flag & KDJ_LO_PREC)
	{
		PutImageDataLoPrec(image);	// perform the conversion here!!!!
		return;	
	}

	UserObjTypePrec *Amark = NULL;

// set the key as the name
	strncpy(m_UserObjData.Name, GetKey(true).GetBuffer(20), 16);

// some of this works
	*((UserObjTypePrec *)image) = m_UserObjData;

// fix the rest
	PutPoolImage(m_UserObjData.Next, &image[(int)(&Amark->Next)]);
	PutLongImage(m_UserObjData.RA, &image[(int)(&Amark->RA)]);
	PutLongImage(m_UserObjData.Dec, &image[(int)(&Amark->Dec)]);
	PutWordImage(m_UserObjData.Size, &image[(int)(&Amark->Size)]);
	PutWordImage(m_UserObjData.Mag, &image[(int)(&Amark->Mag)]);

}


/////////////////////////////////////////////
//
//	Name		:PutImageData
//
//	Description :Formats the User Object prior to sending to the handbox
//
//  Input		:flag - to accept unique requirements, such as:
//					KDJ_LOPREC - format for lo precision hbx (494/495/497)
//
//	Output		:image - formatted data image
//
////////////////////////////////////////////
void CUserObj::PutImageDataLoPrec(unsigned char *image)
{
	UserObjType *Amark = NULL;

	// create a lo precision copy of the user object
	UserObjType loObject;
	loObject = ConvertToLoPrec();

// set the key as the name
	strncpy(loObject.Name, GetKey(true).GetBuffer(20), 16);

// some of this works
	*((UserObjType *)image) = loObject;

// fix the rest
	PutPoolImage(loObject.Next, &image[(int)(&Amark->Next)]);
	PutWordImage(loObject.RA, &image[(int)(&Amark->RA)]);
	PutWordImage(loObject.Dec, &image[(int)(&Amark->Dec)]);
	PutWordImage(loObject.Size, &image[(int)(&Amark->Size)]);
	PutWordImage(loObject.Mag, &image[(int)(&Amark->Mag)]);

}

/////////////////////////////////////////////
//
//	Name		:Convert From Lo Precision Object
//
//	Description :This function will take a void pointer to data stored
//				 as a lo precision object, and convert it to a high
//				 precision object and return that object
//
//  Input		:pData - void ptr to data
//
//	Output		:UserObjTypePrec structure
//
////////////////////////////////////////////
UserObjTypePrec CUserObj::ConvertFromLoPrec(void *pData)
{
	UserObjType loObject;
	UserObjTypePrec hiObject;

	// cast void pointer to UserObjType structure
	loObject = *((UserObjType *) pData);

	// copy the members that dont require conversion
	hiObject.Active = loObject.Active;
	hiObject.Endtag = loObject.Endtag;
	hiObject.Mag = loObject.Mag;
	strcpy(hiObject.Name, loObject.Name);
	hiObject.Next = loObject.Next;
	hiObject.Size = loObject.Size;

	// convert the members that require it
	hiObject.Dec = (long) loObject.Dec * 60;
	hiObject.RA =  (long) loObject.RA * 60;

	return hiObject;

}


/////////////////////////////////////////////
//
//	Name		:Convert To Lo Precision Object
//
//	Description :This function returns a lo precision copy of
//					the m_UserObjData member 
//
//  Input		:none
//
//	Output		:UserObjType structure
//
////////////////////////////////////////////
UserObjType CUserObj::ConvertToLoPrec()
{
	UserObjType loObject;

	// copy all the straightforward members
	loObject.Active = m_UserObjData.Active;
	loObject.Endtag = m_UserObjData.Endtag;
	loObject.Mag = m_UserObjData.Mag;
	strcpy(loObject.Name, m_UserObjData.Name);
	loObject.Next = m_UserObjData.Next;
	loObject.Size = m_UserObjData.Size;

	// convert the more involved members
	loObject.Dec = (word) (static_cast<float>(m_UserObjData.Dec) / 60.0 + .5);
	loObject.RA =  (word) (static_cast<float>(m_UserObjData.RA) / 60 + .5);

	return loObject;
}

/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to UserObjs either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CUserObj::Serialize(CPersist &per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + sizeof(m_UserObjData) + 10));

	// copy the key and data into the space just provided
		per << m_key;
		*((UserObjTypePrec *)per.m_indexPtr) = m_UserObjData;

	// move the index up for the data only
		per.IncrementIndex(sizeof(m_UserObjData));
	}
	else
	{
	// get the key and the data from the buffer
		per >> m_key;
		if (per.m_version == 1)	// version 1 is lo precision
		{
			m_UserObjData = ConvertFromLoPrec(per.m_indexPtr);
			// increment the index
			per.IncrementIndex(sizeof(UserObjType));
		}
		else
		{
			m_UserObjData = *((UserObjTypePrec *)per.m_indexPtr);
			// increment the index
			per.IncrementIndex(sizeof(m_UserObjData));
		}
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
bool CUserObj::ReadTxtData(CPersist &per)
{
	bool	result;

	// call the base class version
	result = CBodyData::ReadTxtData(per);

	// go to the next line in case the put some comments at the end
	ParseToEOL(per);

	return result;
}


int * CUserObj::GetImportTable()
{
	return m_importTable;
}

/////////////////////////////////////////////
//
//	Name		:ReadImageDataLoPrec
//
//	Description :This will take a pointer to an UserObj data image and convert
//				 it to the UserObjType structure and store it in the m_UserObjData
//				 member variable. - for Autostar 494/495/497
//
//  Input		:char *ptr - autostar memory image of User Object data
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CUserObj::ReadImageDataLoPrec(unsigned char *ptr)
{
	UserObjType *Amark = NULL;

// some of it reads ok	
	ReadData(ptr);

// fix the stuff that didn't convert
	m_UserObjData.Next = ConvertPoolImage(ptr);
	m_UserObjData.RA = ConvertWordImage(&ptr[(int)(&Amark->RA)]) * 60;
	m_UserObjData.Dec = (short)(ConvertWordImage(&ptr[(int)(&Amark->Dec)])) * 60;
	m_UserObjData.Size = ConvertWordImage(&ptr[(int)(&Amark->Size)]);
	m_UserObjData.Mag = ConvertWordImage(&ptr[(int)(&Amark->Mag)]);

	return true;
}


