#if !defined(AFX_COMPORTDLG_H__17A0E750_8C18_11D5_860C_0060081FFE37__INCLUDED_)
#define AFX_COMPORTDLG_H__17A0E750_8C18_11D5_860C_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ComPortDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CComPortDlg dialog

class CComPortDlg : public CDialog
{
// Construction
public:
	CComPortDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CComPortDlg)
	enum { IDD = IDD_COMPORT_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComPortDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CComPortDlg)
	afx_msg void OnComSelect();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAutoDetect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void CheckComButton();
	void UncheckAllButtons();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPORTDLG_H__17A0E750_8C18_11D5_860C_0060081FFE37__INCLUDED_)
