// Tour.h: interface for the CTour class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOUR_H__7A91E3AF_16CE_4372_90D8_8EFEDB896ECB__INCLUDED_)
#define AFX_TOUR_H__7A91E3AF_16CE_4372_90D8_8EFEDB896ECB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BodyData.h"

#define MAX_TOUR_FIELDS 3

class CTour : public CBodyData  
{
public:
	virtual bool IsExtended();
	virtual bool IsDynamic();
	virtual CBodyData &operator=(CBodyData &Rdata);
	void InitFieldDesc();
	virtual bool ReadTxtData(CPersist& per);
	virtual bool ReadRomData(CPersist& per);
	void Serialize(CPersist &per);
	void PutImageData(unsigned char *image, int flag = KDJ_NONE);
	void * m_tourBody;
	CTour();
	virtual ~CTour();
	CTour(CTour &cpy);
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

	sfieldDesc m_fieldDesc[MAX_TOUR_FIELDS];

private:
	bool m_extended;
	short m_length;
	TourType m_TourData;

};

#endif // !defined(AFX_TOUR_H__7A91E3AF_16CE_4372_90D8_8EFEDB896ECB__INCLUDED_)
