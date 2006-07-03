// Persist.cpp: implementation of the CPersist class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Persist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPersist::CPersist()
{
	m_dataPtr = NULL;
	m_indexPtr = NULL;
	m_dataIndex = 0;
	m_storing = false;
	m_version = 0;
	m_dataReadCnt = 0;

}

CPersist::CPersist(bool stor)
{
	m_dataPtr = NULL;
	m_indexPtr = NULL;
	m_dataIndex = 0;
	m_storing = stor;
	m_version = 0;
	m_dataReadCnt = 0;
}

CPersist::CPersist(void *ptr, bool stor)
{
	m_dataPtr = ptr;
	m_indexPtr = NULL;
	m_dataIndex = 0;
	m_storing = stor;
	m_version = 0;
	m_dataReadCnt = 0;
}

CPersist::~CPersist()
{

}

/////////////////////////////////////////////
//
//	Name		:operator >>
//
//	Description :Takes the current index position and assigns it
//				 to the passed CString
//
//  Input		:str
//
//	Output		:this
//
////////////////////////////////////////////
CPersist& CPersist::operator >>(CString &str)
{
word len;
void *tmp;

// get the length of the string
	len = *(WORD *)((char *)m_indexPtr);
	IncrementIndex(sizeof(word));

// allocate space for a temporary buffer
	tmp = malloc(len + 2);

// copy the data into the temp area
	memcpy(tmp, (char *)m_indexPtr, len);

// put a null at the end and assign it to the CString
	((char *)tmp)[len] = 0;
	str = (char *)tmp;

// get rid of the temp buffer
	free(tmp);

// reposition the index
	IncrementIndex(len);

	return *this;
}

/////////////////////////////////////////////
//
//	Name		:operator <<
//
//	Description :Takes the passed CString and stores it
//				 to the current index position 
//
//  Input		:str
//
//	Output		:this
//
////////////////////////////////////////////
CPersist& CPersist::operator <<(CString &str)
{
word len;
char *tmp;

	// get and store the length first
	len = str.GetLength();
	operator <<(len);

	// get the CString as a string
	tmp = str.GetBuffer(len);

	// copy it to the index pointer position
	memcpy(((char *)m_indexPtr), tmp, len);

	// increment the index
	IncrementIndex(len);

	return *this;
}

CPersist& CPersist::operator <<(const word &data)
{
	*((word *)m_indexPtr) = data;
	IncrementIndex(sizeof(word));
	
	return *this;
}

void CPersist::IncrementIndex(int size)
{
	m_dataIndex += size;
	m_indexPtr = (char *)m_dataPtr + m_dataIndex;


}

CPersist& CPersist::operator >>(word &data)
{
	data = *((word *)m_indexPtr);
	IncrementIndex(sizeof(word));

	return *this;
}

void CPersist::FreeBuffer()
{
	free(m_dataPtr);
	m_dataIndex = 0;
	m_dataPtr = NULL;
	m_indexPtr = NULL;
}

void CPersist::SetPointer(void *ptr)
{
	m_dataPtr = ptr;
	m_indexPtr = m_dataPtr;
	m_dataIndex = 0;
}

/////////////////////////////////////////////
//
//	Name		:ReadFile
//
//	Description :Reads the entire file into memory it allocates and returns
//
//  Input		:Filename
//
//	Output		:file stat, file data
//
////////////////////////////////////////////
CPersist::FileStat CPersist::ReadFile(CString filename)
{
	DWORD dataLen;
	CFile file;
	CFileStatus	fStat;


	// check if the file is there first
	if (!file.GetStatus(filename, fStat))
		return FILEERROR;

// first try to open the file
	try {
		file.Open(filename, CFile::modeRead);
		dataLen = file.GetLength();

		// allocate space for the data
		m_dataPtr = malloc(dataLen + 10);

		// read all the data into the buffer
		m_dataReadCnt = file.Read(m_dataPtr, dataLen);

		// initialize perisistance
		SetPointer(m_dataPtr);
		ResetIndex();

		// set the file name, extention, and path
		m_fileName = filename;
		m_fileExtention = filename.Right(3);
		m_fileExtention.MakeLower();
		m_filePath = filename.Left(filename.ReverseFind('\\') + 1);
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
		return FILEERROR;
	}

	return READCOMPLETE;
}

void CPersist::ResetIndex()
{
	m_dataIndex = 0;
	m_indexPtr = m_dataPtr;
}

void CPersist::SetIndex(DWORD index)
{
	m_dataIndex = index;
	m_indexPtr = (unsigned char *)m_dataPtr + m_dataIndex;
}

CPersist& CPersist::operator >>(short &data)
{
	data = *((short *)m_indexPtr);
	IncrementIndex(sizeof(data));

	return *this;

}

CPersist & CPersist::operator <<(const short &data)
{
	*((short *)m_indexPtr) = data;
	IncrementIndex(sizeof(data));
	
	return *this;

}

CPersist& CPersist::operator >>(char &data)
{
	data = *((char *)m_indexPtr);
	IncrementIndex(sizeof(data));

	return *this;

}

CPersist & CPersist::operator <<(const char &data)
{
	*((char *)m_indexPtr) = data;
	IncrementIndex(sizeof(data));
	
	return *this;

}

CPersist & CPersist::operator >>(bool &data)
{
	data = *((bool *)m_indexPtr);
	IncrementIndex(sizeof(data));

	return *this;
}

CPersist & CPersist::operator <<(const bool &data)
{
	*((bool *)m_indexPtr) = data;
	IncrementIndex(sizeof(data));
	
	return *this;

}



CString CPersist::ToString(int i)
{
	char buffer[10];

	itoa(i, buffer, 10);

	return (CString) buffer;
}


CPersist & CPersist::operator >>(poolposition &data)
{
	data.page = *((unsigned char *)m_indexPtr);
	IncrementIndex(sizeof(data.page));

	data.offset = *((unsigned short *)m_indexPtr);
	IncrementIndex(sizeof(data.offset));

	return *this;
}

CPersist & CPersist::operator <<(const poolposition &data)
{
	*((unsigned char *)m_indexPtr) = data.page;
	IncrementIndex(sizeof(data.page));

	*((unsigned short *)m_indexPtr) = data.offset;
	IncrementIndex(sizeof(data.offset));

	return *this;
}


CPersist & CPersist::operator >>(char* data)
{
	lstrcpy(data,((char *)m_indexPtr));
	IncrementIndex(sizeof(data));

	return *this;
}

CPersist & CPersist::operator <<(const char* data)
{
	lstrcpy(((char *)m_indexPtr),data);
	IncrementIndex(sizeof(data));
	
	return *this;
}


CPersist& CPersist::operator >>(long &data)
{
	data = *((long *)m_indexPtr);
	IncrementIndex(sizeof(data));

	return *this;

}

CPersist & CPersist::operator <<(const long &data)
{
	*((long *)m_indexPtr) = data;
	IncrementIndex(sizeof(data));
	
	return *this;

}


CPersist & CPersist::operator >>(medium &data)
{
	data.msb = *((unsigned char *)m_indexPtr);
	IncrementIndex(sizeof(data.msb));

	data.lsw = *((unsigned short *)m_indexPtr);
	IncrementIndex(sizeof(data.lsw));

	return *this;
}

CPersist & CPersist::operator <<(const medium &data)
{
	*((unsigned char *)m_indexPtr) = data.msb;
	IncrementIndex(sizeof(data.msb));

	*((unsigned short *)m_indexPtr) = data.lsw;
	IncrementIndex(sizeof(data.lsw));

	return *this;
}

/////////////////////////////////////////////
//
//	Name		:RemoveBadChars
//
//	Description :removes potentially harmful character
//				 from a string, such as commas, colons
//				 and pound signs
//
//  Input		:CString reference
//
//	Output		:number of characters removed
//
////////////////////////////////////////////
int CPersist::RemoveBadChars(CString &input)
{
	int removed = 0;

	// do not allow ,'s or :'s or #'s
	const int numBad = 3;
	char bad[numBad] = {':', ',', '#'};

	for (int i = 0; i < numBad; i++)
		removed += input.Remove(bad[i]);

	return removed;
}

/////////////////////////////////////////////
//
//	Name		:Abbreviate
//
//	Description :reduces the length of a string
//				 by removing selected characters
//
//  Input		:CString reference, int length
//
//	Output		:abbreviated length
//
////////////////////////////////////////////
CString CPersist::Abbreviate(CString text, int length)
{
	// first check if it is already short enough
	if (text.GetLength() <= length)
		return text;

	const int numChars = 9;
	char removeChars[numChars] = {' ', '-', ':', '_', 'a', 'e', 'i', 'o', 'u'};

	// remove the characters one at a time, until the
	// length spec is reached

	for (int i = 0; i < numChars; i++)
	{
		text.Remove(removeChars[i]);
		if (text.GetLength() <= length)
			return text;
	}

	// if that didn't work, just cut it off
	text = text.Left(length);

	return text;
}
