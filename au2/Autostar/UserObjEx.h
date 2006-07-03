// UserObjEx.h: interface for the CUserObjEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USEROBJEX_H__6B275C3E_2B51_4F26_86A0_388B9B2604FD__INCLUDED_)
#define AFX_USEROBJEX_H__6B275C3E_2B51_4F26_86A0_388B9B2604FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UserObj.h"
#include "BodyData.h"	// Added by ClassView



#define MAX_USEROBJEX_FIELDS 9
#define REQD_USEROBJEX_FIELDS 3


// Structure to define Custom User Objects
typedef struct {
	poolposition Next;
	char Active;
	char Name[16];
	long RA;
	long Dec;
	short numFields;
	CString fieldName[MAX_USEROBJEX_FIELDS - REQD_USEROBJEX_FIELDS];
	CString data[MAX_USEROBJEX_FIELDS - REQD_USEROBJEX_FIELDS];
	char Endtag;
	} UserObjTypeEx;

typedef struct {              // Downloadable catalog record format 
      poolposition Next;
      char Active;
      char Name[16];
      medium RA;
      medium Dec;
      char Fields[256];
      } CustomCatalogRecType;




class CUserObjEx : public CUserObj  
{
public:
	bool SetFieldDataFromString(CString data);
	void PutImageData(unsigned char *image, int flag);
	int GetFieldSize(int index);
	CString GetFieldName(int index);
	CString GetCatalogName();
	CBodyData * CopyTemplate();
	sfieldDesc * GetFieldDesc(int fIndex);
	bool SetFieldName(int index, CString name);
	void SetNumFields(short num);
	int GetNumFields();
	CBodyData & operator=(CBodyData &copy);
	CBodyData * Copy();
	void SetCatalogName(CString name);
	CUserObjEx(CUserObjEx &copy);
	void Serialize(CPersist &per);
	void InitFieldDesc();
	bool ReadImageData(unsigned char *ptr, int flag = KDJ_NONE);
	int GetSizeOf();
	bool IsCustom();
	virtual ~CUserObjEx();
	CUserObjEx(BodyType bodyType, CString catName = "");
	sfieldDesc m_fieldDesc[MAX_USEROBJEX_FIELDS];

private:
	bool SetFieldFromString(int index, CString data);
	void SetFieldSize(int index, int size);
	int ParseFieldHeader(int index, CString data);
	int GetUserObjDataSize();
	CString m_catalogName;
//	void ReadData(unsigned char *ptr);
	UserObjTypeEx m_UserObjData;

};

#endif // !defined(AFX_USEROBJEX_H__6B275C3E_2B51_4F26_86A0_388B9B2604FD__INCLUDED_)
