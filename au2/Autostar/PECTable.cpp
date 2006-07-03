// PECTable.cpp: implementation of the CPECTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PECTable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPECTable::CPECTable()
{
// set the body type
	SetBodyType(PECTable);

// set initial values
	m_PECTable.Active = (char)0xFF;
	m_PECTable.EndTag = 0;
	m_PECTable.Next.offset = 0;
	m_PECTable.Next.page = 0;
	m_PECTable.Sessions = 1;
	for (int i = 0; i < PECTABLESIZE; i++)
		m_PECTable.Map[i] = (char) 0x80;

}

CPECTable::~CPECTable()
{

}

CPECTable::CPECTable(CPECTable &cpy) : CBodyData(cpy)
{
	SetBodyType(PECTable);
	m_PECTable = cpy.m_PECTable;
}

bool CPECTable::ReadImageData(unsigned char *ptr, int flag)
{

// all of it reads ok	
	m_PECTable = *((PECTableType *)ptr);

	return true;
}	

CBodyData * CPECTable::Create()
{
	CBodyData* obj = new CPECTable();

	return obj;
}


int CPECTable::GetSizeOf()
{
	return sizeof(PECTableType);
}

int CPECTable::GetNumFields()
{
	return 0;
}

void CPECTable::PutImageData(unsigned char *image, int flag)
{
	PECTableType *Amark = NULL;

	// all of this works
	*((PECTableType *)image) = m_PECTable;


}

sfieldDesc * CPECTable::GetFieldDesc(int fIndex)
{
	return NULL;
}


CBodyData * CPECTable::Copy()
{
	return new CPECTable(*this);
}

/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to User Info either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CPECTable::Serialize(CPersist& per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + GetSizeOf() + 20));

	// copy the user info data into the space just provided
		per << m_key;
		*((PECTableType *)per.m_indexPtr) = m_PECTable;

	// move the index up
		per.IncrementIndex(sizeof(m_PECTable));

	}
	else
	{
	// get the user data from the buffer
		per >> m_key;
		m_PECTable = *((PECTableType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_PECTable));

	}

}


/////////////////////////////////////////////
//
//	Name		:SetActiveFlag
//
//	Description :This will set the active flag
//
//  Input		:bool
//
//	Output		:None
//
////////////////////////////////////////////
void CPECTable::SetActiveFlag(bool flag)
{
	if (flag)
		m_PECTable.Active = (char) 0xFF;
	else
		m_PECTable.Active = (char) 0x00;
}

/////////////////////////////////////////////
//
//	Name		:SetActiveFlag
//
//	Description :This will set the active flag
//
//  Input		:bool
//
//	Output		:None
//
////////////////////////////////////////////
bool CPECTable::GetActiveFlag()
{
	if (m_PECTable.Active != (char) 0xFF)
		return false;
	else
		return true;
}