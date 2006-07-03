// Satellite.h: interface for the CSatellite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SATELLITE_H__72E52A75_46DC_4EE1_B0FD_4465291E54FD__INCLUDED_)
#define AFX_SATELLITE_H__72E52A75_46DC_4EE1_B0FD_4465291E54FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "autoglob.h"

#include "AstroBody.h"

#define MAX_SATELLITE_FIELDS 10

class CSatellite : public CAstroBody  
{
public:
	void InitFieldDesc();
	virtual int * GetImportTable();
	typedef enum {
		ImpName		= 0,
		ImpLn1,
		ImpCatNum,
		ImpIntID,
		ImpDate,
		ImpTimeDer1,
		ImpTimeDer2,
		ImpBSTAR,
		ImpEphemType,
		ImpEleNum,
		ImpLn2,
		ImpCatNum2,
		ImpIncl,
		ImpRAAN,
		ImpEcc,
		ImpAoP,
		ImpMeanAnom,
		ImpMeanMot,
		ImpLast} ImportFields;

	virtual CBodyData &operator=(CBodyData &Rdata);
	int	m_importTable[ImpLast + 3];	
	virtual bool ReadTxtData(CPersist& per);
	void Serialize(CPersist &per);
	void PutImageData(unsigned char *image, int flag = KDJ_NONE);
	CSatellite();
	virtual ~CSatellite();
	CSatellite(CSatellite &cpy);
	sfieldDesc * GetFieldDesc(int fIndex);
	sfieldDesc m_fieldDesc[MAX_SATELLITE_FIELDS];
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
	CString		m_CatNum;
	CString		m_IntId;

private:
	SatelliteType m_SatelliteData;

};

#endif // !defined(AFX_SATELLITE_H__72E52A75_46DC_4EE1_B0FD_4465291E54FD__INCLUDED_)
