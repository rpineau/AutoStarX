#if !defined(AFX_AU2REPLACEDLG_H__9469E815_7F7C_11D5_85FF_0060081FFE37__INCLUDED_)
#define AFX_AU2REPLACEDLG_H__9469E815_7F7C_11D5_85FF_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AU2ReplaceDlg.h : header file
//

typedef enum {YES,YESTOALL,NO,CANCEL} outcome;
/////////////////////////////////////////////////////////////////////////////
// CAU2ReplaceDlg dialog

class CAU2ReplaceDlg : public CDialog
{
// Construction
public:
	outcome m_outcome;
	bool m_multiSelect;
	CAU2ReplaceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAU2ReplaceDlg)
	enum { IDD = IDD_REPLACE_DLG };
	CString	m_oldObjectText;
	CString	m_newObjectText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAU2ReplaceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAU2ReplaceDlg)
	afx_msg void OnReplaceNo();
	afx_msg void OnReplaceYes();
	afx_msg void OnReplaceYesToAll();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AU2REPLACEDLG_H__9469E815_7F7C_11D5_85FF_0060081FFE37__INCLUDED_)
