#if !defined(AFX_SELECTDLG_H__F9B0E064_84FA_11D5_8604_0060081FFE37__INCLUDED_)
#define AFX_SELECTDLG_H__F9B0E064_84FA_11D5_8604_0060081FFE37__INCLUDED_

#include "AUTOSTAR\BodyData.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectDlg dialog

class CSelectDlg : public CDialog
{
// Construction
public:
	BodyType m_selectBodyType;
	bool SetBodyType(BodyType type);
	CString m_title;
	BodyType GetBodyType();
	CString m_selectedType;
	CStringArray m_comboText;
	CSelectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectDlg)
	enum { IDD = IDD_SELECT_DLG };
	CComboBox	m_combo;
	CString	m_prompt;
	BOOL	m_delete;
	BOOL	m_update;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnOK();
	afx_msg void OnCheckUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTDLG_H__F9B0E064_84FA_11D5_8604_0060081FFE37__INCLUDED_)
