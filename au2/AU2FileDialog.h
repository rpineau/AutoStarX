#if !defined(AFX_AU2FILEDIALOG_H__E42FA513_85C5_11D5_8605_0060081FFE37__INCLUDED_)
#define AFX_AU2FILEDIALOG_H__E42FA513_85C5_11D5_8605_0060081FFE37__INCLUDED_

#include "autostar\BodyData.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxcmn.h>//
#include <afxdlgs.h>//
#include <commdlg.h>//
#include <cderr.h>//

// AU2FileDialog.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CAU2FileDialog dialog

class CAU2FileDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CAU2FileDialog)

public:
	enum extType {TYPE_TEXT = 1, TYPE_TLE, TYPE_ROM, TYPE_MTF, TYPE_ALL};
	static CString GetExtText(extType ext);
	CString m_selectedExt;
	CString m_selectedType;
	bool m_update;
	bool m_delete;
	BodyType GetBodyType();
	void SetFontSize(int size);
	virtual  ~CAU2FileDialog();
	CAU2FileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
protected:
	//{{AFX_MSG(CAU2FileDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	const CString ASTEROID, SATELLITE, COMET, LANDMARK, TOUR, USEROBJ;
	BOOL OnFileNameOK();
	void OnTypeChange();
	CButton* m_buttonDelete;
	CButton* m_buttonUpdate;
	void OnDestroy();
	CStatic* m_static;
	CFont* m_pFont;
	CComboBox* m_comboType;
};

LRESULT CALLBACK Hooker(int nCode, WPARAM wParam,  LPARAM lParam);


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AU2FILEDIALOG_H__E42FA513_85C5_11D5_8605_0060081FFE37__INCLUDED_)
