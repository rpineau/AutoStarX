// BodyData.h: interface for the CBodyData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BodyData_H__F1C4EFE1_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_)
#define AFX_BodyData_H__F1C4EFE1_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// instead of including this
 #include "autoglob.h"
// these are defined instead

#include "persist.h"

#define		NEW_ENTRY	"AAA New"

typedef enum { OK, RANGEERROR, INVALIDTYPE, NOT_MODIFIABLE } RANGESTAT;

typedef enum { KDJ_NONE = 0, KDJ_EPOCH_DATE = 1, KDJ_LO_PREC = 2, KDJ_ZERO_BASED = 4 } KlugeDuJour;

typedef enum{
	Asteroid,		//0
	Comet,			//1
	Satellite,		//2
	UserObj,		//3
	LandMark,		//4
	Tour,			//5
	UserObj20,		//6
	UserObj21,
	UserObj22,
	UserObj23,
	UserObj24,
	UserObj25,
	UserObj26,
	UserObj27,
	UserObj28,
	UserObj29,
	UserObj30,
	UserObj31,
	UserObj32,
	UserObj33,
	UserObj34,
	UserObj35,
	UserObj36,
	UserObj37,
	UserObj38,
	UserObj39,
	BodyTypeMax,
	Hip,
	HipXref,
	GCVS,
	UserInfo,
	SiteInfo,
	PECTable,
	All
} BodyType;


struct date {
	char	Epoch_mm;
	char	Epoch_dd;
	short	Epoch_yy;
};

struct jdate {
	short	EpochYear;
	float	EpochDay;
};



typedef enum {	KEY_TYPE,
				STRING_TYPE, 
				DATE_TYPE, 
				JDATE_TYPE, 
				FLOAT_TYPE,
				DOUBLE_TYPE,
				RA_TYPE,
				PREC_RA_TYPE,
				LONG_RA_TYPE,
				DEC_TYPE, 
				AZ_TYPE,
				PREC_DEC_TYPE,
				LONG_DEC_TYPE,
				SHORT_TYPE,
				BOOL_TYPE,
				LONG_TYPE,
				LONG_LONG_TYPE,
				TIMEZONE_TYPE} DataType;

struct sfieldDesc {
	CString		Label;
	DataType	Type;
	CString		Format;
	CString		*StringPtr;
	date		*DatePtr;
	jdate		*JDatePtr;
	float		*FloatPtr;
	double		*DoublePtr;
	bool		*BoolPtr;
	long		*LongPtr;
	double		HiLimit;
	double		LoLimit;
	bool		Modifiable;
	short		*ShortPtr;
	short		fieldSize;
	char		*CharPtr;
	sfieldDesc();
};

class CBodyData
{
public:
	long ConvertMediumImage(unsigned char *image);
	void PutMediumImage(long data, unsigned char *ptr);
	long ConvertLongImage(unsigned char *image);
	virtual bool IsCustom();
	virtual bool IsExtended();
	virtual bool IsDynamic();
	virtual CBodyData &operator=(CBodyData &Rdata);
	void PutLongImage(unsigned long int data, unsigned char *ptr);
	void PutDoubleImage(double data, unsigned char *ptr);
	virtual void * GetFieldDataDirect(int field);
	void ParseBackEOL(CPersist &per);
	void ParseToEOL(CPersist &per);
	virtual int * GetImportTable();
	bool FieldOutofRange(sfieldDesc *desc, void *ptr=NULL);
	virtual int CheckFieldRanges();
	void PutFloatImage(float data, unsigned char * ptr);
	void PutWordImage(word data, unsigned char *ptr);
	void PutPoolImage(poolposition pool, unsigned char *ptr);
	bool NumStr(CString &Str, int &Num);
	bool Compare(CBodyData *data, bool Asc, int field1, int field2, int field3);
	word ConvertWordImage(unsigned char *ptr);
	float ConvertFloatImage(unsigned char *ptr);
	poolposition ConvertPoolImage(unsigned char *image);
	BodyType GetBodyType();
	void SetBodyType(BodyType type);
	CBodyData(CBodyData &cpy);
	virtual ~CBodyData();
	virtual void SetKey(CString key);
	virtual CString GetKey(bool replaceCommas = false);
	virtual CString GetFieldLabel(int i);
	virtual CString GetFieldData(int i);
	virtual RANGESTAT SetFieldData(int i, CString data, bool import=false);
	virtual CString GetFieldRangeHigh(int i);
	virtual CString GetFieldRangeLow(int i);
	virtual bool ReadTxtData(CPersist& per);

	virtual void SetPosition(const poolposition& pos);
	virtual const poolposition &GetPosition();
	virtual void SetActiveFlag(bool flag);
	virtual bool GetActiveFlag();
	virtual void ReadData(unsigned char *data);

	virtual CBodyData * Create() = 0;
	virtual int GetSizeOf() = 0;
	virtual int GetNumFields() = 0;
	virtual void PutImageData(unsigned char *image, int flag = KDJ_NONE) = 0;
	virtual sfieldDesc * GetFieldDesc(int fIndex) = 0;
	virtual bool ReadImageData(unsigned char *ptr, int flag = KDJ_NONE) = 0;
	virtual CBodyData * Copy() = 0;
	virtual void Serialize(CPersist& per) = 0;
	virtual bool ReadRomData(CPersist &per);

protected:
	int ParseField(CPersist &per, int indx, int *iTbl);
	CString m_key;
	CBodyData();

private:
	CString m_parseFile;
	BodyType m_BodyType;
};

#endif // !defined(AFX_BodyData_H__F1C4EFE1_E1B6_11D4_A1FC_00E098887D2E__INCLUDED_)
