// LandMark.h: interface for the CLandMark class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LANDMARK_H__8CA20346_DBC4_4D96_82C6_6A82C755CEB7__INCLUDED_)
#define AFX_LANDMARK_H__8CA20346_DBC4_4D96_82C6_6A82C755CEB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BodyData.h"

#define MAX_LANDMARK_FIELDS 3

class CLandMark : public CBodyData  
{
public:
	virtual CBodyData &operator=(CBodyData &Rdata);
	int * GetImportTable();
	void InitFieldDesc();
	virtual bool ReadTxtData(CPersist& per);
	void Serialize(CPersist &per);
	void PutImageData(unsigned char *image, int flag = KDJ_NONE);
	CLandMark();
	virtual ~CLandMark();
	CLandMark(CLandMark &cpy);
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

	sfieldDesc m_fieldDesc[MAX_LANDMARK_FIELDS];
	int m_importTable[MAX_LANDMARK_FIELDS + 4];

	typedef enum {
		ImpName		= 0,
		ImpAz,
		ImpAlt,
		ImpLast} ImportFields;

private:
	LandmarkType m_LandMarkData;


};

#endif // !defined(AFX_LANDMARK_H__8CA20346_DBC4_4D96_82C6_6A82C755CEB7__INCLUDED_)
