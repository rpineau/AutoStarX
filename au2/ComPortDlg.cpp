// ComPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AU2.h"
#include "ComPortDlg.h"
#include "autostar\Autostar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CComPortDlg dialog


CComPortDlg::CComPortDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CComPortDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CComPortDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CComPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CComPortDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CComPortDlg, CDialog)
	//{{AFX_MSG_MAP(CComPortDlg)
	ON_BN_CLICKED(IDC_COM1, OnComSelect)
	ON_BN_CLICKED(IDC_COM2, OnComSelect)
	ON_BN_CLICKED(IDC_COM3, OnComSelect)
	ON_BN_CLICKED(IDC_COM4, OnComSelect)
	ON_BN_CLICKED(IDC_COM_OTHER, OnComSelect)
	ON_BN_CLICKED(IDC_COMPORT_AUTO, OnAutoDetect)
	//}}AFX_MSG_MAP
//	ON_COMMAND_RANGE(IDC_COM1,IDC_COM_OTHER,OnComSelect)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComPortDlg message handlers

void CComPortDlg::OnComSelect() 
{

	if (GetCheckedRadioButton(IDC_COM1,IDC_COM_OTHER) == IDC_COM_OTHER)
	{
		CEdit* edit = (CEdit *) GetDlgItem(IDC_COM_EDIT);
		edit->EnableWindow();
	}
	else
	{
		CEdit* edit = (CEdit *) GetDlgItem(IDC_COM_EDIT);
		edit->EnableWindow(FALSE);
	}


	CButton* buttonOK = (CButton *) GetDlgItem(IDOK);

	// something was selected, so enable OK button
	buttonOK->EnableWindow();	
}

// perform dialog box initialization
BOOL CComPortDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// initially gray out edit window
	CEdit* edit = (CEdit *) GetDlgItem(IDC_COM_EDIT);
	edit->EnableWindow(FALSE);

	// initially gray out OK button
	CButton* buttonOK = (CButton *) GetDlgItem(IDOK);
	buttonOK->EnableWindow(FALSE);

	CheckComButton();
	
	//Make window the topmost window
	SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// set registry key based on the chosen com port
void CComPortDlg::OnOK() 
{
	CUserSettings userSettings;
	CEdit* edit = (CEdit *) GetDlgItem(IDC_COM_EDIT);

	switch (GetCheckedRadioButton(IDC_COM1,IDC_COM_OTHER))
	{
	case IDC_COM1:
		userSettings.SetComPort("COM1");
		break;
	case IDC_COM2:
		userSettings.SetComPort("COM2");
		break;
	case IDC_COM3:
		userSettings.SetComPort("COM3");
		break;
	case IDC_COM4:
		userSettings.SetComPort("COM4");
		break;
	case IDC_COM_OTHER:
		CString number;
		edit->GetWindowText(number);
		// number must be specified in this option, or dont exit
		if (number == "")	
		{
			MessageBox("Must enter a valid number for \'Other\'","Error",MB_OK);
			return;
		}
		userSettings.SetComPort("COM" + number);
		break;
	}
	CDialog::OnOK();
}

void CComPortDlg::OnAutoDetect() 
{
	CAutostar autostar;
	CUserSettings userSettings;
	if (autostar.FindAutostar())
		MessageBox("Found autostar at " + userSettings.GetComPort(),
					"Auto-Detect Successful",MB_OK | MB_TOPMOST);
	else
	{
		MessageBox("Could not find Autostar.  Check connections.",
					"Error",MB_OK | MB_TOPMOST);
		return;
	}

	// update the dialog box to have the proper radio button checked
	CheckComButton();

	OnComSelect();

}

// queries the registry, and checks the appropriate radio button
void CComPortDlg::CheckComButton()
{
	CUserSettings userSettings;
	
	UncheckAllButtons();

	// if registry does not contain COM Port info, return w/o doing anything
	if (userSettings.GetComPort() == "")
		return;

	if (userSettings.GetComPort() == "COM1")
	{
		CButton* button = (CButton *) GetDlgItem(IDC_COM1);
		button->SetCheck(1);
	}
	else 
		if (userSettings.GetComPort() == "COM2")
		{
			CButton* button = (CButton *) GetDlgItem(IDC_COM2);
			button->SetCheck(1);
		}
		else
			if (userSettings.GetComPort() == "COM3")
			{
				CButton* button = (CButton *) GetDlgItem(IDC_COM3);
				button->SetCheck(1);
			}
			else
				if (userSettings.GetComPort() == "COM4")
				{
					CButton* button = (CButton *) GetDlgItem(IDC_COM4);
					button->SetCheck(1);
				}
				else
				{
					CButton* button = (CButton *) GetDlgItem(IDC_COM_OTHER);
					button->SetCheck(1);
					CEdit* edit = (CEdit *) GetDlgItem(IDC_COM_EDIT);
					CString comNumber = userSettings.GetComPort();
					comNumber = comNumber.Right(comNumber.GetLength() - 3);
					edit->SetWindowText(comNumber);
					edit->EnableWindow();
				}

	CButton* buttonOK = (CButton *) GetDlgItem(IDOK);

	// something was selected, so enable OK button
	buttonOK->EnableWindow();	
}

void CComPortDlg::UncheckAllButtons()
{
	const int num = 5;
	UINT buttons[num] = {IDC_COM1,IDC_COM2,IDC_COM3,IDC_COM4,IDC_COM_OTHER};
	for (int i = 0; i < num; i++)
	{
		CButton* button = (CButton *) GetDlgItem(buttons[i]);
		button->SetCheck(0);
	}
}


