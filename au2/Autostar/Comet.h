// Comet.h: interface for the CComet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMET_H__FDDD9B8E_3023_4927_AF4A_86D0DB050FCB__INCLUDED_)
#define AFX_COMET_H__FDDD9B8E_3023_4927_AF4A_86D0DB050FCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_COMET_FIELDS 8

#include "AstroBody.h"

class CComet : public CAstroBody  
{
public:
	virtual CBodyData &operator=(CBodyData &cpy);
	void InitFieldDesc();
	int * GetImportTable();
	int m_importTable[MAX_COMET_FIELDS + 4];
	void Serialize(CPersist &per);
	void PutImageData(unsigned char *image, int flag = KDJ_NONE);
	CComet();
	virtual ~CComet();
	CComet(CComet &cpy);
	sfieldDesc * GetFieldDesc(int fIndex);
	sfieldDesc m_fieldDesc[MAX_COMET_FIELDS];
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

	typedef enum {
		ImpName		= 0,
		ImpEclEpo,
		ImpDate,
		ImpPerDist,
		ImpEcc,
		ImpArg,
		ImpLong,
		ImpInc,
		ImpMag,
		ImpK,
		ImpRem,
		ImpLast} ImportFields;


private:
	CometType m_CometData;

};

#endif // !defined(AFX_COMET_H__FDDD9B8E_3023_4927_AF4A_86D0DB050FCB__INCLUDED_)
