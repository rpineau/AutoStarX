// AU2.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AU2.h"

#include "MainFrm.h"
#include "AU2Doc.h"
#include "AU2View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAU2App

BEGIN_MESSAGE_MAP(CAU2App, CWinApp)
	//{{AFX_MSG_MAP(CAU2App)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAU2App construction

CAU2App::CAU2App()
{


}

/////////////////////////////////////////////////////////////////////////////
// The one and only CAU2App object

CAU2App theApp;

/////////////////////////////////////////////////////////////////////////////
// CAU2App initialization

BOOL CAU2App::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CAU2Doc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CAU2View));
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// If a file is not specified on the command line, open the last file
	if (!cmdInfo.m_strFileName.GetLength())
	{ 
	  if (m_pRecentFileList->m_nSize > 0 && 
				  !m_pRecentFileList->m_arrNames[0].IsEmpty())
	  {
		  cmdInfo.m_strFileName = m_pRecentFileList->m_arrNames[0];

			CFileStatus status;
			// if MRFile exists, and option is enabled, load file
			if (CFile::GetStatus(cmdInfo.m_strFileName, status) && m_userSettings.GetOptions(CUserSettings::RECENT)) 
				cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen; 
	  }
	}



	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;


	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	//Refresh List View and change to default body type
	CMainFrame* pMain = (CMainFrame *) m_pMainWnd->GetSafeOwner();
	CAU2View* pView = (CAU2View *) pMain->GetActiveView();
	CAU2Doc* pDoc = (CAU2Doc *) pView->GetDocument();
	pView->ChangeListBodyType(pDoc->m_bodyType);

	// Must do this to use a rich edit control!!!
	AfxInitRichEdit();

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CAU2App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CAU2App message handlers





BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// load the string table entry for the version info and display it
	CStatic* staticVer = (CStatic *) GetDlgItem(IDC_VERSION);
	char buffer[50];
	LoadString(NULL,IDS_VERSION, buffer, 50);
	staticVer->SetWindowText(buffer);


	// add the date stamp of last compile
	CStatic* statDate = (CStatic *) GetDlgItem(IDC_STATIC_DATE);

	CString date = __DATE__;
	date += "  ";
	date += __TIME__;
	date += " PT";

	statDate->SetWindowText(date);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
