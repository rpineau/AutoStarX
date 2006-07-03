// UserObj.h: interface for the CUserObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USEROBJ_H__B057ECC0_2873_11D5_A1FC_444553540001__INCLUDED_)
#define AFX_USEROBJ_H__B057ECC0_2873_11D5_A1FC_444553540001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AstroBody.h"

// Structure to define User Objects in LX-200 Precision
typedef struct {
	poolposition Next;
	char Active;
	char Name[16];
	long RA;
	long Dec;
	short Size;
	short Mag;
	char Endtag;
	} UserObjTypePrec;

#define MAX_USEROBJ_FIELDS 5

class CUserObj : public CAstroBody  
{
public:
	virtual CBodyData &operator=(CBodyData &Rdata);
	int * GetImportTable();
	void InitFieldDesc();
	virtual bool ReadTxtData(CPersist& per);
	void Serialize(CPersist& per);
	void PutImageData(unsigned char *ptr, int flag = KDJ_NONE);
	CUserObj();
	virtual ~CUserObj();
	CUserObj(CUserObj &cpy);
	sfieldDesc * GetFieldDesc(int fIndex);
	virtual bool ReadImageData(unsigned char *ptr, int flag = KDJ_NONE);
	CBodyData * Copy();
	void SetPosition(const poolposition& position);
	const poolposition & GetPosition();
	void SetActiveFlag(bool flag);
	bool GetActiveFlag();
	CBodyData * Create();
	void ReadData(unsigned char *data);
	int GetSizeOf();
	int GetNumFields();

	sfieldDesc m_fieldDesc[MAX_USEROBJ_FIELDS];
	int m_importTable[MAX_USEROBJ_FIELDS + 4];

	typedef enum {
		ImpName		= 0,
		ImpRA,
		ImpDec,
		ImpSize,
		ImpMag,
		ImpLast} ImportFields;

private:
	UserObjType ConvertToLoPrec();
	void PutImageDataLoPrec(unsigned char *image);
	UserObjTypePrec ConvertFromLoPrec(void *pData);
	bool ReadImageDataLoPrec(unsigned char *ptr);
	UserObjTypePrec m_UserObjData;

};

#endif // !defined(AFX_USEROBJ_H__B057ECC0_2873_11D5_A1FC_444553540001__INCLUDED_)
