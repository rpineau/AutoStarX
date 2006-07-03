// ErrorLogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "au2.h"
#include "ErrorLogDlg.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CErrorLogDlg dialog


CErrorLogDlg::CErrorLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CErrorLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CErrorLogDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_title = "Log Dialog";
}


void CErrorLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CErrorLogDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CErrorLogDlg, CDialog)
	//{{AFX_MSG_MAP(CErrorLogDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CErrorLogDlg message handlers

BOOL CErrorLogDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetWindowText(m_title);
	
	CRichEditCtrl* edit = (CRichEditCtrl *) GetDlgItem(IDC_RICHEDIT_LOG);

	edit->SetReadOnly();

	edit->SetOptions(ECOOP_OR,ECO_AUTOVSCROLL | ECO_AUTOHSCROLL);

	edit->SetWindowText(m_text);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

}
