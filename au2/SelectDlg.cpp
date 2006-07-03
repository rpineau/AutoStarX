// SelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AU2.h"
#include "SelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COMBOPROMPT "<Select object Type>"
/////////////////////////////////////////////////////////////////////////////
// CSelectDlg dialog


CSelectDlg::CSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectDlg)
	m_prompt = _T("");
	m_delete = FALSE;
	m_update = FALSE;
	//}}AFX_DATA_INIT
	m_title = _T("");
	m_selectedType = _T("none");
	m_delete = FALSE;
	m_comboText.Add("Asteroid");
	m_comboText.Add("Satellite");
	m_comboText.Add("Comet");
	m_comboText.Add("LandMark");
	m_comboText.Add("Tour");
	m_comboText.Add("User Object");
	m_selectBodyType = All;
}


void CSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectDlg)
	DDX_Control(pDX, IDC_COMBO, m_combo);
	DDX_Text(pDX, IDC_STATIC_PROMPT, m_prompt);
	DDX_Check(pDX, IDC_CHECK_DELETE, m_delete);
	DDX_Check(pDX, IDC_CHECK_UPDATE, m_update);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_UPDATE, OnCheckUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectDlg message handlers

BOOL CSelectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Add the entries for the combo box
	m_combo.AddString(COMBOPROMPT);
	for (int i = 0; i <= m_comboText.GetUpperBound(); i++)
		m_combo.AddString(m_comboText.GetAt(i));
	m_combo.SetCurSel(0);

	SetWindowText(m_title);

	CRect comboRect;
	CComboBox* box = (CComboBox *) GetDlgItem(IDC_COMBO);
	box->GetWindowRect(comboRect);
	ScreenToClient(comboRect);
	comboRect.InflateRect(0,0,0,150);
	box->SetDroppedWidth(200);

	if (SetBodyType(m_selectBodyType))	//if body type has already been selected,
	{									// shrink dialog window size
		CRect dlgRect;
		GetWindowRect(&dlgRect);
		dlgRect.DeflateRect(0,0,0,30);
		SetWindowPos(NULL,0,0,dlgRect.Width(),dlgRect.Height(),SWP_NOMOVE);
	}

	//Make window the topmost window
	SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectDlg::OnDestroy() 
{
	// get selected Object Type	
	CString selectedText;
	GetDlgItemText(IDC_COMBO, selectedText);
	if (selectedText != COMBOPROMPT)
		m_selectedType = selectedText;
	else
		m_selectedType = "none";

}

BodyType CSelectDlg::GetBodyType()
{
	if (m_selectedType == "Asteroid") return Asteroid;
	if (m_selectedType == "Satellite") return Satellite;
	if (m_selectedType == "Comet") return Comet;
	if (m_selectedType == "LandMark") return LandMark;
	if (m_selectedType == "Tour") return Tour;
	if (m_selectedType == "User Object") return UserObj;
	return All;

}

void CSelectDlg::OnOK() 
{
	OnDestroy();
	if (m_selectedType != "none")
		CDialog::OnOK();
	else
		MessageBox("You must select an object type","Error",MB_ICONWARNING);
}

void CSelectDlg::OnCheckUpdate() 
{
	CButton* upButt = (CButton *) GetDlgItem(IDC_CHECK_UPDATE);
	CButton* delButt = (CButton *) GetDlgItem(IDC_CHECK_DELETE);
	if (upButt->GetCheck())
		delButt->EnableWindow(FALSE);
	else
		delButt->EnableWindow(TRUE);

	
}

////////////////////////////////////////////////////////////
//
// Function to set body type in combo box, if body type is known
// in advance.  If so, the combo is not displayed.  If body type
// is not known in advance, the zero index combo entry is displayed
//
// Input:	CBodyType (All if not specified)
//
// Output:	TRUE = body type has been specified
//			FALSE = body type has not been specified
//
/////////////////////////////////////////////////////////////
bool CSelectDlg::SetBodyType(BodyType type)
{
	CComboBox* combo = (CComboBox *) GetDlgItem(IDC_COMBO);
	int index;
	switch (type)
	{
	case Asteroid:
		index = combo->FindString(0,"Asteroid");
		break;
	case Satellite:
		index = combo->FindString(0,"Satellite");
		break;
	case Comet:
		index = combo->FindString(0,"Comet");
		break;
	case LandMark:
		index = combo->FindString(0,"LandMark");
		break;
	case Tour:
		index = combo->FindString(0,"Tour");
		break;
	case UserObj:
		index = combo->FindString(0,"User Object");
		break;
	default:
		index = 0;
		return FALSE; //if a body type is not specified, leave combo window visible
	}
	// set the combo to that item, but make it invisible
	combo->SetCurSel(index);
	WINDOWPLACEMENT wplt;
	wplt.showCmd = SW_HIDE;
	combo->SetWindowPlacement(&wplt);

	return TRUE;
}
