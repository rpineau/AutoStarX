// UserSiteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "au2.h"
#include "UserSiteDlg.h"
#include "AU2View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define USERINFO_HEADER		"asu user info"

/////////////////////////////////////////////////////////////////////////////
// CUserSiteDlg dialog


CUserSiteDlg::CUserSiteDlg(CAutostar *autostar, CWnd* pParent /*=NULL*/)
	: CDialog(CUserSiteDlg::IDD, pParent)
{
	// connect to the autostar & get the user data
	m_pAutostar = autostar;
	BeginWaitCursor();
	m_firstTime = true;
	//{{AFX_DATA_INIT(CUserSiteDlg)
	//}}AFX_DATA_INIT
	m_fontSize = 10;
	m_info = new CUserInfo();
	m_RAPECTable = new CPECTable();
	m_DECPECTable = new CPECTable();
	// overwrite the header to distinguish this file type
	m_dataList.m_header = USERINFO_HEADER;
}


void CUserSiteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_USERCITY, *m_city);
	DDV_MaxChars(pDX, *m_city, 16);
	DDX_Text(pDX, IDC_EDIT_USERNAMEFIRST, *m_firstName);
	DDV_MaxChars(pDX, *m_firstName, 16);
	DDX_Text(pDX, IDC_EDIT_USERNAMELAST, *m_lastName);
	DDX_Text(pDX, IDC_EDIT_USERPOST, *m_postCode);
	DDV_MaxChars(pDX, *m_postCode, 16);
	DDX_Text(pDX, IDC_EDIT_USERSTATE, *m_state);
	DDV_MaxChars(pDX, *m_state, 16);
	DDX_Text(pDX, IDC_EDIT_USERSTREET1, *m_street1);
	DDV_MaxChars(pDX, *m_street1, 30);
	DDX_Text(pDX, IDC_EDIT_USERSTREET2, *m_street2);
	DDV_MaxChars(pDX, *m_street2, 30);
	//{{AFX_DATA_MAP(CUserSiteDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserSiteDlg, CDialog)
	//{{AFX_MSG_MAP(CUserSiteDlg)
	ON_BN_CLICKED(IDC_BUTTON_SITEEDIT, OnButtonSiteEdit)
	ON_LBN_DBLCLK(IDC_LIST_USERSITE, OnDblclkList)
	ON_BN_CLICKED(IDOK, OnButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_SITE_ADD, OnButtonSiteAdd)
	ON_BN_CLICKED(IDC_BUTTON_SITEDELETE, OnButtonSiteDelete)
	ON_BN_CLICKED(IDC_USER_SAVE, OnUserSave)
	ON_BN_CLICKED(IDC_USER_LOAD, OnUserLoad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserSiteDlg message handlers

BOOL CUserSiteDlg::OnInitDialog() 
{
	// if handbox data is not retrieved, abort now
	if (m_firstTime)
	{
		bool retrieve = RetrieveData(true);
		// close the serial port
		m_pAutostar->CloseSerialPort();

		// if retrieve failed, exit dialog
		if (!retrieve)
		{
			EndDialog(IDABORT);
			return false;
		}
	}

	// never attempt to retrieve after the first time
	m_firstTime = false;

	CDialog::OnInitDialog();
	
	// fill up the remaining (non DDX) controls
	CStatic *staticSN = (CStatic *) GetDlgItem(IDC_STATIC_SERIALNUM);
	// if the serial number field is blank
	m_info->m_serialNum.TrimRight();
	if (m_info->m_serialNum == "")
		staticSN->SetWindowText("");	// don't display anything
	else
		staticSN->SetWindowText("Serial Number: " + m_info->m_serialNum);

	// fill the list box
	RefreshList();

	// enable the PEC check box if this is an autostar II
	if (m_RAPECTable->GetActiveFlag())
		((CButton *) GetDlgItem(IDC_CHECK_PEC))->EnableWindow(true);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// send the user data back to the handbox and exit the dialog box
void CUserSiteDlg::OnButtonSend() 
{
	UpdateData(true);
	eAutostarStat stat;
	BeginWaitCursor();

	int siteCount = m_dataList.GetCount(SiteInfo);
	// don't allow more than the maximum number of sites to be sent
	if (siteCount > m_pAutostar->GetMaxSitesAllowed())
	{
		CString output;
		output.Format("This handbox allows a maximum of %i sites.\nPlease delete some sites and resend.",
						m_pAutostar->GetMaxSitesAllowed());
		MessageBoxEx(m_hWnd, output, "Error", MB_ICONEXCLAMATION | MB_TOPMOST, LANG_ENGLISH);
		return;
	}

		// count the number of sites, and update the user record accordingly
	m_info->m_userInfo.MaxSites = siteCount;

	// set the current site to 1, if the the current value is no longer valid
	if (m_info->m_userInfo.CurrentSite > m_info->m_userInfo.MaxSites)
		m_info->m_userInfo.CurrentSite = 1;

	stat = m_pAutostar->InitializeConnection(false, false);

	if (stat != AUTOSTAR_OK)
	{
		MessageBox(m_pAutostar->GetLastError());
		EndDialog(IDABORT);
	}


	// send updated User Info & Personal Information Record
	stat = m_pAutostar->SetUserInfo(m_info);

	if (stat != AUTOSTAR_OK)
	{
		MessageBox(m_pAutostar->GetLastError());
		EndDialog(IDABORT);
	}

	// send updated Site Data
	stat = m_pAutostar->SetSiteInfo(&m_dataList);

	if (stat != AUTOSTAR_OK)
	{
		MessageBox(m_pAutostar->GetLastError());
		EndDialog(IDABORT);
	}

	// if PEC check box was checked, send PEC Tables
	CButton *pec = (CButton *) GetDlgItem(IDC_CHECK_PEC);
	if (pec->GetCheck() == 1)
	{
		stat = m_pAutostar->SetPECTable(m_RAPECTable, AXIS_RA);

		if (stat != AUTOSTAR_OK)
		{
			MessageBox(m_pAutostar->GetLastError());
			EndDialog(IDABORT);
		}

		stat = m_pAutostar->SetPECTable(m_DECPECTable, AXIS_DEC);

		if (stat != AUTOSTAR_OK)
		{
			MessageBox(m_pAutostar->GetLastError());
			EndDialog(IDABORT);
		}
	}

	m_pAutostar->CloseSerialPort();

	EndDialog(IDOK);
}

void CUserSiteDlg::OnButtonSiteEdit() 
{
	EditSite();
}
	


void CUserSiteDlg::RefreshList(bool reSelect)
{
	CListBox* listBox = (CListBox *) GetDlgItem(IDC_LIST_USERSITE);

	// Get the index of all the selected item.
	int selected = listBox->GetCurSel(); 	

	// erase the contents of the list box
	listBox->ResetContent();
	
	POSITION pos = m_dataList.GetHeadPosition(SiteInfo);
	// rebuild the list of sites

	while (pos)
	{
		CBodyData *data = m_dataList.GetNext(pos, SiteInfo);
		listBox->AddString(data->GetKey());
	}	

	if (reSelect)
		listBox->SetCurSel(selected);
}

void CUserSiteDlg::OnDblclkList() 
{
	EditSite();	
}

void CUserSiteDlg::EditSite()
{
	// must call update to avoid losing edit box values
	UpdateData(true);

	CString objectName;			// Key (name) of site
	int item;					// selected list box item index
	CBodyData *data;			// pointer to an object
	int column;


	//Set Labels & Text for Dialog
	CAU2EditDlg dlgEdit;

	dlgEdit.m_fontSize = m_fontSize;

	dlgEdit.m_instructions = "Edit the Site data:";

	CListBox * list = (CListBox *) GetDlgItem(IDC_LIST_USERSITE);
	item = list->GetCurSel();

	if (item == -1)	// if nothing was selected, just exit
		return;

	list->GetText(item, objectName);

	if (!(data = m_dataList.Find(objectName))) return;
	for (column = 0; column <= 9;column++)
	{
		if (column >= data->GetNumFields())
		{
			dlgEdit.m_labelText[column] = " ";
			dlgEdit.m_editText[column] = " ";
			dlgEdit.m_limitsText[column] = " ";
		}
		else
		{
			dlgEdit.m_labelText[column] = data->GetFieldLabel(column);
			dlgEdit.m_editText[column] = data->GetFieldData(column);
			if (data->GetFieldDesc(column)->Modifiable)
				dlgEdit.SetLimitsText(column,data->GetFieldRangeHigh(column),
									data->GetFieldRangeLow(column));
			else
				dlgEdit.m_limitsText[column] = "Not Modifiable";
		}
	}

	// copy the data
	dlgEdit.m_bodyData = data->Copy();

	// Call Dialog
   	if (dlgEdit.DoModal() == IDOK)
	{
	// enter the data from the dialog box into the object
		*data = *dlgEdit.m_bodyData;

		RefreshList();
	}

	// delete copy
	delete dlgEdit.m_bodyData;


}


void CUserSiteDlg::OnButtonSiteAdd() 
{
	if (m_dataList.GetCount(SiteInfo) >= m_pAutostar->GetMaxSitesAllowed())
		MessageBoxEx(m_hWnd, "Maximum Number of Sites Reached.",
					"Error", MB_OK | MB_TOPMOST, LANG_ENGLISH);
	else
		AddSite();	
}


void CUserSiteDlg::AddSite()
{
	// must call update to avoid losing edit box values
	UpdateData(true);

	CBodyDataMaker factory;

	// create the new body data
	CBodyData *data = factory.Make(SiteInfo);

	//Set Labels & Text for Dialog
	CAU2EditDlg dlgEdit;

	if (m_fontSize == LARGE) 
		dlgEdit.m_fontSize = 8;


	dlgEdit.m_instructions = "Enter new site data:";

	for (int column = 0; column <= 9;column++)
	{
		if (column >= data->GetNumFields())
		{
			dlgEdit.m_labelText[column] = " ";
			dlgEdit.m_editText[column] = " ";
			dlgEdit.m_limitsText[column] = " ";
		}
		else
		{
			dlgEdit.m_labelText[column] = data->GetFieldLabel(column);
			dlgEdit.m_editText[column] = data->GetFieldData(column);
			if (data->GetFieldDesc(column)->Modifiable)
				dlgEdit.SetLimitsText(column,data->GetFieldRangeHigh(column),
									data->GetFieldRangeLow(column));
			else
				dlgEdit.m_limitsText[column] = "Not Modifiable";
		}
	}

	// copy the data pointer to the dialogs member data
	dlgEdit.m_bodyData = data;

	// Call Dialog
   	if (dlgEdit.DoModal() == IDOK)
	{
		if (m_dataList.Find(dlgEdit.m_bodyData->GetKey()) &&
			(MessageBox("Name Already Exists. Replace?","Warning",MB_YESNO) == IDNO))
		{
			delete data;
			return;
		}


		// add the new object to the handbox collection
		m_dataList.Add(dlgEdit.m_bodyData);

		// rebuild the list box
		RefreshList();
	}
	else
		delete data;

}

void CUserSiteDlg::OnButtonSiteDelete() 
{
	DeleteSite();

}

void CUserSiteDlg::DeleteSite()
{
	int item;
	CString objectName;

	// get the name of the site to be deleted
	CListBox * list = (CListBox *) GetDlgItem(IDC_LIST_USERSITE);
	item = list->GetCurSel();

	// if nothing is selected, return
	if (item == -1)
		return;

	list->GetText(item, objectName);

	// check if the last remaining site is being deleted
		// if it is, check if this is allowed
if (list->GetCount() == 1 && !m_pAutostar->CheckSiteDelete(false))
		return;
	else
		// check if the current site is being deleted
			// if it is, check if this is allowed
		if ((item + 1 == m_info->m_userInfo.CurrentSite) && !m_pAutostar->CheckSiteDelete(true))
			return;

	// delete it from the collection
	if (m_dataList.Find(objectName))
		m_dataList.Delete(objectName);

	// change the index of the current site if necessary
	if (item + 1 < m_info->m_userInfo.CurrentSite)
		m_info->m_userInfo.CurrentSite--;

	// repaint the list
	RefreshList();
}

// retrieve data from the handbox (upon init dialog)
// set update to true if the user dialog has been displayed, otherwise default is false
bool CUserSiteDlg::RetrieveData(bool update)
{
	// retrieve user info structure
	eAutostarStat stat = m_pAutostar->GetUserInfo(m_info);

	if (stat != AUTOSTAR_OK)
		return false;

	// add user info to collection
	m_dataList.Add(m_info);

	// retrieve site info collection
	stat = m_pAutostar->GetSiteInfo(&m_dataList);

	if (stat != AUTOSTAR_OK)
		return false;

	// retrieve PEC Tables
	stat = m_pAutostar->GetPECTable(m_RAPECTable, AXIS_RA);

	if (stat != AUTOSTAR_OK)
		return false;

	m_dataList.Add(m_RAPECTable);

	stat = m_pAutostar->GetPECTable(m_DECPECTable, AXIS_DEC);

	if (stat != AUTOSTAR_OK)
		return false;

	m_dataList.Add(m_DECPECTable);

	// set the field data
	m_city = &m_info->m_city;
	m_firstName = &m_info->m_firstName;
	m_lastName = &m_info->m_lastName;
	m_postCode = &m_info->m_postCode;
	m_serialNum = &m_info->m_serialNum;
	m_state = &m_info->m_state;
	m_street1 = &m_info->m_street1;
	m_street2 = &m_info->m_street2;

	// update the dialog box's data
	if (update)
		UpdateData(false);

	return true;

}

void CUserSiteDlg::OnUserSave() 
{
	SaveData(true);
}

// set update to true if the user dialog has been displayed, otherwise default is false
void CUserSiteDlg::SaveData(bool update)
{
	if (update)
		UpdateData(true);

	// create a Save As Dialog Box
	TCHAR szFilters[] = _T("User Info files (*.usr)|*.usr||");
	CFileDialog dlg(false, _T("usr"), _T("*.usr"), OFN_OVERWRITEPROMPT, szFilters);

	if (dlg.DoModal() != IDOK)
		return;

	if (m_dataList.SaveToFile(dlg.GetPathName()) != READCOMPLETE)
		MessageBoxEx(m_hWnd, "Error Saving File", "Error", MB_ICONEXCLAMATION | MB_TOPMOST, LANG_ENGLISH);
}


void CUserSiteDlg::OnUserLoad() 
{

	CBodyDataCollection temporaryList;
	temporaryList.m_header = USERINFO_HEADER;

	// create a File Open Dialog Box
	TCHAR szFilters[] = _T("User Info files (*.usr)|*.usr||");
	CFileDialog dlg(true, _T("usr"), _T("*.usr"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

	if (dlg.DoModal() != IDOK)
		return;

	if (temporaryList.LoadFromFile(dlg.GetPathName()) != READCOMPLETE)
	{
		MessageBoxEx(m_hWnd, "Error Loading File", "Error", MB_ICONEXCLAMATION | MB_TOPMOST, LANG_ENGLISH);
		temporaryList.Clear();	// must garbage collect
		return;
	}

	// if the load was successful, copy the temporary data to the permanent
	m_dataList.Clear();
	m_dataList.Add(temporaryList);
	temporaryList.Clear();	// must garbage collect

	POSITION pos = m_dataList.GetHeadPosition(UserInfo);
	if (pos)
		m_info = (CUserInfo *) m_dataList.GetNext(pos, UserInfo);

	pos = m_dataList.GetHeadPosition(PECTable);
	if (pos)
		m_RAPECTable = (CPECTable *) m_dataList.GetNext(pos, PECTable);
	if (pos)
		m_DECPECTable = (CPECTable *) m_dataList.GetNext(pos, PECTable);


	// set the field data
	m_city = &m_info->m_city;
	m_firstName = &m_info->m_firstName;
	m_lastName = &m_info->m_lastName;
	m_postCode = &m_info->m_postCode;
	m_serialNum = &m_info->m_serialNum;
	m_state = &m_info->m_state;
	m_street1 = &m_info->m_street1;
	m_street2 = &m_info->m_street2;

	// update the dialog box's data
	UpdateData(false);
	RefreshList();
}

BOOL CUserSiteDlg::PreTranslateMessage(MSG* pMsg) 
{
	CListBox * list = (CListBox *) GetDlgItem(IDC_LIST_USERSITE);

	switch( pMsg->message ) 
	{
     case WM_KEYUP:
         switch( pMsg->wParam ) 
		 {
			case VK_DELETE:					// process "Delete" key
				if (list->GetCurSel() != -1)	// check if something is selected
					DeleteSite(); // if so, delete it
				else return TRUE; // if not, return
				break;
			default:
				return CDialog::PreTranslateMessage(pMsg);
         }
	}

	return CDialog::PreTranslateMessage(pMsg);
}

