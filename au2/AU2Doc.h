// AU2Doc.h : interface of the CAU2Doc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_AU2DOC_H__3AF9754E_2305_11D5_85B7_0060081FFE37__INCLUDED_)
#define AFX_AU2DOC_H__3AF9754E_2305_11D5_85B7_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Autostar\BodyDataCollection.h"
#include "AUTOSTAR\BodyData.h"	// Added by ClassView
#include "Autostar\Autostar.h"
#include "Autostar\BodyDataMaker.h"
#include "Autostar\BodyDataFactory.h"
#include "autostar\autostarstat.h"

typedef enum {LIBRARY, HANDBOX, NONE} SourceList;

const CString DEFAULT_CAT_NAME = "Undefined";

class CAU2Doc : public CDocument
{
protected: // create from serialization only
	CAU2Doc();
	DECLARE_DYNCREATE(CAU2Doc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAU2Doc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	BodyType m_customDisplay;
	void ResetBodyTypeLabels(SourceList source, CBodyDataCollection *data = NULL, BodyType bodyType = All);
	BodyType GetCustomBodyType(SourceList source, CString label);
	bool m_handboxModified;
	CStringArray m_lBodyTypeLabel,m_hBodyTypeLabel;
	int m_hbx_mem;			// available handbox memory
	CString m_statusText;	// generic status text
	void SendAutostarData();
	CBodyDataMaker m_factory;
	bool m_hbxConnected;	// indicate if handbox has been communicated with
	void LoadAutostarData(bool spawnThread = true);
	// WARNING!  Do not change this member variable directly, use ChangeListBodyType()
	BodyType m_bodyType;	// indicate the display body type
	CBodyDataCollection m_libraryCollection;
	CBodyDataCollection* m_handboxCollection;
	CAutostar m_autostar;
	virtual ~CAU2Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAU2Doc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AU2DOC_H__3AF9754E_2305_11D5_85B7_0060081FFE37__INCLUDED_)
