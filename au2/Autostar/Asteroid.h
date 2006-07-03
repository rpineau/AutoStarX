// Asteroid.h: interface for the CAsteroid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASTEROID_H__F1C4EFE2_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_)
#define AFX_ASTEROID_H__F1C4EFE2_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AstroBody.h"
#include "autoglob.h"

#define MAX_ASTEROID_FIELDS 10

class CAsteroid : public CAstroBody  
{
public:
	void InitFieldDesc();
	typedef enum {
		ImpName		= 0, 
		ImpDate,
		ImpEcc,
		ImpSemi,
		ImpIncl,
		ImpLong,
		ImpPeri,
		ImpEccEpoch,
		ImpMean,
		ImpMag,
		ImpSlope,
		ImpBlank,
		ImpLast} ImportFields;

	virtual CBodyData &operator=(CBodyData &Rdata);
	virtual int * GetImportTable();
	virtual void Serialize(CPersist &per);
	void PutImageData(unsigned char *image, int flag = KDJ_NONE);
	CAsteroid();
	virtual ~CAsteroid();
	CAsteroid(CAsteroid &cpy);
	sfieldDesc * GetFieldDesc(int fIndex);
	sfieldDesc m_fieldDesc[MAX_ASTEROID_FIELDS];
	int	m_importTable[ImpLast + 3];
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

private:
	AsteroidType m_AsteroidData;
};

#endif // !defined(AFX_ASTEROID_H__F1C4EFE2_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_)
