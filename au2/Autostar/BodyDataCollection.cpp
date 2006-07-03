// BodyDataCollection.cpp: implementation of the BodyDataCollection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BodyDataCollection.h"
#include "BodyDataMaker.h"
#include "persist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBodyDataCollection::CBodyDataCollection()
{
	m_factory = new CBodyDataMaker;
	m_header = "Update Nouveau ";
//	m_fileVersion = 1;		// original version released 9/01
//	m_fileVersion = 2;		// changed 3/19/02 to support higher precision user objects
	m_fileVersion = 3;		// changed 3/29/02 to support extended tour format
}

CBodyDataCollection::~CBodyDataCollection()
{
	Clear();		// delete all the body data first
	delete m_factory;
}

/////////////////////////////////////////////
//
//	Name		: Clear
//
//	Description : Removes all the body data of the type specified
//
//  Input		: None
//
//	Output		: None
//
////////////////////////////////////////////
void CBodyDataCollection::Clear(BodyType type)
{
	CBodyData	*data;

	if (type != All)
	{
		// delete all the user data objects of this type
		POSITION pos = GetHeadPosition(type);
		while(pos != NULL)
		{
			data = GetNext(pos, type);
			delete data;
			m_userList.RemoveAt(m_userList.Find(data));
		}
	}
	else	// delete all bodies as fast as possible
	{
		POSITION pos = m_userList.GetHeadPosition();
		while(pos != NULL)
		{
			delete m_userList.GetNext(pos);
		}
		m_userList.RemoveAll();
	}

	
}

/////////////////////////////////////////////
//
//	Name		: Add collection
//
//	Description : Add/Update this collection of data pointers to the list
//                These references will be copied and the copies
//				  are now owned by this collection
//                and will be deleted when its destructor is called. 
//
//  Input		: CBodyDataCollection reference 
//				: BodyType type default is All means to copy all the body types
//				: POSITION from	default is NULL means to copy the whole collection
//				: POSITION to
//
//	Output		: none
//
////////////////////////////////////////////
void CBodyDataCollection::Add(CBodyDataCollection &src, BodyType type, POSITION at, POSITION from, POSITION to)
{
	POSITION pos;
	CBodyData *bodyData;

	if (from == NULL)		// if no start then get the start of the list
		pos = src.GetHeadPosition(type);
	else
		pos = from;

	while(pos != to && pos != NULL) // check for NULL in case the 'to' position is bad
	{
		bodyData = src.GetNext(pos, type);
		bodyData = bodyData->Copy();  // make a copy of the body data
		at = Add(bodyData, at);									// add it to my list
	}
}


/////////////////////////////////////////////
//
//	Name		: Add
//
//	Description : Add/Update this body data pointer to the list
//                This reference is now owned by this collection
//                and will be deleted when its destructor is called. 
//
//  Input		: CBodyData pointer. This has to be a pointer in the heap.
//				  pos , Position to place new or move entry. If its NULL then
//				        New entry is added to the end
//
//	Output		: position for next entry
//
////////////////////////////////////////////
POSITION CBodyDataCollection::Add(CBodyData *data, POSITION pos)
{
	POSITION	delPos = NULL;
	POSITION	retPos = NULL;
	POSITION	fndPos = NULL;
	CBodyData	*delData = NULL;

	// if this POINTER is not there
	if (!(fndPos = m_userList.Find(data)))	
	{

		delData = Find(data->GetKey());	// delete any duplicate key first, if its there
		if (delData)
			delPos = m_userList.Find(delData);

		// check if they want it somewhere specific
		if (pos)
		{
			retPos = m_userList.InsertAfter(pos, data);
			if (delPos)
			{
				m_userList.RemoveAt(delPos);
				delete delData;
			}
		}
		
		// check if there's one to delete and then re-insert
		else if (delPos)
		{
			retPos = m_userList.InsertAfter(delPos, data);
			m_userList.RemoveAt(delPos);
			delete delData;
		}

		// else just add it to the end
		else
			m_userList.AddTail(data);	// then add it.
	}

	// else its already there so check if they must want it moved
	else if (pos)
	{
		retPos = m_userList.InsertAfter(pos, data);	// put it in the new position
		m_userList.RemoveAt(fndPos);		// remove the original reference
	}

	return retPos;


}


/////////////////////////////////////////////
//
//	Name		:operator=
//
//	Description :This will copy all the data in the collection sent to it.
//
//  Input		:CBodyDataCollection &
//
//	Output		:this
//
////////////////////////////////////////////
CBodyDataCollection & CBodyDataCollection::operator =(CBodyDataCollection &right)
{

// clear whats in here first	
	Clear();

// put the new stuff in
	Add(right);

	return *this;
}


/////////////////////////////////////////////
//
//	Name		:operator +=
//
//	Description :This will add the collection passed to it.
//
//  Input		:CBodyDataCollection &right
//
//	Output		:this
//
////////////////////////////////////////////
CBodyDataCollection & CBodyDataCollection::operator +=(CBodyDataCollection &right)
{

// put the new stuff in
	Add(right);

	return *this;
}
 
/////////////////////////////////////////////
//
//	Name		: Delete by name
//
//	Description : Delete this body by name and delete the data
//
//  Input		: Name
//
//	Output		: position - if it's found, 
//                       null if not found.
//
////////////////////////////////////////////
POSITION CBodyDataCollection::Delete(CString name)
{
	CBodyData *temp;
	POSITION pos = NULL;

	if (temp = Find(name))		// if its there
	{
		pos = Remove(temp);
		delete temp;			// delete the object
	}

	return pos;
}

/////////////////////////////////////////////
//
//	Name		: Remove by body data
//
//	Description : Removes the reference to body data without deleteing the data
//
//  Input		: CBodyData pointer
//
//	Output		: Position if found, NULL if not found
//
////////////////////////////////////////////
POSITION CBodyDataCollection::Remove(CBodyData *data)
{
	POSITION pos;
	if (pos = m_userList.Find(data)) // if it's there
		m_userList.RemoveAt(pos);			// then delete it

	return pos;

}


/////////////////////////////////////////////
//
//	Name		: Find
//
//	Description : Find the Body data by key
//
//  Input		: key
//
//	Output		: CBodyData pointer if found.
//                Null if not found
//
////////////////////////////////////////////
CBodyData *CBodyDataCollection::Find(CString key)
{
	CString		tKey;		// Test Key

	key = key.Left(16);		// only compare the first 16 characters
	key.TrimRight();			// and skip the spaces

	POSITION pos = m_userList.GetHeadPosition();

// go through the list looking for the name
	while(pos != NULL)
	{
		CBodyData *data = m_userList.GetNext(pos);
		tKey = data->GetKey();
		tKey = tKey.Left(16);
		tKey.TrimRight();
		if (tKey == key)
			return data;
	}

// went through the whole list
	return NULL;
}

FileStat CBodyDataCollection::Export(CString filename, BodyType type)
{
		return FILEERROR;
}



/////////////////////////////////////////////
//
//	Name		:GetCount
//
//	Description :Returns the count of data of the BodyType passed to it.
//
//  Input		:BodyType - All is the default parameter
//
//	Output		:Body Count
//
////////////////////////////////////////////
int CBodyDataCollection::GetCount(BodyType type)
{
	POSITION pos;
	int		count = 0;

// if they want all the types then return the total count
	if (type == All)
		return m_userList.GetCount();

// else count the number of the type they want
	else
	{
		pos = GetHeadPosition(type);
		while (pos)
		{
			GetNext(pos, type);
			count++;
		}

		return count;
	}
}

/////////////////////////////////////////////
//
//	Name		:GetHeadPosition
//
//	Description :Will return the position of the first data of the 
//				 body type passed to it.
//
//  Input		:BodyType
//
//	Output		:position
//
////////////////////////////////////////////
POSITION CBodyDataCollection::GetHeadPosition(BodyType type)
{
	CBodyData *rBody = NULL;
	POSITION pos = m_userList.GetHeadPosition();

	// if looking for a particular type
	if (type != All)
	{
		while (pos && (rBody = m_userList.GetNext(pos))->GetBodyType() != type);

		// if we found one then backup one
		if (pos)
			m_userList.GetPrev(pos);
		// check if it was the last one in the list
		else if (rBody && rBody->GetBodyType() == type)
			pos = m_userList.GetTailPosition();
	}

	return pos;
}

/////////////////////////////////////////////
//
//	Name		:GetNext
//
//	Description :Will return the next CBodyData pointer of the specified body
//				 type. Will also look for the next one of that type and then back
//				 up one so it can be found on the next call. The reason for this
//				 is, if this call finds the last one of that BodyType the position
//				 variable 'pos' will be set to NULL.
//
//  Input		:pos, type
//
//	Output		:CBodyData pointer
//
////////////////////////////////////////////
CBodyData *CBodyDataCollection::GetNext(POSITION &pos, BodyType type)
{
	CBodyData *rBody = NULL, *nBody = NULL;

//if they want all types then send them the next one
	if (type == All)						
		return m_userList.GetNext(pos);

// else return only the type they want or the end of the list
	else{
		while(pos && (rBody = m_userList.GetNext(pos))->GetBodyType() != type);

	// if we found one then try to find another one
		if (pos)
		{
			while(pos && (nBody = m_userList.GetNext(pos))->GetBodyType() != type);
			if (pos)
				m_userList.GetPrev(pos);	// backup one so we find it in the next call
			// check if it was the last one in the entire list
			else if (nBody && nBody->GetBodyType() == rBody->GetBodyType() && nBody->GetKey() != rBody->GetKey())
				pos = m_userList.GetTailPosition();
		}
		return rBody;
	}
}

/////////////////////////////////////////////
//
//	Name		:GetPrev
//
//	Description :this will get the previous body in the list.
//				 this only works for bodytype all right now.
//
//  Input		:pos, bodytype
//
//	Output		:CBodyData pointer
//
////////////////////////////////////////////
CBodyData * CBodyDataCollection::GetPrev(POSITION &pos, BodyType type)
{
	return m_userList.GetPrev(pos);
}

/////////////////////////////////////////////
//
//	Name		:GetAt
//
//	Description :returns the body data for the position passed
//
//  Input		:position
//
//	Output		:CBodyData pointer
//
////////////////////////////////////////////
CBodyData *CBodyDataCollection::GetAt(POSITION pos)
{
	return m_userList.GetAt(pos);
}

/////////////////////////////////////////////
//
//	Name		:Import
//
//	Description :Calls the standard import routine and passes the import count back as count.
//
//  Input		:File name, BodyType
//
//	Output		:FileStat, Bodies
//
////////////////////////////////////////////
FileStat CBodyDataCollection::Import(CString filename, BodyType type, int &count, CBodyDataCollection::importOption Option)
{
	FileStat	stat;
	stat = Import(filename, type, Option);

	count = m_impCount;
	return stat;
}

/////////////////////////////////////////////
//
//	Name		:Import
//
//	Description :Reads txt files and the old Libxxx.rom files and adds them to the collection
//
//  Input		:File name, BodyType
//
//	Output		:FileStat, Bodies
//
////////////////////////////////////////////
FileStat CBodyDataCollection::Import(CString filename, BodyType type, CBodyDataCollection::importOption Option)
{
	DWORD dataIndex = 0;
	CBodyData *dataObject = NULL;
	FileStat	fStat;
	CPersist	per(NULL, false);
	bool		bStat;
	CString		sTmp;

	// zero the import count
	m_impCount = 0;

	if (type == All)
		return BADBODYTYPE;

// first read the whole file
	// this will allocate a data buffer in m_dataPtr and set m_dataReadCnt to how much was read
	if ((fStat = (FileStat)per.ReadFile(filename)) != READCOMPLETE) 
		return fStat;

	// loop through the whole file
		// if enough data read then create the data object and send the data to it
	while (per.m_dataIndex < (int)per.m_dataReadCnt && m_impCount < 30000)
	{
		// make the empty data object
		dataObject = m_factory->Make(type);
		
		// check for rom file otherwise its a txt file
		if (per.m_fileExtention == "rom")
			bStat = dataObject->ReadRomData(per);
		else
			bStat = dataObject->ReadTxtData(per);

		// if it reads ok and
		// the range of all the fields is ok
		if (bStat && (dataObject->CheckFieldRanges() == 0))
		{
		// then add the data to the list
			dataObject->SetActiveFlag(true);	// set all to active

			// if this is not an un-named entry
			if(dataObject->GetKey() != NEW_ENTRY)
			{
				// then check the import options
				// do it fast don't check for duplicates
				if (Option == fast)
				{
					m_userList.AddTail(dataObject);
					m_impCount++;
					dataObject = NULL;			// it was used
				}
				// if there's already one there then replace it with this one
				// otherwise add it as a new one
				else if (Option == dupchk)
				{
					Add(dataObject);
					m_impCount++;
					dataObject = NULL;			// it was used
				}
				// if it's 
				else if (Option == update)
				{
					if (Find(dataObject->GetKey()))
					{
						Add(dataObject);
						m_impCount++;
						dataObject = NULL;			// it was used
					}
				}
				else
					return BADOPTION;

			}
			else
			{
				// otherwise make sure it's unique
				int		t = 1;
				while(Find(dataObject->GetKey()) != 0)
				{
					// add a number to the end and keep incrementing it till it's unique
					sTmp.Format("%s%d",dataObject->GetKey(), t);
					dataObject->SetKey(sTmp);
					t++;
				}
				m_userList.AddHead(dataObject);	// then add it to the top
				dataObject = NULL;			// it was used
				m_impCount++;
			}
		}

	// delete unused data object
	if (dataObject)
		delete dataObject;

	}
	// free the file buffer
	per.FreeBuffer();
	return READCOMPLETE;
}



/////////////////////////////////////////////
//
//	Name		:SortBy
//
//	Description :Will sort the collection in Ascending order if Asc is true, or
//				 Decending if false, of the body type, by the fields. 
//				 Field1 primary key, Field2 secondary ...
//
//  Input		:Asc True = Ascending False = Decending
//				 Field1 = Primary Key
//				 Field2 = Secondary Key
//				 Field3 = Third Key
//
//	Output		:True = OK
//
////////////////////////////////////////////
bool CBodyDataCollection::SortBy(bool Asc, BodyType type, int Field1, int Field2, int Field3)
{
	int	listCount = 0;	// count of sort types
	int otherCount = 0; // count of other types
	int totalCount = m_userList.GetCount();
	CBodyData *tmp;
	CBodyData **listArray = (CBodyData **)malloc(sizeof(CBodyData*) * totalCount);
	CBodyData **listArray2 = (CBodyData **)malloc(sizeof(CBodyData*) * totalCount);

	// Split the list into the body type to sort and the rest
	POSITION pos = m_userList.GetHeadPosition();
	while(pos)
	{
		tmp = m_userList.GetNext(pos);
		if (tmp->GetBodyType() == type)
			listArray[listCount++] = tmp;
		else
			listArray2[otherCount++] = tmp;
	}


// Heap sort the array
    for( int i = 1; i < listCount; i++ )
		PercolateUp( i, listArray, Asc, Field1, Field2, Field3);
	
	for( i = listCount - 1; i > 0; i--)
	{
		tmp = listArray[0];
		listArray[0] = listArray[i];
		listArray[i] = tmp;

		PercolateDown( i - 1, listArray, Asc, Field1, Field2, Field3);
	}



	// delete the list and re initialize it with the array
	m_userList.RemoveAll();
	// put in the sorted types
	for (i = 0; i< listCount; i++)
		m_userList.AddTail(listArray[i]);
	// put in the rest
	for (i = 0; i< otherCount; i++)
		m_userList.AddTail(listArray2[i]);

	free(listArray);
	free(listArray2);

	return true;
}

//***********************************************************************
// Function:
//
//     PercolateUp()
//
// Purpose:
//
//     Converts elements into a "heap" with the largest element at the
//     top (see the diagram above).
//
// Parameters:
//
//     iMaxLevel - Specifies the list element being moved.
//     pList     - A pointer to the list to be sorted.
//
// Returns:
//
//     void
//
// History:
//
//   Date   Comment                                           Initials
// ======== ================================================= ========
// 10/12/93 Created                                             JKK
//***********************************************************************

void CBodyDataCollection::PercolateUp(int iMaxLevel, CBodyData **pList, bool Asc, int Field1, int Field2, int Field3)
{
    int i = iMaxLevel, iParent;
	CBodyData *tmp;

    // Move the value in pList[iMaxLevel] up the heap until it has
    // reached its proper node (that is, until it is greater than either
    // of its child nodes, or until it has reached 1, the top of the heap).

    while( i )
    {
        iParent = i / 2;    // Get the subscript for the parent node

        if( pList[iParent]->Compare(pList[i], Asc, Field1, Field2, Field3))
        {
            // The value at the current node is bigger than the value at
            // its parent node, so swap these two array elements.
            //
            tmp = pList[iParent];
            pList[iParent] = pList[i];
            pList[i] = tmp;
            i = iParent;

        }
        else
            // Otherwise, the element has reached its proper place in the
            // heap, so exit this procedure.
            break;
    }
}

//***********************************************************************
// Function:
//
//     PercolateDown()
//
// Purpose:
//
//     Converts elements to a "heap" with the largest elements at the
//     bottom. When this is done to a reversed heap (largest elements at
//     top), it has the effect of sorting the elements.
//
// Parameters:
//
//     iMaxLevel - Specifies the list element being moved.
//     pList     - A pointer to the list to be sorted.
//
// Returns:
//
//     void
//
// History:
//
//   Date   Comment                                           Initials
// ======== ================================================= ========
// 10/12/93 Created                                             JKK
//***********************************************************************

void CBodyDataCollection::PercolateDown(int iMaxLevel, CBodyData **pList, bool Asc, int Field1, int Field2, int Field3)
{
    int iChild, i = 0;
	CBodyData *tmp;
	CString Stmp;

    // Move the value in pList[0] down the heap until it has reached
    // its proper node (that is, until it is less than its parent node
    // or until it has reached iMaxLevel, the bottom of the current heap).

    while( TRUE )
    {
        // Get the subscript for the child node.
        iChild = 2 * i;

        // Reached the bottom of the heap, so exit this procedure.
        if( iChild > iMaxLevel )
            break;

        // If there are two child nodes, find out which one is bigger.
        if( iChild + 1 <= iMaxLevel )
        {
            if( pList[iChild]->Compare(pList[iChild + 1], Asc, Field1, Field2, Field3 ))
                iChild++;
        }

        if( i != iChild && pList[i]->Compare(pList[iChild], Asc, Field1, Field2, Field3 ))
        {
            // Move the value down since it is still not bigger than
            // either one of its children.
            tmp = pList[i];
            pList[i] = pList[iChild];
            pList[iChild] = tmp;

            i = iChild;
        }
        else
            // Otherwise, pList has been restored to a heap from 1 to
            // iMaxLevel, so exit.
            break;
    }
}

/////////////////////////////////////////////
//
//	Name		:GetNumFields
//
//	Description :Returns the number of fields for the body type passed.
//
//  Input		:BodyType
//
//	Output		:Number of Data Fields
//
////////////////////////////////////////////
int CBodyDataCollection::GetNumFields(BodyType type)
{
CBodyData *temp;
int howMany;

	POSITION pos = GetHeadPosition(type);
	CBodyData *data = NULL;
	if (pos)
		data = GetNext(pos, type);
	
	// make one and ask it (if one was found, copy that one)
	if (temp = m_factory->Make(type, data))
	{
		howMany = temp->GetNumFields();
		delete temp;
	}
	else
		howMany = 0;


	return howMany;
	
}



/////////////////////////////////////////////
//
//	Name		:GetFieldLabel
//
//	Description :Returns the field label for the index and body type passed
//
//  Input		:index, BodyType
//
//	Output		:Label
//
////////////////////////////////////////////
CString CBodyDataCollection::GetFieldLabel(int i, BodyType type)
{
CBodyData	*temp;
CString		label;

	POSITION pos = GetHeadPosition(type);
	CBodyData *data = NULL;
	if (pos)
		data = GetNext(pos, type);

	if (temp = m_factory->Make(type, data))
	{
		label = temp->GetFieldLabel(i);
		delete temp;
	}
	else
		label = "";

	return label;
}

/////////////////////////////////////////////
//
//	Name		:GetTotalSizeOf
//
//	Description :Returns the total bytes as used in the Autostar of all the bodies of the type passed.
//				 If type is All then the total of all bodies is returned.
//
//  Input		:type
//
//	Output		:Total bytes as would be used in Autostar
//
////////////////////////////////////////////
int CBodyDataCollection::GetTotalSizeOf(BodyType type)
{
int TotalSize = 0;
POSITION pos;

	pos = GetHeadPosition(type);

	while(pos)
		TotalSize += GetNext(pos, type)->GetSizeOf();

	return TotalSize;
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
FileStat CBodyDataCollection::ReadFile(CString filename)
{
	DWORD dataLen;
	CFile file;


// first try to open the file
	try {
		file.Open(filename, CFile::modeRead);
		dataLen = file.GetLength();

		// allocate space for the data
		m_dataPtr = malloc(dataLen);

		// read all the data into the buffer
		m_dataReadCnt = file.Read(m_dataPtr, dataLen);
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
		return FILEERROR;
	}

	return READCOMPLETE;
}


/////////////////////////////////////////////
//
//	Name		:LoadFromFile
//
//	Description :Loads the specified object data from the file
//
//  Input		:
//
//	Output		:
//
////////////////////////////////////////////
FileStat CBodyDataCollection::LoadFromFile(CString FileName, BodyType type)
{
	CBodyData *dataObject;
	FileStat	fStat;
	CString		header;
	word		ver, wTmp;
	BodyType	fileBtype;

	CPersist	per(NULL, false);

// first read the whole file
	// this will allocate a data buffer in m_dataPtr and set m_dataReadCnt to how much was read
	if ((fStat = ReadFile(FileName)) != READCOMPLETE) 
		return fStat;

	// check the size
	if (m_dataReadCnt < (DWORD)m_header.GetLength())
		return WRONGFILETYPE;

	// initialize persistance object
	per.SetPointer(m_dataPtr);

	// read and check the header
	per >> 	header;
	if (header != m_header)
	{
		per.FreeBuffer();
		return WRONGFILETYPE;
	}

	// get the version
	per >> ver;
	per.m_version = ver;

	// loop through the whole file
		// if enough data read then create the data object and send the data to it
	while (per.m_dataIndex < (int)m_dataReadCnt)
	{
		// get the body type
		per >> wTmp;
		fileBtype = (BodyType)wTmp;

		// make an empty data object of the type from the file
		dataObject = m_factory->Make(fileBtype);

		// if we got something then Read its data
		if (dataObject)
		{
			dataObject->Serialize(per);

			// if this is the right type or were set to all
			if (type == All || type == fileBtype)
			{
			// then add the data to the list
				m_userList.AddTail(dataObject);
			}
			else // else get rid of the object
				delete dataObject;
		}
		else	// something went wrong so end it now
			per.m_dataIndex = (int)m_dataReadCnt;

	}
	free(m_dataPtr);
	return READCOMPLETE;
		
}

/////////////////////////////////////////////
//
//	Name		:SaveToFile
//
//	Description :Loads the specified object data from the file
//
//  Input		:Filename, BodyType
//
//	Output		:FileStat
//
////////////////////////////////////////////
FileStat CBodyDataCollection::SaveToFile(CString FileName, BodyType type)
{
	CFile file;
	CPersist	per(NULL, true);
	CBodyData	*body;
	CFileException	e;

// first try to open the file
	if (!file.Open(FileName, CFile::modeReadWrite | CFile::modeCreate, &e))
	{
		e.ReportError();
		return FILEERROR;
	}

// allocate space for the header
	per.SetPointer(malloc(m_header.GetLength() + sizeof(m_fileVersion) + 10));

// put the header in the buffer
	per << m_header;
	per << m_fileVersion;

// write the header to the file and reset the index
	file.Write(per.m_dataPtr, per.m_dataIndex);
	per.FreeBuffer();	

// loop through all the bodies of this type
	POSITION pos = GetHeadPosition(type);

	while(pos)
	{
	// get the next body
		body = GetNext(pos, type);

	// save the type
		per.SetPointer(malloc(sizeof(word) + 5));
		per << (word)body->GetBodyType();
		file.Write(per.m_dataPtr, per.m_dataIndex);
		per.FreeBuffer();


	// serialize it
		body->Serialize(per);

	// save it to disk and free the memory
		file.Write(per.m_dataPtr, per.m_dataIndex);
		per.FreeBuffer();
	}

// all done the file will close when we go out of scope
	return READCOMPLETE;
		
}




POSITION CBodyDataCollection::GetTailPosition(BodyType type)
{
	return m_userList.GetTailPosition();	
}

/////////////////////////////////////////////
//
//	Name		:Update
//
//	Description :This will update the data in this list with the list passed to it.
//				 No new data will be added
//
//  Input		:newData
//
//	Output		:Number updated
//
////////////////////////////////////////////
int CBodyDataCollection::Update(CBodyDataCollection *pData, BodyType pType)
{
	POSITION	newDataPos;
	CBodyData	*newData, *myData;
	int			count=0;

// iterate through the passed list of this body type
	newDataPos = pData->GetHeadPosition(pType);
	while(newDataPos)
	{
		newData = pData->GetNext(newDataPos, pType);
	// search my list for the same key
		if (Find(newData->GetKey()))
		{
		// create a copy of the new data
			myData = newData->Copy();
		// replace my data with the new data
			Add(myData);// this will delete the current one and replace it with the copy
			count++;
		}
	}

	return count;

}
