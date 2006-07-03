// Persist.h: interface for the CPersist class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSIST_H__DA3DBAD7_522C_4CF6_8A55_26ECA70911F8__INCLUDED_)
#define AFX_PERSIST_H__DA3DBAD7_522C_4CF6_8A55_26ECA70911F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Autoglob.h"


typedef unsigned short word;

typedef struct { 
    byte msb;
    word lsw;
    } medium;



class CPersist  
{
public:
	static CString Abbreviate(CString text,int length);
	static int RemoveBadChars(IN OUT CString &input);
	static CString ToString(int i);
	CString m_filePath;
	void SetIndex(DWORD index);
	void ResetIndex();
	CString m_fileName;
	CString m_fileExtention;
typedef enum{READCOMPLETE, FILEERROR, WRONGFILETYPE} FileStat;
	CPersist();
	CPersist(bool stor);
	DWORD m_dataReadCnt;
	FileStat ReadFile(CString filename);
	void SetPointer(void *ptr);
	void FreeBuffer();
	void IncrementIndex(int size);
	void * m_indexPtr;
	bool m_storing;
	int m_version;
	DWORD m_dataIndex;
	void * m_dataPtr;
	CPersist(void *ptr, bool stor);
	virtual ~CPersist();
	CPersist &operator >>(CString &str);
	CPersist &operator <<(CString &str);

	CPersist &operator <<(const word &dat);
	CPersist &operator >>(word &dat);

	CPersist &operator <<(const short &dat);
	CPersist &operator >>(short &dat);

	CPersist &operator <<(const char &dat);
	CPersist &operator >>(char &dat);

	CPersist &operator <<(const bool &data);
	CPersist &operator >>(bool &data);

	CPersist &operator <<(const poolposition &data);
	CPersist &operator >>(poolposition &data);

	CPersist &operator <<(const char* data);
	CPersist &operator >>(char* data);

	CPersist &operator <<(const long &dat);
	CPersist &operator >>(long &dat);

	CPersist &operator <<(const medium &dat);
	CPersist &operator >>(medium &dat);

private:
};

#endif // !defined(AFX_PERSIST_H__DA3DBAD7_522C_4CF6_8A55_26ECA70911F8__INCLUDED_)
