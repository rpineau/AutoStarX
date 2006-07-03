#if !defined(AFX_DEFINECATALOG_H__9602F5AA_4425_4038_9511_D258A6CDB34C__INCLUDED_)
#define AFX_DEFINECATALOG_H__9602F5AA_4425_4038_9511_D258A6CDB34C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DefineCatalog.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CDefineCatalog dialog

class CDefineCatalog : public CDialog
{
// Construction
public:
	CStringArray m_fieldNames;
	CString m_catName;
	CDefineCatalog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDefineCatalog)
	enum { IDD = IDD_DEFINECATALOG_DLG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefineCatalog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDefineCatalog)
	afx_msg void OnChangeEditField();
	virtual BOOL OnInitDialog();
	afx_msg void OnFieldAdd();
	afx_msg void OnFieldRemove();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFINECATALOG_H__9602F5AA_4425_4038_9511_D258A6CDB34C__INCLUDED_)
