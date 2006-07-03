// Satellite.cpp: implementation of the CSatellite class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Satellite.h"
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
//	Description :This will initialize the Satellite data
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
CSatellite::CSatellite()
{

// set the body type
	SetBodyType(Satellite);

// initialize the field Descriptions
	InitFieldDesc();

// Initialize data
	m_SatelliteData.Active		= (char)0xFF;
	m_SatelliteData.ArgPerigee	= 0;
	m_SatelliteData.Eccentricity= 0;
	m_SatelliteData.Endtag		= 0;
	m_SatelliteData.EpochDay	= 0;
	m_SatelliteData.EpochYear	= 1900;
	m_SatelliteData.Inclination = 0;
	m_SatelliteData.MeanAnomaly = 0;
	m_SatelliteData.MeanMotion	= 0;
	m_SatelliteData.Name[0]		= 0;
	m_SatelliteData.Next.offset	= 0;
	m_SatelliteData.Next.page	= 0;
	m_SatelliteData.RAAN		= 0;
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
void CSatellite::InitFieldDesc()
{

	int i = 0;

// initialize the import table to all unused
	for (int j=0; j<ImpLast; j++)
		m_importTable[j] = MAX_SATELLITE_FIELDS + 1;
	m_importTable[ImpLast] = -1;	// init the end of the table


// Name
	m_fieldDesc[i].Label	= "Name";
	m_fieldDesc[i].Type		= KEY_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpName]	= i;

	// increment
	i++;

// Catalog Number
	m_fieldDesc[i].Label	= "Catalog";
	m_fieldDesc[i].Type		= STRING_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].StringPtr = &m_CatNum;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpCatNum]	= i;


	// increment
	i++;

// Internation ID Number
	m_fieldDesc[i].Label	= "Int ID";
	m_fieldDesc[i].Type		= STRING_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].StringPtr = &m_IntId;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpIntID]	= i;


	// increment
	i++;

// Epoch Date
	m_fieldDesc[i].Label	= "Epoch Date";
	m_fieldDesc[i].Type		= JDATE_TYPE;
	m_fieldDesc[i].Format	= "%02d/%08.4f";
	m_fieldDesc[i].JDatePtr	= (jdate *)&m_SatelliteData.EpochYear;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpDate]	= i;

	// increment
	i++;

// Inclination
	m_fieldDesc[i].Label	= "Inclination";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%5.4f";
	m_fieldDesc[i].FloatPtr	= &m_SatelliteData.Inclination;
	m_fieldDesc[i].HiLimit	= 360.0;
	m_fieldDesc[i].LoLimit	= -360.0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpIncl]	= i;

	// increment
	i++;

// RA of the Ascending Node (RAAN)
	m_fieldDesc[i].Label	= "RAAN";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_SatelliteData.RAAN;
	m_fieldDesc[i].HiLimit	= FLT_MAX;
	m_fieldDesc[i].LoLimit	= -FLT_MAX;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpRAAN]	= i;


	// increment
	i++;

// Eccentricity
	m_fieldDesc[i].Label	= "Eccentricity";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_SatelliteData.Eccentricity;
	m_fieldDesc[i].HiLimit	= 1.5;
	m_fieldDesc[i].LoLimit	= -1.5;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpEcc]	= i;


	// increment
	i++;

// Argument of Peregee
	m_fieldDesc[i].Label	= "Arg of Per";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_SatelliteData.ArgPerigee;
	m_fieldDesc[i].HiLimit	= 1000.0;
	m_fieldDesc[i].LoLimit	= -1000.0;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpAoP]	= i;


	// increment
	i++;

// MeanAnomaly
	m_fieldDesc[i].Label	= "Mean Anomaly";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_SatelliteData.MeanAnomaly;
	m_fieldDesc[i].HiLimit	= FLT_MAX;
	m_fieldDesc[i].LoLimit	= -FLT_MAX;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpMeanAnom]	= i;


	// increment
	i++;

// Mean Motion
	m_fieldDesc[i].Label	= "Mean Motion";
	m_fieldDesc[i].Type		= FLOAT_TYPE;
	m_fieldDesc[i].Format	= "%f";
	m_fieldDesc[i].FloatPtr	= &m_SatelliteData.MeanMotion;
	m_fieldDesc[i].HiLimit	= FLT_MAX;
	m_fieldDesc[i].LoLimit	= -FLT_MAX;
	m_fieldDesc[i].Modifiable = true;
	m_importTable[ImpMeanMot]	= i;


}
/////////////////////////////////////////////
//
//	Name		:Copy constructor
//
//	Description :This will copy the data from the Satellite passed to it.
//
//  Input		:Satellite to copy
//
//	Output		:None
//
////////////////////////////////////////////
CSatellite::CSatellite(CSatellite &cpy) : CAstroBody(cpy)
{
	SetBodyType(Satellite);
	m_SatelliteData = cpy.m_SatelliteData;

// copy the ID fields
	m_CatNum		= cpy.m_CatNum;
	m_IntId			= cpy.m_IntId;

// initialize the field Descriptions
	InitFieldDesc();

}

CBodyData &CSatellite::operator=(CBodyData &cpy)
{
	CAstroBody::operator =(cpy);

// don't allow copies to self
	if ( &cpy != this)
	{
	// copy all the data
		SetBodyType(Satellite);
		m_SatelliteData = dynamic_cast<CSatellite *>(&cpy)->m_SatelliteData;
		m_CatNum = dynamic_cast<CSatellite *>(&cpy)->m_CatNum;
		m_IntId = dynamic_cast<CSatellite *>(&cpy)->m_IntId;

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
CSatellite::~CSatellite()
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
int CSatellite::GetSizeOf()
{
	return sizeof(m_SatelliteData);
}

/////////////////////////////////////////////
//
//	Name		:Create
//
//	Description :Makes a new empty Satellite.
//				 Used by the factory.
//
//  Input		:None
//
//	Output		:CBodyData pointer to a new CSatellite
//
////////////////////////////////////////////
CBodyData * CSatellite::Create()
{
	CBodyData* obj = new CSatellite;

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
bool CSatellite::GetActiveFlag()
{
	bool flag;
	if ((unsigned char)m_SatelliteData.Active == 0xFF)
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
void CSatellite::SetActiveFlag(bool flag)
{
	if (flag)
		m_SatelliteData.Active = (char)0xFF;
	else
		m_SatelliteData.Active = 0;
}

/////////////////////////////////////////////
//
//	Name		:GetPosition
//
//	Description :returns the pool position for this Satellite
//
//  Input		:None
//
//	Output		:poolposition of this Satellite
//
////////////////////////////////////////////
const poolposition & CSatellite::GetPosition()
{
	return m_SatelliteData.Next;
}

/////////////////////////////////////////////
//
//	Name		:SetPosition
//
//	Description :Sets the poolposition for this Satellite
//
//  Input		:poolposition
//
//	Output		:None
//
////////////////////////////////////////////
void CSatellite::SetPosition(const poolposition &position)
{
	m_SatelliteData.Next = position;
}

/////////////////////////////////////////////
//
//	Name		:ReadData
//
//	Description :Converts char ptr to 
//				 Satellite data. This is used by import of 
//				 the old Lib files.
//
//  Input		:pointer to memory image
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
void CSatellite::ReadData(unsigned char *ptr)
{
// get the structure from the pointer
	m_SatelliteData = *((SatelliteType *)ptr);

// set the name as the key
	char temp[20];
	strncpy(temp, m_SatelliteData.Name, 16);
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
int CSatellite::GetNumFields()
{
	return MAX_SATELLITE_FIELDS;	// tell them how many fields
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
CBodyData * CSatellite::Copy()
{
	return new CSatellite(*this);
}

/////////////////////////////////////////////
//
//	Name		:ReadImageData
//
//	Description :This will take a pointer to an Satellite data image and convert
//				 it to the SatelliteType structure and store it in the m_SatelliteData
//				 member variable.
//
//  Input		:char *ptr - autostar memory image of Satellite data
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CSatellite::ReadImageData(unsigned char *ptr, int flag)
{
	SatelliteType *Amark = NULL;

// some of it reads ok	
	ReadData(ptr);

// fix the stuff that didn't convert
	m_SatelliteData.Next = ConvertPoolImage(ptr);
	m_SatelliteData.EpochYear = ConvertWordImage(&ptr[(int)(&Amark->EpochYear)]);
	m_SatelliteData.EpochDay = ConvertFloatImage(&ptr[(int)(&Amark->EpochDay)]);
	m_SatelliteData.Inclination = ConvertFloatImage(&ptr[(int)(&Amark->Inclination)]);
	m_SatelliteData.RAAN = ConvertFloatImage(&ptr[(int)(&Amark->RAAN)]);
	m_SatelliteData.Eccentricity = ConvertFloatImage(&ptr[(int)(&Amark->Eccentricity)]);
	m_SatelliteData.ArgPerigee = ConvertFloatImage(&ptr[(int)(&Amark->ArgPerigee)]);
	m_SatelliteData.MeanAnomaly = ConvertFloatImage(&ptr[(int)(&Amark->MeanAnomaly)]);
	m_SatelliteData.MeanMotion = ConvertFloatImage(&ptr[(int)(&Amark->MeanMotion)]);

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
sfieldDesc * CSatellite::GetFieldDesc(int fIndex)
{
	if (fIndex >= 0 && fIndex < MAX_SATELLITE_FIELDS)
		return &m_fieldDesc[fIndex];
	else
		return NULL;
}

void CSatellite::PutImageData(unsigned char *image, int flag)
{

	SatelliteType *Amark = NULL;

// set the key as the name
	strncpy(m_SatelliteData.Name, GetKey(true).GetBuffer(20), 16);

// space fill the name
	

// some of this works
	*((SatelliteType *)image) = m_SatelliteData;

// fix the rest
	PutPoolImage(m_SatelliteData.Next, &image[(int)(&Amark->Next)]);
	PutWordImage(m_SatelliteData.EpochYear, &image[(int)(&Amark->EpochYear)]);
	PutFloatImage(m_SatelliteData.EpochDay, &image[(int)(&Amark->EpochDay)]);
	PutFloatImage(m_SatelliteData.Inclination, &image[(int)(&Amark->Inclination)]);
	PutFloatImage(m_SatelliteData.RAAN, &image[(int)(&Amark->RAAN)]);
	PutFloatImage(m_SatelliteData.Eccentricity, &image[(int)(&Amark->Eccentricity)]);
	PutFloatImage(m_SatelliteData.ArgPerigee, &image[(int)(&Amark->ArgPerigee)]);
	PutFloatImage(m_SatelliteData.MeanAnomaly, &image[(int)(&Amark->MeanAnomaly)]);
	PutFloatImage(m_SatelliteData.MeanMotion, &image[(int)(&Amark->MeanMotion)]);
}

/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to Satellites either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CSatellite::Serialize(CPersist &per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + sizeof(m_SatelliteData) + 
			m_CatNum.GetLength() + m_IntId.GetLength() + 20));

	// copy the key, the 2 IDs, and the data into the space just provided
		per << m_key;
		per << m_CatNum;
		per << m_IntId;
		*((SatelliteType *)per.m_indexPtr) = m_SatelliteData;

	// move the index up for the data only
		per.IncrementIndex(sizeof(m_SatelliteData));
	}
	else
	{
	// get the key, the 2 IDs, and the data from the buffer
		per >> m_key;
		per >> m_CatNum;
		per >> m_IntId;
		m_SatelliteData = *((SatelliteType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_SatelliteData));

	// space fill the key to 16
		while(m_key.GetLength() < 16)
			m_key += " ";
	}


}

/////////////////////////////////////////////
//
//	Name		:ReadTxtData
//
//	Description :Converts char ptr to text into Satellite data.
//				 This is used to import Ephemeris data.
//
//  Input		:pointer to memory image
//
//	Output		:ok
//
////////////////////////////////////////////
bool CSatellite::ReadTxtData(CPersist &per)
{

int			i;
int			*iTbl;
int			len;
CString		sTmp;
RANGESTAT	tStat, rStat = OK;


	if ((iTbl = GetImportTable()) != NULL)
	{
	// search for the beginning of my data
		do
		{
		// find a name		
			len = ParseField(per, ImpName, iTbl);
			sTmp = CString((char *)per.m_indexPtr, len);

		// if the name found is really the first line of data
			if (sTmp.Left(2) == "1 " && len > 50)
			{
			// then blank the name and start decoding			
				sTmp = "";
				break;
			}
			else
			{
			// else check if the next thing is the first line of data
				per.IncrementIndex(len);
				len = ParseField(per, ImpLn1, iTbl);
				if (len == 0)
					return FALSE;
			}

		}while (len != 1 && *((char *)per.m_indexPtr) != '1');

	// check if there is a name at the top

		if (sTmp == "")
		{

	// if no name at the top then set the name to NEW_ENTRY if its blank
			if (GetKey() == "")
				SetKey(NEW_ENTRY);
		}
		else
		{
			SetKey(sTmp);
		}

		i = 1;
		while ( iTbl[i] != -1 )
		{
		// parse this field
			len = ParseField(per, i, iTbl);

			if (len > 0)
			{
			// separate this piece	
				sTmp = CString((char *)per.m_indexPtr, len);
				sTmp.TrimRight();

			// check for Line 1 and Line 2 tags
				if ((i == ImpLn1 && sTmp != "1") || (i == ImpLn2 && sTmp != "2"))
				{
					ParseToEOL(per);
					return false;
				}


			// check for Eccentricity and insert the decimal point
				if (i == ImpEcc)
					sTmp = "." + sTmp;

			// check for Mean Motion and trim it to 8 characters past the decimal point
				if (i == ImpMeanMot){
					sTmp = sTmp.Left(sTmp.Find('.') + 9);
				// if this string was less than 14 characters then there was a space before the end
					// we must parse through the extra field
					if (len < 14)
					{
						per.IncrementIndex(len);
						len = ParseField(per, i, iTbl);
					}
				}


			// if this field is used then run it through
				if (iTbl[i] < GetNumFields() && (tStat = SetFieldData(iTbl[i], sTmp, true)) != OK)
				{
					rStat = tStat;
					ParseToEOL(per);
					return false;
				}

			// increment to the next field
				per.IncrementIndex(len);
				i++;
			}
			else
			{
				ParseToEOL(per);
				return false;
			}
		}
	}

	if (rStat != OK)
		return false;
	else
		return true;

}

int * CSatellite::GetImportTable()
{
	return m_importTable;
}

