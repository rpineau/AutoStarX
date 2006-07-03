// UserInfo.cpp: implementation of the CUserInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserInfo::CUserInfo()
{
// set the body type
	SetBodyType(UserInfo);

// set initial values
	m_userInfo.Active = (char)0xFF;
	m_userInfo.Brightness = 0x03;
	m_userInfo.Contrast = 0x0a;
	m_userInfo.CurrentSite = 1;
	m_userInfo.dd = 2;
	m_userInfo.EndTag = 0;
	m_userInfo.Flags = 0;
	m_userInfo.MaxSites = 0;
	m_userInfo.mm = 1;
	m_userInfo.Next.offset = 0;
	m_userInfo.Next.page = 0;
	m_userInfo.ScrollRate =0x15;
	m_userInfo.Update = (char) 0xFF;
	m_userInfo.yy = 2002;

	strcpy(m_personalInfo.City, "");
	m_personalInfo.DOBDay = 1;
	m_personalInfo.DOBMon = 1;
	m_personalInfo.DOBYear = 1950;
	strcpy(m_personalInfo.FirstName, "");
	strcpy(m_personalInfo.LastName, "");
	m_personalInfo.PinNum = 0;
	strcpy(m_personalInfo.PostCode, "");
	strcpy(m_personalInfo.SerialNum, "");
	strcpy(m_personalInfo.State, "");
	strcpy(m_personalInfo.Street1, "");
	strcpy(m_personalInfo.Street2, "");

	// create a unique key based on the time
	COleDateTime time = COleDateTime::GetCurrentTime();
	CString key = time.Format("%y%m%d%H%M%S");
	SetKey(key);
}

CUserInfo::~CUserInfo()
{

}

CUserInfo::CUserInfo(CUserInfo &cpy) : CBodyData(cpy)
{
	SetBodyType(UserInfo);
	m_userInfo = cpy.m_userInfo;
	m_personalInfo = cpy.m_personalInfo;

	m_city = cpy.m_city;
	m_firstName = cpy.m_firstName;
	m_lastName = cpy.m_lastName;
	m_postCode = cpy.m_postCode;
	m_serialNum = cpy.m_serialNum;
	m_state = cpy.m_state;
	m_street1 = cpy.m_street1;
	m_street2 = cpy.m_street2;
}

bool CUserInfo::ReadImageData(unsigned char *ptr, int flag)
{
	UserInfoType *Amark = NULL;
	PersonalInfoType *Amark2 = NULL;

// some of it reads ok	
	m_userInfo = *((UserInfoType *)ptr);

// fix the stuff that didn't convert
	m_userInfo.CurrentSite = ConvertWordImage(&ptr[(int)(&Amark->CurrentSite)]);
	m_userInfo.MaxSites = ConvertWordImage(&ptr[(int)(&Amark->MaxSites)]);
	m_userInfo.yy = ConvertWordImage(&ptr[(int)(&Amark->yy)]);

// advance the pointer to the personal info data
	ptr += sizeof(UserInfoType);

// read the easy stuff
	m_personalInfo = *((PersonalInfoType *) ptr);

// fix the rest
	m_personalInfo.PinNum = ConvertWordImage(&ptr[(int)(&Amark2->PinNum)]);
	m_personalInfo.DOBYear = ConvertWordImage(&ptr[(int)(&Amark2->DOBYear)]);
	ConvertCharData(16, &m_firstName, m_personalInfo.FirstName);
	ConvertCharData(16, &m_lastName, m_personalInfo.LastName);
	ConvertCharData(30, &m_street1, m_personalInfo.Street1);
	ConvertCharData(30, &m_street2, m_personalInfo.Street2);
	ConvertCharData(16, &m_city, m_personalInfo.City);
	ConvertCharData(16, &m_state, m_personalInfo.State);
	ConvertCharData(16, &m_postCode, m_personalInfo.PostCode);
	ConvertCharData(16, &m_serialNum, m_personalInfo.SerialNum);

// insert null characters

	return true;
}	

CBodyData * CUserInfo::Create()
{
	CBodyData* obj = new CUserInfo;

	return obj;
}


int CUserInfo::GetSizeOf()
{
	return sizeof(UserInfoType) + sizeof(PersonalInfoType);
}

int CUserInfo::GetNumFields()
{
	return 7;
}

void CUserInfo::PutImageData(unsigned char *image, int flag)
{
	UserInfoType *Amark = NULL;
	PersonalInfoType *Amark2 = NULL;

	int currentSite = m_userInfo.CurrentSite;
	int maxSites = m_userInfo.MaxSites;

// some of this works
	*((UserInfoType *)image) = m_userInfo;

// convert to zero-based site indexing if necessary
	if (flag & KDJ_ZERO_BASED)
	{
		currentSite--;
		maxSites--;
	}

// fix the rest
	PutWordImage(m_userInfo.yy, &image[(int)(&Amark->yy)]);	
	PutWordImage(currentSite, &image[(int)(&Amark->CurrentSite)]);
	PutWordImage(maxSites, &image[(int)(&Amark->MaxSites)]);

// advance the pointer to the personal info data
	image += sizeof(UserInfoType);

// copy the strings
	strncpy(m_personalInfo.FirstName, m_firstName.GetBuffer(20), 16);
	strncpy(m_personalInfo.LastName, m_lastName.GetBuffer(20), 16);
	strncpy(m_personalInfo.Street1, m_street1.GetBuffer(34), 30);
	strncpy(m_personalInfo.Street2, m_street2.GetBuffer(34), 30);
	strncpy(m_personalInfo.City, m_city.GetBuffer(20), 16);
	strncpy(m_personalInfo.State, m_state.GetBuffer(20), 16);
	strncpy(m_personalInfo.PostCode, m_postCode.GetBuffer(20), 16);
	strncpy(m_personalInfo.SerialNum, m_serialNum.GetBuffer(20), 16);


// set the easy stuff
	*((PersonalInfoType *) image) = m_personalInfo;

// fix the rest
	PutWordImage(m_personalInfo.PinNum, &image[(int)(&Amark2->PinNum)]);
	PutWordImage(m_personalInfo.DOBYear, &image[(int)(&Amark2->DOBYear)]);


}

sfieldDesc * CUserInfo::GetFieldDesc(int fIndex)
{
	return NULL;
}


CBodyData * CUserInfo::Copy()
{
	return new CUserInfo(*this);
}

/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to User Info either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CUserInfo::Serialize(CPersist& per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + GetSizeOf() + 20));

	// copy the user info data into the space just provided
		per << m_key;
		*((UserInfoType *)per.m_indexPtr) = m_userInfo;

	// move the index up
		per.IncrementIndex(sizeof(m_userInfo));

	// copy the strings
		strncpy(m_personalInfo.FirstName, m_firstName.GetBuffer(20), 16);
		strncpy(m_personalInfo.LastName, m_lastName.GetBuffer(20), 16);
		strncpy(m_personalInfo.Street1, m_street1.GetBuffer(34), 30);
		strncpy(m_personalInfo.Street2, m_street2.GetBuffer(34), 30);
		strncpy(m_personalInfo.City, m_city.GetBuffer(20), 16);
		strncpy(m_personalInfo.State, m_state.GetBuffer(20), 16);
		strncpy(m_personalInfo.PostCode, m_postCode.GetBuffer(20), 16);
		strncpy(m_personalInfo.SerialNum, m_serialNum.GetBuffer(20), 16);


	// copy the user info data into the space just provided
		*((PersonalInfoType *)per.m_indexPtr) = m_personalInfo;

	// move the index up
		per.IncrementIndex(sizeof(m_personalInfo));

	}
	else
	{
	// get the user data from the buffer
		per >> m_key;
		m_userInfo = *((UserInfoType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_userInfo));

	// get the personal info data from the buffer
		m_personalInfo = *((PersonalInfoType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_personalInfo));

	// read the strings
		ConvertCharData(16, &m_firstName, m_personalInfo.FirstName);
		ConvertCharData(16, &m_lastName, m_personalInfo.LastName);
		ConvertCharData(30, &m_street1, m_personalInfo.Street1);
		ConvertCharData(30, &m_street2, m_personalInfo.Street2);
		ConvertCharData(16, &m_city, m_personalInfo.City);
		ConvertCharData(16, &m_state, m_personalInfo.State);
		ConvertCharData(16, &m_postCode, m_personalInfo.PostCode);
		ConvertCharData(16, &m_serialNum, m_personalInfo.SerialNum);
	
	}

}




void CUserInfo::ConvertCharData(int length, CString *string, char *input)
{
	char *temp;
	temp = (char *) malloc(length + 3);
	strncpy(temp, input, length);
	temp[length] = 0;
	*string = temp;
	string->TrimRight();
	free(temp);
}
