#if !defined(AFX_USERSITEDLG_H__1817426B_248A_423D_8E97_AF8C02F85884__INCLUDED_)
#define AFX_USERSITEDLG_H__1817426B_248A_423D_8E97_AF8C02F85884__INCLUDED_

#include "AUTOSTAR\Autostar.h"	// Added by ClassView
#include "AUTOSTAR\UserInfo.h"	// Added by ClassView
#include "AUTOSTAR\BodyDataCollection.h"	// Added by ClassView
#include "AUTOSTAR\PECTable.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserSiteDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserSiteDlg dialog

class CUserSiteDlg : public CDialog
{
// Construction
public:
	void SaveData(bool update = false);
	void EditSite();
	bool RetrieveData(bool update = false);
	int m_fontSize;
	CAutostar *m_pAutostar;
	CUserSiteDlg(CAutostar *autostar, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUserSiteDlg)
	enum { IDD = IDD_USERSITE_DLG };
	//}}AFX_DATA
	CString	*m_city;
	CString	*m_firstName;
	CString	*m_lastName;
	CString	*m_postCode;
	CString	*m_serialNum;
	CString	*m_state;
	CString	*m_street1;
	CString	*m_street2;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserSiteDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUserSiteDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSiteEdit();
	afx_msg void OnDblclkList();
	afx_msg void OnButtonSend();
	afx_msg void OnButtonSiteAdd();
	afx_msg void OnButtonSiteDelete();
	afx_msg void OnUserSave();
	afx_msg void OnUserLoad();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CPECTable *m_DECPECTable;
	CPECTable *m_RAPECTable;
	bool m_firstTime;
	void DeleteSite();
	void AddSite();
	void RefreshList(bool reSelect = true);
	CBodyDataCollection m_dataList;
	CUserInfo *m_info;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERSITEDLG_H__1817426B_248A_423D_8E97_AF8C02F85884__INCLUDED_)
