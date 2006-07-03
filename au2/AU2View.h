// AU2View.h : interface of the CAU2View class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_AU2VIEW_H__3AF97550_2305_11D5_85B7_0060081FFE37__INCLUDED_)
#define AFX_AU2VIEW_H__3AF97550_2305_11D5_85B7_0060081FFE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "AU2ListView.h"
#include "autostar\autostarstat.h"
#include "colorbtn.h"	// Added by ClassView
#include "autostar\UserSettings.h"
#include "Label.h"	// Added by ClassView
#include "AUTOSTAR\BodyData.h"	// Added by ClassView
#include "Image.h"	// Added by ClassView
#include "HTTPDownload.h"
#include "HTTPDownloadStat.h"
#include "SatelliteDlg.h"

#include <afxtempl.h>


typedef enum {SMALL, LARGE} FontSize;
typedef enum {CHECK_VERSION, RETRIEVE_FILE} DownloadTask;
typedef enum {CAT_SAME, CAT_DIFFERENT, CAT_EMPTY, CAT_ERROR} eCatCompare;

struct UpgradeInfo{
		CString fileName;
		CString root;
		CString ext;
};

const COLORREF CLOUDBLUE = RGB(128, 184, 223);
const COLORREF WHITE = RGB(255, 255, 255);
const COLORREF BLACK = RGB(1, 1, 1);
const COLORREF CHARCOAL = RGB(40,40,40);
const COLORREF DKGRAY = RGB(128, 128, 128);
const COLORREF LTGRAY = RGB(192, 192, 192);
const COLORREF YELLOW = RGB(255, 255, 0);
const COLORREF LTYELLOW = RGB(255,255,180);
const COLORREF DKYELLOW = RGB(128, 128, 0);
const COLORREF RED = RGB(255, 0, 0);
const COLORREF DKRED = RGB(128, 0, 0);
const COLORREF PINK	= RGB(255,200,200);
const COLORREF BLUE = RGB(0, 0, 255);
const COLORREF LTBLUE = RGB(159,192,224);
const COLORREF DKBLUE = RGB(21, 20, 155);
const COLORREF CYAN = RGB(0, 255, 255);
const COLORREF DKCYAN = RGB(0, 128, 128);
const COLORREF GREEN = RGB(0, 255, 0);
const COLORREF DKGREEN = RGB(0, 128, 0);
const COLORREF MAGENTA = RGB(255, 0, 255);
const COLORREF DKMAGENTA = RGB(128, 0, 128);
const COLORREF MEADEBLUE = RGB(21,20,155);

const int RADIO_SIZE = 13;	// max number of chars for radio buttons


class CAU2View : public CFormView, public CAutostarStat, CHTTPDownloadStat
{
protected: // create from serialization only
	CAU2View();
	DECLARE_DYNCREATE(CAU2View)

public:
	//{{AFX_DATA(CAU2View)
	enum { IDD = IDD_AU2_FORM };
	CLabel	m_labelSelect;
	CLabel	m_labelRefresh;
	CLabel	m_labelAll;
	CLabel	m_labelHbx2;
	//}}AFX_DATA
	enum UpgradeTask{UT_DOWNLOAD, UT_UPGRADE, UT_DOWNLOAD_UPGRADE, UT_NOTHING};


// Attributes
public:
	CAU2Doc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAU2View)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL

// Implementation
public:
	bool m_realTimeMode;
	bool m_eraseBanks;
	ASType m_ASType;
	CArray<UpgradeInfo, UpgradeInfo&> m_upgradeInfo;
	int m_importCount;
	CArray<CHTTPDownloadParams, CHTTPDownloadParams&> m_downloadParams;
	CHTTPDownload m_session;
	VOID UpdateStatus(CString statString, int percent = 0);
	CString m_upgradeFile;
	bool m_downLoadingBodyData;
	void UpdateMemInfo();
	CString m_backupFile;
	void EnableButtons(int upgrade = 2, int transfer = 2, int autostar = 2, int realTime = 2);
	void DownloadComplete(bool doneWithThread = FALSE);
	UpgradeTask m_upgradeTask;
	CString m_doingProcess;
	void DoingProcess (CString stat);
	void DeleteListView(SourceList list);
	FontSize m_SystemFontSize;
	void SendComplete(eAutostarStat stat, bool noPopup = false);
	UINT IDD1;
	UINT GetDialogID();
	void EditObject(CAU2ListView* list);
	static int CALLBACK CompareFuncDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareFuncAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	virtual ~CAU2View();
	void RetrieveComplete(eAutostarStat stat, bool noPopup = false);
	void PercentComplete(int val);
	void ChangeListBodyType(BodyType newBodyType);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	void NewObject(CAU2ListView* list);
	void SynchRadioButtons();
	//{{AFX_MSG(CAU2View)
	afx_msg void OnPaint();
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnExit();
	afx_msg void OnAdvanced();
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRadioClickedLibrary();
	afx_msg void OnDestroy();
	afx_msg void OnRadioClickedHandbox();
	afx_msg void OnButtonSend();
	afx_msg void OnButtonToHbx();
	afx_msg void OnButtonToLib();
	afx_msg void OnButtonRetrieve();
	afx_msg void OnButtonConnect();
	afx_msg void OnFileImport();
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnUpgrade();
	afx_msg void OnFileSaveHbx();
	afx_msg void OnFileOpenHbx();
	afx_msg void OnSetFocusHandboxList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileRestoreHbx();
	afx_msg void OnButtonToHbxRefresh();
	afx_msg void OnButtonToHbxAll();
	afx_msg void OnButtonToLibAll();
	afx_msg void OnButtonToLibRefresh();
	afx_msg void OnFileDownload();
	afx_msg void OnButtonRealTime();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTools(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	eCatCompare CompareCatalogs(CBodyDataCollection *source, CBodyDataCollection *dest);
	void HideCustomObjects();
	void OnRadioClickedUserObj(UINT id);
	bool DefineCatalog(CAU2ListView *list, CBodyDataCollection *bodyData, OUT CBodyData **data);
	void OnContextMenuUserObj(CWnd* source);
	DWORD m_mainThreadID;
	void Retrieve(bool spawnThread = true);
	void ReplaceListViewItem(int nItem, CAU2ListView *list, CBodyData *data, bool moveToEnd = false);
	void TransferBodyType(BodyType type, CBodyDataCollection *dCollection, CBodyDataCollection *sCollection, bool transferCustom = false);
	void TransferObjectsAll(CBodyDataCollection *dCollection, CBodyDataCollection *sCollection);
	bool Connect(bool closeComPort = true, bool setDownload = true);
	void TestFunction();
	int UpdateRealTime(CBodyDataCollection* sourceCollection, CBodyDataCollection* destCollection, BodyType pType);
	void CopySelectedObjects(CAU2ListView *sourceList, CBodyDataCollection *sourceCollection, CBodyDataCollection *destCollection);
	void CopyObject(CBodyData* sourceData, CBodyDataCollection* destCollection, CAU2ListView *sourceList, CString key);
	eAutostarStat SendCatalogRealTime(BodyType bodyType);
	void EnableRealTimeControls(bool state = TRUE);
	CBrush m_brush;
	eAutostarStat SendOneObjectRealTime(CBodyData *body);
	eAutostarStat DeleteObjectRealTime(CString objectName);
	void EnableUpgradeButtons(bool state);
	VOID EnableTransferControls(bool state = TRUE);
	void EnableAutostarControls(bool state);
	void EnableRealTimeMode(bool state);
	VOID QuerySafeMode();
	VOID SetASType();
	VOID ActivateSafeMode(bool state = TRUE);
	bool SearchComPorts(bool closeComPort = true);
	CBodyDataCollection::importOption m_importOption;
	bool SaveFileComplete(CHTTPDownloadParams info);
	void SetBackground(CString fileName = "");
	CImage m_background;
	CString GetReplaceText(CBodyDataCollection* collection, CString key);
	void SelectAllItems(CAU2ListView *);
	CImageList* CreateDragImageEx(CListCtrl* pList, LPPOINT lpPoint);
	CImageList* m_dragImage;
	SourceList m_dragSource;
	void OnOptions(UINT nID);
	void OnOptionsBaud(UINT nID);
	void OnEditMenu(UINT nID);
	void DeleteObject(CAU2ListView* list);
	void TransferObjects(SourceList source, bool allObjects = false);
	void SortColumns(CAU2ListView* list, int item);
	CUserSettings m_userSettings;

};

#ifndef _DEBUG  // debug version in AU2View.cpp
inline CAU2Doc* CAU2View::GetDocument()
   { return (CAU2Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAU2EditDlg dialog

class CAU2EditDlg : public CDialog
{
// Construction
public:
	int m_fontSize;
	CString m_instructions;
	CBodyData *m_bodyData;
	void SetLimitsText(int fieldNum, CString hiLimit, CString  loLimit);
	int DoModal();
	CString m_editText[10];	// array of edit control text contents
	CEdit* m_edit[10];	// array of edit controls
	CString m_labelText[10];	// array of label texts
	CStatic* m_label[10];	// array of static labels controls
	CString m_limitsText[10]; // array of static limits texts
	CStatic* m_limits[10];	// array of static limits controls
	CAU2EditDlg(CWnd* pParent = NULL);   // standard constructor
	CLabel m_rangeLabel;	// specially formatted static control

// Dialog Data
	//{{AFX_DATA(CAU2EditDlg)
	enum { IDD = IDD_EDITOBJECTS };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAU2EditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAU2EditDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CUpgradeHbxDlg dialog


class CUpgradeHbxDlg : public CDialog
{
// Construction
public:
	bool m_eraseBanks;
	CString m_wwwVer;
	CString m_hdWarning;
	CHTTPDownload* m_pSession;
	CString m_upgradeFile;
	bool m_downloadFlag;
	CString m_hdText;
	CString m_wwwText;
	CString m_hbxText;
	CUpgradeHbxDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUpgradeHbxDlg)
	enum { IDD = IDD_UPGRADEHBXDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpgradeHbxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpgradeHbxDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnButtonCheckWWW();
	afx_msg void OnSelChangeUpgradeType();
	virtual void OnOK();
	afx_msg void OnUpgradeErase();
	afx_msg void OnSelChangeLocalVers();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	bool m_checkedWWW;	// indicates user has hit check WWW version at least once
	VOID BuildComboList(ASType type);


};
/////////////////////////////////////////////////////////////////////////////
// CAU2AllDlg dialog

class CAU2AllDlg : public CDialog
{
// Construction
public:
	CAU2AllDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAU2AllDlg)
	enum { IDD = IDD_ALL_DLG };
	CString	m_staticText;
	BOOL	m_selectAll;
	BOOL	m_selectAsteroids;
	BOOL	m_selectComets;
	BOOL	m_selectLandmarks;
	BOOL	m_selectTours;
	BOOL	m_selectUserObj;
	BOOL	m_selectSatellites;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAU2AllDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAU2AllDlg)
	afx_msg void OnCheckAll();
	afx_msg void OnCheckObject(UINT nID);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CUIntArray m_checkArray;
};





//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AU2VIEW_H__3AF97550_2305_11D5_85B7_0060081FFE37__INCLUDED_)
