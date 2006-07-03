#if !defined(AFX_SATELLITEDLG_H__5C78CC43_8822_11D5_8607_0060081FFE37__INCLUDED_)
#define AFX_SATELLITEDLG_H__5C78CC43_8822_11D5_8607_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SatelliteDlg.h : header file
//
#include <afxinet.h>

#include "AU2.h"
#include "AU2Doc.h"
#include "AU2View.h"
#include "HTTPDownload.h"

#define STATUS_HEIGHT 30
#define SATELLITE_EXT ".txt"
#define SATELLITE_PREFIX "S_"
#define ASTEROID_EXT ".txt"
#define ASTEROID_PREFIX "A_"
#define COMET_EXT ".txt"
#define COMET_PREFIX "C_"
#define TOUR_EXT ".mtf"
#define TOUR_PREFIX "T_"
#define DEFAULT_CHILD_TEXT "data not available..."
#define NUM_TYPES 4

#define DOWNLOAD_SUBDIR "downloads\\"

/////////////////////////////////////////////////////////////////////////////
// CSatelliteDlg dialog

class CSatelliteDlg : public CDialog, CHTTPDownloadStat
{
// Construction
public:
	bool m_clearBodyType[4];
	CStringArray m_satNames;
	CString m_satelliteURL;
	CSatelliteDlg(CWnd* pParent = NULL);   // standard constructor
	CArray<CHTTPDownloadParams, CHTTPDownloadParams&> m_selectedFiles;

// Dialog Data
	//{{AFX_DATA(CSatelliteDlg)
	enum { IDD = IDD_SATELLITE_DLG };
	BOOL	m_delete;
	BOOL	m_update;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSatelliteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSatelliteDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemExpandingTreeSatellites(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickTreeSatellites(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int GetTours(CHTTPDownload* session, HTREEITEM hParent, bool checked = FALSE);
	int GetComets(CHTTPDownload* session, HTREEITEM hParent, bool checked = FALSE);
	int GetAsteroids(CHTTPDownload* session, HTREEITEM hParent, bool checked = FALSE);
	int GetSatellites(CHTTPDownload* session, HTREEITEM hParent, bool checked = FALSE);
	VOID GetNamesFromWeb(HTREEITEM hParent, BodyType bodyType, bool checked = FALSE);
	int ParseHTML(CHttpFile* downloadFile,CString searchExt, CString prefix,
		CString server, CString uri, HTREEITEM hParent, bool noComets = FALSE,
		bool checked = FALSE);
	HTREEITEM m_hRoots[NUM_TYPES];
	CString m_statusString;
	CStatusBarCtrl m_wndStatusBar;
	VOID UpdateStatus(CString statString, int percent);
	CString m_types[NUM_TYPES];

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SATELLITEDLG_H__5C78CC43_8822_11D5_8607_0060081FFE37__INCLUDED_)
