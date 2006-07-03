// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__3AF9754C_2305_11D5_85B7_0060081FFE37__INCLUDED_)
#define AFX_MAINFRM_H__3AF9754C_2305_11D5_85B7_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	CStatusBar  m_wndStatusBar;
	void ToggleStatusBar(int state);
	int m_winSizeState;		// window size, 0 = basic, 1 = advanced
	void SetWinSize(int state);
	static const CRect m_winSize[2];
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
    CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateFileRestore(CCmdUI* pCmdUI);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg void OnUpdateIndicator(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOptions(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOptionsBaud(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMemInfo(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__3AF9754C_2305_11D5_85B7_0060081FFE37__INCLUDED_)
