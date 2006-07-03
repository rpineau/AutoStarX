// Site.h: interface for the CSite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SITE_H__985599E9_AFF7_486C_B177_B634EEF9234C__INCLUDED_)
#define AFX_SITE_H__985599E9_AFF7_486C_B177_B634EEF9234C__INCLUDED_

#include "Autostar.h"	// Added by ClassView
#include "BodyData.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_SITE_FIELDS		4

typedef struct {
   poolposition Next;   // Pointer to next record of this type on heap 
   char  Active;        // FF = Active, 00=Deleted, FE = No Delete, FC = No Edit 
   char  Name[17];      // User Site name 
   long  Latitude;      // in arc sec 
   long  Longitude;     // in arc sec 
   long  AzError;       // SiteSpecific AzError
   long  ElError;       // SiteSpecific ElError
   word  NearestCity;   // offset into city db 
   char  TimeZone;      // offset from UTC 
   char  EndTag;
   } SiteInfoType;


#include "BodyData.h"

class CSite : public CBodyData  
{
public:
	virtual CBodyData &operator=(CBodyData &Rdata);
	CSite();
	CSite(CSite &cpy);
	virtual ~CSite();
	bool ReadImageData(unsigned char *ptr, int flag = KDJ_NONE);
	bool ReadImageDataLoPrec(unsigned char *ptr);
	CBodyData * Create();
	int GetSizeOf();
	int GetNumFields();
	void PutImageData(unsigned char *image, int flag = KDJ_NONE);
	void PutImageDataLoPrec(unsigned char *image);
	sfieldDesc * GetFieldDesc(int fIndex);
	CBodyData * Copy();
	void Serialize(CPersist& per);
	SiteInfoType m_siteInfo;

private:
	void CopyKey(char dest[], int length);
	SiteType ConvertToLoPrec();
	SiteInfoType ConvertFromLoPrec(void *pData);
	void InitFieldDesc();
	sfieldDesc m_fieldDesc[MAX_SITE_FIELDS];
};

#endif // !defined(AFX_SITE_H__985599E9_AFF7_486C_B177_B634EEF9234C__INCLUDED_)
