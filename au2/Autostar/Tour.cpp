// Tour.cpp: implementation of the CTour class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tour.h"
#include "..\AU2.h"	// unfortunately this is necessary to recognize the dlg id
#include "..\ErrorLogDlg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAX_TOUR_LEN	SHRT_MAX

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////
//
//	Name		:Default Constructor
//
//	Description :This will initialize the Tour data
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
CTour::CTour()
{
	m_tourBody = NULL;
	m_length = sizeof(m_TourData);

// set the body type
	SetBodyType(Tour);

// initialize the field Descriptions
	InitFieldDesc();

// initially assume tour is not extended
	m_extended = false;
}

/////////////////////////////////////////////
//
//	Name		:InitFieldDesc
//
//	Description :This will initialize the Field Descriptor table
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
void CTour::InitFieldDesc()
{

	int i = 0;

// Name
	m_fieldDesc[i].Label	= "Name";
	m_fieldDesc[i].Type		= KEY_TYPE;
	m_fieldDesc[i].Format	= "%s";
	m_fieldDesc[i].Modifiable = true;

	// increment
	i++;

// Length
	m_fieldDesc[i].Label	= "Length";
	m_fieldDesc[i].Type		= SHORT_TYPE;
	m_fieldDesc[i].Format	= "%u";
	m_fieldDesc[i].ShortPtr	= (short *)&m_length;
	m_fieldDesc[i].HiLimit  = 65535;
	m_fieldDesc[i].LoLimit	= 0;
	m_fieldDesc[i].Modifiable = false;

	// increment
	i++;

// Extended
	m_fieldDesc[i].Label	= "Extended";
	m_fieldDesc[i].Type		= BOOL_TYPE;
	m_fieldDesc[i].Format	= "%i";
	m_fieldDesc[i].BoolPtr	= &m_extended;
	m_fieldDesc[i].Modifiable = false;

// Initialize Data
	m_TourData.Active		= (char)0xFF;
	m_TourData.body[0]		= 0;
	m_TourData.Endtag		= 0;
	m_TourData.Next.offset	= 0xFFFF;
	m_TourData.Next.page	= 0xFF;

}
/////////////////////////////////////////////
//
//	Name		:Copy constructor
//
//	Description :This will copy the data from the Tour passed to it.
//
//  Input		:Tour to copy
//
//	Output		:None
//
////////////////////////////////////////////
CTour::CTour(CTour &cpy) : CBodyData(cpy)
{
	// copy the basic data
	SetBodyType(Tour);
	m_TourData = cpy.m_TourData;
	m_length = cpy.m_length;


// initialize the field Descriptions
	InitFieldDesc();

// copy extended flag
	m_extended = cpy.m_extended;

	// copy the tour body
	if (cpy.m_tourBody)
	{
		m_tourBody = malloc(m_length);
		memcpy(m_tourBody, cpy.m_tourBody, m_length);
	}
	else
		m_tourBody = NULL;
}

CBodyData &CTour::operator=(CBodyData &Pcpy)
{
// call the one above
	CBodyData::operator =(Pcpy);

// get a pointer to my part of the object
	CTour *cpy = dynamic_cast<CTour *>(&Pcpy);

	if ( &Pcpy != this)
	{

	// initialize the field Descriptions
		InitFieldDesc();

		// copy the basic data
		SetBodyType(Tour);
		m_TourData = cpy->m_TourData;
		m_length = cpy->m_length;


	// initialize the field Descriptions
		InitFieldDesc();

	// clear the previous tour if any
		if (m_tourBody)
			free(m_tourBody);
		m_tourBody = NULL;

	// copy the tour body
		if (cpy->m_tourBody)
		{
			m_tourBody = malloc(m_length);
			memcpy(m_tourBody, cpy->m_tourBody, m_length);
		}
	}

	return *this;
}
/////////////////////////////////////////////
//
//	Name		:Default Destructor
//
//	Description :No dynamic data to delete
//
//  Input		:None
//
//	Output		:None
//
////////////////////////////////////////////
CTour::~CTour()
{
	if (m_tourBody)
		free(m_tourBody); 
}

/////////////////////////////////////////////
//
//	Name		:GetSizeOf
//
//	Description :This will return the size of the data as it is
//				 stored in the autostar.
//
//  Input		:None
//
//	Output		:size of Data
//
////////////////////////////////////////////
int CTour::GetSizeOf()
{
	return m_length;
}

/////////////////////////////////////////////
//
//	Name		:Create
//
//	Description :Makes a new empty Tour.
//				 Used by the factory.
//
//  Input		:None
//
//	Output		:CBodyData pointer to a new CTour
//
////////////////////////////////////////////
CBodyData * CTour::Create()
{
	CBodyData* obj = new CTour;

	return obj;
}

/////////////////////////////////////////////
//
//	Name		:GetActiveFlag
//
//	Description :Returns the state of the active flag;
//
//  Input		:None
//
//	Output		:active flag
//
////////////////////////////////////////////
bool CTour::GetActiveFlag()
{
	bool flag;
	if ((unsigned char)m_TourData.Active == 0xFF)
		flag = true;
	else
		flag = false;

	return flag;
}

/////////////////////////////////////////////
//
//	Name		:SetActiveFlag
//
//	Description :Sets the state of the active flag
//
//  Input		:flag
//
//	Output		:None
//
////////////////////////////////////////////
void CTour::SetActiveFlag(bool flag)
{
	if (flag)
		m_TourData.Active = (char)0xFF;
	else
		m_TourData.Active = 0;
}

/////////////////////////////////////////////
//
//	Name		:GetPosition
//
//	Description :returns the pool position for this Tour
//
//  Input		:None
//
//	Output		:poolposition of this Tour
//
////////////////////////////////////////////
const poolposition & CTour::GetPosition()
{
	return m_TourData.Next;
}

/////////////////////////////////////////////
//
//	Name		:SetPosition
//
//	Description :Sets the poolposition for this Tour
//
//  Input		:poolposition
//
//	Output		:None
//
////////////////////////////////////////////
void CTour::SetPosition(const poolposition &position)
{
	m_TourData.Next = position;
}

/////////////////////////////////////////////
//
//	Name		:ReadData
//
//	Description :This reads the record portion only.
//				 Use ReadImageData to convert the tour body as well.
//
//  Input		:pointer to memory image
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
void CTour::ReadData(unsigned char *ptr)
{
	TourType *Amark = NULL;

// read the struct directly from the pointer
	m_TourData = *((TourType *)(ptr));
	
// fix the stuff that didn't convert
	m_TourData.Next = ConvertPoolImage(ptr);
	m_TourData.Len = ConvertWordImage(&ptr[(int)(&Amark->Len)]);

// set the total length
	m_length = (m_TourData.Len & SHRT_MAX) + sizeof(m_TourData) - 1;

// set the name as the key
	char temp[20];
	strncpy(temp, m_TourData.Name, 16);
	temp[16] = 0;
	m_key = temp;

}

/////////////////////////////////////////////
//
//	Name		:ReadImageData
//
//	Description :This will take a pointer to a Tour data image and convert
//				 it to the TourType structure and store it in the m_TourData
//				 member variable.
//
//  Input		:char *ptr - autostar memory image of User Object data
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CTour::ReadImageData(unsigned char *ptr, int flag)
{
	TourType *Amark = NULL;

// some of it reads ok	
	m_TourData = *((TourType *)ptr);

// fix the stuff that didn't convert
	m_TourData.Next = ConvertPoolImage(ptr);
	m_TourData.Len = ConvertWordImage(&ptr[(int)(&Amark->Len)]);

	// check the high order bit of the length field, it indicates extended tours
	if (m_TourData.Len & SHRT_MIN)
		m_extended = true;

// set the total length
	m_length = (m_TourData.Len & SHRT_MAX) + sizeof(m_TourData) - 1;

// check the length for error
	if (m_length > MAX_TOUR_LEN || m_length < 0)
		return false;

// get memory for the body
	if (m_tourBody)
		free(m_tourBody);		// get rid of any old one first
	m_tourBody = malloc(m_length + 10);

// copy the data and body into the block
	memcpy(m_tourBody, ptr, m_length);

// set the name as the key
	char temp[20];
	strncpy(temp, m_TourData.Name, 16);
	temp[16] = 0;
	m_key = temp;

	return true;
}

/////////////////////////////////////////////
//
//	Name		:GetNumFields
//
//	Description :Returns the total number of data fields.
//
//  Input		:None
//
//	Output		:number of data fields
//
////////////////////////////////////////////
int CTour::GetNumFields()
{
	return MAX_TOUR_FIELDS;	// tell them how many fields
}


/////////////////////////////////////////////
//
//	Name		:Copy
//
//	Description :Creates a new copy of this object and returns
//				 as a CBodyData pointer.
//
//  Input		:None
//
//	Output		:CBodyData pointer
//
////////////////////////////////////////////
CBodyData * CTour::Copy()
{
	return new CTour(*this);
}

/////////////////////////////////////////////
//
//	Name		:GetFieldDesc
//
//	Description :Returns a pointer to the field descriptor.
//
//  Input		:Field Index
//
//	Output		:Field Descriptor pointer
//
////////////////////////////////////////////
sfieldDesc * CTour::GetFieldDesc(int fIndex)
{
	if (fIndex >= 0 && fIndex < MAX_TOUR_FIELDS)
		return &m_fieldDesc[fIndex];
	else
		return NULL;
}

/////////////////////////////////////////////
//
//	Name		:PutImageData
//
//	Description :Puts the body data into the image pointer
//				 formated for the Autostar.
//
//  Input		:Image pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CTour::PutImageData(unsigned char *image, int flag)
{

	TourType *Amark = NULL;

// set the key as the name
	strncpy(m_TourData.Name, GetKey(true).GetBuffer(20), 16);

// copy the data and body into the image
	memcpy(image, m_tourBody, m_length);

// copy the structure part
	memcpy(image, &m_TourData, sizeof(m_TourData) - 2);	// copy all but the end tag and body[1]

// fix the rest
	PutPoolImage(m_TourData.Next, &image[(int)(&Amark->Next)]);
	PutWordImage(m_TourData.Len, &image[(int)(&Amark->Len)]);

}

/////////////////////////////////////////////
//
//	Name		:Serialize
//
//	Description :This will serialize all the data pertaining to Tours either in or out.
//
//  Input		:Persistance class
//
//	Output		:None
//
////////////////////////////////////////////
void CTour::Serialize(CPersist &per)
{
	if (per.m_storing)
	{
	// allocate space and reset the persistant index
		per.SetPointer(malloc(m_key.GetLength() + m_length  + sizeof(m_extended) + sizeof(m_length) + sizeof(m_TourData) + 10));

	// copy the key, length and data into the space just provided
		per << m_key;
		per << m_length;
		per << m_extended;
		*((TourType *)per.m_indexPtr) = m_TourData;

	// move the index up for the data only
		per.IncrementIndex(sizeof(m_TourData));  
		
	// copy the tour itself
		memcpy(per.m_indexPtr, m_tourBody, m_length);

	// move up the index
		per.IncrementIndex(m_length);
	}
	else
	{
	// get the key, length and the data from the buffer
		per >> m_key;
		per >> m_length;
		if (per.m_version >= 3)	// tour extension flag added in version 3
			per >> m_extended;
		m_TourData = *((TourType *)per.m_indexPtr);

	// increment the index
		per.IncrementIndex(sizeof(m_TourData));

	// make some space
		if (m_tourBody != NULL)
			free(m_tourBody);
		m_tourBody = malloc(m_length);

	// copy the tour data
		memcpy(m_tourBody, per.m_indexPtr, m_length);

	// increment the index
		per.IncrementIndex(m_length);  // see above

	// space fill the key to 16
		while(m_key.GetLength() < 16)
			m_key += " ";
	}


}

/////////////////////////////////////////////
//
//	Name		:ReadRomData
//
//	Description :Converts char ptr to 
//				 Tour data. This is used by import of 
//				 the old Lib files.
//
//  Input		:pointer to memory image
//
//	Output		:Data ok = true
//
////////////////////////////////////////////
bool CTour::ReadRomData(CPersist &per)
{
// get the length of the tour
	m_length = (short)*((unsigned long *)per.m_indexPtr);
	per.IncrementIndex(4);		// increment past length

// check the length 
	if (m_length > MAX_TOUR_LEN)		// a tour can never be this big
		return false;

// get memory for the body
	if (m_tourBody)
		free(m_tourBody);		// get rid of any old one first
	m_tourBody = malloc(m_length + 10);

// copy the data part
	m_TourData = *((TourType *)(per.m_indexPtr));
	
// copy the data and body into the block
	memcpy(m_tourBody, per.m_indexPtr, m_length);

// set the name as the key
	char temp[20];
	strncpy(temp, m_TourData.Name, 16);
	temp[16] = 0;
	m_key = temp;

// fix the length
	m_TourData.Len = (unsigned short)(m_length - sizeof(m_TourData));

// adjust the persistance pointer
	per.IncrementIndex(m_length);

	return true;
}

/////////////////////////////////////////////
//
//	Name		:ReadTxtData
//
//	Description :This function uses the global C function process_tour
//				 to compile the text tour into the compiled data. It then
//				 uses the file TEMP1.ROM from process_tour and imports
//				 that into the body of the this tour.
//
//  Input		:per - used for the filename only
//
//	Output		:true - Success
//
////////////////////////////////////////////
extern "C" FILE *destfile;
extern "C" FILE *srcfile;
extern "C" int ProcessTour(char);

bool CTour::ReadTxtData(CPersist &per)
{
	FILE *stream;

	CPersist per2;

	int oversize = 0;

	// redirect standard out for tour errors
	stream = freopen( "tourErr.txt", "w", stdout ); 

	// open the source and destination files for process_tour
    destfile=fopen("TEMP1.ROM","wb");
    srcfile=fopen(per.m_fileName.GetBuffer(5),"rt");

	bool compiledOK = false;

	if (ProcessTour(0) == 0)	// try compiling with the basic set
		compiledOK = true;

	if (!compiledOK)	// if it didn't compile, try the extended set
	{
		// open the files again
		destfile=fopen("TEMP1.ROM","wb");
		srcfile=fopen(per.m_fileName.GetBuffer(5),"rt");

		if (ProcessTour(1) == 0)	// try compiling with extended set
		{
			compiledOK = true;
			m_extended = true;	// set the extended flag to true
		}
	}

	if (compiledOK)
	{
		// open output file of process_tour
		per2.ReadFile("TEMP1.ROM");

		// check the file for the minumum length
		if (per2.m_dataReadCnt > 16)
		{

			// reassign standard out to the console. This will close the error file
			stream = freopen( "CON", "w", stdout ); 

			// get the name from the head of the file
			SetKey(CString((char *)per2.m_indexPtr, 16));			// set it as the key
			strncpy(m_TourData.Name, GetKey().GetBuffer(20), 16);	// put it in the data
			per2.IncrementIndex(16);

			// before trying to malloc, check that the tour is < 32K (defined in Autoglob.h)
			if (per2.m_dataReadCnt <= SHRT_MAX)
			{
				// set the length of the tour
				m_TourData.Len = (short)per2.m_dataReadCnt - 16;// the 16 is the size of the title

				m_length = m_TourData.Len + sizeof(m_TourData) - 1;

				// set the extended flag in the high order bit if necessary
				if (m_extended)
					m_TourData.Len |= SHRT_MIN;

				// allocate memory for the body of the tour
				if (m_tourBody)
					free (m_tourBody);
			
				m_tourBody = malloc(m_length + 10);// just a fudge

				// copy the data followed by the body
				memcpy(m_tourBody, (void *)&m_TourData, sizeof(m_TourData));
				memcpy((char *)m_tourBody + sizeof(m_TourData) - 2, per2.m_indexPtr, (m_TourData.Len & SHRT_MAX));// the -2 is to skip the EndTag and the char[1]

				// insert the end tag
				*((char *)m_tourBody + m_length - 1) = 0;

				// free the buffer and increment the passed persistance object to the end
				per2.FreeBuffer();
				CFile::Remove("TEMP1.ROM");			// delete the temporary file
				per.IncrementIndex(per.m_dataReadCnt);
				CFile::Remove("tourErr.txt");			// delete the error file
				return true;
			}
			else
			{
				oversize = per2.m_dataReadCnt - SHRT_MAX;
				// free the buffer
				per2.FreeBuffer();
				
			}
		}
	}
// An error occured so display the error file to the user.

// increment the passed persistance object to the end
	per.IncrementIndex(per.m_dataReadCnt);	

// delete the temporary file
	CFile::Remove("TEMP1.ROM");

	// reassign standard out to the console. This will close the error file
	stream = freopen( "CON", "w", stdout ); 
	
	// read the error file and put it in a message box
	CPersist err;
	err.ReadFile("tourErr.txt");
	CString errStr((char *)err.m_dataPtr, err.m_dataReadCnt);
	if (oversize)	// if this was the error
	{
		CString tempStr;
		tempStr.Format("\n*** Error Importing \"%s\" ***\nMaximum Length of %i Exceeded by %i\n", per.m_fileName, SHRT_MAX, oversize);
		errStr += tempStr;
	}
	err.FreeBuffer();
//	MessageBox(NULL, errStr + CString("\nErrors saved in tourErr.txt"), "Tour Compile Error", MB_OK);
	CErrorLogDlg tDlg;
	tDlg.m_text = errStr;
	tDlg.m_title = "Import Error Log";
	tDlg.DoModal();


	return false;
		
}


/////////////////////////////////////////////
//
//	Name		:IsDynamic
//
//	Description :returns true if data length is dynamic
//
//  Input		:none
//
//	Output		:true - dynamic
//				 false - fixed length
//
////////////////////////////////////////////
bool CTour::IsDynamic()
{
	return true;
}



bool CTour::IsExtended()
{
	return m_extended;
}
