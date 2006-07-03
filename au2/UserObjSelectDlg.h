#if !defined(AFX_USEROBJSELECTDLG_H__DDC5DD8F_7682_46A9_AAA5_85E817F819F1__INCLUDED_)
#define AFX_USEROBJSELECTDLG_H__DDC5DD8F_7682_46A9_AAA5_85E817F819F1__INCLUDED_

#include "AU2View.h"	// Added by ClassView
#include "AUTOSTAR\BodyData.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserObjSelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserObjSelectDlg dialog

class CUserObjSelectDlg : public CDialog
{
// Construction
public:
	CStringArray* m_pButtonLabels;
	CBodyDataCollection * m_pCollection;
	BodyType GetBodyType();
	BodyType m_bodyTypeSelected;
	CString GetSelectedText();
	FontSize m_fontSize;
	CPoint m_origin;
	CUserObjSelectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUserObjSelectDlg)
	enum { IDD = IDD_POPUP_USEROBJ };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserObjSelectDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUserObjSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSelect();
	//}}AFX_MSG
	afx_msg void OnUserObjClicked(UINT nID);
	DECLARE_MESSAGE_MAP()
private:
	BodyType ConvertBodyType(CButton* buttonClicked);
	CString m_selectedText;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USEROBJSELECTDLG_H__DDC5DD8F_7682_46A9_AAA5_85E817F819F1__INCLUDED_)
