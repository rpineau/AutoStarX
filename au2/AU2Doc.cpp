// AU2Doc.cpp : implementation of the CAU2Doc class
//

#include "stdafx.h"
#include "AU2.h"
#include "AU2Doc.h"

#include "image.h"
#include "MaskedBitmap.h"
#include "AU2ListView.h"
#include "AU2View.h"
#include "afxcview.h"
#include "Autostar\BodyDataCollection.h"
#include "Autostar\BodyData.h"
#include "Autostar\Asteroid.h"
#include "Autostar\BodyDataFactory.h"
#include "Autostar\BodyDataMaker.h"
#include "Autostar\Persist.h"
#include "Autostar\UserObjEx.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAU2Doc

IMPLEMENT_DYNCREATE(CAU2Doc, CDocument)

BEGIN_MESSAGE_MAP(CAU2Doc, CDocument)
	//{{AFX_MSG_MAP(CAU2Doc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAU2Doc construction/destruction

CAU2Doc::CAU2Doc()
{
	m_hbxConnected = FALSE;	//handbox is initially not connected
	// initialize handbox collection
	m_handboxCollection = m_autostar.GetHandboxData();	
	m_hbx_mem = 0;	// initialize handbox memory
	m_lBodyTypeLabel.SetSize(BodyTypeMax);
	m_hBodyTypeLabel.SetSize(BodyTypeMax);
	ResetBodyTypeLabels(LIBRARY);
	ResetBodyTypeLabels(HANDBOX);
	m_bodyType = Asteroid;
	m_handboxModified			= false;
	m_customDisplay				= UserObj;
}

CAU2Doc::~CAU2Doc()
{
//	m_libraryCollection.Clear();
}

// initialize handbox and library collections
BOOL CAU2Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CAU2View* pView;

	POSITION pos = GetFirstViewPosition();
	while(pos)
	{
		pView = (CAU2View *) GetNextView(pos);

		// store the user obj catalog that is displayed in the radio buttons
		CString buttonText;
		((CButton *) pView->GetDlgItem(IDC_LUSEROBJECTSRADIO))->GetWindowText(buttonText);
		m_customDisplay = GetCustomBodyType(LIBRARY, buttonText);

		// delete the contents of the list views
		pView->DeleteListView(LIBRARY);
		pView->DeleteListView(HANDBOX);
	}

	m_libraryCollection.Clear();

	ResetBodyTypeLabels(LIBRARY);

	m_handboxCollection = m_autostar.GetHandboxData();

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CAU2Doc serialization

void CAU2Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAU2Doc diagnostics

#ifdef _DEBUG
void CAU2Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAU2Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAU2Doc commands

// load data from autostar into handbox collection
void CAU2Doc::LoadAutostarData(bool spawnThread)
{

	if (m_autostar.RetrieveUserData(spawnThread) != AUTOSTAR_DOWNLOADING)
	{
		MessageBox(NULL, m_autostar.GetLastError(), _T("Autostar Error"), MB_OK);

	}

	m_handboxCollection = m_autostar.GetHandboxData();
}

// send handbox collection to autostar
void CAU2Doc::SendAutostarData()
{
	if (m_autostar.SendUserData() != AUTOSTAR_UPLOADING)
	{
		MessageBoxEx(NULL, m_autostar.GetLastError(), _T("Autostar Error"), MB_OK | MB_TOPMOST, LANG_ENGLISH);
	}
}

// open an existing ".aud" document
BOOL CAU2Doc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	CString		fileName = lpszPathName;
	CString		fileExt	= fileName.Right(3);
	fileExt.MakeLower();
	CAU2View* pView;

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	POSITION pos = GetFirstViewPosition();
	while(pos)
	{
		pView = (CAU2View *) GetNextView(pos);

		// store the user obj catalog that is displayed in the radio buttons
		CString buttonText;
		((CButton *) pView->GetDlgItem(IDC_LUSEROBJECTSRADIO))->GetWindowText(buttonText);
		m_customDisplay = GetCustomBodyType(LIBRARY, buttonText);

		// delete the contents of the list views
		pView->DeleteListView(LIBRARY);
		pView->DeleteListView(HANDBOX);
	}
	
	if (fileExt == "aud")
	{
		m_libraryCollection.Clear();	
		if (m_libraryCollection.LoadFromFile(lpszPathName) != READCOMPLETE)
			return FALSE;
	}
	else if (fileExt == "tle")
	{
		if (m_libraryCollection.Import(fileName, Satellite) != READCOMPLETE)
			return FALSE;
		else
			pView->ChangeListBodyType(Satellite);
	}
	else if (fileExt == "mtf")
	{
		if (m_libraryCollection.Import(fileName, Tour) != READCOMPLETE)
			return FALSE;
		else
			pView->ChangeListBodyType(Tour);
	}
	else 
		return FALSE;

	ResetBodyTypeLabels(LIBRARY, &m_libraryCollection);

	return TRUE;
}

// save an ".aud" document
BOOL CAU2Doc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	if (m_libraryCollection.SaveToFile(lpszPathName) != READCOMPLETE)
		return FALSE;
	SetModifiedFlag(FALSE);
	return TRUE;
}


// initializes the labels used on the radio buttons
// (i.e., reset or load any custom labels)
// input:	source: LIBRARY or HANDBOX
//			CBodyDataCollection*  (= NULL default to reset labels)
//			BodyType (= ALL default to apply to all body types)
void CAU2Doc::ResetBodyTypeLabels(SourceList source, CBodyDataCollection *data, BodyType bodyType)
{
	CStringArray *bodyTypeLabel;

	// set the pointer to the appropriate source array
	if (source == LIBRARY)
		bodyTypeLabel = &m_lBodyTypeLabel;
	else
		if (source == HANDBOX)
			bodyTypeLabel = &m_hBodyTypeLabel;
		else 
			return;

	bodyTypeLabel->SetAt(Asteroid,"Asteroids");
	bodyTypeLabel->SetAt(Comet,"Comets");
	bodyTypeLabel->SetAt(Satellite,"Satellites");
	bodyTypeLabel->SetAt(UserObj,"User Objects");
	bodyTypeLabel->SetAt(LandMark,"LandMarks");
	bodyTypeLabel->SetAt(Tour,"Tours");

	BodyType startType = UserObj20;
	BodyType endType = UserObj39;

	// UNLESS a specific bodyType is passed, the above settings will iterate through all
	// user objects. Otherwise, just reset the specified type
	if (bodyType != All && bodyType >= UserObj20)
	{
		startType = bodyType;
		endType = bodyType;
	}

	for (int i = startType; i <= endType; i++)
	{
		if (data != NULL)
		// if collection was passed, look for an object of each type
		{
			POSITION pos = data->GetHeadPosition((BodyType) i);
			if (pos)
			{
				CBodyData* body = data->GetNext(pos,(BodyType) i);
				// get its catalog label
				CString temp;
				bodyTypeLabel->SetAt(i, ((CUserObjEx *) body)->GetCatalogName());
				temp = bodyTypeLabel->GetAt(i);
				temp.TrimLeft();
				temp.TrimRight();
				bodyTypeLabel->SetAt(i,temp);
				continue;
			}
		}

		// if we got to here, either no collection was specified or no object found
		// so use default label
		bodyTypeLabel->SetAt(i, DEFAULT_CAT_NAME + CPersist::ToString(i - 5));
	}


}


// identifies a custom body type by the label of the button
BodyType CAU2Doc::GetCustomBodyType(SourceList source, CString label)
{

	CStringArray *bodyTypeLabel;

	// set the pointer to the appropriate source array
	if (source == LIBRARY)
		bodyTypeLabel = &m_lBodyTypeLabel;
	else
		if (source == HANDBOX)
			bodyTypeLabel = &m_hBodyTypeLabel;
		else 
			return All;

	label.TrimRight();
	label.TrimLeft();

	bool found = false;

	for (int i = UserObj20; i <= UserObj39; i++)
	{
		if (bodyTypeLabel->GetAt(i) == label ||
			CPersist::Abbreviate(bodyTypeLabel->GetAt(i),RADIO_SIZE) == label)
		{
			found = true;
			break;
		}
	}

	if (found)
		return (BodyType) i;

	else	// default response
		return UserObj;
	

	
}
