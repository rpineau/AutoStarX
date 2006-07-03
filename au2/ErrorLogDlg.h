#if !defined(AFX_ERRORLOGDLG_H__D0924F86_67F8_460A_ACDD_35A98A73F78C__INCLUDED_)
#define AFX_ERRORLOGDLG_H__D0924F86_67F8_460A_ACDD_35A98A73F78C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ErrorLogDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CErrorLogDlg dialog

class CErrorLogDlg : public CDialog
{
// Construction
public:
	CString m_title;
	CString m_text;
	CErrorLogDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CErrorLogDlg)
	enum { IDD = IDD_ERRORLOG_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CErrorLogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CErrorLogDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ERRORLOGDLG_H__D0924F86_67F8_460A_ACDD_35A98A73F78C__INCLUDED_)
