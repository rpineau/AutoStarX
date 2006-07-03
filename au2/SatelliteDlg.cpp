// SatelliteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SatelliteDlg.h"

#include <afxcmn.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CSatelliteDlg dialog


CSatelliteDlg::CSatelliteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSatelliteDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSatelliteDlg)
	m_delete = FALSE;
	m_update = FALSE;
	//}}AFX_DATA_INIT
}


void CSatelliteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSatelliteDlg)
	DDX_Check(pDX, IDC_CHECK_DELETE, m_delete);
	DDX_Check(pDX, IDC_CHECK_UPDATE, m_update);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSatelliteDlg, CDialog)
	//{{AFX_MSG_MAP(CSatelliteDlg)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_SATELLITES, OnItemExpandingTreeSatellites)
	ON_NOTIFY(NM_CLICK, IDC_TREE_SATELLITES, OnClickTreeSatellites)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSatelliteDlg message handlers



BOOL CSatelliteDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_types[0] = "Asteroids";
	m_types[1] = "Satellites";
	m_types[2] = "Comets";
	m_types[3] = "Tours";

	// add names to tree view control
	CTreeCtrl* tree = (CTreeCtrl *) GetDlgItem(IDC_TREE_SATELLITES);

	for (int i = 0; i < NUM_TYPES; i++)
	{
		m_hRoots[i] = tree->InsertItem(m_types[i]);
		tree->InsertItem(DEFAULT_CHILD_TEXT,m_hRoots[i]);
		// initialize array that tracks which objects should be cleared
		m_clearBodyType[i] = FALSE;
	}

	//Make window the topmost window
	SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);

	// Get info about the dialog window size
	CRect clientRect;
	GetClientRect(&clientRect);

	// Add the status bar
	m_wndStatusBar.Create(WS_CHILD | WS_VISIBLE | CCS_BOTTOM, 
						  CRect(clientRect.left,clientRect.bottom - STATUS_HEIGHT,clientRect.Width(),clientRect.Height()),
						  this, ID_STATUSBAR);
	int StatusWidths[1] = {-1};
	m_wndStatusBar.SetParts(1,StatusWidths);
	m_statusString = "Ready";
	m_wndStatusBar.SetText(m_statusString,0,0);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}




//////////////////////////////////////////////////////////////////////
// 
// Function used to update the status bar on the dialog
//
// Input:	CString status text
//			integer percent complete (optional - not used in this dlg)
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CSatelliteDlg::UpdateStatus(CString statString, int percent)
{
	if (m_statusString == statString)	//nothing has changed!
		return;

	if (statString == "HTTP Status Code: 200, Reason: OK")
		m_statusString = "Successfully Retrieved Object List";
	else
		m_statusString = statString;
	m_wndStatusBar.SetText(m_statusString,0,0);

	CRect statusRect;
	m_wndStatusBar.GetClientRect(&statusRect);
	ClientToScreen(&statusRect);
	InvalidateRect(statusRect);	
}


//////////////////////////////////////////////////////////////////////
// 
// Function called by Windows when tree item is expanded
//		use this to retrieve available items from the WWW and build the tree
//
// Input:	N/A
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
void CSatelliteDlg::OnItemExpandingTreeSatellites(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Get notification message structure
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// if action = collapse, don't do anything
	if (pNMTreeView->action == TVE_COLLAPSE)
	{
		*pResult = 0;
		return;
	}
	
	CTreeCtrl* tree = (CTreeCtrl *) GetDlgItem(IDC_TREE_SATELLITES);
	
	// get the text in the parent item
	HTREEITEM hParent = pNMTreeView->itemNew.hItem;
	CString parentText = tree->GetItemText(hParent);

	// get the text in the first child item
	HTREEITEM hChild = tree->GetChildItem(hParent);
	CString childText = tree->GetItemText(hChild);

	if (childText == DEFAULT_CHILD_TEXT)	// if children data has not been obtained yet
	{
		// test to see which header was selected
		for (int findIndex = 0; findIndex < NUM_TYPES; findIndex++)
			if (parentText == m_types[findIndex])
				break;

		// pass the appropriate body type to the function to fill the tree
		switch (findIndex)
		{
		case 0:	//(Asteroid)
			GetNamesFromWeb(hParent,Asteroid,(tree->GetCheck(hParent) == 0) ? false : true);
			break;
		case 1: //(Satellite)
			GetNamesFromWeb(hParent,Satellite,(tree->GetCheck(hParent) == 0) ? false : true);
			break;
		case 2: //(Comet)
			GetNamesFromWeb(hParent,Comet,(tree->GetCheck(hParent) == 0) ? false : true);
			break;
		case 3: //(Tour)
			GetNamesFromWeb(hParent,Tour,(tree->GetCheck(hParent) == 0) ? false : true);
			break;
		}
	}



	*pResult = 0;
}


//////////////////////////////////////////////////////////////////////
// 
// Function used to parse an HTTP file for desired hyperlinks
//
// Input:	ptr to opened and connecter CHttpFile
//			CString extension to search for, e.g., ".txt"
//			CString prefix to prepend to saved file, e.g., "C_" for comets
//			CString server name
//			CString URI
//			HTREEITEM parent (root) tree item to insert children into
//			bool flag to indicate ignoring comets (used for asteroid parse)
//			bool flag to check item as it is added to the tree
//
// Output:	integer number of items successfully added to tree
//
//////////////////////////////////////////////////////////////////////
int CSatelliteDlg::ParseHTML(CHttpFile *downloadFile,
							 CString searchExt, CString prefix,
							 CString server, CString uri,
							 HTREEITEM hParent, bool noComets, bool checked)
{
	int dotIndex,	// index of first character in searchExt (-1 if not found)
		startFile,	// index of the first character of the URI
		startDesc,	// index of the first character of the hypertext
		endDesc,	// index of the last character of the hypertext
		count=0;	// number of items added to the tree
	CString oneLine,	// starting line that is read, then trimmed to remote URI
			desc;		// text of hyperlink
	CUserSettings user;
	CTreeCtrl* tree = (CTreeCtrl *) GetDlgItem(IDC_TREE_SATELLITES);

	while (downloadFile->ReadString(oneLine))
	{
		// if line contains ".txt", record the position of it
			dotIndex = oneLine.Find(searchExt);
		if (dotIndex != -1)
		{
			// starting from that position, search forward for </
			endDesc = oneLine.Find("</",dotIndex);
			if (endDesc != -1)	// assume description fits on one line
			{
				// truncate the string at this location
				desc = oneLine.Left(endDesc);
				// with the truncated string, search backwards for >
				startDesc = desc.ReverseFind('>');
				// discard all characters leading up to this point
				// this is the string for the description
				desc = desc.Right(desc.GetLength() - startDesc - 1);
			}
			// if the </ tag was not found, look on the next string
			else
			{
				// search backwards for >
				startDesc = oneLine.ReverseFind('>');
				// discard all but the (partial) description
				// this is the incomplete description string
				desc = oneLine.Right(oneLine.GetLength() - startDesc - 1);
				CString tempLine;
				// read the next line
				downloadFile->ReadString(tempLine);
				// trim leading whitespace characters
				tempLine.TrimLeft();
				// add this new line to the incomplete description
				desc += " " + tempLine;
				// find the end of the description
				endDesc = desc.Find("</");
				// discard all characters leading up to this point
				desc = desc.Left(endDesc);	
			}
			// truncate the original string at .txt
			oneLine = oneLine.Left(dotIndex);
			// with the truncated string, search backwards for "
			startFile = oneLine.ReverseFind('"');
			// discard all characters leading up to this point
			oneLine = oneLine.Right(oneLine.GetLength() - startFile - 1);
			// add the .txt back in
			// this is the string for the filename

			// check if the entry is a comet
			if (noComets && (oneLine.Find("Comet") != -1))
			{
				continue;
			}

			// insert description of satellite into listbox
			HTREEITEM newChild = tree->InsertItem(desc,hParent);
			
			// create DP object for item data
			CHTTPDownloadParams* aud = new CHTTPDownloadParams;
			oneLine += searchExt;	//append file extension
			aud->serverName = server;
			aud->URIName = uri + oneLine;
			aud->saveFilePath = user.GetEphemDirectory() + DOWNLOAD_SUBDIR;
			aud->saveFileName = prefix + CHTTPDownload::GetURINoPath(oneLine);	// prefix to indicate body type
			tree->SetItemData(newChild,(DWORD) aud);
			tree->SetCheck(newChild,checked);
			count++;
		}	// end if (dotIndex != -1)
	}	// end while
	return count;
}


//////////////////////////////////////////////////////////////////////
// 
// Function called by Windows when a tree item is clicked
//		use to recursively check/uncheck all Children when Parent is checked/unchecked
//
// Input:	N/A
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
void CSatelliteDlg::OnClickTreeSatellites(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW *) pNMHDR;
	
	CTreeCtrl* tree = (CTreeCtrl *) GetDlgItem(IDC_TREE_SATELLITES);

	// test if it is the check box that was selected
	UINT uFlags;
	CPoint oPoint;
	GetCursorPos(&oPoint);
	tree->ScreenToClient(&oPoint);
	HTREEITEM hSelected = tree->HitTest(oPoint,&uFlags);
	bool checked = FALSE;

	if (tree->GetCheck(hSelected))
		checked = FALSE;
	else
		checked = TRUE;

	tree->SelectItem(hSelected);

	// if the selected item is a root & its check box was clicked
	if (tree->ItemHasChildren(hSelected) && (uFlags & TVHT_ONITEMSTATEICON))
	{
		// Check/Uncheck all children
		HTREEITEM hChild = tree->GetChildItem(hSelected);	// get the first child
		if (hChild && tree->GetItemData(hChild))
			tree->SetCheck(hChild,checked);
		while (hChild = tree->GetNextSiblingItem(hChild)) // get the rest of the children
			if (hChild && tree->GetItemData(hChild))
				tree->SetCheck(hChild,checked);
	}	

	*pResult = 0;
}



//////////////////////////////////////////////////////////////////////
// 
// Function to retrieve available items from WWW and build the tree children
//
// Input:	HTREEITEM parent (root) tree item to add children to
//			BodyType of object(s) being added
//			bool flag to check all objects as they are added
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CSatelliteDlg::GetNamesFromWeb(HTREEITEM hParent, BodyType bodyType, bool checked)
{
	BeginWaitCursor();
	CTreeCtrl* tree = (CTreeCtrl *) GetDlgItem(IDC_TREE_SATELLITES);
	CHTTPDownload session;
	int totalCount = 0;	// initialize variable which counts items added

	// set callback for status bar messages
	session.SetCallbackPointer(this);

	CString server,uri;

	// Call the specific functions to build tree based on the body type
	switch (bodyType)
	{
	case Satellite:
		totalCount = GetSatellites(&session, hParent, checked);
		break;
	case Asteroid:
		totalCount = GetAsteroids(&session, hParent, checked);
		break;
	case Comet:
		totalCount = GetComets(&session, hParent, checked);
		break;
	case Tour:
		totalCount = GetTours(&session, hParent, checked);
		break;
	}

	//Delete original (default) child if successful download
	if (totalCount)
	{
		HTREEITEM hChild = tree->GetChildItem(hParent);
		tree->DeleteItem(hChild);
	}

	// Give a friendly beep
	if (session.GetLastStatusCode() == 200)
		MessageBeep(MB_OK);
	else
		MessageBeep(MB_ICONEXCLAMATION);

	// close the CHTTPDownload object and garbage collect;
	session.Close();
	delete session;

	EndWaitCursor();
}


//////////////////////////////////////////////////////////////////////
// 
// Function to retrieve available satellites from WWW and build the tree children
//
// Input:	CHTTPDownload object pointer
//			HTREEITEM parent (root) tree item to add children to
//			bool flag to check all objects as they are added
//
// Output:	number of items added to tree
//
//////////////////////////////////////////////////////////////////////
int CSatelliteDlg::GetSatellites(CHTTPDownload *session, HTREEITEM hParent, bool checked)
{
	CHttpFile *downloadFile;
	CUserSettings user;
	int totalCount = 0;

	// get server and uri for the satellite data
	CString server = CHTTPDownload::ParseURL(user.GetSatelliteURL(),
											 CHTTPDownload::PI_SERVER);
	CString uri = CHTTPDownload::ParseURL(user.GetSatelliteURL(),
											 CHTTPDownload::PI_OBJECT);

	// get an http file for parsing the satellite names
	downloadFile = session->GetHttpFilePtr(server,uri);

	// if successful, search file for matching hyperlinks and add items to tree
	if (session->GetLastStatusCode() == 200)
		totalCount = ParseHTML(downloadFile,SATELLITE_EXT,SATELLITE_PREFIX,
						server,uri,hParent,FALSE,checked);
	// garbage collect
	session->CloseAll();
	return totalCount;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to retrieve available asteroids from WWW and build the tree children
//
// Input:	CHTTPDownload object pointer
//			HTREEITEM parent (root) tree item to add children to
//			bool flag to check all objects as they are added
//
// Output:	number of items added to tree
//
//////////////////////////////////////////////////////////////////////
int CSatelliteDlg::GetAsteroids(CHTTPDownload *session, HTREEITEM hParent, bool checked)
{
	CHttpFile *downloadFile;
	CUserSettings user;
	int totalCount = 0;

	// get server and uri for the asteroid data
	CString server = CHTTPDownload::ParseURL(user.GetAsteroidURL(),
											 CHTTPDownload::PI_SERVER);
	CString uri = CHTTPDownload::ParseURL(user.GetAsteroidURL(),
											 CHTTPDownload::PI_OBJECT);

	// get an http file for parsing the satellite names
	downloadFile = session->GetHttpFilePtr(server,uri + "Soft16.html");

	// if successful, search file for matching hyperlinks and add items to tree
	if (session->GetLastStatusCode() == 200)
		totalCount = ParseHTML(downloadFile,ASTEROID_EXT,ASTEROID_PREFIX,
						server,uri,hParent,TRUE,checked);
	session->CloseAll();
	return totalCount;
}

//////////////////////////////////////////////////////////////////////
// 
// Function to test the availability of the comet server and add the tree child
//
// Input:	CHTTPDownload object pointer
//			HTREEITEM parent (root) tree item to add children to
//			bool flag to check all objects as they are added
//
// Output:	number of items added to tree
//
//////////////////////////////////////////////////////////////////////
int CSatelliteDlg::GetComets(CHTTPDownload *session, HTREEITEM hParent, bool checked)
{
	CHttpFile *downloadFile;
	CUserSettings user;
	CTreeCtrl* tree = (CTreeCtrl *) GetDlgItem(IDC_TREE_SATELLITES);
	int totalCount = 0;

	CString server = CHTTPDownload::ParseURL(user.GetCometURL(),
											 CHTTPDownload::PI_SERVER);
	CString uri = CHTTPDownload::ParseURL(user.GetCometURL(),
											 CHTTPDownload::PI_OBJECT) +
											 "Soft16Cmt.txt";

	// test server to make sure file is available
	downloadFile = session->GetHttpFilePtr(server,uri);

	// if successful, add the only comet URI to the tree
	if (session->GetLastStatusCode() == 200)
	{
		// insert description of satellite into listbox
		HTREEITEM newChild = tree->InsertItem("Observable Comets",hParent);
		tree->SetCheck(newChild,checked);

		// create DP object for item data
		CHTTPDownloadParams* aud = new CHTTPDownloadParams;
		aud->serverName = server;
		aud->URIName = uri;
		aud->saveFilePath = user.GetEphemDirectory() + DOWNLOAD_SUBDIR;
		aud->saveFileName = COMET_PREFIX;	// prefix to indicate body type
		aud->saveFileName += "Soft16Cmt.txt";	
		// add the item data to the tree item
		tree->SetItemData(newChild,(DWORD) aud);	
		totalCount++;
	}

	// garbage collect
	session->CloseAll();
	return totalCount;

}

//////////////////////////////////////////////////////////////////////
// 
// Function to retrieve available tours from WWW and build the tree children
//
// Input:	CHTTPDownload object pointer
//			HTREEITEM parent (root) tree item to add children to
//			bool flag to check all objects as they are added
//
// Output:	number of items added to tree
//
//////////////////////////////////////////////////////////////////////
int CSatelliteDlg::GetTours(CHTTPDownload *session, HTREEITEM hParent, bool checked)
{
	CHttpFile *downloadFile;
	CUserSettings user;
	int totalCount = 0;

	CString server = CHTTPDownload::ParseURL(user.GetTourURL(),
											 CHTTPDownload::PI_SERVER);
	// specify the "root directory" URI
	CString uri = CHTTPDownload::ParseURL(user.GetTourURL(),
											 CHTTPDownload::PI_OBJECT);

	const int numDir = 3;
	// define 4 subdirectories to look through
	CString directories[numDir] = {"Educational/","Catalogs/Messier/","DeepSky/"};

	for (int i = 0; i < numDir; i++)
	{
		// add subdirectory to root directory to complete URI
		CString fullUri = uri + directories[i];

		// get an http file for parsing the satellite names
		downloadFile = session->GetHttpFilePtr(server,fullUri);

		// if successful, search file for matching hyperlinks and add items to tree
		if (session->GetLastStatusCode() == 200)
			totalCount += ParseHTML(downloadFile,TOUR_EXT,TOUR_PREFIX,
							server,fullUri,hParent,FALSE,checked);

		// garbage collect
		session->CloseAll();
	}

	return totalCount;

}

// function to enable selection by highlighting the item
// (to support Windows 95 users who do not have the checkbox)
//DEL void CSatelliteDlg::OnSelChangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
//DEL {
//DEL 	static bool firstTime = true;
//DEL 
//DEL 	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
//DEL 	
//DEL 	if (firstTime)
//DEL 	{
//DEL 		firstTime = false;
//DEL 		return;
//DEL 	}
//DEL 
//DEL 	CTreeCtrl* tree = (CTreeCtrl *) GetDlgItem(IDC_TREE_SATELLITES);
//DEL 
//DEL 	HTREEITEM hSelected = tree->GetSelectedItem();
//DEL 
//DEL 	bool checked;
//DEL 
//DEL 	if (tree->GetCheck(hSelected))
//DEL 	{
//DEL 		tree->SetCheck(hSelected,false);
//DEL 		checked = false;
//DEL 	}
//DEL 	else
//DEL 	{
//DEL 		tree->SetCheck(hSelected,true);
//DEL 		checked = true;
//DEL 	}
//DEL 
//DEL 	// if the selected item is a root
//DEL 	if (tree->ItemHasChildren(hSelected))
//DEL 	{
//DEL 		// Check/Uncheck all children
//DEL 		HTREEITEM hChild = tree->GetChildItem(hSelected);	// get the first child
//DEL 		if (hChild && tree->GetItemData(hChild))
//DEL 			tree->SetCheck(hChild,checked);
//DEL 		while (hChild = tree->GetNextSiblingItem(hChild)) // get the rest of the children
//DEL 			if (hChild && tree->GetItemData(hChild))
//DEL 				tree->SetCheck(hChild,checked);
//DEL 	}	
//DEL 
//DEL 	*pResult = 0;
//DEL }

//////////////////////////////////////////////////////////////////////
// 
// Function called by Windows when dialog is closed
//		use this to grab the selected items from the tree control
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
void CSatelliteDlg::OnOK() 
{
	CTreeCtrl* tree = (CTreeCtrl *) GetDlgItem(IDC_TREE_SATELLITES);
	const int maxNum = 50;
	int* rgIndex = NULL;
	int array[maxNum];
	rgIndex = array;

	for (int i = 0; i < NUM_TYPES; i++)	// step through the roots of the tree ctrl
	{
		HTREEITEM hChild = tree->GetChildItem(m_hRoots[i]);	// get the first child

		// first check to see if any roots have been checked without expanding first
		// if so, get the data to fill the tree for that body type
		if (tree->GetCheck(m_hRoots[i]) && !tree->GetItemData(hChild))
		{
			m_clearBodyType[i] = TRUE;
			switch(i)
			{
			case 0:
				GetNamesFromWeb(m_hRoots[i],Asteroid,TRUE);
				break;
			case 1:
				GetNamesFromWeb(m_hRoots[i],Satellite,TRUE);
				break;
			case 2:
				GetNamesFromWeb(m_hRoots[i],Comet,TRUE);
				break;
			case 3:
				GetNamesFromWeb(m_hRoots[i],Tour,TRUE);
				break;
			}
			hChild = tree->GetChildItem(m_hRoots[i]);	// get the new first child
		}

		// delete objects created for the tree item data
		// but first copy selected ones to m_selectedFiles
		if (hChild && tree->GetItemData(hChild))	// if child exists and contains data
		{
			if (tree->GetCheck(hChild))					// if it is also checked
			{
				// copy the data
				m_selectedFiles.Add(*(CHTTPDownloadParams *) tree->GetItemData(hChild));
				m_clearBodyType[i] = TRUE;
			}
			// either way, delete the data
			delete (CHTTPDownloadParams *) tree->GetItemData(hChild);
		}
		// step through the rest of the children
		while (hChild = tree->GetNextSiblingItem(hChild))
		{
			if (hChild && tree->GetItemData(hChild)) // if child exists and contains data
			{
				if (tree->GetCheck(hChild))	// if it is also checked
				{
					// copy the data
					m_selectedFiles.Add(*(CHTTPDownloadParams *) tree->GetItemData(hChild));
					m_clearBodyType[i] = TRUE;
				}
				// either way, delete the data
				delete (CHTTPDownloadParams *) tree->GetItemData(hChild);
			}
		}
	}
	
	CDialog::OnOK();
}


