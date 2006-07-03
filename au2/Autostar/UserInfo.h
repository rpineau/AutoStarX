// UserInfo.h: interface for the CUserInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERINFO_H__AD27DF48_4009_4895_B03B_3ABEBC05B064__INCLUDED_)
#define AFX_USERINFO_H__AD27DF48_4009_4895_B03B_3ABEBC05B064__INCLUDED_

#include "Autostar.h"	// Added by ClassView
#include "BodyData.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef struct {
   poolposition Next;      // Pointer to next record of this type on heap 
   char Active;            // FF = Active, 00=Deleted, FE = No Delete, FC = No Edit 
   char   Flags;           // UserConfig Settings
   char   ScrollRate;      // Non-Volatile ScrollRate
   char   Brightness;      // Non-Volatile brightness
   char   Contrast;        // Non-Volatile contrast
   short  CurrentSite;     // Index to current site;
   short  MaxSites;        // Number of Currently Defined Sites
   char   Update;          // A static value has changed
   short  yy;              // >>Temporary unitl a clock is installed
   char   mm;              // >>
   char   dd;              // >> Todo - delete on scope board
   char EndTag;
   } UserInfoType;

typedef struct {   
   poolposition Next;    // Pointer to next record of this type on heap 
   char Active;          // FF = Active, 00=Deleted, FE = No Delete, FC = No Edit 
   char FirstName[16];   // customer inf 
   char LastName[16];
   short PinNum;
   char Street1[30];
   char Street2[30];
   char City[16];
   char State[16];
   char PostCode[16];
   char DOBMon;
   char DOBDay;
   short DOBYear;
   char SerialNum[16];
   char EndTag;
   } PersonalInfoType;


class CUserInfo : public CBodyData  
{
public:
	CString m_firstName, m_lastName, m_street1, m_street2, m_city, m_state, m_postCode, m_serialNum;
	PersonalInfoType m_personalInfo;
	UserInfoType m_userInfo;
	CUserInfo();
	CUserInfo(CUserInfo &cpy);
	virtual ~CUserInfo();
	CBodyData * Create();
	int GetSizeOf();
	int GetNumFields();
	void PutImageData(unsigned char *image, int flag = KDJ_NONE);
	bool ReadImageData(unsigned char *ptr, int flag = KDJ_NONE);
	sfieldDesc * GetFieldDesc(int fIndex);
	CBodyData * Copy();
	void Serialize(CPersist& per);

private:
	void ConvertCharData(int length, CString *string, char *input);
};

#endif // !defined(AFX_USERINFO_H__AD27DF48_4009_4895_B03B_3ABEBC05B064__INCLUDED_)
