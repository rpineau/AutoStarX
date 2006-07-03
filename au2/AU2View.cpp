// AU2View.cpp : implementation of the CAU2View class
//

#include "stdafx.h"
#include "AU2.h"

#include "AU2Doc.h"
#include "AU2View.h"
#include "AU2ListView.h"
#include "AU2FileDialog.h"
#include "AU2ReplaceDlg.h"
#include "SatelliteDlg.h"
#include "SelectDlg.h"
#include "ComPortDlg.h"
#include "ErrorLogDlg.h"
#include "UserObjSelectDlg.h"
#include "DefineCatalog.h"
#include "UserSiteDlg.h"

#include "MainFrm.h"

#include "Image.h"
#include "MaskedBitmap.h"
#include "Label.h"
#include "autostar\autostar.h"


#include <afxinet.h>	//for internet functions
#include <direct.h>		//for directory functions





#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//const char MEADE_WEB_SITE[] = "http://www.meade.com/support/auto/Autostar/";
//const char MEADE_WEB_SITE[] = "http://i17.yimg.com/17/1469d26f/h/2c3ba3d0/";
const char BUILD_ROM_FILENAME[] = "Build.rom";
const char BUILD_ROM_LX_FILENAME[] = "BuildLX.rom";
const char BUILD_ROM_RCX_FILENAME[] = "BuildRCX.rom";
const char BUILD_ROM_ROOT[] = "Build";
const char BUILD_ROM_LX_ROOT[] = "BuildLX";
const char BUILD_ROM_RCX_ROOT[] = "BuildRCX";
const char BUILD_ROM_EXT[] = ".rom";
//const char BUILD_ROM_VERSION[] = "Build.ver";
//const char ETX_FOLDER[] = "EtxAutostar/";
const CString RT_WARNING = "\nNOTE: In Real Time Mode, edited objects will be moved to\nthe BOTTOM of the list.";

/////////////////////////////////////////////
//
//Function to control the thread for downloading ephemeride files
//
////////////////////////////////////////////
UINT EphemerideThread( LPVOID pParam )
{
	// Get Pointer to Dialog Object
	CAU2View* pView = (CAU2View *) pParam;

	// Get Pointer to Document
	CAU2Doc* pDoc = pView->GetDocument();

	// Disable the send/receive buttons
	pView->EnableButtons(0,2,0,0);

	// prevent the library list view from redrawing while the data is being modified
	CAU2ListView* list = (CAU2ListView *) pView->GetDlgItem(IDC_LIBRARYLIST);
	list->EnableWindow(FALSE);
	list->SetRedraw(FALSE);

	// Download the file(s)
	int successful = pView->m_session.DownloadFile();

	// Beep Beep
	if (pView->m_session.GetLastStatusCode() == 200)
		MessageBeep(MB_OK);
	else
		MessageBeep(MB_ICONEXCLAMATION);

	// Add import status information
	CString importStatus;
	importStatus.Format("%i of %i File(s) Successfully Imported Into Library",
						pView->m_importCount,pView->m_downloadParams.GetSize());
	pView->m_session.AddTransferLogEntry(importStatus);	// error log
	pView->UpdateStatus(importStatus);	// status bar

	// Check if all the files succeeded, if not, offer to show the errors
	if (successful < pView->m_downloadParams.GetSize() ||	// compare saved vs. selected
		(pView->m_importCount < successful))	// compare imported vs. saved
	{
		if (MessageBoxEx(pView->m_hWnd,
			"There were download errors or warnings.\nWould you like to view them?",
			"File Download Error",
			MB_YESNO | MB_TOPMOST, LANG_ENGLISH) == IDYES)
		{
			CErrorLogDlg tDlg;
			tDlg.m_text = pView->m_session.GetTransferLog();
			tDlg.m_title = "File Download Log";
			tDlg.DoModal();
		}
	}

	// enable the send/receive buttons
	pView->EnableButtons(1,2,1,1);
	
	//enable the library list view
	list->EnableWindow();

	// change the body type to whatever was just imported
	CString lastFile = pView->m_downloadParams.GetAt(pView->m_downloadParams.GetUpperBound()).saveFileName;
	BodyType newType;

	if (lastFile.Left(2) == ASTEROID_PREFIX)
		newType = Asteroid;
	if (lastFile.Left(2) == SATELLITE_PREFIX)
		newType = Satellite;
	if (lastFile.Left(2) == COMET_PREFIX)
		newType = Comet;
	if (lastFile.Left(2) == TOUR_PREFIX)
		newType = Tour;

	pView->ChangeListBodyType(newType);

	//VERY IMPORTANT:	thread is exiting, so only now is it safe to allow the
	//					library list view to update itself
	list->SetRedraw();

	// set the flag indicating that downloading is complete
	pView->m_downLoadingBodyData = false;

	// enable the menu buttons
	CMainFrame* pParent = (CMainFrame *) pView->GetSafeOwner();
	CMenu* pMenu = pParent->GetMenu();
	pMenu->EnableMenuItem(ID_FILE_WWW, MF_BYCOMMAND | MF_ENABLED);
	pMenu->EnableMenuItem(1, MF_BYPOSITION | MF_ENABLED);

	// enable the tranfer control buttons
	pView->EnableButtons(2,1,2,2);

	return 0;
}


/////////////////////////////////////////////
//
//Function to control the thread for downloading and upgrading the handbox ROM file
//
////////////////////////////////////////////
UINT UpgradeThread( LPVOID pParam )
{
	CAU2View* pView = (CAU2View *) pParam;
	CAU2Doc* pDoc = pView->GetDocument();	// get pointer to document

	CUserSettings userSettings;	//for retrieving registry info

	// prepare the parameters for downloading;
	CString server, uri, saveFileName, infoString;
	server = CHTTPDownload::ParseURL(userSettings.GetSupportURL(),CHTTPDownload::PI_SERVER);
	uri = CHTTPDownload::ParseURL(userSettings.GetSupportURL(),CHTTPDownload::PI_OBJECT);
	uri += pView->m_upgradeInfo[pView->m_ASType].fileName;

	// Prepare the text for the info message based on handbox type and version
	// the default for most cases is:
	infoString = "User Objects, Blacklash Training, User Sites and Motor Calibration\nwill be retained. \
Use <SETUP> - <RESET> from the handbox to reset.";
	// however, going TO version 1.1a of the ASII gets this additional text:
	if (pView->m_ASType == TYPE_AUTOSTAR2 && 
		(pView->m_upgradeFile.Find("11A.rom") != -1 
		|| pView->m_upgradeFile.Find("11a.rom") != -1))
			infoString += "\n\nOnce the upgrade to this version is complete, please press <SETUP>\n\
- <TELESCOPE> - <CAL. SENSORS> to get improved pointing accuracy.";


	switch (pView->m_upgradeTask)
	{
	case CAU2View::UT_DOWNLOAD:
		pView->DoingProcess("Opening Build.rom, please be patient");
		if (!pView->m_session.DownloadFile(server,uri,pView->m_upgradeFile))
		{
			pView->EnableButtons(1,2,1,1);
			return 0;
		}
		else
			MessageBoxEx(pView->m_hWnd,"Note: The handbox has not been upgraded.  To upgrade handbox, connect the \
handbox to your PC first, hit the \"Upgrade\" button again, and select the file from your local hard drive",
			"Warning",MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND, LANG_ENGLISH);
			pView->DownloadComplete(TRUE);
			pView->DownloadComplete(TRUE);

		break;


	case CAU2View::UT_DOWNLOAD_UPGRADE:
		pView->DoingProcess("Opening Build.rom, please be patient");
		if (!pView->m_session.DownloadFile(server,uri,pView->m_upgradeFile))
		{
			pView->EnableButtons(1,2,1,1);
			return 0;
		}
		else
			pView->DownloadComplete();

	case CAU2View::UT_UPGRADE:	 // Note there is no break: UT_DOWNLOAD_UPGRADE continues

		if (MessageBoxEx(pView->m_hWnd,"Are you sure you want to upgrade now?","Warning",MB_OKCANCEL | MB_ICONWARNING | MB_TOPMOST, LANG_ENGLISH) == IDOK)
		{
			// disabled this message box for AS2 because everything always gets wiped out in recent versions of firmware
			if (pView->m_ASType != TYPE_AUTOSTAR2)
				MessageBoxEx(pView->m_hWnd,infoString,"Clear User Data",MB_OK | MB_TOPMOST, LANG_ENGLISH);
			pDoc->m_autostar.SetStatCallBack(pView);
			pDoc->m_autostar.SendProgram(pView->m_upgradeFile, true, pView->m_eraseBanks);
		}
		else
			pView->EnableButtons(1,2,1,1);
		break;

	default:
		return 0;

	}
	return 1;
}



void AFXAPI DDX_BodyData(CDataExchange* pDX, int nIDC, CBodyData *pData, int field)
{
// check if field is used by this data type
	if (field >= pData->GetNumFields())
		return;

	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	TCHAR szWindowText[50];
	::GetWindowText(hWndCtrl, szWindowText, 49);
	CString		errorString = "Enter Value Between ";

	if (pDX->m_bSaveAndValidate)
	{
		switch (pData->SetFieldData(field, CString(szWindowText)))
		{
		case NOT_MODIFIABLE :
		case OK :
			break;

		case RANGEERROR	:
		case INVALIDTYPE :
			errorString += pData->GetFieldRangeLow(field) + " and " + pData->GetFieldRangeHigh(field);
			AfxMessageBox(errorString);
			pDX->Fail();
			break;

		}

	}
	else
	{
		::SetWindowText(hWndCtrl, pData->GetFieldData(field).GetBuffer(20));
	}
}




/////////////////////////////////////////////////////////////////////////////
// CAU2EditDlg dialog


CAU2EditDlg::CAU2EditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAU2EditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAU2EditDlg)
	//}}AFX_DATA_INIT
	m_instructions = "";
	m_fontSize = 10;
}

// map edit controls to member variables
void CAU2EditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAU2EditDlg)
	DDX_Control(pDX, IDC_STATIC_RANGE, m_rangeLabel);
	DDX_BodyData(pDX, IDC_EDIT1, m_bodyData, 0);
	DDX_BodyData(pDX, IDC_EDIT2, m_bodyData, 1);
	DDX_BodyData(pDX, IDC_EDIT3, m_bodyData, 2);
	DDX_BodyData(pDX, IDC_EDIT4, m_bodyData, 3);
	DDX_BodyData(pDX, IDC_EDIT5, m_bodyData, 4);
	DDX_BodyData(pDX, IDC_EDIT6, m_bodyData, 5);
	DDX_BodyData(pDX, IDC_EDIT7, m_bodyData, 6);
	DDX_BodyData(pDX, IDC_EDIT8, m_bodyData, 7);
	DDX_BodyData(pDX, IDC_EDIT9, m_bodyData, 8);
	DDX_BodyData(pDX, IDC_EDIT10, m_bodyData, 9);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAU2EditDlg, CDialog)
	//{{AFX_MSG_MAP(CAU2EditDlg)
//	ON_EN_UPDATE(IDC_EDIT1, OnUpdateEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAU2EditDlg message handlers
BOOL CAU2EditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// create array of pointers to static field name controls
	m_label[0] = (CStatic *)GetDlgItem(IDC_KEYTEXT);
	m_label[1] = (CStatic *)GetDlgItem(IDC_DATA1TEXT);
	m_label[2] = (CStatic *)GetDlgItem(IDC_DATA2TEXT);
	m_label[3] = (CStatic *)GetDlgItem(IDC_DATA3TEXT);
	m_label[4] = (CStatic *)GetDlgItem(IDC_DATA4TEXT);
	m_label[5] = (CStatic *)GetDlgItem(IDC_DATA5TEXT);
	m_label[6] = (CStatic *)GetDlgItem(IDC_DATA6TEXT);
	m_label[7] = (CStatic *)GetDlgItem(IDC_DATA7TEXT);
	m_label[8] = (CStatic *)GetDlgItem(IDC_DATA8TEXT);
	m_label[9] = (CStatic *)GetDlgItem(IDC_DATA9TEXT);

	// create array of pointers to static limits controls
	m_limits[1] = (CStatic *)GetDlgItem(IDC_DATA1LIMITS);
	m_limits[2] = (CStatic *)GetDlgItem(IDC_DATA2LIMITS);
	m_limits[3] = (CStatic *)GetDlgItem(IDC_DATA3LIMITS);
	m_limits[4] = (CStatic *)GetDlgItem(IDC_DATA4LIMITS);
	m_limits[5] = (CStatic *)GetDlgItem(IDC_DATA5LIMITS);
	m_limits[6] = (CStatic *)GetDlgItem(IDC_DATA6LIMITS);
	m_limits[7] = (CStatic *)GetDlgItem(IDC_DATA7LIMITS);
	m_limits[8] = (CStatic *)GetDlgItem(IDC_DATA8LIMITS);
	m_limits[9] = (CStatic *)GetDlgItem(IDC_DATA9LIMITS);

	// create array of pointers to edit controls
	m_edit[0] = (CEdit *)GetDlgItem(IDC_EDIT1);
	m_edit[1] = (CEdit *)GetDlgItem(IDC_EDIT2);
	m_edit[2] = (CEdit *)GetDlgItem(IDC_EDIT3);
	m_edit[3] = (CEdit *)GetDlgItem(IDC_EDIT4);
	m_edit[4] = (CEdit *)GetDlgItem(IDC_EDIT5);
	m_edit[5] = (CEdit *)GetDlgItem(IDC_EDIT6);
	m_edit[6] = (CEdit *)GetDlgItem(IDC_EDIT7);
	m_edit[7] = (CEdit *)GetDlgItem(IDC_EDIT8);
	m_edit[8] = (CEdit *)GetDlgItem(IDC_EDIT9);
	m_edit[9] = (CEdit *)GetDlgItem(IDC_EDIT10);

	m_rangeLabel.SetFontUnderline(TRUE); //underline static text
	m_rangeLabel.SetFontBold(TRUE); // make static text bold

	//set the focus to the first edit window
	m_edit[0]->SetFocus();

	// Get pointer to the document
//	CMainFrame* pParent = (CMainFrame *) GetParent();
//	CAU2Doc* pDoc = (CAU2Doc *) pParent->GetActiveDocument();
	
	// set static and edit control text
	for (int i = 0; i < m_bodyData->GetNumFields(); i++)
	{
		// limit the number of chars that can be entered
		if (m_bodyData->GetFieldDesc(i)->fieldSize)
			m_edit[i]->SetLimitText(m_bodyData->GetFieldDesc(i)->fieldSize);

		// set edit box label text
		m_label[i]->SetWindowText(m_bodyData->GetFieldLabel(i));

		// set values of edit box
/*		if (m_editText[i] != " ")
			m_edit[i]->SetWindowText(m_editText[i]);		*/
		// set range limits text
		if (i != 0)
			m_limits[i]->SetWindowText(m_limitsText[i]);
		// disable fields that cannot be modified
		if (m_limitsText[i] == "Not Modifiable")
			m_edit[i]->EnableWindow(FALSE);
	}

	// display instructions text
	CStatic* statInst = (CStatic *) GetDlgItem(IDC_EDITTERLABEL);
	if (m_instructions != "")
		statInst->SetWindowText(m_instructions);

	// adjust dialog size based on number of fields
	CRect rect, controlSize;
	GetWindowRect(&rect);	// size of full dialog
//	m_edit[pDoc->m_libraryCollection.GetNumFields(pDoc->m_bodyType) - 1]->GetWindowRect(&controlSize);
	m_edit[m_bodyData->GetNumFields() - 1]->GetWindowRect(&controlSize);
	rect.bottom = controlSize.bottom + 10;

	SetWindowPos(&wndTopMost, 0, 0, rect.Width(), rect.Height(), SWP_NOMOVE | SWP_SHOWWINDOW);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//override the DoModal function
int CAU2EditDlg::DoModal()
{
	CDialogTemplate dlt;
	int nResult;

	//load dialog template
	if (!dlt.Load(MAKEINTRESOURCE(CAU2EditDlg::IDD))) return -1;

//	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
//	CAU2View* pView = (CAU2View *) pParent->GetActiveView();

	//set font size
//	if (pView->m_SystemFontSize == SMALL)
//		dlt.SetFont("MS Sans Serif", 10);
//	else
		dlt.SetFont("MS Sans Serif",m_fontSize);


	// Get pointer to the modified dialog template
	LPSTR pData = (LPSTR)GlobalLock(dlt.m_hTemplate);

	// Declare using a custom template
	m_lpszTemplateName = NULL;
	InitModalIndirect(pData);

	// Display dialog box
	nResult = CDialog::DoModal();

	// Unlock memory object
	GlobalUnlock(dlt.m_hTemplate);

	return nResult;
}

////////////////////////////////////////////////////////////////////////////
// Function to generate the string for the limits text and assign it to
// m_limitsText[fieldNum]
//
// Input:	fieldNum - column number of field
//			hiLimit - upper limit of data
//			loLimit - lower limit of data
//
// Output:	None
/////////////////////////////////////////////////////////////////////////////
void CAU2EditDlg::SetLimitsText(int fieldNum, CString hiLimit, CString loLimit)
{
	if (fieldNum == 0) return;	// no limits for "key" field
	CString text;
	text.Format("(%s to %s)",loLimit,hiLimit);
	m_limitsText[fieldNum] = text;
}

/////////////////////////////////////////////////////////////////////////////
// CUpgradeHbxDlg dialog


CUpgradeHbxDlg::CUpgradeHbxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpgradeHbxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpgradeHbxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_downloadFlag = TRUE;	//initially assume download will be performed
	m_pSession = NULL;
	m_hdWarning = "";
	m_wwwVer = "";
	m_checkedWWW = FALSE;
	m_eraseBanks = false;
}


void CUpgradeHbxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpgradeHbxDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpgradeHbxDlg, CDialog)
	//{{AFX_MSG_MAP(CUpgradeHbxDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_CHECK_WWW, OnButtonCheckWWW)
	ON_CBN_SELCHANGE(IDC_UPGRADE_TYPE, OnSelChangeUpgradeType)
	ON_BN_CLICKED(IDC_UPGRADE_ERASE, OnUpgradeErase)
	ON_CBN_SELCHANGE(IDC_LOCAL_VERS, OnSelChangeLocalVers)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpgradeHbxDlg message handlers



BOOL CUpgradeHbxDlg::OnInitDialog() 
{

	CDialog::OnInitDialog();

	CFrameWnd* pParent = (CFrameWnd *) GetParent();
	CAU2View* pView = (CAU2View *) pParent->GetActiveView();

	CComboBox* combo = (CComboBox *) GetDlgItem(IDC_UPGRADE_TYPE);
	CButton* wwwButt = (CButton *) GetDlgItem(IDC_UPGRADE_WWW);
	CButton* hdButt = (CButton *) GetDlgItem(IDC_UPGRADE_HD);
	CStatic* wwwText = (CStatic *) GetDlgItem(IDC_UPGRADE_WWW_TEXT);
	CStatic* hdText = (CStatic *) GetDlgItem(IDC_UPGRADE_HD_TEXT);
	CComboBox* hdCB = (CComboBox *) GetDlgItem(IDC_LOCAL_VERS);
	CStatic* hbxText = (CStatic *) GetDlgItem(IDC_UPGRADE_HBX_TEXT);
	CButton* checkButt = (CButton *) GetDlgItem(IDC_BUTTON_CHECK_WWW);
	CButton* eraseButt = (CButton *) GetDlgItem(IDC_UPGRADE_ERASE);

	wwwText->EnableWindow();
	wwwButt->EnableWindow();
	hdText->EnableWindow();
	hdCB->EnableWindow();
	checkButt->EnableWindow();
	hbxText->EnableWindow();

	combo->SetCurSel(pView->m_ASType);
		
	if (pView->m_ASType == TYPE_UNKNOWN)
	{

		wwwText->EnableWindow(FALSE);
		wwwButt->EnableWindow(FALSE);
		hdText->EnableWindow(FALSE);
		hdCB->EnableWindow(FALSE);
		checkButt->EnableWindow(FALSE);
		hbxText->EnableWindow(FALSE);
	}
	else
	{
		BuildComboList(pView->m_ASType);
	}

	
	// Disable buttons that do not apply
	if (m_wwwText == "")
	{
		m_wwwText = "(Internet version information not available)";
//		hdButt->SetCheck(1);
	}
	
	// Set the button description text
	wwwText->SetWindowText(m_wwwText);
	hbxText->SetWindowText(m_hbxText);


	// handle the case of the handbox not being connected
	if (m_hdWarning != "")	// if warning text has been specified,
	{
		CStatic* warning = (CStatic *) GetDlgItem(IDC_STATIC_HD_WARNING);
		warning->SetWindowText(m_hdWarning);	//display warning text
		hdText->SetWindowText("(Handbox is not connected)");

		// disable dlg items that do not apply
		hdButt->EnableWindow(FALSE);
		wwwButt->SetCheck(1);
		hdButt->SetCheck(0);
		hdCB->EnableWindow(FALSE);
		eraseButt->EnableWindow(false);
	}
	else
	{
		combo->EnableWindow(FALSE);
		if (pView->m_ASType == TYPE_AUTOSTAR2 || pView->m_ASType == TYPE_RCX)
			eraseButt->EnableWindow(true);
	}

	//Make window the topmost window
	SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);




	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUpgradeHbxDlg::OnUpgradeErase() 
{
	CButton* check = (CButton *) GetDlgItem(IDC_UPGRADE_ERASE);
	
	if (check->GetCheck() == 1)
	{
		MessageBoxEx(this->m_hWnd,"This option will erase all user data, including objects, site information,\n\
telescope calibrations, etc.  Do not select this option unless you are\nhaving serious \
problems upgrading your handbox","Warning", MB_ICONEXCLAMATION | MB_TOPMOST, LANG_ENGLISH);
		m_eraseBanks = true;
	}
	else
		m_eraseBanks = false;
}


// sort the items in the upgrade version list box
int CUpgradeHbxDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// lParamSort contains a pointer to the list view control.
   // The lParam of an item is just its index.
   CListCtrl* pListCtrl = (CListCtrl*) lParamSort;
   CString    strItem1 = pListCtrl->GetItemText(lParam1, 0);
   CString    strItem2 = pListCtrl->GetItemText(lParam2, 0);

   return -strcmp(strItem2, strItem1);
}


void CUpgradeHbxDlg::OnSelChangeLocalVers() 
{
	CButton* hdButt = (CButton *) GetDlgItem(IDC_UPGRADE_HD);
	CButton* wwwButt = (CButton *) GetDlgItem(IDC_UPGRADE_WWW);

	hdButt->SetCheck(1);
	wwwButt->SetCheck(0);
}

void CUpgradeHbxDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// set the download flag based on the state of the radio buttons.
	// I know this code is redundant, but the button may be checked, even
	// when it is disabled, therefore the double if statement to be sure
	CButton* wwwButt = (CButton *) GetDlgItem(IDC_UPGRADE_WWW);
	CButton* hdButt = (CButton *) GetDlgItem(IDC_UPGRADE_HD);
	if (wwwButt->GetCheck()) 
		m_downloadFlag = true;
	else if (hdButt->GetCheck())
		m_downloadFlag = false;

	CComboBox* hdCB = (CComboBox *) GetDlgItem(IDC_LOCAL_VERS);

	if (hdCB->GetCount())	//only do something if items exist in the combo box in the first place
	{	
		// copy the selected version file name to m_upgradeFile
		if (hdCB->GetCurSel() != CB_ERR)
		{
			m_upgradeFile = *(CString *)hdCB->GetItemDataPtr(hdCB->GetCurSel());
		}
		// if nothing selected use the first one
		else
			m_upgradeFile = *(CString *)hdCB->GetItemDataPtr(0);

		// delete all the file names. Plug those leaks!
		for (int i = 0; i < hdCB->GetCount(); i++)
			delete (CString *)hdCB->GetItemDataPtr(i);

	}

	// check if user obtained the version from the internet, if not, get it
	if (m_nModalResult != IDCANCEL && m_downloadFlag && m_wwwVer == "")
		OnButtonCheckWWW();
}

VOID CUpgradeHbxDlg::BuildComboList(ASType type)
{
	CUserSettings		uSettings;
	CComboBox* hdCB = (CComboBox *) GetDlgItem(IDC_LOCAL_VERS);
	CButton* wwwButt = (CButton *) GetDlgItem(IDC_UPGRADE_WWW);
	CButton* hdButt = (CButton *) GetDlgItem(IDC_UPGRADE_HD);
	CStatic* hdText = (CStatic *) GetDlgItem(IDC_UPGRADE_HD_TEXT);

	hdCB->ResetContent();

	CFileFind	finder;
	CString		*foundFile;
	CFile		sendFile;
	CString		fileVer;
	CString		pattern;

	// pattern directory string
	if (type == TYPE_AUTOSTAR)
		pattern = uSettings.GetEphemDirectory() + CString(_T("build*.rom"));
	else if (type == TYPE_AUTOSTAR2)
		pattern = uSettings.GetEphemDirectory() + CString(_T("buildLX*.rom"));
	else
		pattern = uSettings.GetEphemDirectory() + CString(_T("buildRCX*.rom"));

	// start the file search
	BOOL bWorking = finder.FindFile(pattern);

	// if no files found then disable the window
	if (!bWorking)
	{
		hdButt->EnableWindow(FALSE);
		hdCB->EnableWindow(FALSE);
		m_hdText = "(No upgrade files found on hard drive)";
		hdText->SetWindowText(m_hdText);
		wwwButt->SetCheck(1);
	}

	int itemCount = -1;

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		CFile file;
		// skip . and .. files but this should not happen
		if (finder.IsDots())
			continue;

		// if Autostar Type 1 ignore "buildLx*.rom" files also
		if ((type == TYPE_AUTOSTAR) && (finder.GetFileTitle().Find("LX") != -1))
			continue;

		// Get a file to test
		foundFile = new CString(finder.GetFilePath());

		char buff[sizeof(ReleaseHdType)];
		file.Open(*foundFile, CFile::modeRead);

		// read it all in
		file.Read((void *)buff, sizeof(ReleaseHdType));

		// put the version in the list
		int iNum = hdCB->AddString((char *)(((ReleaseHdType *)buff)->version));
		hdCB->SetItemDataPtr(iNum, (void *)foundFile);

		// keep track of the index of the last item
		itemCount ++;
		
	}	

	// select the last item (highest rev level)
	hdCB->SetCurSel(itemCount);

	
}



void CUpgradeHbxDlg::OnSelChangeUpgradeType() 
{
	CFrameWnd* pParent = (CFrameWnd *) GetParent();
	CAU2View* pView = (CAU2View *) pParent->GetActiveView();

	CComboBox* combo = (CComboBox *) GetDlgItem(IDC_UPGRADE_TYPE);

	pView->m_ASType = (ASType) combo->GetCurSel();

	combo->ResetContent();

	// delete all the file names. Plug those leaks!
	CComboBox* hdCB = (CComboBox *) GetDlgItem(IDC_LOCAL_VERS);
	for (int i = 0; i < hdCB->GetCount(); i++)
		delete (CString *)hdCB->GetItemDataPtr(i);

	OnInitDialog();
	if (m_checkedWWW)
		OnButtonCheckWWW();
}

void CUpgradeHbxDlg::OnOK() 
{
	// Make sure something was selected!
	CFrameWnd* pParent = (CFrameWnd *) GetParent();
	CAU2View* pView = (CAU2View *) pParent->GetActiveView();

	if (pView->m_ASType == TYPE_UNKNOWN)
	{
		MessageBox("You must specify the handbox type","Error");
		return;
	}
	
	CDialog::OnOK();
}

//////////////////////////////////////////////////////////////////////
// 
// Function to check the version of the software on the WWW
//		and store it in member variable m_wwwVer
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
void CUpgradeHbxDlg::OnButtonCheckWWW() 
{

	if (!m_pSession)
		return;

	m_checkedWWW = TRUE;	// remember to check version automatically if type changes

	CFrameWnd* pParent = (CFrameWnd *) GetParent();
	CAU2View* pView = (CAU2View *) pParent->GetActiveView();

	BeginWaitCursor();
	CUserSettings user;
	CString url = user.GetSupportURL(),	// get support URL from registry
			server,
			uri;
	INTERNET_PORT port;
	DWORD type;
	
	if (AfxParseURL(url, type, server, uri, port))	// parse the URL into its components
	{
		m_pSession->SetServerName(server);	// set the server name
		uri += pView->m_upgradeInfo[pView->m_ASType].fileName;
		m_pSession->SetURIName(uri);		// set the URI
	}
	
	unsigned int numBytes = sizeof(ReleaseHdType);
	char* buffer = m_pSession->DownloadFile(numBytes);	// get the header from the WWW
	ReleaseHdType* pHdr = (ReleaseHdType *) buffer;		

	// make sure something was returned
	if (!numBytes || !buffer)
		MessageBoxEx(m_hWnd,"Could not retrieve version information.  Check internet connection.",
					 "Internet Error",MB_OK | MB_SETFOREGROUND,LANG_ENGLISH);
	else
	{
		//update the member variables with the version info
		m_wwwVer = pHdr->version;
		m_wwwText = "Download version " + m_wwwVer + " from the WWW";
		CStatic* wwwText = (CStatic *) GetDlgItem(IDC_UPGRADE_WWW_TEXT);
		wwwText->SetWindowText(m_wwwText);
	}

	free(buffer);
	EndWaitCursor();
}

/////////////////////////////////////////////////////////////////////////////
// CAU2AllDlg dialog

CAU2AllDlg::CAU2AllDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAU2AllDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAU2AllDlg)
	m_staticText = _T("");
	m_selectAll = FALSE;
	m_selectAsteroids = FALSE;
	m_selectComets = FALSE;
	m_selectLandmarks = FALSE;
	m_selectTours = FALSE;
	m_selectUserObj = FALSE;
	m_selectSatellites = FALSE;
	//}}AFX_DATA_INIT
}


void CAU2AllDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAU2AllDlg)
	DDX_Text(pDX, IDC_STATIC_ALL, m_staticText);
	DDV_MaxChars(pDX, m_staticText, 500);
	DDX_Check(pDX, IDC_CHECK_ALL_ALL, m_selectAll);
	DDX_Check(pDX, IDC_CHECK_ALL_ASTEROIDS, m_selectAsteroids);
	DDX_Check(pDX, IDC_CHECK_ALL_COMETS, m_selectComets);
	DDX_Check(pDX, IDC_CHECK_ALL_LANDMARKS, m_selectLandmarks);
	DDX_Check(pDX, IDC_CHECK_ALL_TOURS, m_selectTours);
	DDX_Check(pDX, IDC_CHECK_ALL_USEROBJ, m_selectUserObj);
	DDX_Check(pDX, IDC_CHECK_ALL_SATELLITES, m_selectSatellites);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAU2AllDlg, CDialog)
	//{{AFX_MSG_MAP(CAU2AllDlg)
	ON_BN_CLICKED(IDC_CHECK_ALL_ALL, OnCheckAll)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(IDC_CHECK_ALL_ASTEROIDS,IDC_CHECK_ALL_USEROBJ,OnCheckObject)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAU2AllDlg message handlers


void CAU2AllDlg::OnCheckObject(UINT nID) 
{

}

void CAU2AllDlg::OnCheckAll() 
{
	bool state;
	CButton* all = (CButton *) GetDlgItem(IDC_CHECK_ALL_ALL);
	// if all is checked
	if (all->GetCheck()) state = FALSE;
	else state = TRUE;
	for (int i = 0; i <= m_checkArray.GetUpperBound(); i++)
	{
		CWnd* box = GetDlgItem(m_checkArray[i]);
		box->EnableWindow(state);
	}
	
}

BOOL CAU2AllDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_checkArray.Add(IDC_CHECK_ALL_ASTEROIDS);
	m_checkArray.Add(IDC_CHECK_ALL_SATELLITES);
	m_checkArray.Add(IDC_CHECK_ALL_COMETS);
	m_checkArray.Add(IDC_CHECK_ALL_LANDMARKS);
	m_checkArray.Add(IDC_CHECK_ALL_TOURS);
	m_checkArray.Add(IDC_CHECK_ALL_USEROBJ);
	
	//Make window the topmost window
	SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



/////////////////////////////////////////////////////////////////////////////
// CAU2View

IMPLEMENT_DYNCREATE(CAU2View, CFormView)

BEGIN_MESSAGE_MAP(CAU2View, CFormView)
	//{{AFX_MSG_MAP(CAU2View)
	ON_WM_PAINT()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIBRARYLIST, OnGetDispInfo)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIBRARYLIST, OnColumnClick)
	ON_BN_CLICKED(ID_EXIT, OnExit)
	ON_BN_CLICKED(IDC_ADVANCED, OnAdvanced)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIBRARYLIST, OnEndLabelEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIBRARYLIST, OnDoubleClickList)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_LASTEROIDSRADIO, OnRadioClickedLibrary)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_HASTEROIDSRADIO, OnRadioClickedHandbox)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_TOHBX, OnButtonToHbx)
	ON_BN_CLICKED(IDC_BUTTON_TOLIB, OnButtonToLib)
	ON_BN_CLICKED(IDC_BUTTON_RETRIEVE, OnButtonRetrieve)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnButtonConnect)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIBRARYLIST, OnBeginDrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_UPGRADE, OnUpgrade)
	ON_COMMAND(ID_FILE_SAVE_HBX, OnFileSaveHbx)
	ON_COMMAND(ID_FILE_OPEN_HBX, OnFileOpenHbx)
	ON_NOTIFY(NM_SETFOCUS, IDC_HANDBOXLIST, OnSetFocusHandboxList)
	ON_COMMAND(ID_FILE_RESTORE, OnFileRestoreHbx)
	ON_BN_CLICKED(IDC_BUTTON_TOHBX_REFRESH, OnButtonToHbxRefresh)
	ON_BN_CLICKED(IDC_BUTTON_TOHBX_ALL, OnButtonToHbxAll)
	ON_BN_CLICKED(IDC_BUTTON_TOLIB_ALL, OnButtonToLibAll)
	ON_BN_CLICKED(IDC_BUTTON_TOLIB_REFRESH, OnButtonToLibRefresh)
	ON_COMMAND(ID_FILE_WWW, OnFileDownload)
	ON_BN_CLICKED(IDC_REALTIME, OnButtonRealTime)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_HANDBOXLIST, OnGetDispInfo)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_HANDBOXLIST, OnColumnClick)
	ON_COMMAND(ID_VIEW_ADVANCED, OnAdvanced)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_HANDBOXLIST, OnEndLabelEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_HANDBOXLIST, OnDoubleClickList)
	ON_BN_CLICKED(IDC_LSATELLITESRADIO, OnRadioClickedLibrary)
	ON_BN_CLICKED(IDC_LCOMETSRADIO, OnRadioClickedLibrary)
	ON_BN_CLICKED(IDC_LLANDMARKSRADIO, OnRadioClickedLibrary)
	ON_BN_CLICKED(IDC_LTOURSRADIO, OnRadioClickedLibrary)
	ON_BN_CLICKED(IDC_LUSEROBJECTSRADIO, OnRadioClickedLibrary)
	ON_BN_CLICKED(IDC_HCOMETSRADIO, OnRadioClickedHandbox)
	ON_BN_CLICKED(IDC_HLANDMARKSRADIO, OnRadioClickedHandbox)
	ON_BN_CLICKED(IDC_HSATELLITESRADIO, OnRadioClickedHandbox)
	ON_BN_CLICKED(IDC_HTOURSRADIO, OnRadioClickedHandbox)
	ON_BN_CLICKED(IDC_HUSEROBJECTSRADIO, OnRadioClickedHandbox)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_HANDBOXLIST, OnBeginDrag)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_OPTIONS_RECENT,ID_OPTIONS_BACKGROUND,OnOptions)
	ON_COMMAND_RANGE(ID_EDIT_EDIT,ID_EDIT_CLEAR_HBX,OnEditMenu)
	ON_COMMAND_RANGE(ID_OPTIONS_BAUD_115K, ID_OPTIONS_BAUD_9600, OnOptionsBaud)
	ON_COMMAND_RANGE(ID_TOOLS_GC, ID_TOOLS_USERSITE, OnTools)


END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CAU2View construction/destruction

CAU2View::CAU2View()
	: CFormView(GetDialogID())

{
	//{{AFX_DATA_INIT(CAU2View)
	//}}AFX_DATA_INIT
	// TODO: add construction code here


	//initialize the drag source variable to NONE
	m_dragSource = NONE;

	//initialize the upgrade thread task variable to NOTHING
	m_upgradeTask = UT_NOTHING;

	//check the registry to determine the path of the handbox backup file
	m_backupFile = m_userSettings.GetInstallDirectory() + "handbox.bak";

	// init downloadingbodydata flag
	m_downLoadingBodyData = false;

	// init imported objects count
	m_importCount = 0;

	// init realTimeMode to false
	m_realTimeMode = false;

	// you can't be too safe.  Set Erase Banks (during Upgrade) to FALSE!!!
	m_eraseBanks = false;

	// Initialize the type of autostar to unknown
	m_ASType = TYPE_UNKNOWN;

	// Initialize the array of upgrade file parameters
	//	m_upgradeInfo
	m_upgradeInfo.SetSize(4);
	m_upgradeInfo[TYPE_AUTOSTAR].fileName = BUILD_ROM_FILENAME;
	m_upgradeInfo[TYPE_AUTOSTAR].root = BUILD_ROM_ROOT;
	m_upgradeInfo[TYPE_AUTOSTAR].ext = BUILD_ROM_EXT;
	m_upgradeInfo[TYPE_AUTOSTAR2].fileName = BUILD_ROM_LX_FILENAME;
	m_upgradeInfo[TYPE_AUTOSTAR2].root = BUILD_ROM_LX_ROOT;
	m_upgradeInfo[TYPE_AUTOSTAR2].ext = BUILD_ROM_EXT;
	m_upgradeInfo[TYPE_RCX].fileName = BUILD_ROM_RCX_FILENAME;
	m_upgradeInfo[TYPE_RCX].root = BUILD_ROM_RCX_ROOT;
	m_upgradeInfo[TYPE_RCX].ext = BUILD_ROM_EXT;

	// record the ID of the main thread
	m_mainThreadID = AfxGetThread()->m_nThreadID;
}

CAU2View::~CAU2View()
{
}

void CAU2View::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAU2View)
	DDX_Control(pDX, IDC_LABEL_SELECT, m_labelSelect);
	DDX_Control(pDX, IDC_LABEL_REFRESH, m_labelRefresh);
	DDX_Control(pDX, IDC_LABEL_ALL, m_labelAll);
	DDX_Control(pDX, IDC_STATIC_HANDBOX2, m_labelHbx2);
	//}}AFX_DATA_MAP

}


BOOL CAU2View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);


}

// initialize the main view
// note: this function is called each time a new document is opened
void CAU2View::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	GetParentFrame()->RecalcLayout();
	SIZE size;
	size.cx = 0;
	size.cy = 0;
	SetScaleToFitSize(size);	//eliminate the scroll bars

	CAU2Doc* pDoc = GetDocument();

	static int afterFirstTime = 0;	//indicate if first time initializing window


	if (!m_brush.m_hObject)
		m_brush.CreateSolidBrush(CHARCOAL); // color background brush 

	if (!afterFirstTime)	// only do these things once
		m_background.LoadFile(m_userSettings.GetBackground());

	// reset the custom user object button labels
	CButton* userButton = (CButton *) GetDlgItem(IDC_LUSEROBJECTSRADIO);
	userButton->SetWindowText(CPersist::Abbreviate(pDoc->m_lBodyTypeLabel[pDoc->m_customDisplay], RADIO_SIZE));
	userButton = (CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO);
	userButton->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[pDoc->m_customDisplay], RADIO_SIZE));

	// Initialize List Views

	CAU2ListView* m_lList = (CAU2ListView *)GetDlgItem(IDC_LIBRARYLIST);
	CAU2ListView* m_hList = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);

	// Specify Extended formatting for list views
	m_lList->GetListCtrl().SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_hList->GetListCtrl().SetExtendedStyle(LVS_EX_FULLROWSELECT);


	// Put object data into List View
	m_lList->InitializeList(&pDoc->m_libraryCollection,pDoc->m_bodyType);
	m_hList->InitializeList(pDoc->m_handboxCollection,pDoc->m_bodyType);

	// "Check" the asteroid buttons as default items
	SynchRadioButtons();

	// If no handbox.bak file exists, disable the restore menu item
	// Get a pointer to the Parent Window
	CMainFrame* pParent = (CMainFrame* ) GetParent();	
	// Get a pointer to the menu
	CMenu* menu = (CMenu *) pParent->GetMenu();

	// Check if backup file exists and is readable
	CFileStatus fStat;
	if (!CFile::GetStatus(m_backupFile, fStat))	
		// Disable the menu item
		menu->EnableMenuItem(ID_FILE_RESTORE,MF_GRAYED);

	if (!afterFirstTime)	// do all these things ONLY the first time program is launched
	{
		// Set the max baud rate based on the registry value
		pDoc->m_autostar.SetMaxBaudRate(m_userSettings.GetBaud());

		// Start in advanced mode if registry option is set (first time only)
		if (m_userSettings.GetOptions(CUserSettings::ADVANCED))
			OnAdvanced();

		// Connect to handbox if registry option is set
		if (m_userSettings.GetOptions(CUserSettings::CONNECT))
			OnButtonConnect();

		// Set Verify Mode to registry entry
		pDoc->m_autostar.SetVerifyMode(m_userSettings.GetOptions(CUserSettings::VERIFY));

		// Connect to handbox & retrieve data if option is set
		if (m_userSettings.GetOptions(CUserSettings::RETRIEVE))
		{
			CAU2Doc* pDoc = GetDocument();	// get pointer to document

			OnButtonConnect();	// connect first

			// disable the send/ receive buttons
			EnableButtons(0,2,0,0);

			//retrieve data from handbox and store in pDoc->m_handboxCollection
			pDoc->m_autostar.SetStatCallBack(this);
			if (pDoc->m_hbxConnected)
				pDoc->LoadAutostarData();
		}
	}

/*	if (!::IsWindow(m_staticHbx2.m_hWnd))  // skip if already attached
		// attach handbox memory static control to CStaticST
		m_staticHbx2.SubclassDlgItem(IDC_STATIC_HANDBOX2, this);
*/

	// set the callback pointer for download operations
	m_session.SetCallbackPointer(this);

	afterFirstTime++;	// indicate it's not the first time anymore

	int fontSize = 8;
	if (m_SystemFontSize == SMALL) fontSize = 10;
	m_labelAll.SetFontSize(fontSize);
	m_labelSelect.SetFontSize(fontSize);
	m_labelRefresh.SetFontSize(fontSize);
	m_labelAll.SetBkColor(MEADEBLUE);
	m_labelSelect.SetBkColor(MEADEBLUE);
	m_labelRefresh.SetBkColor(MEADEBLUE);
	m_labelAll.SetTextColor(WHITE);
	m_labelSelect.SetTextColor(WHITE);
	m_labelRefresh.SetTextColor(WHITE);


}

/////////////////////////////////////////////////////////////////////////////
// CAU2View diagnostics

#ifdef _DEBUG
void CAU2View::AssertValid() const
{
	CFormView::AssertValid();
}

void CAU2View::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

// get a pointer to the document class
CAU2Doc* CAU2View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAU2Doc)));
	return (CAU2Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAU2View message handlers

// Draw bitmap on background of application
void CAU2View::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Load Bitmaps
	CMaskedBitmap meadeLogo,title;
	CRect rect;
	GetClientRect(&rect);

	CRect wndRect;
	GetClientRect(&wndRect);
	if (m_background.IsPictureValid())
		m_background.DrawImage(dc,&wndRect);

	meadeLogo.LoadBitmap(IDB_MEADE_LOGO);
	meadeLogo.Draw(&dc,14,24);
	title.LoadBitmap(IDB_TITLE);
	title.Draw(&dc,100,25);
}

// Answer List View callback request with values for fields of List View
void CAU2View::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CAU2Doc* pDoc = GetDocument();
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
 
	// get the number of fields for the list being painted
	int numFields;
	if (pDispInfo->hdr.idFrom == IDC_LIBRARYLIST)
		numFields = pDoc->m_libraryCollection.GetNumFields(pDoc->m_bodyType);
	else 
		numFields = pDoc->m_handboxCollection->GetNumFields(pDoc->m_bodyType);
	
	if (pDispInfo->item.mask & LVIF_TEXT) 
	{
       ITEMINFO* pItem = (ITEMINFO*) pDispInfo->item.lParam;
		// with the addition of custom objects, you now need a collection that contains one of
		// that object to get the number of fields
	   if (pDispInfo->item.iSubItem < numFields)
			 ::lstrcpy (pDispInfo->item.pszText, pItem->strData[pDispInfo->item.iSubItem]);
	}
	*pResult = 0;

}

//Compare List View Items in Ascending Order
int CALLBACK CAU2View::CompareFuncAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	ITEMINFO* pItem1 = (ITEMINFO*) lParam1;
	ITEMINFO* pItem2 = (ITEMINFO*) lParam2;
	int nResult;

	CBodyData* pData1 = pItem1->data;
	CBodyData* pData2 = pItem2->data;

	//nResult = pItem1->strData[lParamSort].CompareNoCase (pItem2->strData[lParamSort]);
	if (pData1->Compare(pData2,TRUE,lParamSort,0,0))
		nResult = -1;
	else
		nResult = 1;
	return nResult;
}


//Compare List View Items in Descending Order
int CALLBACK CAU2View::CompareFuncDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	ITEMINFO* pItem1 = (ITEMINFO*) lParam1;
	ITEMINFO* pItem2 = (ITEMINFO*) lParam2;
	int nResult;

	CBodyData* pData1 = pItem1->data;
	CBodyData* pData2 = pItem2->data;

	//nResult = pItem1->strData[lParamSort].CompareNoCase (pItem2->strData[lParamSort]);
	if (pData1->Compare(pData2,FALSE,lParamSort,0,0))
		nResult = -1;
	else
		nResult = 1;
	return nResult;
}

// Sort List View Items when user clicks on column heading (Library)
void CAU2View::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	CAU2ListView* lList = (CAU2ListView *)GetDlgItem(IDC_LIBRARYLIST);
	CAU2ListView* hList = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);
	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();

	BeginWaitCursor();
	switch (pNMHDR->idFrom)
	{
	case IDC_LIBRARYLIST:
		SortColumns(lList,pNMListView->iSubItem);
		//indicate a change has been made to the document
		pDoc->SetModifiedFlag();
		break;
	case IDC_HANDBOXLIST:
		// disable this function in real time mode
		if (m_realTimeMode)
		{
			DoingProcess("Cannot sort in Real Time Mode");
			return;
		}
		SortColumns(hList,pNMListView->iSubItem);
		pDoc->m_handboxModified = true;
		break;
	}
	EndWaitCursor();
	*pResult = 0;
}

// Sort column after column heading has been clicked
void CAU2View::SortColumns(CAU2ListView *list, int item)
{
	static int sortDirectionLibrary[10] = {1,1,1,1,1,1,1,1,1,1};
	static int sortDirectionHandbox[10] = {1,1,1,1,1,1,1,1,1,1};
	//1=Ascending, 0=Descending
	static int sortColumnLibrary = 0;
	static int sortColumnHandbox = 0;

	CAU2Doc* pDoc = GetDocument();	// get pointer to document


	if (list->GetDlgCtrlID() == IDC_LIBRARYLIST)
	{
		//sort the collection object
		pDoc->m_libraryCollection.SortBy
				((sortDirectionLibrary[item] == 1) ? TRUE : FALSE,
				  pDoc->m_bodyType,item);
		//sort the list view
		list->GetListCtrl().SortItems (
				(sortDirectionLibrary[item] == 1) ? CompareFuncAsc : CompareFuncDes, item);
		sortColumnLibrary = item;
		//toggle so next time it's different
		sortDirectionLibrary[item]
			= (sortDirectionLibrary[item] + 1) % 2;		
	}
	else if (list->GetDlgCtrlID() == IDC_HANDBOXLIST)
	{
		//sort the collection object
		pDoc->m_handboxCollection->SortBy
				((sortDirectionHandbox[item] == 1) ? TRUE : FALSE,
				  pDoc->m_bodyType,item);
		//sort the list view
		list->GetListCtrl().SortItems (
				(sortDirectionHandbox[item] == 1) ? CompareFuncAsc : CompareFuncDes, item);

		sortColumnHandbox = item;
		//toggle so next time it's different
		sortDirectionHandbox[item]
			= (sortDirectionHandbox[item] + 1) % 2;		
	}

}

// process Exit button command
void CAU2View::OnExit() 
{
	// check if the autostar is busy doing upload or download
	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();
	if (pDoc->m_autostar.m_mode == BUSY)
		if (MessageBox("Exiting the program while uploading may\ndamage \
the handbox. Proceed with exit?","Confirm Exit",MB_OKCANCEL) == IDCANCEL)
			return;
	
	// check if handbox data was modified but not downloaded
	if (pDoc->m_handboxModified)
		if (MessageBox("Handbox data has not been saved to Autostar!\n Proceed with exit?", 
			"Confirm Exit", MB_OKCANCEL) == IDCANCEL)
			return;


	//prompt user to save modifications before exit
	if (pDoc->SaveModified())
	{
//		if (pDoc->m_hbxConnected)
//			OnButtonRestart();

		// set the serial port value back to 9600
		pDoc->m_autostar.SetMaxBaudRate(CSerialPort::b9600, true);

		GetParentFrame()->DestroyWindow();
	}


}

// process Advanced button command
void CAU2View::OnAdvanced() 
{
	static int state = 1;	//set Window to Advanced (1) on first time

	CButton* AButton = (CButton *)GetDlgItem(IDC_ADVANCED);

	// Get a pointer to the Parent Window
	CMainFrame* pParent = (CMainFrame* ) GetParent();	

	// Get a pointer to the menu
	CMenu* menu = (CMenu *) pParent->GetMenu();

	//ToggleWindowSize(CMainFrame.m_winSize[state]);
	if (state == 1)
	{
		menu->CheckMenuItem(ID_VIEW_ADVANCED,MF_CHECKED);
		AButton->SetWindowText("<<Advanced Functions <<");
		pParent->SetWinSize(1);
		pParent->m_winSizeState = state;
		pParent->ToggleStatusBar(state);
	}
	else
	{
		menu->CheckMenuItem(ID_VIEW_ADVANCED,MF_UNCHECKED);
		AButton->SetWindowText(">>Advanced Functions >>");
		pParent->SetWinSize(0);
		pParent->m_winSizeState = state;
		pParent->ToggleStatusBar(state);
	}

	pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);

	state = (state + 1) % 2;		//toggle so next time it's different

	// update memory & object counts info
	UpdateMemInfo();

}

// Change key when an item label is clicked and modified
void CAU2View::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	CAU2Doc* pDoc = GetDocument();	// get pointer to document

	CAU2ListView* list;
	CBodyDataCollection* collection;

	switch (pNMHDR->idFrom)	//determine which list sent the message
	{
	case IDC_LIBRARYLIST:
		list = (CAU2ListView *)GetDlgItem(IDC_LIBRARYLIST);
		collection = &pDoc->m_libraryCollection;
		break;
	case IDC_HANDBOXLIST:
		list = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);
		collection = pDoc->m_handboxCollection;
		break;
	}

	//Get info about item selected
	LV_ITEM* pItem= &(pDispInfo)->item;
	int iItemIndx= pItem->iItem;

	//if new name is empty, or name change is aborted exit function
	if (((CString) pItem->pszText).IsEmpty()) return;
	
	//Get info about data to be replaced
	CString oldName = list->GetListCtrl().GetItemText(iItemIndx,0);

	// check to see if name already exists
	if (collection->Find(pItem->pszText))
	{
		MessageBox("Name already exists.  Please re-enter.","Error",MB_ICONWARNING);
		*pResult = FALSE;
		return;
	}

	// set library object text to entered text
	CBodyData *data;
	if (data = collection->Find(oldName))
	{
		// change the item name in the collection
		data->SetKey(pItem->pszText);


		// if real time mode is activated and the handbox list view is chosen
		if (m_realTimeMode && list->GetDlgCtrlID() == IDC_HANDBOXLIST)
		{
			BeginWaitCursor();
			EnableButtons(2,2,2,0);	// disable real-time button (if applicable)

			// open the serial port
			pDoc->m_autostar.InitializeConnection(false);

			// delete the handbox object
			DeleteObjectRealTime(oldName);

			// send the new object to the handbox
			SendOneObjectRealTime(data);

			// close the serial port
			pDoc->m_autostar.CloseSerialPort();

			// create a new copy of the object
			CBodyData* newData = data->Copy();

			// delete the original object from the collection
			pDoc->m_handboxCollection->Delete(data->GetKey());

			// add the object to the end of the collection;
			pDoc->m_handboxCollection->Add(newData);

			// replace the list view item with the new item
			ReplaceListViewItem(iItemIndx, list, newData, true);

			EnableButtons(2,2,2,1);	// enable real-time button (if applicable)
		}
		else
			// replace the list view item with the new item
			ReplaceListViewItem(iItemIndx, list, data);

		list->Invalidate();
		list->RedrawWindow();

		*pResult = TRUE;
	}

}

// Call up EditObject function when user double-clicks list item
void CAU2View::OnDoubleClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CAU2ListView* list;

	// determine which list is being clicked
	switch (pNMHDR->idFrom)
	{
	case IDC_LIBRARYLIST:
		list = (CAU2ListView *)GetDlgItem(IDC_LIBRARYLIST);
		break;
	case IDC_HANDBOXLIST:
		list = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);
		break;
	}

	EditObject(list);

	*pResult = 0;
}

// Call up Edit dialog and edit object data
void CAU2View::EditObject(CAU2ListView* list)
{
	CStringArray objectArray;	// array of object names to be modified
	CStringArray *bodyTypeLabel;// ptr to array of body labels
	CUIntArray itemArray;		// array of object itemes to be modified
	CString objectName;			// Key (name) of one object
	int item;					// list view item item
	CBodyData *data;			// pointer to an object
	int column;

	// Get pointer to document
	CAU2Doc* pDoc = GetDocument();
	
	// Get position of first item selected
	POSITION posList = list->GetListCtrl().GetFirstSelectedItemPosition();

	// step through all items selected and build arrays of itemes and names
	while (posList)
		{
			item = list->GetListCtrl().GetNextSelectedItem(posList);
			objectName = list->GetListCtrl().GetItemText(item,0);
			objectArray.Add(objectName);
			itemArray.Add(item);
		}

	//Get pointers to Corresponding Object Data
	CBodyDataCollection* collection;
	if (list->GetDlgCtrlID() == IDC_LIBRARYLIST)
	{
		collection = &pDoc->m_libraryCollection;
		bodyTypeLabel = &pDoc->m_lBodyTypeLabel;
	}
	else if (list->GetDlgCtrlID() == IDC_HANDBOXLIST)
	{
		collection = pDoc->m_handboxCollection;
		bodyTypeLabel = &pDoc->m_hBodyTypeLabel;
	}
	else return;

	// iterate loop for every selected object
	for (item = 0; item <= itemArray.GetUpperBound(); item++)
	{
		//Set Labels & Text for Dialog
		CAU2EditDlg dlgEdit;

		if (m_SystemFontSize == LARGE) 
			dlgEdit.m_fontSize = 8;

		dlgEdit.m_instructions = "Edit the " + bodyTypeLabel->GetAt(pDoc->m_bodyType) + " data:";
		if (m_realTimeMode && list->GetDlgCtrlID() == IDC_HANDBOXLIST)
			dlgEdit.m_instructions += RT_WARNING;

		if (!(data = collection->Find(objectArray[item]))) return;
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

			// update the handbox if in realtime mode
			if (m_realTimeMode && list->GetDlgCtrlID() == IDC_HANDBOXLIST)
			{
				BeginWaitCursor();
				EnableButtons(2,2,2,0);	// disable real-time button (if applicable)

				// open the serial port
				pDoc->m_autostar.InitializeConnection(false);

				// delete the handbox object
				DeleteObjectRealTime(objectArray[item]);

				// send the new object to the handbox
				SendOneObjectRealTime(data);

				// close the serial port
				pDoc->m_autostar.CloseSerialPort();

				// create a new copy of the object
				CBodyData* newData = data->Copy();

				// delete the original object from the collection
				pDoc->m_handboxCollection->Delete(data->GetKey(true));

				// add the object to the end of the collection;
				pDoc->m_handboxCollection->Add(newData);

				//add the new item to the end of the list view list
				ReplaceListViewItem(itemArray[item], list, newData, true);

				EnableButtons(2,2,2,1);	// enable real-time button (if applicable)
			}
			else
				// change the list view item to the new data
				ReplaceListViewItem(itemArray[item], list, data);

		}

		// delete copy
		delete dlgEdit.m_bodyData;
			
	}
	list->Invalidate();
	list->RedrawWindow();
}

// replace the contents of a single list view item with new data
void CAU2View::ReplaceListViewItem(int nItem, CAU2ListView *list, CBodyData *data, bool moveToEnd)
{
	CAU2Doc *pDoc = GetDocument();
	
	//Delete old listview structure
	delete (ITEMINFO*) list->GetListCtrl().GetItemData(nItem);



	if (moveToEnd)
	{
		// delete the existing list view entry
		list->GetListCtrl().DeleteItem(nItem);
		// rebuild the list view
		list->Refresh(pDoc->m_handboxCollection, pDoc->m_bodyType);
	}
	else
	{
		//Create new ListView Structure and copy data to it
		ITEMINFO* pItemNew;
		try 
		{
			pItemNew = new ITEMINFO;
		}
		catch (CMemoryException* e) 
		{
			e->Delete ();
		}

		// fill the columns of the list view item with the new data
		pItemNew->strData[0] = data->GetKey();
		for (int column = 1; column < data->GetNumFields(); column++)
		{
			pItemNew->strData[column] = data->GetFieldData(column);
		}
		pItemNew->data = data;
		
		// Set List View item to new data
		list->GetListCtrl().SetItemData(nItem,(LPARAM) pItemNew);
	}

	list->GetListCtrl().UpdateWindow();

	if (list->GetDlgCtrlID() == IDC_LIBRARYLIST)
		pDoc->SetModifiedFlag();	//indicate a change has been made to the document	
}

// Process right-mouse click and call context menu
void CAU2View::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// get pointers to list views
	CAU2ListView* libWindow = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
	CAU2ListView* hbxWindow = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);
	CAU2ListView* list;
	CPoint lPos = point, hPos = point, lUPos = point, hUPos = point;
	libWindow->ScreenToClient(&lPos);
	hbxWindow->ScreenToClient(&hPos);

	CRect lRect,hRect;
	libWindow->GetClientRect(&lRect);
	hbxWindow->GetClientRect(&hRect);

	// get pointers to the user object buttons
	CWnd* lUser = GetDlgItem(IDC_LUSEROBJECTSRADIO);
	CWnd* hUser = GetDlgItem(IDC_HUSEROBJECTSRADIO);
	lUser->ScreenToClient(&lUPos);
	hUser->ScreenToClient(&hUPos);
	CRect lURect, hURect;
	lUser->GetClientRect(&lURect);
	hUser->GetClientRect(&hURect);


	// determine which list window or button is being clicked
	if (lRect.PtInRect(lPos))
		list = libWindow;
	else if (hRect.PtInRect(hPos))
		list = hbxWindow;
	else if (lURect.PtInRect(lUPos))
	{
		OnContextMenuUserObj(lUser);
		return;
	}
	else if (hURect.PtInRect(hUPos))
	{
		OnContextMenuUserObj(hUser);
		return;
	}
	else
		return;

	// get index of selected item (if any)
	int selectedItem = list->GetListCtrl().GetSelectionMark();

	// Load Context Menu
	CMenu menu;
	menu.LoadMenu(IDR_MENU2);

	// Get pointer to Context Menu
	CMenu* pContextMenu = menu.GetSubMenu(0);

	// Process individual menu items
	if (selectedItem == -1) // check to see if nothing was selected
	{
		pContextMenu->EnableMenuItem(ID_CONTEXT_EDIT,MF_GRAYED);
		pContextMenu->EnableMenuItem(ID_CONTEXT_DELETE,MF_GRAYED);
	}

	// get pointer to document
	CAU2Doc* pDoc = GetDocument();	

	// Call Context Menu
	int cmd = (int) pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON
												| TPM_RIGHTBUTTON | TPM_RETURNCMD,
												point.x,point.y, list);

	// Process command depending on which menu item was selected
	switch (cmd)	
	{
	case ID_CONTEXT_EDIT:		// edit object(s)
		EditObject(list);
		break;
	case ID_CONTEXT_DELETE:		// delete object(s)
		DeleteObject(list);
		break;
	case ID_CONTEXT_NEW:
		NewObject(list);		// add a new object
		break;
	}
	UpdateWindow();

}

// display the Custom User Objects Context Menu & select object type
void CAU2View::OnContextMenuUserObj(CWnd* source)
{
	// disable for 497's
	if (m_ASType == TYPE_AUTOSTAR)
		return;

	CAU2Doc *pDoc = GetDocument();
	
	CButton* userRadio = (CButton *) source;

	// immediately change to User Objects, so old object type will no longer be pressed
	ChangeListBodyType(UserObj);

	CUserObjSelectDlg dlg;

	// get the location of the button pressed
	CRect sourceRect;
	userRadio->GetWindowRect(sourceRect);
	// pass that info to to the dlg, so it knows where to draw itself
	dlg.m_origin.x = sourceRect.left;
	dlg.m_origin.y = sourceRect.top;

	// set the button labels
	CStringArray *bodyTypeLabel;
	if (userRadio->GetDlgCtrlID() == IDC_LUSEROBJECTSRADIO)
		bodyTypeLabel = &pDoc->m_lBodyTypeLabel;
	else
		bodyTypeLabel = &pDoc->m_hBodyTypeLabel;

	dlg.m_pButtonLabels = bodyTypeLabel;

	// tell the dialog what size fonts to scale to
	dlg.m_fontSize = m_SystemFontSize;
	if (dlg.DoModal() != IDOK)
		return;

	// change the list views to that body type
	ChangeListBodyType(dlg.GetBodyType());

	// get pointers to the two main buttons
	CButton* lButton = (CButton *) GetDlgItem(IDC_LUSEROBJECTSRADIO);
	CButton* hButton = (CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO);

	// update the selected button's text
	lButton->SetWindowText(CPersist::Abbreviate(pDoc->m_lBodyTypeLabel[pDoc->m_bodyType], RADIO_SIZE));
	hButton->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[pDoc->m_bodyType], RADIO_SIZE));

	// if undefined, send a status instruction
	if (bodyTypeLabel->GetAt(pDoc->m_bodyType).Find(DEFAULT_CAT_NAME) != -1)
		DoingProcess("To define a custom object catalog, right mouse-click and select \"New\"");
	else
		DoingProcess("Ready");

}


// delete an object from the collection indicated by list
void CAU2View::DeleteObject(CAU2ListView *list)
{
	// Get pointer to document
	CAU2Doc* pDoc = GetDocument();
	
	// Get position of first item selected
	POSITION posList = list->GetListCtrl().GetFirstSelectedItemPosition();

	CStringArray objectArray;	// array of object names to be modified
	CUIntArray indexArray;		// array of object indexes to be modified
	CString objectName;			// Key (name) of one object
	int item;					// list view item index

	// step through all items selected and build arrays of indexes and names
	while (posList)
		{
			item = list->GetListCtrl().GetNextSelectedItem(posList);
			objectName = list->GetListCtrl().GetItemText(item,0);
			objectArray.Add(objectName);
			indexArray.Add(item);
		}

	//Get pointers to Corresponding Object Data
	CBodyDataCollection* collection;
	SourceList source;
	UINT radioID;
	if (list->GetDlgCtrlID() == IDC_LIBRARYLIST)
	{
		collection = &pDoc->m_libraryCollection;
		source = LIBRARY;
		radioID = IDC_LUSEROBJECTSRADIO;
	}
	else if (list->GetDlgCtrlID() == IDC_HANDBOXLIST)
	{
		collection = pDoc->m_handboxCollection;
		source = HANDBOX;
		radioID = IDC_HUSEROBJECTSRADIO;
	}
	else return;

	if(MessageBox("Delete the selected item(s)?","Confirm Delete",MB_YESNO) == IDYES)
	{
		BeginWaitCursor();
		EnableButtons(2,2,2,0);	// disable real-time button (if applicable)

		list->SetRedraw(false);
		//delete items from list view in descending order
		for (int i = indexArray.GetUpperBound(); i >= 0 ; i--)
		{
		 	delete (ITEMINFO*) list->GetListCtrl().GetItemData(indexArray[i]);
			list->GetListCtrl().DeleteItem(indexArray[i]);
		}
		if (m_realTimeMode && list->GetDlgCtrlID() == IDC_HANDBOXLIST)
			pDoc->m_autostar.InitializeConnection(false);
		//delete items from collection
		for (i = 0; i <= objectArray.GetUpperBound(); i++)
		{
			collection->Delete(objectArray[i]);
			// if in real time mode, delete the handbox object NOW
			if (list->GetDlgCtrlID() == IDC_HANDBOXLIST && m_realTimeMode)
				DeleteObjectRealTime(objectArray[i]);
		}

		// if this was the last item in a custom collection, reset the catalog name
		if (pDoc->m_bodyType >= UserObj20 && collection->GetCount(pDoc->m_bodyType) == 0)
		{
			pDoc->ResetBodyTypeLabels(source, collection, pDoc->m_bodyType);
			((CButton *) GetDlgItem(radioID))->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[pDoc->m_bodyType], RADIO_SIZE));
		}

		if (m_realTimeMode && list->GetDlgCtrlID() == IDC_HANDBOXLIST)
		{
			pDoc->m_autostar.CloseSerialPort();
			// send a status message
			DoingProcess("Delete Complete");
		}

		EndWaitCursor();
		EnableButtons(2,2,2,1);	// enable real-time button (if applicable)
		list->SetRedraw();
	}		
	// Clear the selection list
	list->GetListCtrl().SetSelectionMark(-1);

	//update the list
	list->Invalidate();
	list->RedrawWindow();

	// update memory & object counts info
	UpdateMemInfo();
	//indicate a change has been made to the document
	if (list->GetDlgCtrlID() == IDC_LIBRARYLIST)
		pDoc->SetModifiedFlag();
}

// Function to delete an object from the handbox in real time
eAutostarStat CAU2View::DeleteObjectRealTime(CString objectName)
{
	eAutostarStat stat;
	// send a status message
	DoingProcess("Deleting: " + objectName);

	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();
	stat = pDoc->m_autostar.DeleteOneObject(pDoc->m_bodyType,objectName);

	if (stat != AUTOSTAR_OK)
		return stat;

	// if this was the last of this body type, then also delete the catalog
	if (pDoc->m_handboxCollection->GetCount(pDoc->m_bodyType) == 0)
		stat = pDoc->m_autostar.DeleteCatalog(pDoc->m_bodyType);

	return stat;
}


// Add a new object to the collection indicated by list
void CAU2View::NewObject(CAU2ListView *list)
{
	//Get pointer to document
	CAU2Doc* pDoc = GetDocument();

	// can't create tours from scratch yet
	if (pDoc->m_bodyType == Tour)
	{
		AfxMessageBox("Can't create Tours from scratch yet.");
		return;
	}
		

	//Get pointers to Corresponding Object Data
	CBodyDataCollection* collection;
	CStringArray *bodyTypeLabel;
	if (list->GetDlgCtrlID() == IDC_LIBRARYLIST)
	{
		collection = &pDoc->m_libraryCollection;
		bodyTypeLabel = &pDoc->m_lBodyTypeLabel;
	}
	else if (list->GetDlgCtrlID() == IDC_HANDBOXLIST)
	{
		collection = pDoc->m_handboxCollection;
		bodyTypeLabel = &pDoc->m_hBodyTypeLabel;
	}
	else return;

	// if this is the first item in a Custom User Catalog, call the Define Catalog functino
	CBodyData *data = NULL;
	if (collection->GetCount(pDoc->m_bodyType) == 0 && pDoc->m_bodyType >= UserObj20 && pDoc->m_bodyType < BodyTypeMax)
	{
		if (!DefineCatalog(list, collection, &data))
			return;
	}
	else
	{
		// see if one already exists, if so, copy it
		POSITION pos = collection->GetHeadPosition(pDoc->m_bodyType);
		if (pos)
			data = collection->GetNext(pos, pDoc->m_bodyType);

		// create the new body data
		data = pDoc->m_factory.Make(pDoc->m_bodyType, data);
	}

	//Set Labels & Text for Dialog
	CAU2EditDlg dlgEdit;

	if (m_SystemFontSize == LARGE) 
		dlgEdit.m_fontSize = 8;


	dlgEdit.m_instructions = "Enter new " + bodyTypeLabel->GetAt(pDoc->m_bodyType) + " data:";

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
		if (collection->Find(dlgEdit.m_bodyData->GetKey()) &&
			(MessageBox("Name Already Exists. Replace?","Warning",MB_YESNO) == IDNO))
		{
			delete data;
			return;
		}


		// add the new object to the handbox collection
		collection->Add(dlgEdit.m_bodyData);

		// if we're in realTime mode and the source is the library list,
		// also send it to the handbox
		if (list->GetDlgCtrlID() == IDC_HANDBOXLIST && m_realTimeMode)
		{
			BeginWaitCursor();
			// open the serial port
			pDoc->m_autostar.InitializeConnection(false);

			// send it on its way
			SendOneObjectRealTime(data);

			// close the serial port
			pDoc->m_autostar.CloseSerialPort();
		}

		// rebuild the list view
		ChangeListBodyType(pDoc->m_bodyType);
		list->Refresh(collection, pDoc->m_bodyType);
	}
	else
		delete data;

	// update memory & object counts info
	UpdateMemInfo();
	//indicate a change has been made to the document
	if (list->GetDlgCtrlID() == IDC_LIBRARYLIST)
		pDoc->SetModifiedFlag();
}


// Define a new custom user object catalog
bool CAU2View::DefineCatalog(CAU2ListView *list, CBodyDataCollection *bodyData, CBodyData **data)
{
	CDefineCatalog dlg;
	CAU2Doc *pDoc = GetDocument();
	CStringArray *bodyTypeLabel;

	if (list->GetDlgCtrlID() == IDC_LIBRARYLIST)
		bodyTypeLabel = &pDoc->m_lBodyTypeLabel;
	else if (list->GetDlgCtrlID() == IDC_HANDBOXLIST)
		bodyTypeLabel = &pDoc->m_hBodyTypeLabel;


	// set the initial value of the catalog name
	dlg.m_catName = bodyTypeLabel->GetAt(pDoc->m_bodyType);

	// display the dialog box
	if (dlg.DoModal() != IDOK)
		return false;

	// create a new BodyData object as a template
	*data = pDoc->m_factory.Make(pDoc->m_bodyType);

	// fill in the new object's data
	((CUserObjEx *) *data)->SetCatalogName(dlg.m_catName);
	for (int index = 0; index <= dlg.m_fieldNames.GetUpperBound(); index++)
		((CUserObjEx *) *data)->SetFieldName(index + REQD_USEROBJEX_FIELDS, dlg.m_fieldNames.GetAt(index));
	((CUserObjEx *) *data)->SetNumFields(REQD_USEROBJEX_FIELDS + dlg.m_fieldNames.GetSize());


	// update the body type and labels
	bodyTypeLabel->SetAt(pDoc->m_bodyType, dlg.m_catName);
	ChangeListBodyType(pDoc->m_bodyType);

	// get pointers to the two main buttons
	CButton* lButton = (CButton *) GetDlgItem(IDC_LUSEROBJECTSRADIO);
	CButton* hButton = (CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO);

	// update the selected button's text
	lButton->SetWindowText(CPersist::Abbreviate(pDoc->m_lBodyTypeLabel[pDoc->m_bodyType], RADIO_SIZE));
	hButton->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[pDoc->m_bodyType], RADIO_SIZE));

	return true;
}

// Query the system font size and set the appropriate dialog resource
UINT CAU2View::GetDialogID()
{
	IDD1 = IDD_AU2_FORM2;
	// Check the system font to find out size
	CDC ScreenDC;
	ScreenDC.CreateIC(_T("DISPLAY"),NULL,NULL,NULL);
	const int nLogDPIX = ScreenDC.GetDeviceCaps(LOGPIXELSX),
			  nLogDPIY = ScreenDC.GetDeviceCaps(LOGPIXELSY);


	if (nLogDPIX == 96 && nLogDPIY == 96)	// Small Fonts
	{
		m_SystemFontSize = SMALL;
		return CAU2View::IDD1; 
	}
	else
		if (nLogDPIX == 120 && nLogDPIY == 120)	// Large Fonts
			m_SystemFontSize = LARGE;
		else
		{
			m_SystemFontSize = LARGE;
			MessageBox("You are not using a standard system font size. The dialog boxes may not appear correctly.",
						"Warning",MB_ICONWARNING);
		}

	return CAU2View::IDD;  
}

// answer thread callback to indicate retrieve operation is completed
void CAU2View::RetrieveComplete(eAutostarStat stat, bool noPopup)
{
	//Get reference to Parent
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	
	CAU2Doc* pDoc = GetDocument();	// get pointer to document

	CAU2ListView* list = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);	// ptr to list view

	if (stat == AUTOSTAR_OK)
	{
		pDoc->m_hbxConnected = TRUE;
		pDoc->m_statusText = "Data retrieved";

		// Copy handbox collection to handbox list view
		list->Refresh(pDoc->m_handboxCollection, pDoc->m_bodyType);

		// Reset the radio button labels
		pDoc->ResetBodyTypeLabels(HANDBOX, pDoc->m_handboxCollection);
		// update the radio button display text
		((CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO))->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[pDoc->m_bodyType], RADIO_SIZE));


		if (!noPopup)
			MessageBox("Data successfully retrieved from Autostar","Complete",MB_ICONINFORMATION);

	// save a backup copy of the handbox list to m_backupFile
		if (pDoc->m_handboxCollection->SaveToFile(m_backupFile) != READCOMPLETE)
			MessageBox("Error creating backup file","Warning",MB_ICONWARNING);
	}
	else
	{
		MessageBox("Check Autostar cable and try again.","Error",MB_ICONWARNING);
		pDoc->m_statusText = pDoc->m_autostar.GetLastError();
		pDoc->m_hbxConnected = FALSE;
	}

	if (!noPopup)	// do not enable anything in this mode...just pass on through
	{
		// enable the hbx list view 
		list->EnableWindow();
		list->SetRedraw();

		if (!m_realTimeMode)
			//enable all buttons
			EnableButtons(1,1,1,1);
		else
		{
			// enable the transfer & upgrade buttons
			EnableButtons(1,1,2,1);
			pDoc->m_statusText = "Real-time Mode Enabled";
		}
	}

	UpdateMemInfo(); // update memory & object counts info

	// determine which thread is calling this function, 
	TRACE("\nMain Thread: %i          Current Thread: %i\n", m_mainThreadID, AfxGetThread()->m_nThreadID);
	if (AfxGetThread()->m_nThreadID == m_mainThreadID)
		// if the main thread, update status bar directly
		pParent->m_wndStatusBar.GetStatusBarCtrl().SetText(pDoc->m_statusText,0,SBT_NOBORDERS);
	else
		// otherwise, post a message
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);


}

// answer thread callback to indication send operation is completed
void CAU2View::SendComplete(eAutostarStat stat, bool noPopup)
{

	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	
	CAU2Doc* pDoc = GetDocument();	// get pointer to document
	CAU2ListView* hList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
	
	BeginWaitCursor();
//	stat = AUTOSTAR_OK;//debug only!
	if (stat == AUTOSTAR_OK)	
	{
		pDoc->m_hbxConnected = TRUE;
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_VER);
		if (!noPopup)
		{
			DeleteListView(HANDBOX);
			hList->InitializeList(pDoc->m_handboxCollection,pDoc->m_bodyType);
			MessageBox("Data successfully sent to Autostar","Complete",MB_ICONINFORMATION);
		}
			pDoc->m_statusText = "Upload Complete";
	}
	else
	{
		pDoc->m_statusText = pDoc->m_autostar.GetLastError();
		if (pDoc->m_autostar.m_lastError == BAD_CHECKSUM)
			MessageBox("ROM file corrupt, Press Upgrade again and select Internet: version instead of Local:","Error", MB_ICONWARNING);
		else
			MessageBox("Check Autostar cable then power cycle Autostar and try again.","Error",MB_ICONWARNING);
		pDoc->m_hbxConnected = FALSE;
	}

	// determine which thread is calling this function, 
	TRACE("\nMain Thread: %i          Current Thread: %i\n", m_mainThreadID, AfxGetThread()->m_nThreadID);
	if (AfxGetThread()->m_nThreadID == m_mainThreadID)
		// if the main thread, update status bar directly
		pParent->m_wndStatusBar.GetStatusBarCtrl().SetText(pDoc->m_statusText,0,SBT_NOBORDERS);
	else
		// otherwise, post a message
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);

	UpdateMemInfo(); // update memory & object counts info

	if (!noPopup)	// do not enable anything in this mode, just pass on through
	{
		// enable the hbx list view 
		hList->EnableWindow();
		hList->SetRedraw();

	//enable the send/ receive buttons
//		if (m_ASType == TYPE_AUTOSTAR2) 
			ActivateSafeMode(FALSE);
//		else
//			EnableButtons(1,1,1,2);
	}
	EndWaitCursor();
	Invalidate();
}

// answer thread callback during upload/download
void CAU2View::PercentComplete(int val)
{
	CString stat;
	//Get reference to Parent
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CAU2Doc* pDoc = GetDocument();
	stat.Format("%s: %d%% Complete", m_doingProcess.GetBuffer(5),val);

	pDoc->m_statusText = stat;

	// determine which thread is calling this function, 
	if (AfxGetThread()->m_nThreadID == m_mainThreadID)
		// if the main thread, update status bar directly
		pParent->m_wndStatusBar.GetStatusBarCtrl().SetText(stat,0,SBT_NOBORDERS);
	else
		// otherwise, post a message
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);

}

void CAU2View::DoingProcess(CString newstat)
{
	m_doingProcess = newstat;

	CString stat;

	//Get reference to Parent
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CAU2Doc* pDoc = GetDocument();
	stat.Format("%s", m_doingProcess.GetBuffer(5));
	pDoc->m_statusText = stat;

	// determine which thread is calling this function, 
	TRACE("\nMain Thread: %i          Current Thread: %i\n", m_mainThreadID, AfxGetThread()->m_nThreadID);
	if (AfxGetThread()->m_nThreadID == m_mainThreadID && (pParent))
		// if the main thread, update status bar directly
		pParent->m_wndStatusBar.GetStatusBarCtrl().SetText(stat,0,SBT_NOBORDERS);
	else
		// otherwise, post a message
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);

}



// Process Radio Button Clicks from Library Group
void CAU2View::OnRadioClickedLibrary() 
{
	int selected = GetCheckedRadioButton(IDC_LASTEROIDSRADIO,IDC_LTOURSRADIO);
	CAU2Doc* pDoc = GetDocument();

	switch (selected)
	{
	case IDC_LASTEROIDSRADIO:
		ChangeListBodyType(Asteroid);
		break;
	case IDC_LSATELLITESRADIO:
		ChangeListBodyType(Satellite);
		break;
	case IDC_LCOMETSRADIO:
		ChangeListBodyType(Comet);
		break;
	case IDC_LLANDMARKSRADIO:
		ChangeListBodyType(LandMark);
		break;
	case IDC_LTOURSRADIO:
		ChangeListBodyType(Tour);
		break;
	case IDC_LUSEROBJECTSRADIO:
		OnRadioClickedUserObj(IDC_LUSEROBJECTSRADIO);
		break;
	}

}


// determine if a custom user object was selected,
// if so find the name and switch the body type
void CAU2View::OnRadioClickedUserObj(UINT id)
{
	CAU2Doc *pDoc = GetDocument();

	CButton* userButton = (CButton *) GetDlgItem(id);
	CString buttonLabel;
	userButton->GetWindowText(buttonLabel);

	SourceList source = (id == IDC_LUSEROBJECTSRADIO) ? LIBRARY : HANDBOX;

	BodyType custom = pDoc->GetCustomBodyType(source, buttonLabel);

	ChangeListBodyType(custom);
}

// Process Radio Button Clicks from Handbox Group
void CAU2View::OnRadioClickedHandbox() 
{
	int selected = GetCheckedRadioButton(IDC_HASTEROIDSRADIO,IDC_HTOURSRADIO);
	CAU2Doc* pDoc = GetDocument();

	switch (selected)
	{
	case IDC_HASTEROIDSRADIO:
		ChangeListBodyType(Asteroid);
		break;
	case IDC_HSATELLITESRADIO:
		ChangeListBodyType(Satellite);
		break;
	case IDC_HCOMETSRADIO:
		ChangeListBodyType(Comet);
		break;
	case IDC_HLANDMARKSRADIO:
		ChangeListBodyType(LandMark);
		break;
	case IDC_HTOURSRADIO:
		ChangeListBodyType(Tour);
		break;
	case IDC_HUSEROBJECTSRADIO:
		OnRadioClickedUserObj(IDC_HUSEROBJECTSRADIO);
		break;
	}

}

/////////////////////////////////////////////////////////////////////
//
// set both lists to display the body type indicated by newBodyType
//
// IMPORTANT!  You should ONLY change m_bodyType by calling this function
//
//////////////////////////////////////////////////////////////////////
void CAU2View::ChangeListBodyType(BodyType newBodyType)
{
	CAU2Doc* pDoc = GetDocument();

	// Get pointers to list views
	CAU2ListView* lList = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
	CAU2ListView* hList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);

	// get pointers to the two main buttons
	CButton* lButton = (CButton *) GetDlgItem(IDC_LUSEROBJECTSRADIO);
	CButton* hButton = (CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO);

	// grab the text from the library user obj button
	CString buttonText;
	lButton->GetWindowText(buttonText);

	// remember which user object catalog was being displayed by this button
	pDoc->m_customDisplay = pDoc->GetCustomBodyType(LIBRARY, buttonText);

	// delete the list view contents
	DeleteListView(LIBRARY);
	DeleteListView(HANDBOX);

	// assign the new body type to the document's member variable
	pDoc->m_bodyType = newBodyType;		

	// rebuild the list views with this new body type
	lList->InitializeList(&pDoc->m_libraryCollection,pDoc->m_bodyType);
	hList->InitializeList(pDoc->m_handboxCollection,pDoc->m_bodyType);

	// update the radio button display text
	lButton->SetWindowText(CPersist::Abbreviate(pDoc->m_lBodyTypeLabel[pDoc->m_customDisplay], RADIO_SIZE));
	hButton->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[pDoc->m_customDisplay], RADIO_SIZE));

	UpdateMemInfo();
	SynchRadioButtons();
	DoingProcess("Ready");	//  clear the status bar message

}

// Force both groups of radio buttons to have identical display
void CAU2View::SynchRadioButtons()
{
	//Get reference to document
	CAU2Doc* pDoc = GetDocument();
	//Get bodyType
	BodyType bodyType = pDoc->m_bodyType;

	// select/deselect the first six object types
	for (int type = Asteroid; type <= Tour; type++)
	{
		if (type == bodyType)
		{
			((CButton *) GetDlgItem(type + 2300))->SetCheck(1);
			((CButton *) GetDlgItem(type + 2306))->SetCheck(1);
		}
		else
		{
			((CButton *) GetDlgItem(type + 2300))->SetCheck(0);
			((CButton *) GetDlgItem(type + 2306))->SetCheck(0);
		}
	}

	// select/deselect Custom User Objects
	for (type = UserObj20; type <= UserObj39; type++)
	{
		if (type == bodyType)
		{
			((CButton *) GetDlgItem(IDC_LUSEROBJECTSRADIO))->SetCheck(1);
			((CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO))->SetCheck(1);
		}
	}



	UpdateMemInfo(); // update memory & object counts info

}


//Destroy List Views when application closes
void CAU2View::OnDestroy() 
{
	CFormView::OnDestroy();
	CAU2ListView* lList = (CAU2ListView *)GetDlgItem(IDC_LIBRARYLIST);
	lList->FreeMemory();
	lList->DestroyWindow();
	CAU2ListView* hList = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);
	hList->FreeMemory();
	hList->DestroyWindow();

}

// process Send button command
void CAU2View::OnButtonSend() 
{
	CString		msgString;

	CAU2Doc* pDoc = GetDocument();

	if (!pDoc->m_hbxConnected) //if not connected, connect so memory can be checked
		Connect();

	if (!pDoc->m_hbxConnected)	// if still not connected, go no further
		return;

	// check if enough memory exists
	if (pDoc->m_autostar.GetAvailableMemory() < 0)
	{
		MessageBox("The objects in the Handbox List View exceed the memory capacity \
of the handbox.\nRemove some objects and try again.","Error",MB_ICONWARNING);
		return;
	}


	if (pDoc->m_handboxCollection->GetCount() == 0)
		msgString = "Handbox list is empty. Clear all objects in Autostar Handbox?";
	else
		msgString = "Replace all objects in Autostar Handbox with objects in Handbox List?";

	// check if there are any custom objects going to a 497
	if (m_ASType == TYPE_AUTOSTAR)
	{
		bool found = false;
		for (int i = UserObj20; i < BodyTypeMax; i++)
			if (pDoc->m_handboxCollection->GetCount((BodyType)i) > 0)
			{
				found = true;
				break;
			}
		if (found)
			msgString += "\n\nNOTE: Custom objects are not supported by this handbox.\nThey will be skipped.";
	}

	if (MessageBox(msgString,"Warning",MB_YESNO) == IDNO) return;

	// disable all buttons
	EnableButtons(0,0,0,0);

	// save a backup copy of the handbox list to m_backupFile

	if (pDoc->m_handboxCollection->SaveToFile(m_backupFile) != READCOMPLETE)
		if (MessageBox("Error creating backup file, proceed anyway?","Warning",MB_OKCANCEL) != IDOK) 
		{
			EnableButtons(1,2,1,1);
			return;		
		}

	// disable the hbx list view until the thread is completed
	CAU2ListView* pList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
	pList->EnableWindow(FALSE);
	pList->SetRedraw(FALSE);

	pDoc->m_autostar.SetStatCallBack(this);
	pDoc->SendAutostarData();

	pDoc->m_handboxModified = false;

}

// transfer selected object(s) from library to handbox collection
void CAU2View::OnButtonToHbx() 
{
	TransferObjects(LIBRARY);
}

// transfer selected object(s) from handbox to library collection
void CAU2View::OnButtonToLib() 
{
	TransferObjects(HANDBOX);	
}

// transfer ALL objects from library to handbox collection
void CAU2View::OnButtonToHbxAll() 
{
	TransferObjects(LIBRARY, true);
}

// transfer ALL objects from handbox to library collection
void CAU2View::OnButtonToLibAll() 
{
	TransferObjects(HANDBOX, true);
}

//Copy Library Objects only to Handbox Objects that already exist
void CAU2View::OnButtonToHbxRefresh() 
{
	if (MessageBox("Update existing handbox objects with data from library collection?",
"Confirm Object Replace",MB_YESNO) == IDYES)
	{
		BeginWaitCursor();
		EnableButtons(2,2,2,0);	// disable real-time button (if applicable)
		CAU2Doc *pDoc = (CAU2Doc *) GetDocument();
		if (m_realTimeMode)
			UpdateRealTime(&pDoc->m_libraryCollection,pDoc->m_handboxCollection,pDoc->m_bodyType);
		else
		{
			pDoc->m_handboxCollection->Update(&pDoc->m_libraryCollection,pDoc->m_bodyType);
			pDoc->m_handboxModified = true;
		}
			ChangeListBodyType(pDoc->m_bodyType);
		EndWaitCursor();
		EnableButtons(2,2,2,1);	// enable real-time button (if applicable)
	}
}

//Copy Handbox Objects only to Library Objects that already exist
void CAU2View::OnButtonToLibRefresh() 
{
	if (MessageBox("Update existing library objects with data from handbox collection?",
"Confirm Object Replace",MB_YESNO) == IDYES)
	{
		BeginWaitCursor();
		CAU2Doc *pDoc = (CAU2Doc *) GetDocument();
		pDoc->m_libraryCollection.Update(pDoc->m_handboxCollection,pDoc->m_bodyType);
		ChangeListBodyType(pDoc->m_bodyType);
		EndWaitCursor();
	}
}

// Function to update the handbox collection in real time
int CAU2View::UpdateRealTime(CBodyDataCollection *sourceCollection, CBodyDataCollection *destCollection, BodyType pType)
{
	// if not in real time mode, use the standard function
	if (!m_realTimeMode)
		return destCollection->Update(sourceCollection, pType);
	
	POSITION	newDataPos;
	CBodyData	*newData, *myData;
	int			count=0;
	CAU2Doc *pDoc = GetDocument();

	// open the serial port
	pDoc->m_autostar.InitializeConnection(false);

	// NOTE: This code is duplicated from the CBodyDataCollection class.
	// I know that is bad, but I can't think of a more straightforward way
	// of updating the handbox simultaneously with the collection

	// iterate through the passed list of this body type
	newDataPos = sourceCollection->GetHeadPosition(pType);
	while(newDataPos)
	{
		newData = sourceCollection->GetNext(newDataPos, pType);
	// search my list for the same key
		if (destCollection->Find(newData->GetKey(true)))
		{
		// create a copy of the new data
			myData = newData->Copy();
		// replace my data with the new data,
		// BUT first delete the old data, to mimic what the handbox is doing
		// (and keep sorting consistent between the hbx and listview)
			destCollection->Delete(myData->GetKey(true));
			destCollection->Add(myData);// this will delete the current one and replace it with the copy
			count++;

		// delete the old handbox object and replace with the new one
			DeleteObjectRealTime(myData->GetKey(true));
			SendOneObjectRealTime(myData);
		}
	}
	// send a status message
	((CMainFrame *) GetParent())->m_wndStatusBar.GetStatusBarCtrl().
			SetText("Update Complete" ,0,SBT_NOBORDERS);


	return count;

}

// copy entire catalog to handbox in real time
eAutostarStat CAU2View::SendCatalogRealTime(BodyType bodyType)
{
	// don't even bother unless we are running in real time mode
	if (!m_realTimeMode)
		return WRONG_MODE;

	CAU2Doc *pDoc = GetDocument();
	CBodyData *bodyData;
	eAutostarStat stat;

	// open the serial port
	pDoc->m_autostar.InitializeConnection(false);

	// delete the catalog from the handbox first
	stat = pDoc->m_autostar.DeleteCatalog(bodyType);

	if (stat != AUTOSTAR_OK)
		return stat;

	// step through the collection, extracting bodyType objects
	POSITION pos = pDoc->m_handboxCollection->GetHeadPosition(bodyType);
	while (pos != NULL && stat == AUTOSTAR_OK)
	{
		bodyData = pDoc->m_handboxCollection->GetNext(pos,bodyType);
		stat = SendOneObjectRealTime(bodyData);
	}

	//close the serial port
	pDoc->m_autostar.CloseSerialPort();

	return stat;

}

// copy object(s) from source list collection to other collection
void CAU2View::TransferObjects(SourceList source, bool allObjects)
{
	// Get pointer to the document
	CAU2Doc* pDoc = GetDocument();

	CAU2ListView* dList;				// pointer to destination list view
	CAU2ListView* sList;				// pointer to source list view
	CBodyDataCollection* dCollection;	// pointer to destination collection
	CBodyDataCollection* sCollection;	// pointer to source collection

	BeginWaitCursor();
	EnableButtons(2,2,2,0);	// disable real-time button (if applicable)
	// Setup pointers to lists and collection, based on direction of Xfer
	if (source == LIBRARY)
	{
		sList = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
		dList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
		sCollection = &pDoc->m_libraryCollection;
		dCollection = pDoc->m_handboxCollection;
		if (!m_realTimeMode)
			pDoc->m_handboxModified = true;

	}
	else
	{
		dList = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
		sList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
		dCollection = &pDoc->m_libraryCollection;
		sCollection = pDoc->m_handboxCollection;
	}

	if (allObjects)	// branch off to another function for ALL objects transfer
	{
		TransferObjectsAll(dCollection, sCollection);
		// send a status message
		if (m_realTimeMode && dList->GetDlgCtrlID() == IDC_HANDBOXLIST)
			DoingProcess("Send Complete");
		return;
	}
	
	// If this is a custom user object, check for compatibility between source and dest
	eCatCompare compare = CompareCatalogs(sCollection, dCollection);

	switch (compare)
	{
	case CAT_ERROR:
		DoingProcess("Transfer Error");
		break;
	case CAT_EMPTY:
		// iterate through the selections and transfer objects
		CopySelectedObjects(sList, sCollection, dCollection);
		// Reset the body type labels
		pDoc->ResetBodyTypeLabels((SourceList) ((source + 1) % 2), dCollection);
		// repaint the list
		ChangeListBodyType(pDoc->m_bodyType);
		break;
	case CAT_SAME:
		// iterate through the selections and transfer objects
		CopySelectedObjects(sList, sCollection, dCollection);
		break;
	case CAT_DIFFERENT:
		// issue warning
		MessageBox("Source and Destination Catalogs are not identical.\nObjects will not be copied","Error", MB_ICONEXCLAMATION);	
		break;
	}


	// update the destination list view
	dList->Refresh(dCollection, pDoc->m_bodyType);
	UpdateMemInfo(); // update memory & object counts info

	// clear selection list
	sList->GetListCtrl().SetSelectionMark(-1);

	//indicate a change has been made to the document
	if (dList->GetDlgCtrlID() == IDC_LIBRARYLIST)
		pDoc->SetModifiedFlag();

	// send a status message
	if (m_realTimeMode && dList->GetDlgCtrlID() == IDC_HANDBOXLIST)
		DoingProcess("Send Complete");

	EndWaitCursor();
	EnableButtons(2,2,2,1);	// enable real-time button (if applicable)
}


// Compare the catalog definition of the current body type in the source list
// versus the destination list. 
// Return an enumeration CAT_SAME, CAT_DIFFERENT, or CAT_EMPTY (i.e., the destination
// does not have a catalog defined for this bodytype)
eCatCompare CAU2View::CompareCatalogs(CBodyDataCollection *source, CBodyDataCollection *dest)
{
	POSITION pos;
	CAU2Doc *pDoc = GetDocument();
	CBodyData *sData = NULL, *dData = NULL;

	// get a source list object
	pos = source->GetHeadPosition(pDoc->m_bodyType);
	if (pos) 
		sData = source->GetNext(pos, pDoc->m_bodyType);
	if (!sData) 
		return CAT_ERROR;	// error: nothing to copy!

	// get a destination list object
	pos = dest->GetHeadPosition(pDoc->m_bodyType);
	if (pos)
		dData = dest->GetNext(pos, pDoc->m_bodyType);
 	if (!dData)
		return CAT_EMPTY;	// destination is empty, have a party

	// if it's not a custom object, check no further
	if (!sData->IsCustom())
		return CAT_SAME;

	// compare the sizes of the body objects
	if (sData->GetSizeOf() != dData->GetSizeOf())
		return CAT_DIFFERENT;

	// compare the names of the catalogs
	if ( ((CUserObjEx *) sData)->GetCatalogName() != ((CUserObjEx *) dData)->GetCatalogName() )
		return CAT_DIFFERENT;

	// compare the number of fields
	if ( ((CUserObjEx *) sData)->GetNumFields() != ((CUserObjEx *) sData)->GetNumFields() )
		return CAT_DIFFERENT;

	// compare the size of each field
	for (int i = 0; i < ((CUserObjEx *) sData)->GetNumFields(); i++)
	{
		if ( ((CUserObjEx *) sData)->GetFieldSize(i) != ((CUserObjEx *) sData)->GetFieldSize(i) )
			return CAT_DIFFERENT;
	}
	
	// whew!  if you made it this far, congratulations!
	return CAT_SAME;
}


// Transfer All Objects of Selected Types - specified via Dialog
void CAU2View::TransferObjectsAll(CBodyDataCollection *dCollection, CBodyDataCollection *sCollection)
{
	CAU2Doc *pDoc = (CAU2Doc *) GetDocument();
	CAU2AllDlg dlg;
	dlg.m_staticText = "Warning: Proceeding will erase any objects of the specified type already\
 in the destination list.\nIndicate types of objects to copy:";
	if (dlg.DoModal() == IDOK && dlg.m_selectAll)
	{
		BeginWaitCursor();
		EnableButtons(2,2,2,0);	// disable real-time button (if applicable)
		dCollection->Clear();
		if (m_ASType != TYPE_AUTOSTAR)		// only copy EVERYTHING if autostar II
			dCollection->Add(*sCollection);
		else								// otherwise omit the custom user objects
		{
			dCollection->Add(*sCollection, Asteroid);
			dCollection->Add(*sCollection, Satellite);
			dCollection->Add(*sCollection, Comet);
			dCollection->Add(*sCollection, Tour);
			dCollection->Add(*sCollection, LandMark);
			dCollection->Add(*sCollection, UserObj);
		}
		if (m_realTimeMode && dCollection == pDoc->m_handboxCollection)
		{	
			for (int i = Asteroid; i < BodyTypeMax; i++)
				SendCatalogRealTime((BodyType) i);
		}
		SourceList source = (dCollection == &pDoc->m_libraryCollection) ? LIBRARY : HANDBOX;
		pDoc->ResetBodyTypeLabels(source,dCollection);
		ChangeListBodyType(pDoc->m_bodyType);
		EndWaitCursor();
		EnableButtons(2,2,2,1);	// enable real-time button (if applicable)
	}
	else
	{
		BeginWaitCursor();
		EnableButtons(2,2,2,0);	// disable real-time button (if applicable)

		if (dlg.m_selectAsteroids)
			TransferBodyType(Asteroid, dCollection, sCollection);

		if (dlg.m_selectSatellites)
			TransferBodyType(Satellite, dCollection, sCollection);

		if (dlg.m_selectComets)
			TransferBodyType(Comet, dCollection, sCollection);

		if (dlg.m_selectLandmarks)
			TransferBodyType(LandMark, dCollection, sCollection);

		if (dlg.m_selectTours)
			TransferBodyType(Tour, dCollection, sCollection);

		if (dlg.m_selectUserObj)
			TransferBodyType(UserObj, dCollection, sCollection, (m_ASType == TYPE_AUTOSTAR) ? false : true);

		EndWaitCursor();
		EnableButtons(2,2,2,1);	// enable real-time button (if applicable)
	}
		
}


// copy ALL the objects of a single body type
void CAU2View::TransferBodyType(BodyType type, CBodyDataCollection *dCollection, CBodyDataCollection *sCollection, bool transferCustom)
{
	CAU2Doc* pDoc = GetDocument();

	// this code is executed every time, regardless of custom objects or not
	dCollection->Clear(type);
	dCollection->Add(*sCollection, type);
	if (m_realTimeMode && dCollection == pDoc->m_handboxCollection)
	{
		SendCatalogRealTime(type);
	}

	if (transferCustom)
	{
		for (int i = UserObj20; i < BodyTypeMax; i++)
		{
			dCollection->Clear((BodyType) i);
			dCollection->Add(*sCollection, (BodyType) i);
			if (m_realTimeMode && dCollection == pDoc->m_handboxCollection)
			{
				SendCatalogRealTime((BodyType) i);
			}
		}
		pDoc->ResetBodyTypeLabels((dCollection == pDoc->m_handboxCollection) ? HANDBOX : LIBRARY, dCollection);
		type = pDoc->m_bodyType;
	}

	ChangeListBodyType(type);

}
// copy all selected items from source list to destination list
void CAU2View::CopySelectedObjects(CAU2ListView *sourceList, CBodyDataCollection *sourceCollection, CBodyDataCollection *destCollection)
{
	CBodyData* data;					// pointer to object to be copied

	CAU2Doc *pDoc = GetDocument();

	// Get information about the selected item(s)
	POSITION pos = sourceList->GetListCtrl().GetFirstSelectedItemPosition();

	bool skip = FALSE;	// indicates "Yes To All" or "Cancel" was chosen

	// open the serial port if real time mode is enabled
	if (m_realTimeMode && sourceList->GetDlgCtrlID() == IDC_LIBRARYLIST)
		pDoc->m_autostar.InitializeConnection(false);

	while (pos)
	{
		int selectedItem = sourceList->GetListCtrl().GetNextSelectedItem(pos);
		CString key = sourceList->GetListCtrl().GetItemText(selectedItem,0);

		// Look for body data object from library collection
		if (!(data = sourceCollection->Find(key))) return;

		// remove white spaces from right end of name
		CString formattedKey = key;	
		formattedKey.TrimRight();

		if (destCollection->Find(key) && (!skip)) // if object already exists in destination
		{
			EndWaitCursor();
			CAU2ReplaceDlg dlg;	// create replace object dialog box
			dlg.m_oldObjectText = GetReplaceText(destCollection,key);
			dlg.m_newObjectText = GetReplaceText(sourceCollection,key);

			dlg.DoModal();	//display replace dialog box

			if (dlg.m_outcome == YES || dlg.m_outcome == YESTOALL) //if OK to replace
			{	//copy the object
				CopyObject(data,destCollection,sourceList,key);
				if (dlg.m_outcome == YESTOALL) skip = TRUE; // skip further replace prompts
			}
			else if (dlg.m_outcome == CANCEL) pos = 0; // if cancel, exit loop
			BeginWaitCursor();
		}
		else // object does not already exist, or "Yes To All" was answered
		{
			//copy the object
			CopyObject(data,destCollection,sourceList,key);
		}

	}

	// close the serial port if real time mode is enabled
	if (m_realTimeMode && sourceList->GetDlgCtrlID() == IDC_LIBRARYLIST)
		pDoc->m_autostar.CloseSerialPort();

}


// Copy an object from one list to the other, and possibly the handbox
void CAU2View::CopyObject(CBodyData *sourceData, CBodyDataCollection *destCollection, CAU2ListView *sourceList, CString key)
{
	// create a new body data object
	CBodyData* newObject;

	// copy the source object to this new object
	newObject = sourceData->Copy();

	// check if object is already in handbox in realtime mode
	if (m_realTimeMode && destCollection->Find(key) && sourceList->GetDlgCtrlID() == IDC_LIBRARYLIST)
	{
		DeleteObjectRealTime(key); // if so, delete it
		destCollection->Delete(key);
	}

	// add the new object to the destination collection
	destCollection->Add(newObject);

	// if we're in realTime mode and the source is the library list,
	// also send it to the handbox
	if (sourceList->GetDlgCtrlID() == IDC_LIBRARYLIST && m_realTimeMode)
		SendOneObjectRealTime(newObject);

}


// Process key stroke messages
BOOL CAU2View::PreTranslateMessage(MSG* pMsg) 
{
	CAU2ListView* lList;
	CAU2ListView* hList;


	short ctrlPressed = ::GetKeyState(VK_CONTROL) >> 8;	// examine hi order bit to test ctrl press

	switch( pMsg->message ) {
     case WM_KEYUP:
         switch( pMsg->wParam ) {
		 case 'T':	// Process "Ctrl-T" command to test new functions
			 if (ctrlPressed)
			 {
				TestFunction();
			 }
			 else
				return CFormView::PreTranslateMessage(pMsg);
			 break;
         case VK_DELETE:					// process "Delete" key
			 // get pointers to list views
			 lList = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
			 hList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);

			 // check if library list has a selected item
			 if (lList->GetListCtrl().GetSelectionMark() != -1)
				DeleteObject(lList); // if so, delete it
			 // check if handbox list has a selected item
			 else if (hList->GetListCtrl().GetSelectionMark() != -1)
				DeleteObject(hList); // if so, delete it
			 else return TRUE; // if not, return
             break;
		 case 'A':	// Process "Ctrl-A" command to select all items
			 if (ctrlPressed)
			 {
				 // get pointers to list views
				 lList = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
				 hList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
				 // check if library list has a selected item
				 if (lList->GetListCtrl().GetSelectionMark() != -1)
					SelectAllItems(lList); // if so, select all
				 // check if handbox list has a selected item
				 else if (hList->GetListCtrl().GetSelectionMark() != -1)
					SelectAllItems(hList); // if so, select all
				 else return TRUE; // if not, return
			 }
			 else
				return CFormView::PreTranslateMessage(pMsg);
			 break;
          default:
             return CFormView::PreTranslateMessage(pMsg);
         }

     default:
         return CFormView::PreTranslateMessage(pMsg);
         }
     return TRUE;
}

// Select All items in the referenced listview
void CAU2View::SelectAllItems(CAU2ListView *list)
{
	for (int i = 0; i < list->GetListCtrl().GetItemCount(); i++)
		list->GetListCtrl().SetItemState(i,LVIS_SELECTED,LVIS_SELECTED);
}



// retrieve collection from the autostar
void CAU2View::OnButtonRetrieve() 
{
	//Get reference to Parent
	CMainFrame* pParent = (CMainFrame *) GetParent();
	
	CAU2Doc* pDoc = GetDocument();	// get pointer to document

	Connect();	// always connect (to put hbx in download mode)

	if (!pDoc->m_hbxConnected)	// if still not connected, go no further
	return;

	// disable all buttons
	EnableButtons(0,0,0,0);

	// Retrieve the data in a thread
	Retrieve();

}

// function to retrieve data from handbox and update the modified variable
void CAU2View::Retrieve(bool spawnThread)
{
	// disable the hbx list view until the thread is completed
	CAU2ListView* pList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
	pList->EnableWindow(FALSE);
	pList->SetRedraw(FALSE);

	//retrieve data from handbox and store in pDoc->m_handboxCollection
	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();
	pDoc->m_autostar.SetStatCallBack(this);
	pDoc->LoadAutostarData(spawnThread);

	pDoc->m_handboxModified = false;
}


// prompt user to search com ports to find autostar handbox port
bool CAU2View::SearchComPorts(bool closeComPort)
{
	CUserSettings userSettings;
	bool connected = FALSE;
	CAU2Doc* pDoc = GetDocument();	// get pointer to document

	if (pDoc->m_autostar.FindAutostar(closeComPort)) //search com ports
	{
		EndWaitCursor();
		connected = TRUE;
		MessageBox("Autostar found at " + userSettings.GetComPort(),
					"Autostar Found",MB_OK);
	}
	else
	{
		EndWaitCursor();
		MessageBox("Could not find COM Port.\nCheck your connection and try again.",
			"Error",MB_OK);
	}

	return connected;
}

// disable all buttons, so only software can be downloaded
VOID CAU2View::ActivateSafeMode(bool state)
{
	// this function only applies to Autostar II
//	if (m_ASType != TYPE_AUTOSTAR2) return;

	CAU2Doc* pDoc = GetDocument();	// get pointer to document

	if (state)
	{
		pDoc->m_autostar.m_hbxSafeMode = TRUE;
		//disable ALL functions except upgrade button
		EnableButtons(1,0,0,0);
		// Set the baud rate to 9600
		OnOptionsBaud(ID_OPTIONS_BAUD_9600);
	}
	else
	{
		// don't exit safe mode unless the version is no longer 0.9A (or GPS)  FOR AN AUTOSTAR II
		if (
			((m_ASType == TYPE_AUTOSTAR2) &&
				(pDoc->m_autostar.GetVersion().GetAt(0) == '0' || pDoc->m_autostar.GetVersion().GetAt(0) == 'G'))
			||
			(pDoc->m_autostar.m_mode == DOWNLOAD)	// this works for a 497
			)
			EnableButtons(1,2,2,2);
		else
		{
			pDoc->m_autostar.m_hbxSafeMode=FALSE;
			EnableButtons(1,1,1,(m_ASType == TYPE_AUTOSTAR2) ? 1 : 2);
		}
	}
}

// connect to autostar without transferring any data
void CAU2View::OnButtonConnect() 
{
	CAU2Doc *pDoc = GetDocument();

	if (Connect())	// if connection succeeds
	{
		if (m_ASType == TYPE_AUTOSTAR && !pDoc->m_autostar.m_hbxSafeMode)	// and handbox is 495/494/497 and not in safe mode
			pDoc->m_autostar.RestartHandbox();	// restart the handbox
		pDoc->m_autostar.CloseSerialPort();
	}

}

// connect to the handbox & update status variables
bool CAU2View::Connect(bool closeComPort, bool setDownload)
{
	CAU2Doc* pDoc = GetDocument();	// get pointer to document

	//Get reference to Parent
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CUserSettings userSettings;

	bool connected = false;	//set to true only when a connection is established
	bool found = true;		// causes 2nd QuerySafeMode() to be called if set to false the first time

	BeginWaitCursor();
	if (pDoc->m_autostar.CheckDownLoadMode() == AUTOSTAR_OK)
	{
		if (!pDoc->m_autostar.m_hbxSafeMode && pDoc->m_autostar.m_mode == DOWNLOAD)
			QuerySafeMode();
		connected = true;
	}
	else
		found = false;

	// try to connect
	if (pDoc->m_autostar.InitializeConnection(closeComPort, setDownload) != AUTOSTAR_OK)
	{

		if (pDoc->m_autostar.m_lastError != OLD_FIRMWARE && MessageBox("Could not connect to Autostar.  Autostar Update will\nnow\
  perform a search of all COM Ports. OK to Proceed?","Error",MB_YESNO) == IDYES)
			{
				if (connected = SearchComPorts(closeComPort))
					found = true;
				
			}
	}

	if (connected)
	{
		SetASType();	// set type of Autostar (1 or 2)
		if ((!found) || (!pDoc->m_autostar.m_hbxSafeMode && m_ASType == TYPE_AUTOSTAR2))	// if not already in safe mode
			QuerySafeMode();	// prompt user if in safe mode
		pDoc->m_hbxConnected = TRUE;
		EnableButtons(1,2,1,1);	//update status of send/receive buttons
		pDoc->m_statusText = "Connection completed";
		UpdateMemInfo(); // update memory & object counts info
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_MODEL);
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_VER);
		userSettings.SetLastModel(pDoc->m_autostar.GetModel());
	}
	else 
	{
		pDoc->m_statusText = "Connection attempt failed";
		pDoc->m_hbxConnected = FALSE;
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);
		UpdateMemInfo(); // update memory & object counts info
		EnableButtons(1,2,1,1);	//update status of send/receive buttons
		m_ASType = TYPE_UNKNOWN;
	}
	return connected;
}


// Prompt the user if in safe mode, and disable applicable controls
VOID CAU2View::QuerySafeMode()
{
	CAU2Doc* pDoc = GetDocument();

	CString message = "Is the handbox in Safe Mode?\n(Does the display read, ";

	if (m_ASType == TYPE_AUTOSTAR2 || pDoc->m_autostar.m_modelName == "LX200 GPS")
		message += "\"Downloading... DO NOT POWER OFF\"?)";
	else
		message += "\"FLASH LOAD  READY\"?)";

	if (pDoc->m_autostar.m_mode == DOWNLOAD) // this is simple for an AS2

	{
		if (MessageBox(message,"Handbox Mode",MB_YESNO) == IDYES)
			ActivateSafeMode(TRUE);
		else
			ActivateSafeMode(FALSE);
	}

}


// Queries the handbox to find out what type, and sets the m_ASType variable
VOID CAU2View::SetASType()
{
	CAU2Doc* pDoc = GetDocument();

	if (pDoc->m_autostar.GetModel() == "LX200 GPS")
	{
		m_ASType = TYPE_AUTOSTAR2;
	}
	else if (pDoc->m_autostar.GetModel() == "RCX400")
	{
		m_ASType = TYPE_RCX;
	}
	else
	{
		m_ASType = TYPE_AUTOSTAR;
		HideCustomObjects();
	}
	
}

// hides the display of custom user objects
void CAU2View::HideCustomObjects()
{
	CAU2Doc* pDoc = GetDocument();

	// set the User Object Radio button to legacy User Objects
	((CButton *) GetDlgItem(IDC_LUSEROBJECTSRADIO))->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[UserObj], RADIO_SIZE));
	((CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO))->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[UserObj], RADIO_SIZE));

	// if a custom object is currently selected and displayed,
	// change to the legacy user objects
	if (pDoc->m_bodyType >= UserObj20)
	{
		ChangeListBodyType(UserObj);
		return;
	}

	// if a non-user object is currently selected, BUT a custom
	// object is displayed on the face of the radio button, just change
	// the member variable to User Obj
	if (pDoc->m_customDisplay >= UserObj20)
		pDoc->m_customDisplay = UserObj;

}


// import a ROM, TLE or TXT file
void CAU2View::OnFileImport() 
{
	CAU2Doc* pDoc = GetDocument();

	// static variables to remember the last selections
	static CString lastType = "", lastExt = "";
	
	// initialize custom file dialog box
	CString szFilters = _T(CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_TEXT) + "|*.TXT|"
						+ CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_TLE) + "|*.TLE|" 
						+ CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_ROM) + "|*.ROM|" 
						+ CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_MTF) + "|*.MTF|" 
						+ CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_ALL) + "|*.*||");

	// create dialog
	CAU2FileDialog dlg(TRUE, _T("TXT"),_T("*.TXT"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilters);

	// set the initial extension type: note that this wacky index is NOT zero-based
	if (lastExt == CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_TEXT)) dlg.m_ofn.nFilterIndex = (int)(CAU2FileDialog::TYPE_TEXT);
	else if (lastExt == CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_TLE)) dlg.m_ofn.nFilterIndex = (int) CAU2FileDialog::TYPE_TLE;
	else if (lastExt == CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_ROM)) dlg.m_ofn.nFilterIndex = (int) CAU2FileDialog::TYPE_ROM;
	else if (lastExt == CAU2FileDialog::GetExtText(CAU2FileDialog::TYPE_MTF)) dlg.m_ofn.nFilterIndex = (int) CAU2FileDialog::TYPE_MTF;
	else	// default to type all
		dlg.m_ofn.nFilterIndex = (int) CAU2FileDialog::TYPE_ALL;


	// specify the title
	dlg.m_ofn.lpstrTitle = "Import Data File";
	dlg.m_ofn.lpstrInitialDir = NULL;
	dlg.SetFontSize((m_SystemFontSize == SMALL) ? 0 : 1);

	// create a larger buffer, so a large number of files can be specified
	TCHAR* tbuf = (TCHAR*)calloc(65535, sizeof(TCHAR)); 
	dlg.m_ofn.lpstrFile = tbuf; 
	dlg.m_ofn.nMaxFile = 65535; 

	// set the dialog to the last type selected
	dlg.m_selectedExt = lastExt;
	dlg.m_selectedType = lastType;

	if (dlg.DoModal() == IDOK)	// display the dialog box
	{
		if (dlg.m_delete && !dlg.m_update)	// if delete checked, clear collection
			pDoc->m_libraryCollection.Clear(dlg.GetBodyType());
		POSITION pos = dlg.GetStartPosition();
		BeginWaitCursor();
		while (pos)
		{
			// if update checked, set update option
			CBodyDataCollection::importOption import;
			if (dlg.m_update) import = CBodyDataCollection::update;
			else import = CBodyDataCollection::dupchk;
			// attempt to import file
			if (pDoc->m_libraryCollection.Import(dlg.GetNextPathName(pos), dlg.GetBodyType(),import) != READCOMPLETE)
				{
					EndWaitCursor();
					MessageBox("Error Importing File","ERROR",MB_ICONEXCLAMATION);
					return;
				}
		}
		EndWaitCursor();
		ChangeListBodyType(dlg.GetBodyType());	// refresh list and change pDoc->m_bodyType
		SynchRadioButtons();

		// record the last selected type
		lastExt = dlg.m_selectedExt;
		lastType = dlg.m_selectedType;
	}
	
	free(tbuf);
}


// Process options menu commands
void CAU2View::OnOptions(UINT nID)
{
	// Get a pointer to the menu
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CMenu* menu = (CMenu *) pParent->GetMenu();

	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();

	CComPortDlg dlg;	//declare com port dialog
	
	int state[8];		//states of the controls:
							// 0 = connect on startup
							// 1 = start in advanced view
							// 2 = verify file transfer
							// 3 = load MRF automatically
							// 4 = connect & retrieve on startup
							// 5 = com port (not used as a toggle)
							// 6 = background (not used as a toggle)
							// 7 = background reset (not used as a toggle)

	state[0] = m_userSettings.GetOptions(CUserSettings::CONNECT);
	state[1] = m_userSettings.GetOptions(CUserSettings::ADVANCED);
	state[2] = m_userSettings.GetOptions(CUserSettings::VERIFY);
	state[3] = m_userSettings.GetOptions(CUserSettings::RECENT);
	state[4] = m_userSettings.GetOptions(CUserSettings::RETRIEVE);
	state[5] = 0;	//meaningless
	state[6] = 0;	//meaningless
	state[7] = 0;	//meaningless

	CUserSettings::optionName option;	// enum of option selected

	// determine which menu item was selected
	int stateIndex;
	switch(nID)
	{
	case ID_OPTIONS_CONNECT:
		option = CUserSettings::CONNECT;
		stateIndex = 0;
		// if turning retrieve on, turn connect off (mutually exclusive options)
		if (state[stateIndex] == 0)
			m_userSettings.SetOptions(CUserSettings::RETRIEVE,0);
		break;
	case ID_OPTIONS_ADVANCED:
		option = CUserSettings::ADVANCED;
		stateIndex = 1;
		break;
	case ID_OPTIONS_VERIFY:
		option = CUserSettings::VERIFY;
		stateIndex = 2;
		//change verify mode setting in autostar object
		pDoc->m_autostar.SetVerifyMode(((state[2] + 1) % 2) ? TRUE : FALSE); 
		break;
	case ID_OPTIONS_RECENT:
		option = CUserSettings::RECENT;
		stateIndex = 3;
		break;
	case ID_OPTIONS_RETRIEVE:
		option = CUserSettings::RETRIEVE;
		stateIndex = 4;
		// if turning retrieve on, turn connect off (mutually exclusive options)
		if (state[stateIndex] == 0)
			m_userSettings.SetOptions(CUserSettings::CONNECT,0);
		break;
	case ID_OPTIONS_COMPORT:
		dlg.DoModal();
		stateIndex = 5; //(this is meaningless)
		break;
	case ID_OPTIONS_BACKGROUND:
		SetBackground();
		stateIndex = 6; //(this is meaningless)
		break;	
	case ID_OPTIONS_BACKGROUND_RESET:
		SetBackground(m_userSettings.GetInstallDirectory() + "rosette.jpg");
		stateIndex = 7; //(this is meaningless)
		break;	
	default:
		return;
	}

	state[stateIndex] = (state[stateIndex] + 1) % 2;		//toggle state value

	// make change to registry
	if (stateIndex < 5)
		m_userSettings.SetOptions(option,state[stateIndex]);
}

// Process tools menu commands
void CAU2View::OnTools(UINT nID) 
{
	// Get a pointer to the menu
//	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
//	CMenu* menu = (CMenu *) pParent->GetMenu();

	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();

//	int count = 1;

		
	switch(nID)
	{
	case ID_TOOLS_GC:
		MessageBeep(MB_ICONEXCLAMATION);
		BeginWaitCursor();
		// send a status message
		DoingProcess("Garbage Collecting... Please Wait");

		pDoc->m_autostar.SetStatCallBack(this);
		pDoc->m_autostar.ForceGarbageCollection();
		break;
	case ID_TOOLS_RESET:
		BeginWaitCursor();
		// restart the handbox
		pDoc->m_autostar.PowerCycleHandbox();
		EndWaitCursor();
		break;
	case ID_TOOLS_USERSITE:
		BeginWaitCursor();
		if (!pDoc->m_hbxConnected)
			Connect(false);
		else
			if (pDoc->m_autostar.InitializeConnection(false) != AUTOSTAR_OK)
				Connect(false);
		if (pDoc->m_hbxConnected)
		{
			CUserSiteDlg dlg(&pDoc->m_autostar);
			if (m_SystemFontSize == LARGE)
				dlg.m_fontSize = 8;
			if (dlg.DoModal() == IDABORT)
			{
				MessageBox("Answer questions on handbox, then try again", "Bad Response", MB_OK | MB_ICONEXCLAMATION );
/*				BeginWaitCursor();
				DoingProcess("Restarting Handbox...");
				pDoc->m_autostar.SetStatCallBack(this);
				pDoc->m_autostar.RestartHandbox(); */
			}
		}

		break;

	}

	pDoc->m_autostar.CloseSerialPort();
}


// Process options Baud menu commands
void CAU2View::OnOptionsBaud(UINT nID)
{
	// Get a pointer to the menu
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CMenu* menu = (CMenu *) pParent->GetMenu();

	CAU2Doc *pDoc = (CAU2Doc *) GetDocument();

	const int numOptions = 7;
	UINT optionsArray[numOptions] = {ID_OPTIONS_BAUD_115K, ID_OPTIONS_BAUD_56K, ID_OPTIONS_BAUD_38K,
									 ID_OPTIONS_BAUD_28K, ID_OPTIONS_BAUD_19K,
									 ID_OPTIONS_BAUD_14K, ID_OPTIONS_BAUD_9600};

	// UNcheck all of the submenu items
	for (int i = 0; i < numOptions; i++)
		menu->CheckMenuItem(optionsArray[i], MF_BYCOMMAND | MF_UNCHECKED);

	// Now check the one that was selected
	menu->CheckMenuItem(nID, MF_BYCOMMAND | MF_CHECKED);

	// Store the appropriate enumeration in the registry
	CSerialPort::eBaud value;
	switch (nID)
	{
	case ID_OPTIONS_BAUD_115K:
		value = CSerialPort::b115k;
		break;
	case ID_OPTIONS_BAUD_56K:
		value = CSerialPort::b56k;
		break;
	case ID_OPTIONS_BAUD_38K:
		value = CSerialPort::b38k;
		break;
	case ID_OPTIONS_BAUD_28K:
		value = CSerialPort::b28k;
		break;
	case ID_OPTIONS_BAUD_19K:
		value = CSerialPort::b19k;
		break;
	case ID_OPTIONS_BAUD_14K:
		value = CSerialPort::b14k;
		break;
	case ID_OPTIONS_BAUD_9600:
		value = CSerialPort::b9600;
		break;
	default:
		value = CSerialPort::b56k;
		break;
	}

	// set the serial port value
	pDoc->m_autostar.SetMaxBaudRate(value);

}
///////////////////////////////////////////////////////////////////
//
// Function to set the background image file and update the registry
//
// input: CString fileName (optional), otherwise user is prompted
//
// output: none
//
///////////////////////////////////////////////////////////////////
void CAU2View::SetBackground(CString fileName)
{
	CUserSettings userSettings;

	if (fileName != "")	// if fileName is passed to function
	{
		m_background.LoadFile(fileName);
		Invalidate();

		userSettings.SetBackground(fileName);
		return;
	}
	
	static char szTitle[] = _T("Select Image File");
	TCHAR szFilters[] = _T ("JPEGs (*.JPG,*.JPE,*.JPEG)|*.JPG;*.JPE;*.JPEG|Bitmaps (*.BMP)|*.BMP|GIFs (*.GIF)|*.GIF|All Image Files (JPG,BMP,GIF)|*.JPG;*.JPE;*.JPEG;*.BMP;*.GIF|");

	CFileDialog dlg (TRUE, _T ("jpg"), _T ("*.jpg"), OFN_FILEMUSTEXIST |
					OFN_HIDEREADONLY, szFilters);

	dlg.m_ofn.lpstrTitle = szTitle;

	if (dlg.DoModal() == IDOK)
	{
		m_background.LoadFile(dlg.GetPathName());
		Invalidate();

		userSettings.SetBackground(dlg.GetPathName());
	}
	else
		return;
}


// Function to process commands from the Main Menu, Edit Submenu
void CAU2View::OnEditMenu(UINT nID)
{
	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();
	// get pointers to list views
	CAU2ListView* lList = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
	CAU2ListView* hList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
	CAU2ListView* selectedList;

	// check if library list has a selected item
	if (lList->GetListCtrl().GetSelectionMark() != -1)
		selectedList = lList; 
	// check if handbox list has a selected item
	else if (hList->GetListCtrl().GetSelectionMark() != -1)
		selectedList = hList; 
	else
		if (nID != ID_EDIT_CLEAR_LIB && nID != ID_EDIT_CLEAR_HBX)
		{
			MessageBox("Select an Item from Either List View","Error",MB_ICONWARNING);
			return;	// if the menu selection is not list-specific, and
		}			// no list has been selected, return

	switch (nID)	// process specific menu items
	{
	case ID_EDIT_EDIT:
		EditObject(selectedList);
		break;
	case ID_EDIT_DELETE:
		DeleteObject(selectedList);
		break;
	case ID_EDIT_NEW:
		NewObject(selectedList);
		break;
	case ID_EDIT_CLEAR_LIB:
		pDoc->m_libraryCollection.Clear();
		// reset the radio buttons
		pDoc->ResetBodyTypeLabels(LIBRARY, &pDoc->m_libraryCollection);
		ChangeListBodyType(pDoc->m_bodyType);
		break;
	case ID_EDIT_CLEAR_HBX:
		pDoc->m_handboxCollection->Clear();
		// reset the radio buttons
		pDoc->ResetBodyTypeLabels(HANDBOX, pDoc->m_handboxCollection);
		ChangeListBodyType(pDoc->m_bodyType);
		break;
	}
}


// Function to initialize drag and drop between list views
void CAU2View::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	CAU2ListView* lList = (CAU2ListView *)GetDlgItem(IDC_LIBRARYLIST);
	CAU2ListView* hList = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);
	POINT pt;
	CPoint ptStart;

	// determine which list is source for drag & create drag images
	switch (pNMHDR->idFrom)
	{
	case IDC_LIBRARYLIST:
		m_dragSource = LIBRARY;
		m_dragImage = CreateDragImageEx(&lList->GetListCtrl(), &pt);
		break;
	case IDC_HANDBOXLIST:
		m_dragSource = HANDBOX;
		m_dragImage = CreateDragImageEx(&hList->GetListCtrl(), &pt);
		break;
	default:
		return;
	}

	// start drag
	ptStart = pNMListView->ptAction;
	ptStart -= pt;
	m_dragImage->BeginDrag(0,ptStart);
	m_dragImage->DragEnter(GetDesktopWindow(),pNMListView->ptAction);
	SetCapture();


	*pResult = 0;
}

//Function to complete drag and drop operation 
void CAU2View::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_dragSource != NONE)	// was drag started from a list view?
	{
		// get pointers to both lists
		CAU2ListView* lList = (CAU2ListView *)GetDlgItem(IDC_LIBRARYLIST);
		CAU2ListView* hList = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);

		// end drag
		::ReleaseCapture();
		m_dragImage->DragLeave(GetDesktopWindow());
		m_dragImage->EndDrag();

		CPoint pt(point);
		ClientToScreen(&pt);
		CWnd* dropWnd = WindowFromPoint(pt);

		// as long as drag started in one list and ended in the
		// opposite list, transfer objects between the lists
		if (((m_dragSource == LIBRARY) && (dropWnd == hList)) || 
			((m_dragSource == HANDBOX) && (dropWnd == lList)))
				TransferObjects(m_dragSource);

		// clean up temporary drag files
		m_dragImage->DeleteImageList();
		delete m_dragImage;
		m_dragImage = NULL;

		// always reset drag source variable
		m_dragSource = NONE;
	}
	CFormView::OnLButtonUp(nFlags, point);
}

// show drag images during mouse movement
void CAU2View::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_dragImage && (m_dragSource != NONE)) // In Drag&Drop mode ?
	{
		CPoint ptDropPoint(point);
		ClientToScreen(&ptDropPoint);
		m_dragImage->DragMove(ptDropPoint);
		CWnd* pDropWnd = CWnd::WindowFromPoint(ptDropPoint);
	}
	
	CFormView::OnMouseMove(nFlags, point);
}

// delete contents of a list view
void CAU2View::DeleteListView(SourceList list)
{
	CAU2Doc* pDoc = GetDocument();
	CBodyDataCollection* collection;

	// Get pointers to list views
	CAU2ListView* pList;
	if (list == LIBRARY)
	{
		collection = &pDoc->m_libraryCollection;
		pList = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
	}
	else
	{
		collection = pDoc->m_handboxCollection;
		pList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
	}

	// Delete all ITEMINFO structures from previous list views
	pList->FreeMemory();

	// Delete all columns and data from previous list view
//	for (int i = 0; i < collection->GetNumFields(pDoc->m_bodyType); i++)
//		pList->GetListCtrl().DeleteColumn(0);
	int nColumnCount = pList->GetListCtrl().GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < nColumnCount; i++)
		pList->GetListCtrl().DeleteColumn(0);

	pList->GetListCtrl().DeleteAllItems();

}

// create image of multiple selected files when dragging
CImageList* CAU2View::CreateDragImageEx(CListCtrl *pList, LPPOINT lpPoint)
{
	if (pList->GetSelectedCount() <= 0)
		return NULL; // no row selected

	CRect rectSingle;
	CRect rectComplete(0,0,0,0);

	// Determine List Control Client width size
	pList->GetClientRect(rectSingle);
	int nWidth  = rectSingle.Width();

	// Start and Stop index in view area
	int nIndex = pList->GetTopIndex() - 1;
	int nBottomIndex = pList->GetTopIndex() + pList->GetCountPerPage() - 1;
	if (nBottomIndex > (pList->GetItemCount() - 1))
		nBottomIndex = pList->GetItemCount() - 1;

	// Determine the size of the drag image (limite for rows visibled and Client width)
	while ((nIndex = pList->GetNextItem(nIndex, LVNI_SELECTED)) != -1)
	{
		if (nIndex > nBottomIndex)
			break; 

		pList->GetItemRect(nIndex, rectSingle, LVIR_BOUNDS);

		if (rectSingle.left < 0) 
			rectSingle.left = 0;

		if (rectSingle.right > nWidth)
			rectSingle.right = nWidth;

		rectComplete.UnionRect(rectComplete, rectSingle);
	}
		
	CClientDC dcClient(this);
	CDC dcMem;
	CBitmap Bitmap;

	if (!dcMem.CreateCompatibleDC(&dcClient))
		return NULL;

	if (!Bitmap.CreateCompatibleBitmap(&dcClient, rectComplete.Width(), rectComplete.Height()))
		return NULL;

	CBitmap *pOldMemDCBitmap = dcMem.SelectObject(&Bitmap);
	// Use green as mask color
 	dcMem.FillSolidRect(0, 0, rectComplete.Width(), rectComplete.Height(), RGB(0,255,0));

	// Paint each DragImage in the DC
	nIndex = pList->GetTopIndex() - 1;
	while ((nIndex = pList->GetNextItem(nIndex, LVNI_SELECTED)) != -1)
	{	
		if (nIndex > nBottomIndex)
			break;

		CPoint pt;
		CImageList* pSingleImageList = pList->CreateDragImage(nIndex, &pt);

		if (pSingleImageList)
		{
			pList->GetItemRect(nIndex, rectSingle, LVIR_BOUNDS);
			pSingleImageList->Draw( &dcMem, 
									0, 
									CPoint(rectSingle.left - rectComplete.left, 
									       rectSingle.top - rectComplete.top), 
									ILD_MASK);
			pSingleImageList->DeleteImageList();
			delete pSingleImageList;
		}
	}

 	dcMem.SelectObject(pOldMemDCBitmap);
	CImageList* pCompleteImageList = new CImageList;
	pCompleteImageList->Create(rectComplete.Width(), rectComplete.Height(), ILC_COLOR | ILC_MASK, 0, 1);
	if (pCompleteImageList->m_hImageList != NULL)
		pCompleteImageList->Add(&Bitmap, RGB(0, 255, 0)); // Green is used as mask color
	Bitmap.DeleteObject();

	if (lpPoint)
	{
		lpPoint->x = rectComplete.left;
		lpPoint->y = rectComplete.top;
	}

	return pCompleteImageList;

}


// Process command to save the current handbox collection
void CAU2View::OnFileSaveHbx() 
{
	// Create save file common dialog
	CFileDialog dlg (FALSE, _T("aud"), _T("Handbox.aud"), OFN_CREATEPROMPT |
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("AU Files (*.aud)|*.aud||"));
	CString dir = m_userSettings.GetInstallDirectory();
	dlg.m_ofn.lpstrInitialDir = dir;

	if (dlg.DoModal() == IDOK)	// if dialog exited with "OK"
	{
		CAU2Doc* pDoc = GetDocument();
		// save collection
		if (pDoc->m_handboxCollection->SaveToFile(dlg.GetFileName()) != READCOMPLETE)
			return;		
	}
}

// Process command to load a saved handbox collection
void CAU2View::OnFileOpenHbx() 
{
	CAU2Doc* pDoc = GetDocument();

	// warn user
	if (pDoc->m_handboxModified && MessageBox("This will delete the current contents of the displayed Handbox List",
		"Warning",MB_OKCANCEL) != IDOK) return;

	// Create load file dialog
	CFileDialog dlg (TRUE, _T("aud"), _T("Handbox.aud"), OFN_FILEMUSTEXIST |
		OFN_HIDEREADONLY, _T("AU Files (*.aud)|*.aud||"));
	CString dir = m_userSettings.GetInstallDirectory();
	dlg.m_ofn.lpstrInitialDir = dir;

	if (dlg.DoModal() == IDOK) // if dialog exited with "OK"
	{
		pDoc->m_handboxCollection->Clear();	// clear collection
		BeginWaitCursor();
		// attempt to load file
		if (pDoc->m_handboxCollection->LoadFromFile(dlg.GetFileName()) != READCOMPLETE)
		{
			EndWaitCursor();
			MessageBox("Could not read data from file","Error");
			return;		
		}
		CAU2ListView* hList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
		// Delete current data in list view
		DeleteListView(HANDBOX);
		// Fill list view with new data
		hList->InitializeList(pDoc->m_handboxCollection,pDoc->m_bodyType);
		UpdateMemInfo(); // update memory & object counts info

		// reset the radio buttons
		pDoc->ResetBodyTypeLabels(HANDBOX, pDoc->m_handboxCollection);
		// update the radio button display text
		((CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO))->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[pDoc->m_customDisplay], RADIO_SIZE));

		pDoc->m_handboxModified = true;
		EndWaitCursor();
	}
}

// Process command to restore Handbox collection from backup file
void CAU2View::OnFileRestoreHbx() 
{
	// warn user
	CAU2Doc* pDoc = GetDocument();
	if (pDoc->m_handboxModified && MessageBox("This will delete the current contents of the displayed Handbox List",
		"Warning",MB_OKCANCEL) != IDOK) return;
	pDoc->m_handboxCollection->Clear();	// clear collection
	// attempt to load file
	if (pDoc->m_handboxCollection->LoadFromFile(m_backupFile) != READCOMPLETE)
	{
		MessageBox("Could not read data from backup file","Error");
		return;		
	}
	CAU2ListView* hList = (CAU2ListView *) GetDlgItem(IDC_HANDBOXLIST);
	// Delete current data in list view
	DeleteListView(HANDBOX);
	// Fill list view with new data
	hList->InitializeList(pDoc->m_handboxCollection,pDoc->m_bodyType);
	UpdateMemInfo(); // update memory & object counts info

	// reset the radio buttons
	pDoc->ResetBodyTypeLabels(HANDBOX, pDoc->m_handboxCollection);
	// update the radio button display text
	((CButton *) GetDlgItem(IDC_HUSEROBJECTSRADIO))->SetWindowText(CPersist::Abbreviate(pDoc->m_hBodyTypeLabel[pDoc->m_customDisplay],RADIO_SIZE));

	pDoc->m_handboxModified = true;

}


// Used to support OnPreTranslateMessage Logic:
// If an item from the handbox list is selected, clear the selection
// list in the library list
void CAU2View::OnSetFocusHandboxList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CAU2ListView* lList = (CAU2ListView *) GetDlgItem(IDC_LIBRARYLIST);
	lList->GetListCtrl().SetSelectionMark(-1);
	
	*pResult = 0;
}


// Function to process the "Upgrade Handbox" Button message and gather
// data about the revision level of the software
void CAU2View::OnUpgrade() 
{
	CUserSettings userSettings;

	// Check to make sure autostar is connected, if not, attempt to connect now
	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();

//	Connect to handbox
	BeginWaitCursor();
	Connect();
	EndWaitCursor();

// Abort function if handbox is a 494
	if (pDoc->m_autostar.GetModel() == "494")
	{
		MessageBox("There is currently no method of upgrading the software \
in a Model #494 Autostar Handbox.","Warning",MB_OK);
		return;
	}

	// if autostar II, prompt to save user data
	CString prompt = "With most versions of this handbox, upgrading the\n\
firmware will cause User and PEC data to be lost.\n\
Do you wish to save this data first?";
	if ((m_ASType == TYPE_AUTOSTAR2 || m_ASType == TYPE_RCX)  && MessageBox(prompt, "Save Data?", MB_YESNO) == IDYES)
	{
			if (pDoc->m_autostar.InitializeConnection(false) != AUTOSTAR_OK)
				Connect(false);
			CUserSiteDlg dlg(&pDoc->m_autostar);
			bool retrieve = dlg.RetrieveData();
			// close the serial port
//			pDoc->m_autostar.CloseSerialPort();		

			// if retrieve failed, exit dialog
			if (retrieve)
				dlg.SaveData();
	}



	//Disable the send/receive buttons
	if (pDoc->m_autostar.GetVersion() == "1.0b" && m_ASType == TYPE_AUTOSTAR2)
		EnableButtons(0,0,0,0);	// we are upgrading an LX from rev. 1.0b (special case)
	else
		EnableButtons(0,2,0,0);

	CUpgradeHbxDlg dlg;
	if (!pDoc->m_hbxConnected)
	{
		dlg.m_hdWarning =  "You may only download from the internet to store the file\n";
		dlg.m_hdWarning += "on your local drive for upgrading later. Connect the\n";
		dlg.m_hdWarning += "handbox first to upgrade immediately.\n";
	}

	dlg.m_hbxText = "Handbox is currently Ver. " + pDoc->m_autostar.GetVersion();
	dlg.m_pSession = &m_session;

	if (dlg.DoModal() == IDOK)
	{
		m_eraseBanks = dlg.m_eraseBanks;

		if (dlg.m_downloadFlag)	// if download is selected,
		{
			if (pDoc->m_hbxConnected)	// and handbox is connected
				m_upgradeTask = UT_DOWNLOAD_UPGRADE;	// do it all
			else
				m_upgradeTask = UT_DOWNLOAD;	// otherwise, just download

			m_upgradeFile = userSettings.GetEphemDirectory();
			// create the filename to be saved (eliminating any .'s)
			m_upgradeFile += m_upgradeInfo[m_ASType].root
								+ dlg.m_wwwVer + m_upgradeInfo[m_ASType].ext;
			m_session.SetCallbackPointer(this);	// send status messages back to this object
			m_session.EnableFileCheck(false);	// disable file exists checking
		}
		else					// if download is not selected,
		{
			if (pDoc->m_hbxConnected)	// but handbox is connected
			{
				m_upgradeTask = UT_UPGRADE;	// upgrade from the HD
				m_upgradeFile = dlg.m_upgradeFile;
			}
			else
				m_upgradeTask = UT_NOTHING;	// otherwise, do nothing
		}
		AfxBeginThread(UpgradeThread, this);
	}
	else
		EnableButtons(1,2,1,1);

}

///////////////////////////////////////////////////////////////////////////
//
// Function to enable/ disable the various GUI buttons
//
// INPUT:	bool upgrade - to toggle the Upgrade Autostar & Exit Buttons
//			bool transfer -	to toggle the 6 transfer arrows
//			bool autostar - to toggle the send, receive, connect, garbage collect buttons
//			bool realTime - to toggle the realTime Mode button
//
//			0 = DISABLE, 1 = ENABLE, 2 = NO CHANGE
//
// OUTPUT:	none
//
///////////////////////////////////////////////////////////////////////////
void CAU2View::EnableButtons(int upgrade, int transfer, int autostar, int realTime)
{
	if (upgrade == 0)
		EnableUpgradeButtons(FALSE);
	if (upgrade == 1)
		EnableUpgradeButtons(TRUE);
	if (transfer == 0)
		EnableTransferControls(FALSE);
	if (transfer == 1)
		EnableTransferControls(TRUE);
	if (autostar == 0)
		EnableAutostarControls(FALSE);
	if (autostar == 1)
		EnableAutostarControls(TRUE);
	if (realTime == 0)
		EnableRealTimeControls(FALSE);
	if (realTime == 1)
		EnableRealTimeControls(TRUE);

}		

// Function to enable/disable the real time mode radio button
void CAU2View::EnableRealTimeControls(bool state)
{
	CAU2Doc* pDoc = GetDocument();
	CWnd* control = GetDlgItem(IDC_REALTIME);

	// do not enable realtime button unless model is Autostar II
	if (m_ASType != TYPE_AUTOSTAR2 || pDoc->m_autostar.m_hbxSafeMode)
	{
		control->EnableWindow(FALSE);
		return;
	}
	else
		control->EnableWindow(state);

}

// Function to enable/disable the six arrows that transfer collection objects
VOID CAU2View::EnableTransferControls(bool state)
{
	CAU2Doc* pDoc = GetDocument();

	if ((state) && (pDoc->m_autostar.m_hbxSafeMode == TRUE)) return;	// do not activate if in safe mode

	const int nControls = 6;
	UINT controls[nControls] = {IDC_BUTTON_TOLIB,IDC_BUTTON_TOHBX,IDC_BUTTON_TOLIB_ALL,
						IDC_BUTTON_TOHBX_ALL,IDC_BUTTON_TOLIB_REFRESH,
						IDC_BUTTON_TOHBX_REFRESH};
	for (int i = 0; i < nControls; i++)
	{
		CWnd* control = GetDlgItem(controls[i]);
		control->EnableWindow(state);
	}

	// also toggle the download from web menu item
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CMenu* menu = (CMenu *) pParent->GetMenu();
	
	const int nMenus = 4;
	UINT menus[nMenus] = {ID_FILE_WWW, ID_FILE_SAVE_HBX, ID_FILE_OPEN_HBX, ID_FILE_RESTORE};

	for (int j = 0; j < nMenus; j++)
	{
		if (state)
			menu->EnableMenuItem(menus[j], MF_BYCOMMAND | MF_ENABLED);
		else
			menu->EnableMenuItem(menus[j], MF_BYCOMMAND | MF_GRAYED);
	}


}

// Function to enable/disable the send, receive, connect buttons
void CAU2View::EnableAutostarControls(bool state)
{
	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();

	if ((state) && (pDoc->m_autostar.m_hbxSafeMode == TRUE)) return;	// do not activate if in safe mode

	const int nControls = 3;//4;
	UINT controls[nControls] = {IDC_BUTTON_RETRIEVE,IDC_BUTTON_SEND,
								IDC_BUTTON_CONNECT};//,IDC_BUTTON_RESTART};

	for (int i = 0; i < nControls; i++)
	{
		// only enable restart button if handbox is connected
//		if (controls[i] == IDC_BUTTON_RESTART && !pDoc->m_hbxConnected)
//			continue;
		CWnd* control = GetDlgItem(controls[i]);
		control->EnableWindow(state);
	}

	// also toggle the garbage collect & baud rate menu items
	// only display GC & baud rate if an AS2 is connected
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CMenu* menu = (CMenu *) pParent->GetMenu();
	CMenu* subMenu = menu->GetSubMenu(2);
	
	if ((m_ASType == TYPE_AUTOSTAR2 || m_ASType == TYPE_RCX) && state)
	{
		menu->EnableMenuItem(ID_TOOLS_GC, MF_BYCOMMAND | MF_ENABLED);
		subMenu->EnableMenuItem(7, MF_BYPOSITION | MF_ENABLED);	// baud rate
		// enable all the baud menu items too (in case we were just in safe mode)
		for (int menuItem = ID_OPTIONS_BAUD_56K; menuItem <= ID_OPTIONS_BAUD_9600; menuItem++)
			menu->EnableMenuItem(menuItem, MF_BYCOMMAND | MF_ENABLED);

		if (m_ASType == TYPE_RCX)
			menu->EnableMenuItem(ID_OPTIONS_BAUD_115K, MF_BYCOMMAND | MF_ENABLED);
		else
			menu->EnableMenuItem(ID_OPTIONS_BAUD_115K, MF_BYCOMMAND | MF_GRAYED);


	}
	else
	{
		menu->EnableMenuItem(ID_TOOLS_GC, MF_BYCOMMAND | MF_GRAYED);

		// if we're not in safe mode on an ASII, just disable everything
		if (m_ASType == TYPE_AUTOSTAR2 && !pDoc->m_autostar.m_hbxSafeMode)
			subMenu->EnableMenuItem(7, MF_BYPOSITION | MF_GRAYED);	// baud rate
		
		else	// if we ARE in safe mode on an ASII, disable all BUT the 56K menu item
		{
			subMenu->EnableMenuItem(7, MF_BYPOSITION | MF_ENABLED);	
			for (int menuItem = ID_OPTIONS_BAUD_115K; menuItem <= ID_OPTIONS_BAUD_9600; menuItem++)
				menu->EnableMenuItem(menuItem, MF_BYCOMMAND | MF_GRAYED);
		}
	}

	// also enable the reset handbox menu item (only if connected)
	if (pDoc->m_hbxConnected && state)
		menu->EnableMenuItem(ID_TOOLS_RESET, MF_BYCOMMAND | MF_ENABLED);
	else
		menu->EnableMenuItem(ID_TOOLS_RESET, MF_BYCOMMAND | MF_GRAYED);

	// also enable the edit user data menu item (but not in safe mode)
	if (state && !pDoc->m_autostar.m_hbxSafeMode)
		menu->EnableMenuItem(ID_TOOLS_USERSITE, MF_BYCOMMAND | MF_ENABLED);
	else
		menu->EnableMenuItem(ID_TOOLS_USERSITE, MF_BYCOMMAND | MF_GRAYED);

}

// Function to enable/disable the upgrade autostar & exit buttons
void CAU2View::EnableUpgradeButtons(bool state)
{
	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();

	//enable the send/ receive buttons
	CButton* uButton = (CButton *) GetDlgItem(IDC_UPGRADE);
	CButton* eButton = (CButton *) GetDlgItem(ID_EXIT);
	if (state)
	{
		if (pDoc->m_autostar.GetModel() == "494")
			uButton->EnableWindow(FALSE);
		else
			uButton->EnableWindow();
		eButton->EnableWindow();
	}
	else
	{
		uButton->EnableWindow(FALSE);
		eButton->EnableWindow(FALSE);
	}
}


//Function to complete download operation
void CAU2View::DownloadComplete(bool doneWithThread)
{ 
	//Get reference to Parent
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	
	CAU2Doc* pDoc = GetDocument();	// get pointer to document

	pDoc->m_statusText = "Download Complete";

	if (doneWithThread)
	{
		//enable the send/ receive buttons
		EnableButtons(1,2,1,1);
	}

	// determine which thread is calling this function, 
	if (AfxGetThread()->m_nThreadID == m_mainThreadID)
		// if the main thread, update status bar directly
		pParent->m_wndStatusBar.GetStatusBarCtrl().SetText(pDoc->m_statusText,0,SBT_NOBORDERS);
	else
		// otherwise, post a message
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);

	UpdateMemInfo();

	// enable transfer objects controls (turned off during download of objects)
	EnableButtons(2,1,2,2);

}

//Function that generates the string used in the Confirm Object Replace Dialog Box
CString CAU2View::GetReplaceText(CBodyDataCollection *collection, CString key)
{
	CString text;

	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();
	CBodyData* data = collection->Find(key);
	text = key;
	text.TrimRight();

	switch (pDoc->m_bodyType)
	{
	case Asteroid:
		text += ", Epoch: ";
		text += data->GetFieldData(1);
		break;
	case Satellite:
		text += ", Epoch: ";
		text += data->GetFieldData(3);
		break;
	case Comet:
		text += ", Epoch: ";
		text += data->GetFieldData(1);
		break;
	case LandMark:
		text += ", Az: ";
		text += data->GetFieldData(1);
		text += ", Alt: ";
		text += data->GetFieldData(2);
		break;
	case Tour:
		text += ", Length: ";
		text += data->GetFieldData(1);
		break;
	case UserObj:
		text += ", R.A.: ";
		text += data->GetFieldData(1);
		text += ", Dec.: ";
		text += data->GetFieldData(2);
		break;
	}

	return text;
}


// Post window messages to update the information at the bottom of each list view
void CAU2View::UpdateMemInfo()
{
	//Get reference to Parent
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();

	pParent->PostMessage(WM_COMMAND,IDC_STATIC_LIBRARY1);
	pParent->PostMessage(WM_COMMAND,IDC_STATIC_LIBRARY2);
	pParent->PostMessage(WM_COMMAND,IDC_STATIC_HANDBOX1);
	pParent->PostMessage(WM_COMMAND,IDC_STATIC_HANDBOX2);
}


// Process the FILE-DOWNLOAD menu selection
void CAU2View::OnFileDownload() 
{
	if (m_downLoadingBodyData)
	{
		AfxMessageBox("Already Downloading");
		return;
	}
		
	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();

	//clear selection lists
	m_downloadParams.RemoveAll();

	//reset import count
	m_importCount = 0;

	// Instantiate dialog box for selecting files to download
	CSatelliteDlg dlg;

	// launch dialog box
	if (dlg.DoModal() != IDOK)
		return;

	// disable transfer objects controls
	EnableButtons(2,0,2,0);

	// disable the menu button
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CMenu* pMenu = pParent->GetMenu();
	pMenu->EnableMenuItem(ID_FILE_WWW, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	pMenu->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

	// copy list of files to download to member variable
	m_downloadParams.Copy(dlg.m_selectedFiles);
	m_session.ClearDownloadParams();
	m_session.SetDownloadParams(m_downloadParams);

	// disable file exists checking
	m_session.EnableFileCheck(FALSE);

	// Make sure destination directory exists, if not, create it
	CString directory;
	CUserSettings user;
	directory = user.GetEphemDirectory() + DOWNLOAD_SUBDIR;
	if (_chdir(directory))	// check if directory exists
		_mkdir(directory);	// if not, create it

	// if update is checked, set update option
	m_importOption = (dlg.m_update) ? CBodyDataCollection::update : CBodyDataCollection::dupchk;

	// check if clear collection was checked, if so clear those objects
	// first test if any of the given object type is being downloaded
	//		(do not delete body types that are not being downloaded)
	// then check if delete objects was checked
	// then make sure they haven't ALSO checked update
	if (dlg.m_clearBodyType[0] && dlg.m_delete && !dlg.m_update)
		pDoc->m_libraryCollection.Clear(Asteroid);
	if (dlg.m_clearBodyType[1] && dlg.m_delete && !dlg.m_update)
		pDoc->m_libraryCollection.Clear(Satellite);
	if (dlg.m_clearBodyType[2] && dlg.m_delete && !dlg.m_update)
		pDoc->m_libraryCollection.Clear(Comet);
	if (dlg.m_clearBodyType[3] && dlg.m_delete && !dlg.m_update)
		pDoc->m_libraryCollection.Clear(Tour);

	// execute download in a separate thread
	AfxBeginThread(EphemerideThread,this);
}

// Answers the callback from the CHTTPDownload process indicating
// that a file was successfully saved to the disk
bool CAU2View::SaveFileComplete(CHTTPDownloadParams info)
{
	CAU2Doc* pDoc = GetDocument();

	BodyType bodyType = All;
	if (info.saveFileName.Left(2) == ASTEROID_PREFIX)
		bodyType = Asteroid;
	if (info.saveFileName.Left(2) == SATELLITE_PREFIX)
		bodyType = Satellite;
	if (info.saveFileName.Left(2) == COMET_PREFIX)
		bodyType = Comet;
	if (info.saveFileName.Left(2) == TOUR_PREFIX)
		bodyType = Tour;

	int objectCount = 0;

	if (bodyType != All)
	{
		CString temp;
		temp = "Importing " + CHTTPDownload::GetURINoPath(info.URIName);
		DoingProcess(temp);
		FileStat importStat = pDoc->m_libraryCollection.Import(info.saveFilePath + info.saveFileName,
								bodyType, objectCount, m_importOption);
		if ((importStat == READCOMPLETE) && (objectCount))
		{
//			pDoc->m_bodyType = bodyType;
			m_importCount++;
			return TRUE;
		}
		else
		{
			if (importStat != READCOMPLETE)	// file could not be read
				m_session.AddTransferLogEntry("Error Importing File: "
						+ CHTTPDownload::GetURINoPath(info.URIName));
			else	// file read, but no objects imported
				m_session.AddTransferLogEntry("Warning: No Objects Imported From "
						+ CHTTPDownload::GetURINoPath(info.URIName));
		}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to answer the callback from CHTTPDownload and update the status
//
// Input:	CString statusString
//			int percent (optional parameter to display percent complete)
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CAU2View::UpdateStatus(CString statString, int percent)
{
	//Get reference to Parent
	CMainFrame* pParent = (CMainFrame *) GetSafeOwner();
	CAU2Doc* pDoc = GetDocument();

	if (pDoc->m_statusText == statString)	//nothing has changed!
		return;
	
	pDoc->m_statusText = statString;
	// determine which thread is calling this function, 
	TRACE("\nMain Thread: %i          Current Thread: %i\n", m_mainThreadID, AfxGetThread()->m_nThreadID);
	if (AfxGetThread()->m_nThreadID == m_mainThreadID)
		// if the main thread, update status bar directly
		pParent->m_wndStatusBar.GetStatusBarCtrl().SetText(pDoc->m_statusText,0,SBT_NOBORDERS);
	else
		// otherwise, post a message
		pParent->PostMessage(WM_COMMAND,ID_INDICATOR_STATUS);

}


void CAU2View::OnButtonRealTime() 
{
	CButton* realTime = (CButton *) GetDlgItem(IDC_REALTIME);

	if (realTime->GetCheck() == 1)
	{
		if (MessageBoxEx(m_hWnd, "Please read Help Text on this command before using.\nTo engage real-time mode, the data from the handbox\n\
must first be retrieved.  This may take a few minutes.\n\
OK to proceed?","Warning",MB_YESNO | MB_HELP, LANG_ENGLISH) == IDNO)
		{
			EnableRealTimeMode(FALSE);
			realTime->SetCheck(0);
		}
		else
			EnableRealTimeMode(TRUE);
	}
	else
		EnableRealTimeMode(FALSE);
	
}

void CAU2View::EnableRealTimeMode(bool state)
{
	// if not an Autostar II, this function should not proceed
	if (m_ASType != TYPE_AUTOSTAR2)
		return;

	// toggle the member variable
	m_realTimeMode = state;

	CAU2ListView* hbxWindow = (CAU2ListView *)GetDlgItem(IDC_HANDBOXLIST);

	// change the list view background color
	if (state)
	{
		// get the contents of the handbox
		BeginWaitCursor();
		Retrieve(false);	// retrieve (within this thread)
		CAU2Doc *pDoc = GetDocument();
		// change the list view background color
		hbxWindow->GetListCtrl().SetTextBkColor(PINK);
		hbxWindow->GetListCtrl().SetBkColor(PINK);
		// disable autostar & transfer buttons
		EnableButtons(0,1,0,1);
	}
	else
	{
		// change the list view background color
		hbxWindow->GetListCtrl().SetTextBkColor(WHITE);
		hbxWindow->GetListCtrl().SetBkColor(WHITE);
		// enable all buttons
		EnableButtons(1,1,1,1);
		// display status message
		DoingProcess("Real-Time Mode Disabled");
	}
	hbxWindow->Invalidate();
	hbxWindow->RedrawWindow();
}


eAutostarStat CAU2View::SendOneObjectRealTime(CBodyData *body)
{
	if (!m_realTimeMode)
		return WRONG_MODE;

	CAU2Doc* pDoc = (CAU2Doc *) GetDocument();
	BodyType type = body->GetBodyType();
	int num;

	// send a status message
	DoingProcess("Sending: " + body->GetKey());

	return pDoc->m_autostar.SendOneObject(body, type, num);

}

HBRUSH CAU2View::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	int temp = pWnd->GetDlgCtrlID();
	
	// TODO: Return a different brush if the default is not desired
	if (pWnd->GetDlgCtrlID() == 0x0000e900/*IDD_AU2_FORM || pWnd->GetDlgCtrlID() == IDD_AU2_FORM2*/)
		return m_brush;
	else
		return hbr;


}



void CAU2View::TestFunction()
{
	CAU2Doc* pDoc = GetDocument();
	
	Connect();
	eAutostarStat stat = pDoc->m_autostar.InitializeConnection(false,true);

	if (stat == AUTOSTAR_OK)

		stat = pDoc->m_autostar.TestFunction(); 
}






