// AU2FileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AU2.h"
#include "AU2FileDialog.h"

#include <direct.h>//
#include <dlgs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




HHOOK HookHandle; //extern 

/////////////////////////////////////////////////////////////////////////////
// CAU2FileDialog

IMPLEMENT_DYNAMIC(CAU2FileDialog, CFileDialog)

CAU2FileDialog::CAU2FileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd),
	// initialize the strings that will be used throughout the class
		ASTEROID("Asteroid"),
		SATELLITE("Satellite"),
		COMET("Comet"),
		LANDMARK("Landmark"),
		TOUR("Tour"),
		USEROBJ("User Object")
{

}


BEGIN_MESSAGE_MAP(CAU2FileDialog, CFileDialog)
	//{{AFX_MSG_MAP(CAU2FileDialog)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()

END_MESSAGE_MAP()

// Override InitDialog to add custom controls
BOOL CAU2FileDialog::OnInitDialog() 
{
	// initialize member variables
	m_delete = FALSE;
	m_update = FALSE;

	// This variable should be changed acording to your wishes
	// about the size of the finished dialog
	const UINT iExtraSize = 0;
	// Number of controls in the File Dialog
	const UINT nControls = 7;    

	// Get a pointer to the original dialog box.
	CWnd *wndDlg = GetParent();

	RECT Rect;
	wndDlg->GetWindowRect(&Rect);
	// Change the size
	wndDlg->SetWindowPos(NULL, 50, 50, 
					  Rect.right - Rect.left, 
					  Rect.bottom - Rect.top + iExtraSize + 66, 
					  NULL);

	// Control ID's - defined in <dlgs.h>
	UINT Controls[nControls] = {stc3, stc2, // The two label controls
							  edt1, cmb1, // The eidt control and the drop-down box
							  IDOK, IDCANCEL, 
							  lst1}; // The Explorer window

	// Go through each of the controls in the dialog box, and move them to a new position
	for (int i=0 ; i<nControls ; i++)
	{
	  CWnd *wndCtrl = wndDlg->GetDlgItem(Controls[i]);
	  if (wndCtrl != NULL)
	  {
		wndCtrl->GetWindowRect(&Rect);
		wndDlg->ScreenToClient(&Rect); // Remember it is child controls

		// Move all the controls according to the new size of the dialog.
		if (Controls[i] != lst1)
			wndCtrl->SetWindowPos(NULL, 
							Rect.left, Rect.top + iExtraSize,
							0, 0, SWP_NOSIZE);
		else // This is the explorer like window. It should be sized - not moved.
			wndCtrl->SetWindowPos(NULL, 0, 0,
								Rect.right - Rect.left, 
								Rect.bottom - Rect.top + iExtraSize, 
								SWP_NOMOVE);
	  }
	}

	// change "Open" to "Import" on button
	CWnd *wndCtrl = wndDlg->GetDlgItem(IDOK);
	wndCtrl->SetWindowText("Import");

	 //Size of dialog box, combo box, static, delete check box, update check box
	CRect dlgRect,comboRect,staticRect,deleteRect,updateRect;
	wndDlg->GetWindowRect(dlgRect); //copy dialog box size to dlgRect
	wndDlg->ScreenToClient(dlgRect);	// convert to client coordinates!!!

	// get conversion factor to account for system font size
	LONG baseUnit = GetDialogBaseUnits();
	int baseX = baseUnit & 0xFFFF;
	int baseY = (baseUnit & 0xFFFF0000) >> 16;


	// get the size and location of the existing combo box
	CWnd *wndCombo = wndDlg->GetDlgItem(cmb1);
	CRect oldComboRect;
	wndCombo->GetWindowRect(oldComboRect);
	wndDlg->ScreenToClient(oldComboRect);	// convert to client coordinates!!!

	// get the size and location of the existing static control
	CWnd *wndStatic = wndDlg->GetDlgItem(stc2);
	CRect oldStaticRect;
	wndStatic->GetWindowRect(oldStaticRect);
	wndDlg->ScreenToClient(oldStaticRect);	// convert to client coordinates!!!

	// set the size of the new combo box
	comboRect = oldComboRect;
	comboRect.top = oldStaticRect.bottom + (6 * baseY) / 8;//(30 * 8) / baseY;
	comboRect.bottom = dlgRect.bottom + 90; //let combo extend past borders of dialog

	// create the combo box
	m_comboType = new CComboBox;
	m_comboType->Create(CBS_DROPDOWN | WS_VSCROLL | WS_CHILD | WS_VISIBLE,
						comboRect,wndDlg,ID_COMBOTYPE);

	// set the font size and add the list strings
	m_comboType->SetFont(wndDlg->GetFont());
	m_comboType->AddString(ASTEROID);
	m_comboType->AddString(SATELLITE);
	m_comboType->AddString(COMET);
	m_comboType->AddString(LANDMARK);
	m_comboType->AddString(TOUR);
	m_comboType->AddString(USEROBJ);

	// set the size of the static text
	staticRect = comboRect;
	staticRect.left = oldStaticRect.left;
	staticRect.right = comboRect.left - (2 * baseX) / 4;//(10 * 4) / baseX;
	staticRect.bottom = staticRect.top + 22;
	//create the static text
	m_static = new CStatic;
	m_static->Create("Object Type:",SS_LEFT | SS_CENTERIMAGE | WS_CHILD |
					WS_VISIBLE, staticRect,wndDlg,ID_COMBOSTATIC);
	m_static->SetFont(wndDlg->GetFont());

	// set the size and location of the delete check box
	deleteRect.top = staticRect.bottom + (4 * baseY) / 8;//(30 * 8) / baseY;
	deleteRect.bottom = deleteRect.top + (6 * baseY) / 8;//(40 * 8) / baseY;
	deleteRect.right = comboRect.right + 300;
	deleteRect.left = comboRect.left;

	// add the delete check box
	m_buttonDelete = new CButton;
	m_buttonDelete->Create("Delete all existing objects of selected type first",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, deleteRect, wndDlg,ID_BUTTONDELETE);
	m_buttonDelete->SetFont(wndDlg->GetFont());

	// set the size and location of the update check box
	updateRect.top = deleteRect.bottom + (5 * baseY) / 8;//(10 * 8) / baseY;
	updateRect.bottom = updateRect.top + (6 * baseY) / 8;//(40 * 8) / baseY;
	updateRect.right = comboRect.right + 300;
	updateRect.left = comboRect.left;

	// add the update check box
	m_buttonUpdate = new CButton;
	m_buttonUpdate->Create("Only import (and overwrite) objects that already exist",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, updateRect, wndDlg,ID_BUTTONUPDATE);
	m_buttonUpdate->SetFont(wndDlg->GetFont());

	// Set hook for filtering File Dialog windows messages
	HookHandle = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC) Hooker, 
		(HINSTANCE)NULL, (DWORD)GetCurrentThreadId());
	
	// set the initial type and extension
	if (m_selectedType != "")
		m_comboType->SelectString(-1,m_selectedType);

	if (m_selectedExt != "")
	{
		CComboBox *box = (CComboBox *) wndDlg->GetDlgItem(cmb1);
		box->SelectString(-1,m_selectedExt);
		OnTypeChange();
	}

	// Remember to call the baseclass.
	return CFileDialog::OnInitDialog();
}



// destroy created controls
CAU2FileDialog::~CAU2FileDialog()
{
	delete m_comboType;
	delete m_pFont;
	delete m_static;
	delete m_buttonDelete;
	delete m_buttonUpdate;

	UnhookWindowsHookEx(HookHandle);
}

// set font size before dialog is created
void CAU2FileDialog::SetFontSize(int size)
{
	// size = 0 for small fonts, 1 for large fonts
	m_pFont = new CFont;
	if (size == 0)
		m_pFont->CreatePointFont(100,"MS San Serif");
	else
		m_pFont->CreatePointFont(80,"MS San Serif");


}

// return body type based on selected combo box text
BodyType CAU2FileDialog::GetBodyType()
{
	if (m_selectedType == ASTEROID) return Asteroid;
	if (m_selectedType == SATELLITE) return Satellite;
	if (m_selectedType == COMET) return Comet;
	if (m_selectedType == LANDMARK) return LandMark;
	if (m_selectedType == TOUR) return Tour;
	if (m_selectedType == USEROBJ) return UserObj;
	return All;


}

// get value of combo box selection before dialog is destroyed
void CAU2FileDialog::OnDestroy()
{

	CString selectedText,selectedExt;

	if (GetParent()->GetDlgItemText(ID_COMBOTYPE, selectedText) > 0)
		m_selectedType = selectedText;
	else
		m_selectedType = "";

	if (GetParent()->GetDlgItemText(cmb1, selectedExt) > 0)
		m_selectedExt = selectedExt;
	else
		m_selectedExt = "";

	if (m_buttonDelete->GetCheck() == 0) m_delete = FALSE;
	else m_delete = TRUE;

	if(m_buttonUpdate->GetCheck() == 0) m_update = FALSE;
	else m_update = TRUE;


}

void CAU2FileDialog::OnTypeChange()
{
	// Get a pointer to the dialog box.
	CWnd *wndDlg = GetParent();

	//get pointer to the object type combo box
	CComboBox *typeCombo = (CComboBox *) wndDlg->GetDlgItem(ID_COMBOTYPE);

	CString extString;
	GetParent()->GetDlgItemText(cmb1,extString);
	if (extString == GetExtText(TYPE_TLE))		// if TLE file
	{
		typeCombo->SetCurSel(1);				// set object type to Satellite
		typeCombo->EnableWindow(false);
	}
	else if (extString == GetExtText(TYPE_MTF))		// if MTF file
	{
		typeCombo->SetCurSel(4);				// set object type to Tour
		typeCombo->EnableWindow(false);
	}
	else
	{
		typeCombo->EnableWindow();
	}


}

// Function to hook windows messages & filter File Dialog messages
LRESULT CALLBACK Hooker(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT* x = (CWPSTRUCT*)lParam;	//get structure for message info

	// intercept wm_command messages to the update button in the File Dialog
	if (x->message == WM_COMMAND && x->wParam == ID_BUTTONUPDATE && lParam != 0)
	{
		CWnd* dlg = CWnd::FromHandle(x->hwnd);	// get pointer to file dialog
		CButton* upButt = (CButton *) dlg->GetDlgItem(ID_BUTTONUPDATE);
		CButton* delButt = (CButton *) dlg->GetDlgItem(ID_BUTTONDELETE);
		if (upButt->GetCheck())
			delButt->EnableWindow(FALSE);
		else
			delButt->EnableWindow(TRUE);
	}
	return CallNextHookEx(HookHandle, nCode, wParam, lParam);

}

BOOL CAU2FileDialog::OnFileNameOK()
{
	OnDestroy();
	if (m_selectedType != "")
		return CFileDialog::OnFileNameOK();
	else
	{
		MessageBox("You must select an object type","Error",MB_ICONWARNING);
		return 1;
	}
}

CString CAU2FileDialog::GetExtText(extType ext)
{
	switch (ext)
	{
	case TYPE_TLE:
		return _T("TLE Files (*.TLE)");
		break;
	case TYPE_TEXT:
		return _T("Text Files (*.TXT)");
		break;
	case TYPE_ROM:
		return _T("ROM Files (*.ROM)");
		break;
	case TYPE_MTF:
		return _T("Tour Files (*.MTF)");
		break;
	default:
		return _T("All Files (*.*)");
	}

}
