#if !defined(AFX_AU2LISTVIEW_H__18B8C7F4_14C0_11D5_85AC_0060081FFE37__INCLUDED_)
#define AFX_AU2LISTVIEW_H__18B8C7F4_14C0_11D5_85AC_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AU2ListView.h : header file
//
#include "afxcview.h"
#include "Autostar\BodyDataCollection.h"
#include "Autostar\BodyData.h"
#include "Autostar\Asteroid.h"

typedef struct tagITEMINFO {
	CString strData[10];		// ListView Structure with 10 Fields
	CBodyData* data;
} ITEMINFO;


/////////////////////////////////////////////////////////////////////////////
// CAU2ListView view

class CAU2ListView : public CListView
{
protected:
	CAU2ListView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CAU2ListView)

// Attributes
public:

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAU2ListView)
	public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
public:
	void Refresh(CBodyDataCollection *pCollection, BodyType bodyType, int topIndex = 0);
	void InitializeList(CBodyDataCollection*,BodyType bodyType);
	void FreeMemory();

protected:
	virtual ~CAU2ListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	BOOL AddItem(int nIndex, CBodyData *pData);

	//CAU2ListView* m_pList;
	//{{AFX_MSG(CAU2ListView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AU2LISTVIEW_H__18B8C7F4_14C0_11D5_85AC_0060081FFE37__INCLUDED_)
