// AU2ReplaceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AU2.h"
#include "AU2ReplaceDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAU2ReplaceDlg dialog


CAU2ReplaceDlg::CAU2ReplaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAU2ReplaceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAU2ReplaceDlg)
	m_oldObjectText = _T("");
	m_newObjectText = _T("");
	//}}AFX_DATA_INIT
	m_multiSelect = FALSE;
}


void CAU2ReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAU2ReplaceDlg)
	DDX_Text(pDX, IDC_REPLACE_OBJECT, m_oldObjectText);
	DDX_Text(pDX, IDC_REPLACE_WITH, m_newObjectText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAU2ReplaceDlg, CDialog)
	//{{AFX_MSG_MAP(CAU2ReplaceDlg)
	ON_BN_CLICKED(ID_REPLACE_NO, OnReplaceNo)
	ON_BN_CLICKED(ID_REPLACE_YES, OnReplaceYes)
	ON_BN_CLICKED(ID_REPLACE_YESTOALL, OnReplaceYesToAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAU2ReplaceDlg message handlers

void CAU2ReplaceDlg::OnReplaceNo() 
{
	m_outcome = NO;
	CDialog::OnCancel();
	
}

void CAU2ReplaceDlg::OnReplaceYes() 
{
	m_outcome = YES;
	CDialog::OnOK();
	
}

void CAU2ReplaceDlg::OnReplaceYesToAll() 
{
	m_outcome = YESTOALL;
	CDialog::OnOK();
	
}

void CAU2ReplaceDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	m_outcome = CANCEL;
	CDialog::OnCancel();
}



BOOL CAU2ReplaceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//Make window the topmost window
	SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
