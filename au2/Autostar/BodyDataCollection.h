// BodyDataCollection.h: interface for the BodyDataCollection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BodyDataCOLLECTION_H__F1C4EFE0_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_)
#define AFX_BodyDataCOLLECTION_H__F1C4EFE0_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BodyData.h"
#include "BodyDataFactory.h"
#include <afxtempl.h>

typedef	CList<CBodyData*, CBodyData*>	BodyDataList;
//typedef CTypedPtrList<CObList, CBodyData*>  BodyDataList;
typedef enum{READCOMPLETE, FILEERROR, WRONGFILETYPE, BADOPTION, BADBODYTYPE} FileStat;

class CBodyDataCollection : public CObject
{
public:
	CString m_header;
	int m_impCount;
	typedef enum{fast, dupchk, update} importOption;
	int Update(CBodyDataCollection *newData, BodyType pType);
	POSITION GetTailPosition(BodyType type=All);
	CBodyData * GetPrev(POSITION &pos, BodyType type=All);
	word m_fileVersion;
	int GetTotalSizeOf(BodyType type=All);
	CString GetFieldLabel(int i, BodyType type);
	int GetNumFields(BodyType type);
	bool SortBy(bool Asc, BodyType type, int Field1, int Field2=0, int Field3=0);
	CBodyDataCollection();
//	CBodyDataCollection(CBodyDataCollection &src, POSITION from, POSITION to);
	virtual ~CBodyDataCollection();
	CBodyDataCollection &operator+=(CBodyDataCollection &right);
	CBodyDataCollection &operator=(CBodyDataCollection &right);
	void Clear(BodyType type=All);
	POSITION Add(CBodyData* data, POSITION pos=NULL);
	void Add(CBodyDataCollection &data, BodyType type=All, POSITION at=NULL, POSITION from=NULL, POSITION to=NULL);
	POSITION Remove(CBodyData *data);
	POSITION Delete(CString name);
	CBodyData * Find(CString name);
	FileStat LoadFromFile(CString fileName, BodyType type=All);
	FileStat SaveToFile(CString fileName, BodyType type=All);
	int GetCount(BodyType type=All);
	virtual FileStat Import(CString filename, BodyType type, importOption Option=dupchk);
	virtual FileStat Import(CString filename, BodyType type, int &count, importOption Option=dupchk);
	FileStat Export(CString filename, BodyType type);
	POSITION GetHeadPosition(BodyType type=All);
	CBodyData *GetNext(POSITION &pos, BodyType type=All);
	CBodyData *GetAt(POSITION pos);

	BodyDataList	m_userList;
private:
	void PercolateUp(int iMaxLevel, CBodyData **pList, bool Asc, int Field1, int Field2, int Field3);
	void PercolateDown(int iMaxLevel, CBodyData **pList, bool Asc, int Field1, int Field2, int Field3);
	DWORD m_dataReadCnt;
	void* m_dataPtr;
	FileStat ReadFile(CString filename);
	CBodyDataFactory *m_factory;
};

#endif // !defined(AFX_BodyDataCOLLECTION_H__F1C4EFE0_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_)
