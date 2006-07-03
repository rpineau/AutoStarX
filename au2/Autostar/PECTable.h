// PECTable.h: interface for the CPECTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PECTABLE_H__2BE67C0D_67C1_4301_A032_233C077F9180__INCLUDED_)
#define AFX_PECTABLE_H__2BE67C0D_67C1_4301_A032_233C077F9180__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BodyData.h"
#include "Autostar.h"	// Added by ClassView

#define PECTABLESIZE 200

typedef struct {
	poolposition Next;
	char Active;
	char Map[PECTABLESIZE];
	char Sessions;
	char EndTag;
}	PECTableType;
	

class CPECTable : public CBodyData  
{
public:
	bool GetActiveFlag();
	void SetActiveFlag(bool flag);
	CPECTable();
	virtual ~CPECTable();
	CPECTable(CPECTable &cpy);
	CBodyData * Create();
	int GetSizeOf();
	int GetNumFields();
	void PutImageData(unsigned char *image, int flag = KDJ_NONE);
	bool ReadImageData(unsigned char *ptr, int flag = KDJ_NONE);
	sfieldDesc * GetFieldDesc(int fIndex);
	CBodyData * Copy();
	void Serialize(CPersist& per);

private:
	PECTableType m_PECTable;
};

#endif // !defined(AFX_PECTABLE_H__2BE67C0D_67C1_4301_A032_233C077F9180__INCLUDED_)
