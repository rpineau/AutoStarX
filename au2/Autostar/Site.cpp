// Site.cpp: implementation of the CSite class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Site.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSite::CSite()
{
	SetBodyType(SiteInfo);
	m_siteInfo.Latitude = 0;
	m_siteInfo.Longitude = 0;
	m_siteInfo.TimeZone = 0;
	strcpy(m_siteInfo.Name, "");
	m_siteInfo.AzError = 0;
	m_siteInfo.ElError = 0;
	m_siteInfo.NearestCity = 1;

	// initialize the field Descriptions
	InitFieldDesc();

}

CSite::~CSite()
{

}

CSite::CSite(CSite &cpy) : CBodyData(cpy)
{
	SetBodyType(SiteInfo);
	m_siteInfo = cpy.m_siteInfo;

// initialize the field Descriptions
	InitFieldDesc();

}

bool CSite::ReadImageData(unsigned char *ptr, int flag)
{
	if (flag & KDJ_LO_PREC)
		return ReadImageDataLoPrec(ptr);

	SiteInfoType *Amark = NULL;

// some of it reads ok	
	m_siteInfo = *((SiteInfoType *)ptr);

// fix the stuff that didn't convert
	m_siteInfo.Latitude = ConvertLongImage(&ptr[(int)(&Amark->Latitude)]);
	m_siteInfo.Longitude = ConvertLongImage(&ptr[(int)(&Amark->Longitude)]);
	m_siteInfo.AzError = ConvertLongImage(&ptr[(int)(&Amark->AzError)]);
	m_siteInfo.ElError = ConvertLongImage(&ptr[(int)(&Amark->ElError)]);
	m_siteInfo.NearestCity = ConvertWordImage(&ptr[(int)(&Amark->NearestCity)]);

// set the name as the key
	char temp[20];
	strncpy(temp, m_siteInfo.Name, 16);
	temp[16] = 0;
	SetKey(temp);
	return true;
}	


/////////////////////////////////////////////
//
//	Name		:ReadImageDataLoPrec
//
//	Description :This will take a pointer to a site data image and convert
//				 it to the SiteInfoType structure and store it in the m_siteInfo
//				 member variable. - for Autostar 494/495/497
//
//  Input		:char *ptr - autostar memory image of User Object data
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CSite::ReadImageDataLoPrec(unsigned char *ptr)
{
	SiteType *Amark = NULL;

// some of it reads ok	
	SiteType loObject;

	loObject = *((SiteType *) ptr);

	// copy the members that dont require conversion
	m_siteInfo.Active = (char) 0xFF;
	m_siteInfo.EndTag = (char) 0;
	m_siteInfo.Next.offset = 0xFFFF;
	m_siteInfo.Next.page = 0xFF;

	strcpy(m_siteInfo.Name, loObject.SiteName);
	m_siteInfo.Name[16] = 0;
	m_siteInfo.TimeZone = loObject.TimeZone;


	// fix the stuff that didn't convert
	loObject.Latitude = ConvertWordImage(&ptr[(int)(&Amark->Latitude)]);
	m_siteInfo.Latitude = (long) loObject.Latitude;
	loObject.Longitude = ConvertWordImage(&ptr[(int)(&Amark->Longitude)]);
	m_siteInfo.Longitude = (long) loObject.Longitude;
	m_siteInfo.NearestCity = ConvertWordImage(&ptr[(int)(&Amark->NearestCity)]);

	// convert the members that require it
	m_siteInfo.Latitude *= 60;
	m_siteInfo.Longitude *= 60;

	// set the name as the key
	char temp[20];
	strncpy(temp, m_siteInfo.Name, 16);
	temp[16] = 0;
	SetKey(temp);
	return true;
}




CBodyData * CSite::Create()
{
	CBodyData* obj = new CSite;

	return obj;
}


int CSite::GetSizeOf()
{
	return sizeof(SiteInfoType);
}

int CSite::GetNumFields()
{
	return 4;
}

void CSite::PutImageData(unsigned char *image, int flag)
{
	if (flag & KDJ_LO_PREC)
	{
		PutImageDataLoPrec(image);
		return;
	}

	SiteInfoType *Amark = NULL;

// set the key as the name
//	strncpy(m_siteInfo.Name, GetKey(true).GetBuffer(20), 16);
	CopyKey(m_siteInfo.Name, 17);

// some of this works
	*((SiteInfoType *)image) = m_siteInfo;

// fix the rest
	PutLongImage(m_siteInfo.Latitude, &image[(int)(&Amark->Latitude)]);	
	PutLongImage(m_siteInfo.Longitude, &image[(int)(&Amark->Longitude)]);
	PutLongImage(m_siteInfo.AzError, &image[(int)(&Amark->AzError)]);	
	PutLongImage(m_siteInfo.ElError, &image[(int)(&Amark->ElError)]);
	PutWordImage(m_siteInfo.NearestCity, &image[(int)(&Amark->NearestCity)]);
	
}


void CSite::PutImageDataLoPrec(unsigned char *image)
{
	SiteType *Amark = NULL;

	// create a lo precision copy of the site object
	SiteType loSite;

	// set the name & data
	strncpy(loSite.SiteName,GetKey(true).GetBuffer(20), 16);

	int temp = m_siteInfo.Latitude / 60;	// convert to low precision
	loSite.Latitude = (short) temp;
	temp = m_siteInfo.Longitude / 60;	// convert to low precision
	loSite.Longitude = (short) temp;
	loSite.NearestCity = m_siteInfo.NearestCity;
	loSite.TimeZone = m_siteInfo.TimeZone;


// some of this works
	*((SiteType *)image) = loSite;

// fix the rest
	PutWordImage(loSite.Latitude, &image[(int)(&Amark->Latitude)]);	
	PutWordImage(loSite.Longitude, &image[(int)(&Amark->Longitude)]);
	PutWordImage(loSite.NearestCity, &image[(int)(&Amark->NearestCity)]);
	
}

sfieldDesc * CSite::GetFieldDesc(int fIndex)
{
	if (fIndex >= 0 && fIndex < MAX_SITE_FIELDS)
		return &m_fieldDesc[fIndex];
	else
		return NULL;
}


CBodyData * CSite::Copy()
{
	return new CSite(*this);
}

/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to Site Info either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CSite::Serialize(CPersist& per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + sizeof(m_siteInfo) + 10));

	// copy the key and data into the space just provided
		per << m_key;
		*((SiteInfoType *)per.m_indexPtr) = m_siteInfo;

	// move the index up for the data only
		per.IncrementIndex(sizeof(m_siteInfo));
	}
	else
	{
	// get the key and the data from the buffer
		per >> m_key;
		m_siteInfo = *((SiteInfoType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_siteInfo));

	// space fill the key to 16
		while(m_key.GetLength() < 16)
			m_key += " ";
	}

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
void CSite::InitFieldDesc()
{
	int i = 0;

// Name
	m_fieldDesc[i].Label	= "Name";
	m_fieldDesc[i].Type		= KEY_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].Modifiable = true;

	// increment
	i++;

// Latitude
	m_fieldDesc[i].Label	= "Latitude (North Positive)";
	m_fieldDesc[i].Type		= LONG_DEC_TYPE;
	m_fieldDesc[i].Format	= "%+02d°%02d'00\"";
	m_fieldDesc[i].LongPtr	= &m_siteInfo.Latitude;
	m_fieldDesc[i].HiLimit	= 90 * 60 * 60;
	m_fieldDesc[i].LoLimit	= -90 * 60 * 60;
	m_fieldDesc[i].Modifiable = true;

	// increment
	i++;

// Longitude
	m_fieldDesc[i].Label	= "Longitude (West Positive)";
	m_fieldDesc[i].Type		= LONG_LONG_TYPE;
	m_fieldDesc[i].Format	= "%+02d°%02d'00\"";
	m_fieldDesc[i].LongPtr	= &m_siteInfo.Longitude;
	m_fieldDesc[i].HiLimit	= 180 * 60 * 60;
	m_fieldDesc[i].LoLimit	= -180 * 60 * 60;
	m_fieldDesc[i].Modifiable = true;

	// increment
	i++;

// Time Zone Offset
	m_fieldDesc[i].Label	= "Time Zone East of GMT";
	m_fieldDesc[i].Type		= TIMEZONE_TYPE;
	m_fieldDesc[i].Format	= "%+02d:%02d";
	m_fieldDesc[i].CharPtr	= (char *)&m_siteInfo.TimeZone;
	m_fieldDesc[i].HiLimit	= 24;
	m_fieldDesc[i].LoLimit	= -24;
	m_fieldDesc[i].Modifiable = true;

}


CBodyData &CSite::operator=(CBodyData &cpy)
{
	CBodyData::operator =(cpy);
	if ( &cpy != this)
	{
		SetBodyType(SiteInfo);
		m_siteInfo = dynamic_cast<CSite *>(&cpy)->m_siteInfo;

	// initialize the field Descriptions
		InitFieldDesc();
	}

	return *this;
}

/*
SiteInfoType CSite::ConvertFromLoPrec(void *pData)
{
	SiteType loObject;
	SiteInfoType hiObject;

	// cast void pointer to UserObjType structure
	loObject = *((SiteType *) pData);

	// copy the members that dont require conversion
	hiObject.Active = (char) 0xFF;
	hiObject.EndTag = (char) 0;
	strcpy(hiObject.Name, loObject.SiteName);
	hiObject.Next.offset = 0xFFFF;
	hiObject.Next.page = 0xFF;

	// convert the members that require it
//	hiObject.Dec = (long) loObject.Dec * 60;
//	hiObject.RA =  (long) loObject.RA * 60;

	return hiObject;
}

SiteType CSite::ConvertToLoPrec()
{
	SiteType loObject;

	// copy all the straightforward members
	strcpy(loObject.SiteName, m_siteInfo.Name);
//todo

	// convert the more involved members
//	loObject.Dec = (word) (static_cast<float>(m_UserObjData.Dec) / 60.0 + .5);
//	loObject.RA =  (word) (static_cast<float>(m_UserObjData.RA) / 60 + .5);

	return loObject;
}
*/


/////////////////////////////////////////////
//
//	Name		:CopyKey
//
//	Description :Copies the 16-character name CString to a specified
//				 length character array, filling the whitespace at
//				 the end with 0's
//
//  Input		:destination character array and length of that array
//
//	Output		:None
//
////////////////////////////////////////////

void CSite::CopyKey(char dest[], int length)
{
// set the key as the name
	CString key = GetKey(true);
	key.TrimRight();
	int textLength = key.GetLength();

	// copy up to 16 characters of the key string
	strncpy(dest, GetKey(true).GetBuffer(20), 16);

	// now fill up the back of the array with 0's
	for (int i = textLength; i < length; i++)
		dest[i] = (char) 0x00;


}
