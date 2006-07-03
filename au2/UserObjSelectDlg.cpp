// UserObjSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "au2.h"
#include "UserObjSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserObjSelectDlg dialog


CUserObjSelectDlg::CUserObjSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserObjSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserObjSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_fontSize = SMALL;	//initially assume font size is small
}


void CUserObjSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserObjSelectDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserObjSelectDlg, CDialog)
	//{{AFX_MSG_MAP(CUserObjSelectDlg)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, OnButtonSelect)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_CANCELMODE()
	ON_WM_CAPTURECHANGED()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(IDC_USEROBJ,IDC_USEROBJ40,OnUserObjClicked)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserObjSelectDlg message handlers

BOOL CUserObjSelectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CFrameWnd* pParent = (CFrameWnd *) GetParent();
	CAU2View* pView = (CAU2View *) pParent->GetActiveView();

	CButton* button;

	SetWindowPos(&wndTop, m_origin.x, m_origin.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

	// Dialog resource was designed based on large fonts.  resize if necessary
	if (m_fontSize == SMALL)
	{
		// get the original sizes
		CRect originalSize, originalButton, newButton;
		GetWindowRect(&originalSize);

		// get the size of the scaled main window button
		button = (CButton *) pView->GetDlgItem(IDC_LUSEROBJECTSRADIO);		// this is the button from the parent GUI
		button->GetWindowRect(&newButton);
		ScreenToClient(&newButton);

		// get the size of the new dialog button (before scaling)
		button = (CButton *) GetDlgItem(IDC_BUTTON_SELECT);
		button->GetWindowRect(&originalButton);
		ScreenToClient(&originalButton);

		// calculate the scale factors
		const float heightScale = (float) newButton.Height() / (float) originalButton.Height();
		const float widthScale = static_cast<float>(newButton.Width()) / originalButton.Width();

		int lastButtonBottom = 0, moveRight = 0;

		// push the "select" button
		button->SetCheck(1);

		// scale & move each button
		int i = IDC_BUTTON_SELECT;
		do
		{
			button = (CButton *) GetDlgItem(i);
			button->GetWindowRect(&originalButton);
			ScreenToClient(&originalButton);
			if (i == IDC_USEROBJ29)	// this button starts the second column
			{
				moveRight = newButton.Width() - originalButton.Width();
				lastButtonBottom = 0;
			}
			button->MoveWindow(originalButton.left + moveRight, lastButtonBottom,
				newButton.Width(), newButton.Height());
			button->GetWindowRect(&originalButton);
			ScreenToClient(&originalButton);
			lastButtonBottom = originalButton.bottom;
			
			i++;
		}
		while(i <= IDC_USEROBJ39);

		// scale the original size of the dialog
		SetWindowPos(&wndTop, 0, 0, (int)(originalSize.Width() * widthScale), newButton.Height() * (IDC_USEROBJ39 - IDC_USEROBJ29 + 1), SWP_NOMOVE | SWP_SHOWWINDOW);
	}
	
	// regardless of the font size, change the button text
	for (int i = IDC_USEROBJ20; i <= IDC_USEROBJ39; i++)
	{
		button = (CButton *) GetDlgItem(i);
		button->SetWindowText((*m_pButtonLabels)[i - IDC_USEROBJ20 + UserObj20]);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUserObjSelectDlg::OnUserObjClicked(UINT nID) 
{
	// get a pointer to the button that was clicked
	CButton* button = (CButton *) GetDlgItem(nID);

	// change the text of the main buttons to the selected button
	button->GetWindowText(m_selectedText.GetBuffer(16), 16);

	ConvertBodyType(button);
	
	EndDialog(IDOK);
	
}



CString CUserObjSelectDlg::GetSelectedText()
{
	return m_selectedText;
}

// evaluates the button pressed to determine which body type correlates to it
BodyType CUserObjSelectDlg::ConvertBodyType(CButton *buttonClicked)
{
	UINT buttonID = buttonClicked->GetDlgCtrlID();

	BodyType bodyType;

	if (buttonID == IDC_USEROBJ)
		bodyType = UserObj;		// this object is treated uniquely
	else
		// Note: 2201 resource ID corresponds to UserObj20 (= 7), so offset = 2195
		bodyType = (BodyType) (buttonID - 2195);

	m_bodyTypeSelected = bodyType;

	return bodyType;
}

BodyType CUserObjSelectDlg::GetBodyType()
{
	return m_bodyTypeSelected;
}

// clicking this button does not select anything, so just exit
void CUserObjSelectDlg::OnButtonSelect() 
{
	EndDialog(IDCANCEL);
}

BOOL CUserObjSelectDlg::PreTranslateMessage(MSG* pMsg) 
{
	static UINT last;

	if (pMsg->message == 0x0118)
		EndDialog(IDCANCEL);		// used when user clicks outside the dlg

	if (pMsg->message != last)
	{
		TRACE("\nUserDlg Message Filtered: %i",	pMsg->message);
		last = pMsg->message;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
