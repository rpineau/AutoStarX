// AU2.h : main header file for the AU2 application
//

#if !defined(AFX_AU2_H__3AF97548_2305_11D5_85B7_0060081FFE37__INCLUDED_)
#define AFX_AU2_H__3AF97548_2305_11D5_85B7_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "AUTOSTAR\UserSettings.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CAU2App:
// See AU2.cpp for the implementation of this class
//

class CAU2App : public CWinApp
{
public:
	CAU2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAU2App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CAU2App)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CUserSettings m_userSettings;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AU2_H__3AF97548_2305_11D5_85B7_0060081FFE37__INCLUDED_)
