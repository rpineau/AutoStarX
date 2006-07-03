// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "AU2.h"

#include "MainFrm.h"
#include "AU2Doc.h"
#include "AU2View.h"

#include "SelectDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const CRect CMainFrame::m_winSize[2] = {CRect(0,0,470,143),		//size of basic window
					               CRect(0,0,780,492)};		//size of advanced window/486/469


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_FILE_RESTORE, OnUpdateFileRestore)
	ON_WM_DROPFILES()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI (ID_INDICATOR_MODEL, OnUpdateIndicator)
	ON_UPDATE_COMMAND_UI (ID_INDICATOR_STATUS, OnUpdateIndicator)
	ON_UPDATE_COMMAND_UI (ID_INDICATOR_VER, OnUpdateIndicator)
	ON_UPDATE_COMMAND_UI_RANGE (ID_OPTIONS_RECENT,ID_OPTIONS_RETRIEVE, OnUpdateOptions)
	ON_UPDATE_COMMAND_UI_RANGE (ID_OPTIONS_BAUD_56K,ID_OPTIONS_BAUD_9600, OnUpdateOptionsBaud)
	ON_UPDATE_COMMAND_UI_RANGE (IDC_STATIC_LIBRARY1, IDC_STATIC_HANDBOX2, OnUpdateMemInfo)
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_MODEL,
	ID_INDICATOR_VER
};


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_winSizeState = 0;
	m_bAutoMenuEnable = FALSE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
// Display toolbar - commented out

//	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
//		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
//		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
//	{
//		TRACE0("Failed to create toolbar\n");
//		return -1;      // fail to create
//	}



	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))


	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}


	ToggleStatusBar(0); //initially set to basic

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
//	EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_wndToolBar);

	// set initial (0 = basic) window size
	SetWindowPos(NULL,10,40,(m_winSize[0].Width()+10),(m_winSize[0].Height()+40),
				 SWP_NOZORDER);


	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_POPUPWINDOW | WS_CAPTION | FWS_ADDTOTITLE | FWS_PREFIXTITLE   
		| WS_SYSMENU | WS_MINIMIZEBOX;
	

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// change window size (state 0 = basic, state 1 = advanced)
void CMainFrame::SetWinSize(int state)
{
	SetWindowPos(NULL,10,40,(m_winSize[state].Width()+10),(m_winSize[state].Height()+40),
				 SWP_NOZORDER | SWP_NOMOVE);
}

// Update status message in status bar
void CMainFrame::OnUpdateIndicator(CCmdUI *pCmdUI)
{
	CString tmp;
	// get a pointer to document
	CView* pView = GetActiveView();
	CAU2Doc* pDoc = (CAU2Doc *) pView->GetDocument();

	pCmdUI->Enable();

	switch (pCmdUI->m_nID)	//resource ID of status pane
	{
	case ID_INDICATOR_MODEL:	//advanced vs. basic pane
		pCmdUI->SetText(CString("Model:") + pDoc->m_autostar.GetModel());
		break;

	case ID_INDICATOR_STATUS:	// generic status text
		//cannot use pCmdUI because ID_INDICATOR_STATUS is not on the status bar
		// the first pane must be called ID_SEPARATOR for the MFC context-help to work
		m_wndStatusBar.SetPaneText(0,pDoc->m_statusText);
		break;

	case ID_INDICATOR_VER:
		pCmdUI->SetText(CString("Handbox Ver.:") + pDoc->m_autostar.GetVersion());
		break;
	}
	
}

// change number and sizes of panes depending on window size state
void CMainFrame::ToggleStatusBar(int state)
{
	if (state)  // Advanced View
	{
		m_wndStatusBar.SetIndicators(indicators,sizeof(indicators)/sizeof(UINT));
		m_wndStatusBar.SetPaneInfo(0,ID_SEPARATOR, SBPS_NOBORDERS | SBPS_STRETCH, 220);
		m_wndStatusBar.SetPaneInfo(1,ID_INDICATOR_MODEL, SBPS_NORMAL, 120);
		m_wndStatusBar.SetPaneInfo(2,ID_INDICATOR_VER,SBPS_NORMAL, 160);
	}
	else	// Basic View
	{
		m_wndStatusBar.SetIndicators(indicators,1);
		m_wndStatusBar.SetPaneInfo(0,ID_SEPARATOR, SBPS_NOBORDERS | SBPS_STRETCH, 220);
	}
}

// Update the File Menu to toggle Restore based on existance of backup file
void CMainFrame::OnUpdateFileRestore(CCmdUI* pCmdUI) 
{
	// get a pointer to document
	CAU2View* pView = (CAU2View *) GetActiveView();
	CAU2Doc* pDoc = (CAU2Doc *) pView->GetDocument();

	// Get a pointer to the menu
	CMenu* menu = (CMenu *) GetMenu();

	CFileStatus status;
	if (CFile::GetStatus(pView->m_backupFile, status))
 		menu->EnableMenuItem(ID_FILE_RESTORE,MF_ENABLED);
	else
 		menu->EnableMenuItem(ID_FILE_RESTORE,MF_GRAYED);
}

// Update the Options Menu to toggle options based on Registry entries
void CMainFrame::OnUpdateOptions(CCmdUI* pCmdUI)
{
	CUserSettings userSettings;
	
	// Get a pointer to the menu
	CMenu* menu = (CMenu *) GetMenu();

	switch (pCmdUI->m_nID)
	{
	case ID_OPTIONS_CONNECT:
		if (userSettings.GetOptions(CUserSettings::CONNECT))
		{
			menu->CheckMenuItem(ID_OPTIONS_CONNECT,MF_CHECKED);
		}
		else
			menu->CheckMenuItem(ID_OPTIONS_CONNECT,MF_UNCHECKED);
		break;
	case ID_OPTIONS_RETRIEVE:
		if (userSettings.GetOptions(CUserSettings::RETRIEVE))
		{
			menu->CheckMenuItem(ID_OPTIONS_RETRIEVE,MF_CHECKED);
		}
		else
			menu->CheckMenuItem(ID_OPTIONS_RETRIEVE,MF_UNCHECKED);
		break;
	case ID_OPTIONS_ADVANCED:
		if (userSettings.GetOptions(CUserSettings::ADVANCED))
			menu->CheckMenuItem(ID_OPTIONS_ADVANCED,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_ADVANCED,MF_UNCHECKED);
		break;
	case ID_OPTIONS_VERIFY:
		if (userSettings.GetOptions(CUserSettings::VERIFY))
			menu->CheckMenuItem(ID_OPTIONS_VERIFY,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_VERIFY,MF_UNCHECKED);
		break;
	case ID_OPTIONS_RECENT:
		if (userSettings.GetOptions(CUserSettings::RECENT))
			menu->CheckMenuItem(ID_OPTIONS_RECENT,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_RECENT,MF_UNCHECKED);
		break;
	}

}


// Update the Options Menu to toggle baud rate options based on Registry entries
void CMainFrame::OnUpdateOptionsBaud(CCmdUI* pCmdUI)
{
	CUserSettings userSettings;
	
	// Get a pointer to the menu
	CMenu* menu = (CMenu *) GetMenu();

	switch (pCmdUI->m_nID)
	{
	case ID_OPTIONS_BAUD_115K:
		if (userSettings.GetBaud() == CSerialPort::b115k)
			menu->CheckMenuItem(ID_OPTIONS_BAUD_115K,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_BAUD_115K,MF_UNCHECKED);
		break;

	case ID_OPTIONS_BAUD_56K:
		if (userSettings.GetBaud() == CSerialPort::b56k)
			menu->CheckMenuItem(ID_OPTIONS_BAUD_56K,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_BAUD_56K,MF_UNCHECKED);
		break;

	case ID_OPTIONS_BAUD_38K:
		if (userSettings.GetBaud() == CSerialPort::b38k)
			menu->CheckMenuItem(ID_OPTIONS_BAUD_38K,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_BAUD_38K,MF_UNCHECKED);
		break;

	case ID_OPTIONS_BAUD_28K:
		if (userSettings.GetBaud() == CSerialPort::b28k)
			menu->CheckMenuItem(ID_OPTIONS_BAUD_28K,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_BAUD_28K,MF_UNCHECKED);
		break;

	case ID_OPTIONS_BAUD_19K:
		if (userSettings.GetBaud() == CSerialPort::b19k)
			menu->CheckMenuItem(ID_OPTIONS_BAUD_19K,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_BAUD_19K,MF_UNCHECKED);
		break;

	case ID_OPTIONS_BAUD_14K:
		if (userSettings.GetBaud() == CSerialPort::b14k)
			menu->CheckMenuItem(ID_OPTIONS_BAUD_14K,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_BAUD_14K,MF_UNCHECKED);
		break;

	case ID_OPTIONS_BAUD_9600:
		if (userSettings.GetBaud() == CSerialPort::b9600)
			menu->CheckMenuItem(ID_OPTIONS_BAUD_9600,MF_CHECKED);
		else
			menu->CheckMenuItem(ID_OPTIONS_BAUD_9600,MF_UNCHECKED);
		break;
	}
	
}
// Update the List View Object Counts & Memory
void CMainFrame::OnUpdateMemInfo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();

	// get a pointer to the view & document
	CAU2View* pView = (CAU2View *) GetActiveView();
	CAU2Doc* pDoc = (CAU2Doc *) pView->GetDocument();

	CStatic* text = (CStatic *) pView->GetDlgItem(pCmdUI->m_nID);
	CString tmp;

	static COLORREF background = pView->m_labelHbx2.GetBkColor();
	static COLORREF textColor = pView->m_labelHbx2.GetTextColor();

	//check the font size on the normal static controls
	CStatic* hbx1Static = (CStatic *) pView->GetDlgItem(IDC_STATIC_HANDBOX1);
	CFont* staticFont = hbx1Static->GetFont();
	LOGFONT staticLogFont;
	staticFont->GetLogFont(&staticLogFont);
	//set the CLabel static control to this same height
	pView->m_labelHbx2.SetFontSize(-staticLogFont.lfHeight);

	CString libLabel = pDoc->m_lBodyTypeLabel[pDoc->m_bodyType];
	libLabel.TrimRight();
	libLabel = CPersist::Abbreviate(libLabel,10);
	CString hbxLabel = pDoc->m_hBodyTypeLabel[pDoc->m_bodyType];
	hbxLabel = CPersist::Abbreviate(hbxLabel,10);
	hbxLabel.TrimRight();
	
	switch (pCmdUI->m_nID)
	{
	case IDC_STATIC_LIBRARY1:
		tmp.Format("%d %s %d Total",pDoc->m_libraryCollection.GetCount(pDoc->m_bodyType),
			libLabel.GetBuffer(5),
			pDoc->m_libraryCollection.GetCount());
		text->SetWindowText(tmp);
		break;
	case IDC_STATIC_LIBRARY2:
		tmp.Format("Total Mem.: %d Bytes",pDoc->m_libraryCollection.GetTotalSizeOf());
		text->SetWindowText(tmp);
		break;
	case IDC_STATIC_HANDBOX1:
		tmp.Format("%d %s %d Total",pDoc->m_handboxCollection->GetCount(pDoc->m_bodyType),
			hbxLabel.GetBuffer(5),
			pDoc->m_handboxCollection->GetCount());
		text->SetWindowText(tmp);
		break;
	case IDC_STATIC_HANDBOX2:
		if (pDoc->m_hbxConnected && (pDoc->m_autostar.m_hbxSafeMode == FALSE))
		{
			int memory = pDoc->m_autostar.GetAvailableMemory();
			if (memory <= 0) // if available memory is negative
			{
				pView->m_labelHbx2.SetTextColor(RED);
				pView->m_labelHbx2.SetBkColor(YELLOW);
				pView->m_labelHbx2.FlashBackground(TRUE);
			}
			else
			{
				pView->m_labelHbx2.SetTextColor(textColor);
				pView->m_labelHbx2.SetBkColor(background);
				pView->m_labelHbx2.FlashBackground(FALSE);
			}

			tmp.Format("Avail. Mem.: %d Bytes",memory);
			text->SetWindowText(tmp);
		}
		else
			text->SetWindowText("Memory Info N/A");
		break;
	}

}

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	// determine # of files dropped
	int nCount = ::DragQueryFile(hDropInfo, (UINT) -1, NULL, 0); 

	if (nCount == 1) // only allow one drop at a time
	{
		CBodyDataCollection* collection;

		// determine drop point
		POINT dropPt;
		::DragQueryPoint(hDropInfo,&dropPt);

		// determine which list view was drop target
		// get a pointer to the view & document
		CAU2View* pView = (CAU2View *) GetActiveView();
		CAU2Doc* pDoc = (CAU2Doc *) pView->GetDocument();

		// Get pointers to the list view regions
		CStatic *lList = (CStatic *) pView->GetDlgItem(IDC_LIBRARY);
		CStatic *hList = (CStatic *) pView->GetDlgItem(IDC_HANDBOX);

		// Get the coordinates of each list view's region
		CRect lRect,hRect;
		lList->GetWindowRect(&lRect);
		hList->GetWindowRect(&hRect);
		ScreenToClient(lRect);
		ScreenToClient(hRect);

		if (lRect.PtInRect(dropPt)) 
		{
			collection = &pDoc->m_libraryCollection;
		}
			else if (hRect.PtInRect(dropPt)) 
			{
				if (pView->m_realTimeMode)	// disable this function in real time mode
					return;
				collection = pDoc->m_handboxCollection;
			}
			else
				return;

		//determine file name and extension
		char szFile[MAX_PATH];
		::DragQueryFile(hDropInfo,0,szFile,sizeof(szFile));
		CString fileName = (CString) szFile;
		CString fileExt = fileName.Right(3);
		fileExt.MakeLower();

		//check if file type is valid
		if (fileExt != "txt" && fileExt != "aud" && fileExt != "cmt"
			&& fileExt != "tle" && fileExt != "rom" && fileExt != "mtf")
		{
			MessageBox("File format not supported for Drag and Drop",
						"Error",MB_ICONWARNING);
			return;
		}

		// process AUD files
		if (fileExt == "aud")
		{
			CFrameWnd::OnDropFiles(hDropInfo);
			return;
		}

		// process Everything else using the Select Object Dialog
		CSelectDlg dlg;

		// preprocess TXT and ROM files
		if (fileExt == "txt" || fileExt == "rom")
		{
			//Set member variables for Object Select Dialog
			dlg.m_title = "Import Object Dialog";
			dlg.m_prompt = "Select the type of object being imported";
		}

		//preprocess CMT files
		if (fileExt == "cmt")
		{
			//Set member variables for Object Select Dialog
			dlg.m_title = "Import Comet Dialog";
			dlg.m_prompt = "Select the import options";
			//Set the body type to comet
			dlg.m_selectBodyType = Comet;
		}

		//preprocess TLE files
		if (fileExt == "tle")
		{
			//Set member variables for Object Select Dialog
			dlg.m_title = "Import Satellite Dialog";
			dlg.m_prompt = "Select the import options";
			//Set the body type to satellite
			dlg.m_selectBodyType = Satellite;
		}

		//preprocess MTF files
		if (fileExt == "mtf")
		{
			//Set member variables for Object Select Dialog
			dlg.m_title = "Import Tour Dialog";
			dlg.m_prompt = "Select the import options";
			//Set the body type to satellite
			dlg.m_selectBodyType = Tour;
		}
		
		//preprocess the dialog box input & import the object
		if (dlg.DoModal() != IDOK) return; // call up the dialog
		if (dlg.m_delete && !dlg.m_update)	// if delete checked, clear collection
			collection->Clear(dlg.GetBodyType());
		// if update checked, set update option
		CBodyDataCollection::importOption import;
		if (dlg.m_update) import = CBodyDataCollection::update;
		else import = CBodyDataCollection::dupchk;
		// attempt to import file
		if (collection->Import(fileName, dlg.GetBodyType(), import) != READCOMPLETE)
		{
			MessageBox("Error Importing File","ERROR",MB_ICONEXCLAMATION);
			return;
		}
		else	// refresh the list views
		{
			pDoc->m_bodyType = dlg.GetBodyType();
			pView->ChangeListBodyType(pDoc->m_bodyType);
		}

	}
}

// check if upload is in process before exiting
void CMainFrame::OnClose() 
{
	CAU2View* pView = (CAU2View *) GetActiveView();
	CAU2Doc* pDoc = (CAU2Doc *) pView->GetDocument();
	if (pDoc->m_autostar.m_mode == BUSY)
		if (MessageBox("Exiting the program while uploading may\ndamage \
the handbox. Proceed with exit?","Confirm Exit",MB_OKCANCEL) == IDCANCEL)
			return;
	
	if (pDoc->m_handboxModified)
		if (MessageBox("Handbox data has not been saved to Autostar!\n Proceed with exit?", 
			"Confirm Exit", MB_OKCANCEL) == IDCANCEL)
			return;

//	if (pDoc->m_hbxConnected && !pDoc->m_autostar.m_hbxSafeMode)
//		pDoc->m_autostar.RestartHandbox();

	// set the serial port value back to 9600
	pDoc->m_autostar.SetMaxBaudRate(CSerialPort::b9600, true);

	CFrameWnd::OnClose();
}
