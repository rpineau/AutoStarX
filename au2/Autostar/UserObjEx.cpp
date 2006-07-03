// UserObjEx.cpp: implementation of the CUserObjEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserObjEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


const int MAX_CUSTOM_FIELDS = MAX_USEROBJEX_FIELDS - REQD_USEROBJEX_FIELDS;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserObjEx::CUserObjEx(BodyType bodyType, CString catName)
{


// set initial values
	m_UserObjData.Active	= (char)0xFF;
	m_UserObjData.Dec		= 0;
	m_UserObjData.Endtag	= 0;
	m_UserObjData.Name[0]	= 0;
	m_UserObjData.Next.offset = 0;
	m_UserObjData.Next.page	= 0;
	m_UserObjData.RA		= 0;
	m_UserObjData.numFields	= 3;	// the minimum number of fields
	for (int i = 0; i <= 5; i++)
	{
		m_UserObjData.fieldName[i] = "";
		m_UserObjData.data[i] = "";
	}
	m_catalogName			= catName;

	SetBodyType(bodyType);

// initialize the field Descriptions
	InitFieldDesc();

	// make the default contructor private (do not call)
	// create a constructor that accepts a parameter of bodytype
	// create more body type enumerations UserObj20, UserObj21...
	// also create a copy constructor
	// instantiate a sample object with the specified field data
	// instead of bodyDataMaker, user the copy constructor for the UserObj
}

CUserObjEx::~CUserObjEx()
{

}

/////////////////////////////////////////////
//
//	Name		:Copy Constructor
//
//	Description :Used to create a copy of an existing object
//
//  Input		:object to be copied
//
//	Output		:new object
//
////////////////////////////////////////////
CUserObjEx::CUserObjEx(CUserObjEx &copy) : CUserObj(copy)
{

	SetBodyType(copy.GetBodyType());
	m_UserObjData = copy.m_UserObjData;
	m_catalogName = copy.m_catalogName;

// initialize the field Descriptions
	InitFieldDesc();
}




/////////////////////////////////////////////
//
//	Name		:IsCustom
//
//	Description :returns true to indicate features supported
//				 only by Autostar II
//
//  Input		:none
//
//	Output		:true - custom user object
//
////////////////////////////////////////////
bool CUserObjEx::IsCustom()
{
	return true;
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
int CUserObjEx::GetSizeOf()
{
	// start with the size of the structure
	int size = sizeof(CustomCatalogRecType);

	// subtract the size of the char array
	size -= 256;

	// add back in the character data sizes
	// there are a total of 6 field/data pairs that are 15 char each
	for (int i = 0; i < MAX_CUSTOM_FIELDS; i++)
		size += m_fieldDesc[i + REQD_USEROBJEX_FIELDS].fieldSize;

	return size;
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
bool CUserObjEx::ReadImageData(unsigned char *ptr, int flag)
{
	CustomCatalogRecType *lxStruct, *Amark = NULL;

	ptr -= 4; // backup the pointer to read a bogus Next and Active

	lxStruct = (CustomCatalogRecType *) ptr;

	// read the easy stuff from the structure
	m_UserObjData.RA = ConvertMediumImage(&ptr[(int)(&Amark->RA)]);
	m_UserObjData.Dec = ConvertMediumImage(&ptr[(int)(&Amark->Dec)]);
	SetKey(CString(&(*lxStruct->Name)).Left(16));

	// get the zero based index to the data we want
	int pFieldData = (int)(&Amark->Fields);

	// add the data for each field to the member variable
	for (int i = 0; i < m_UserObjData.numFields; i++)
	{
		// get a pointer to the character data at the given index
		const unsigned char *temp2 = (const unsigned char *) (&ptr[pFieldData]);

		// convert that data to a CString and pass it to the field data member variable
		SetFieldData(i + REQD_USEROBJEX_FIELDS, CString(temp2).Left(GetFieldSize(i + REQD_USEROBJEX_FIELDS)));

		// increment the index
		pFieldData += GetFieldSize(i + REQD_USEROBJEX_FIELDS);
	}


	return true;
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
/*void CUserObjEx::ReadData(unsigned char *ptr)
{
	CustomCatalogRecType *lxStruct, *Amark = NULL;

	lxStruct = (CustomCatalogRecType *) ptr;

	// read the easy stuff from the structure
	m_UserObjData.Next.offset = lxStruct->Next.offset;
	m_UserObjData.Next.page   = lxStruct->Next.page;
	m_UserObjData.Active = lxStruct->Active;
	m_UserObjData.RA = ConvertMediumImage(&ptr[(int)(&Amark->RA)]);
	m_UserObjData.Dec = ConvertMediumImage(&ptr[(int)(&Amark->Dec)]);
	SetKey(CString(*lxStruct->Name));

// need to assign the number of fields when reading the catalog definition
	int pFieldData = (int)(&Amark->Fields);

	for (int i = 0; i < m_UserObjData.numFields; i++)
	{
		CString dataStr;
		strcpy(dataStr.GetBuffer(15),(const char *) ptr[pFieldData]);
		SetFieldData(i + REQD_USEROBJEX_FIELDS, dataStr);
		pFieldData += GetFieldSize(i + REQD_USEROBJEX_FIELDS);
	}
// get the structure from the pointer
	m_UserObjData = *((UserObjTypeEx *)ptr);

// set the name as the key
	char temp[20];
	strncpy(temp, m_UserObjData.Name, 16);
	temp[16] = 0;
	m_key = temp;
}
*/
void CUserObjEx::InitFieldDesc()
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
	m_fieldDesc[i].Format	= "%+02d°%02d'%02d\"";
	m_fieldDesc[i].LongPtr	= (long *)&m_UserObjData.Dec;
	m_fieldDesc[i].LoLimit	= -90 * 3600;
	m_fieldDesc[i].HiLimit	= 90 * 3600;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpDec]	= i;

	// increment
	i++;

// Custom Field #1
	m_fieldDesc[i].Label	= m_UserObjData.fieldName[0];
	m_fieldDesc[i].Type		= STRING_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].StringPtr	= &m_UserObjData.data[0];
	m_fieldDesc[i].Modifiable	= true;
	m_fieldDesc[i].fieldSize	= (m_UserObjData.fieldName[0] != "") ? 15 - m_UserObjData.fieldName[0].GetLength() : 0;

	// increment
	i++;

// Custom Field #2
	m_fieldDesc[i].Label	= m_UserObjData.fieldName[1];
	m_fieldDesc[i].Type		= STRING_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].StringPtr	= &m_UserObjData.data[1];
	m_fieldDesc[i].Modifiable	= true;
	m_fieldDesc[i].fieldSize	= (m_UserObjData.fieldName[1] != "") ? 15 - m_UserObjData.fieldName[1].GetLength() : 0;

	// increment
	i++;

// Custom Field #3
	m_fieldDesc[i].Label	= m_UserObjData.fieldName[2];
	m_fieldDesc[i].Type		= STRING_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].StringPtr	= &m_UserObjData.data[2];
	m_fieldDesc[i].Modifiable	= true;
	m_fieldDesc[i].fieldSize	= (m_UserObjData.fieldName[2] != "") ? 15 - m_UserObjData.fieldName[2].GetLength() : 0;

	// increment
	i++;

// Custom Field #4
	m_fieldDesc[i].Label	= m_UserObjData.fieldName[3];
	m_fieldDesc[i].Type		= STRING_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].StringPtr	= &m_UserObjData.data[3];
	m_fieldDesc[i].Modifiable	= true;
	m_fieldDesc[i].fieldSize	= (m_UserObjData.fieldName[3] != "") ? 15 - m_UserObjData.fieldName[3].GetLength() : 0;

	// increment
	i++;

// Custom Field #5
	m_fieldDesc[i].Label	= m_UserObjData.fieldName[4];
	m_fieldDesc[i].Type		= STRING_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].StringPtr	= &m_UserObjData.data[4];
	m_fieldDesc[i].Modifiable	= true;
	m_fieldDesc[i].fieldSize	= (m_UserObjData.fieldName[4] != "") ? 15 - m_UserObjData.fieldName[4].GetLength() : 0;

	// increment
	i++;

// Custom Field #6
	m_fieldDesc[i].Label	= m_UserObjData.fieldName[5];
	m_fieldDesc[i].Type		= STRING_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].StringPtr	= (CString *) &m_UserObjData.data[5];
	m_fieldDesc[i].Modifiable	= true;
	m_fieldDesc[i].fieldSize	= (m_UserObjData.fieldName[5] != "") ? 15 - m_UserObjData.fieldName[5].GetLength() : 0;

	// increment
	i++;

	m_importTable[ImpLast]		= -1;

}


/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to UserObjExs either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CUserObjEx::Serialize(CPersist &per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(GetUserObjDataSize() )); 

	// copy the key and data into the space just provided
		per << m_key;
		per << m_catalogName;

		per << m_UserObjData.Next;
		per << m_UserObjData.Active;
		per << m_UserObjData.RA;
		per << m_UserObjData.Dec;
		per << m_UserObjData.numFields;
		per << m_UserObjData.fieldName[0];
		per << m_UserObjData.fieldName[1];
		per << m_UserObjData.fieldName[2];
		per << m_UserObjData.fieldName[3];
		per << m_UserObjData.fieldName[4];
		per << m_UserObjData.fieldName[5];
		per << m_UserObjData.data[0];
		per << m_UserObjData.data[1];
		per << m_UserObjData.data[2];
		per << m_UserObjData.data[3];
		per << m_UserObjData.data[4];
		per << m_UserObjData.data[5];
		per << m_UserObjData.Endtag;

		
	}
	else
	{
	// get the key and the data from the buffer
		CString buffer = "";
		per >> m_key;

		// Use set method for extra initialization
		per >> buffer;
		SetCatalogName(buffer);

		per >> m_UserObjData.Next;
		per >> m_UserObjData.Active;
		per >> m_UserObjData.RA;
		per >> m_UserObjData.Dec;
		per >> m_UserObjData.numFields;

		// Use set method for extra initialization
		for (int i = 0; i < MAX_CUSTOM_FIELDS; i++)
		{
			per >> buffer;
			SetFieldName(REQD_USEROBJEX_FIELDS + i, buffer);
		}

		per >> m_UserObjData.data[0];
		per >> m_UserObjData.data[1];
		per >> m_UserObjData.data[2];
		per >> m_UserObjData.data[3];
		per >> m_UserObjData.data[4];
		per >> m_UserObjData.data[5];
		per >> m_UserObjData.Endtag;

	// space fill the key to 16
		while(m_key.GetLength() < 16)
			m_key += " ";
	}


}


/////////////////////////////////////////////
//
//	Name		:SetCatalogName
//
//	Description :Sets the catalog name member variable
//
//  Input		:CString
//
//	Output		:none
//
////////////////////////////////////////////
void CUserObjEx::SetCatalogName(CString name)
{
	CPersist::RemoveBadChars(name);
	
	//make sure name is always 16 char. no more, no less
	name.Left(16);

	if (name.GetLength() < 16)
	{
		// if less than 16, space fill to the right
		CString spaces16 = "                ";
		name += spaces16.Left(16 - name.GetLength());
	}

	m_catalogName = name;
}



/////////////////////////////////////////////
//
//	Name		:GetCatalogName
//
//	Description :Gets the catalog name member variable
//
//  Input		:none
//
//	Output		:CString
//
////////////////////////////////////////////
CString CUserObjEx::GetCatalogName()
{
	return m_catalogName;
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
CBodyData * CUserObjEx::Copy()
{
	return new CUserObjEx(*this);
}


/////////////////////////////////////////////
//
//	Name		:CopyTemplate
//
//	Description :Treats the copied object as a template to
//				 return a blank template for a new object
//
//  Input		:None
//
//	Output		:CBodyData pointer
//
////////////////////////////////////////////
CBodyData * CUserObjEx::CopyTemplate()
{
	CBodyData *newBody = Copy();

	// reset the field data
	for (int i = 0; i <= 5; i++)
	{
		((CUserObjEx *) newBody)->m_UserObjData.data[i] = "";
	}
	// reset the name and coordinate data
	((CUserObjEx *) newBody)->m_UserObjData.Dec		= 0;
	((CUserObjEx *) newBody)->m_UserObjData.RA		= 0;
	((CUserObjEx *) newBody)->m_UserObjData.Name[0]	= 0;
	newBody->SetKey("");

	return newBody;
}


CBodyData &CUserObjEx::operator =(CBodyData &copy)
{
	CAstroBody::operator =(copy);
	if ( &copy != this)
	{
		SetBodyType(copy.GetBodyType());
		m_UserObjData = dynamic_cast<CUserObjEx *>(&copy)->m_UserObjData;

	// initialize the field Descriptions
		InitFieldDesc();
	}

	return *this;
}

/////////////////////////////////////////////
//
//	Name		:GetNumFields
//
//	Description :Returns the TOTAL number of fields for this object definition
//
//  Input		:None
//
//	Output		:int
//
////////////////////////////////////////////
int CUserObjEx::GetNumFields()
{
	return m_UserObjData.numFields;
}

/////////////////////////////////////////////
//
//	Name		:SetNumFields
//
//	Description :Sets the TOTAL number of fields for this object definition
//
//  Input		:number of fields (between 3 and 9)
//
//	Output		:none
//
////////////////////////////////////////////
void CUserObjEx::SetNumFields(short num)
{
	if (num >= REQD_USEROBJEX_FIELDS && num <= MAX_USEROBJEX_FIELDS)
		m_UserObjData.numFields = num;
	else
		m_UserObjData.numFields = REQD_USEROBJEX_FIELDS;
}

/////////////////////////////////////////////
//
//	Name		:SetFieldName
//
//	Description :Sets the name for the given field index
//
//  Input		:zero-based field index (must be between 3 and 8)
//
//	Output		:bool success
//
////////////////////////////////////////////
bool CUserObjEx::SetFieldName(int index, CString name)
{
	if (index < REQD_USEROBJEX_FIELDS || index >= MAX_USEROBJEX_FIELDS)
		return false;

	CPersist::RemoveBadChars(name);

	m_UserObjData.fieldName[index - REQD_USEROBJEX_FIELDS] = name;
	m_fieldDesc[index].Label = name;

	// set the number of characters remaining for data
	SetFieldSize(index,(name != "") ? 15 - name.GetLength() : 0);

	return true;
}


/////////////////////////////////////////////
//
//	Name		:SetFieldSize
//
//	Description :Sets the field size for the given field index
//				 should only be called privately when retrieving
//				 field header from handbox or by SetFieldName
//
//  Input		:zero-based field index (must be between 3 and 8)
//				 size in bytes
//
//	Output		:none
//
////////////////////////////////////////////
void CUserObjEx::SetFieldSize(int index, int size)
{
	m_fieldDesc[index].fieldSize = size;
}


/////////////////////////////////////////////
//
//	Name		:GetFieldName
//
//	Description :Gets the name for the given field index
//
//  Input		:zero-based field index (must be between 3 and 8)
//
//	Output		:CString
//
////////////////////////////////////////////
CString CUserObjEx::GetFieldName(int index)
{
	if (index < REQD_USEROBJEX_FIELDS || index >= MAX_USEROBJEX_FIELDS)
		return "";

	return m_UserObjData.fieldName[index - REQD_USEROBJEX_FIELDS];


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
sfieldDesc * CUserObjEx::GetFieldDesc(int fIndex)
{
	if (fIndex >= 0 && fIndex < m_UserObjData.numFields)
		return &m_fieldDesc[fIndex];
	else
		return NULL;

}


/////////////////////////////////////////////
//
//	Name		:GetFieldSize
//
//	Description :Returns the size of the field data at the given index
//
//  Input		:Field Index
//
//	Output		:int size
//
////////////////////////////////////////////
int CUserObjEx::GetFieldSize(int index)
{
	if (index < REQD_USEROBJEX_FIELDS || index >= MAX_USEROBJEX_FIELDS)
		return 0;

	return m_fieldDesc[index].fieldSize;
}

/////////////////////////////////////////////
//
//	Name		:PutImageData
//
//	Description :Formats the User Object prior to sending to the handbox
//
//  Input		:flag - not applicable to this class
//
//	Output		:image - formatted data image
//
////////////////////////////////////////////
void CUserObjEx::PutImageData(unsigned char *image, int flag)
{
	CustomCatalogRecType *Amark = NULL;

// set the key as the name
	strncpy(m_UserObjData.Name, (LPCSTR) GetKey(true), 16);//GetKey(true).GetBuffer(16), 16);

// put the data into the data image
	PutPoolImage(m_UserObjData.Next, &image[(int)(&Amark->Next)]);

	((CustomCatalogRecType *) image)->Active = m_UserObjData.Active;
	lstrcpyn(((CustomCatalogRecType *) image)->Name, m_UserObjData.Name, 16);

	PutMediumImage(m_UserObjData.RA, &image[(int)(&Amark->RA)]);
	PutMediumImage(m_UserObjData.Dec, &image[(int)(&Amark->Dec)]);

	// now the field data
	unsigned char *pField = &image[(int) (&Amark->Fields)];

	for (int i = 0; i < m_UserObjData.numFields - REQD_USEROBJEX_FIELDS; i++)
	{
		strncpy((char *) pField, m_UserObjData.data[i], m_fieldDesc[i + REQD_USEROBJEX_FIELDS].fieldSize);
		
		pField += m_fieldDesc[i + REQD_USEROBJEX_FIELDS].fieldSize;
	}

}

/////////////////////////////////////////////
//
//	Name		:GetUserObjDataSize
//
//	Description :returns the size in bytes of the data that is
//				 saved to and read from the disk
//				 DO NOT confuse with GetSizeOf()
//
//  Input		:none
//
//	Output		:int
//
////////////////////////////////////////////
int CUserObjEx::GetUserObjDataSize()
{
	int size = 0;

	// these are straightforward
	size += m_key.GetLength() + 2;
	size += m_catalogName.GetLength() + 2;

	// this is the size of the structure, but strings are 4-byte pointers
	size += sizeof(m_UserObjData);

	// subtract out the useless pointer lengths
	size -= 12 * sizeof(CString);

	// add in the actual lengths of the field and data strings
	for (int i = 0; i < MAX_CUSTOM_FIELDS; i++)
	{
		size += m_UserObjData.fieldName[i].GetLength() + 2;
		size += m_UserObjData.data[i].GetLength() + 2;
	}

	return size;
}

/////////////////////////////////////////////
//
//	Name		:SetFieldDataFromString
//
//	Description :converts the query response string from
//				 the handbox into field labels and sizes
//
//  Input		:CString data
//
//	Output		:TRUE (success) or FALSE (failure)
//
////////////////////////////////////////////
bool CUserObjEx::SetFieldDataFromString(CString data)
{
	int index;

	// first make sure the string has a valid termination
	if ((index = data.Find('#')) == -1 || data.GetLength() == 0)
		return false;

	// cut off the junk at the end
	data = data.Left(index);

	// get the data up to the first comma, this is the name
	CString catalogName;

	if ((index = data.Find(',')) == -1)
		catalogName = data;
	else
		catalogName = data.Left(index);

	// set the catalog name
	SetCatalogName(catalogName);

	// cut off the catalog name
	data = data.Right(data.GetLength() - index - 1);

	// recursively parse the remaining string and define the fields
	if ((m_UserObjData.numFields = ParseFieldHeader(3, data)) != -1)
	{
		m_UserObjData.numFields += REQD_USEROBJEX_FIELDS; // don't forget the reqd fields
		return true;
	}
	else
		return false;

}

/////////////////////////////////////////////
//
//	Name		:ParseFieldHeader
//
//	Description :converts a comma-separated string of field name:size
//				 and sets each field and size
//
//  Input		:index of first field, CString data
//
//	Output		:TRUE (success) or FALSE (failure)
//
////////////////////////////////////////////
int CUserObjEx::ParseFieldHeader(int index, CString data)
{
	CString extractedField;

	int i = -1,
		count = 0;	// count of found fields

	i = data.Find(',');

	if (i == -1)	// no commas were found, string is one or less fields
	{
		if (!SetFieldFromString(index, data))
			return -1;
		count++;
		return count;
	}

	// commas were found, there are multiple fields
	extractedField = data.Left(i);

	if (extractedField != "")
	{
		if (!SetFieldFromString(index, extractedField))
			return -1;
		count++;
	}

	// cut out the extracted field from the remaining string
	data = data.Right(data.GetLength() - i - 1);

	// trim any extra whitespace chars
	data.TrimLeft();
	data.TrimRight();

	// if there is anything left in the string, call this functino recursively
	if (data != "")
		count += ParseFieldHeader(index + count, data);
	else
		return count;

	return count;


}



/////////////////////////////////////////////
//
//	Name		:SetFieldFromString
//
//	Description :converts a field:size string into
//				 the appropriate member variable data
//
//  Input		:index of field, CString data
//
//	Output		:TRUE (success) or FALSE (failure)
//
////////////////////////////////////////////
bool CUserObjEx::SetFieldFromString(int index, CString data)
{
	CString name;
	int i;

	// trim any whitespace chars
	data.TrimLeft();
	data.TrimRight();

	// look for colon
	if ((i = data.Find(':')) == -1)
		return false;
	else
		name = data.Left(i);

	// set the name
	name.TrimRight();
	SetFieldName(index, name);
	
	// get the field size string
	data = data.Right(data.GetLength() - i - 1);
	SetFieldSize(index, atoi((const char *) data.GetBuffer(5)));

	return true;
}


