// AU2ListView.cpp : implementation file
//

#include "stdafx.h"
#include "AU2.h"
#include "AU2ListView.h"
#include "AU2Doc.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAU2ListView

IMPLEMENT_DYNCREATE(CAU2ListView, CListView)

CAU2ListView::CAU2ListView()
{

}

CAU2ListView::~CAU2ListView()
{

}


BEGIN_MESSAGE_MAP(CAU2ListView, CListView)
	//{{AFX_MSG_MAP(CAU2ListView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAU2ListView drawing

void CAU2ListView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();

}

/////////////////////////////////////////////////////////////////////////////
// CAU2ListView diagnostics

#ifdef _DEBUG
void CAU2ListView::AssertValid() const
{
	CListView::AssertValid();
}

void CAU2ListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG


void CAU2ListView::InitializeList(CBodyDataCollection *pData, BodyType bodyType)
{
	//Setup Column Headings and Load Object Data into List View
	
	POSITION pos = pData->GetHeadPosition(bodyType); 

	CBodyData *data;// = pData->GetAt(pos);
	
	for (int columnCount = 0; columnCount < pData->GetNumFields(bodyType); columnCount++)
		GetListCtrl().InsertColumn(columnCount,pData->GetFieldLabel(columnCount,bodyType),
		                           LVCFMT_LEFT,100);
	
	//Add each item to the list

	int nCount = 0;
	
	while(pos)
	{
		data = pData->GetNext(pos,bodyType);
		//if (pos)
			AddItem(nCount++,data);
	}

}

// add an item to the list view
BOOL CAU2ListView::AddItem(int nIndex, CBodyData *pData)
{
	//
	// Allocate a new ITEMINFO structure and initialize it with information
	// about the item.
	//
    ITEMINFO* pItem;
    try {
        pItem = new ITEMINFO;
    }
    catch (CMemoryException* e) {
        e->Delete ();
        return FALSE;
    }

	for (int i = 0; i < pData->GetNumFields(); i++)
	{
		pItem->strData[i] = pData->GetFieldData(i);
	}

	// last item in structure is a pointer to the body data object
	pItem->data = pData;

	//
	// Add the item to the list view.
	//
    LV_ITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM; 
    lvi.iItem = nIndex; 
    lvi.iSubItem = 0; 
    lvi.iImage = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK;
    lvi.lParam = (LPARAM) pItem;

    if (GetListCtrl ().InsertItem (&lvi) == -1)
        return FALSE;

    return TRUE;
}		

// delete all ITEMINFO objects of current list
void CAU2ListView::FreeMemory()
{
 	int nCount = GetListCtrl().GetItemCount();
 	if (nCount) 
 	{
 		for (int i=0; i < nCount; i++)
 			delete (ITEMINFO*) GetListCtrl().GetItemData(i);
 	}
}


void CAU2ListView::Refresh(CBodyDataCollection *pCollection, BodyType bodyType, int topIndex)
 {
 	// delete any existing ITEMINFO structures
	FreeMemory();

 	POSITION pos = pCollection->GetHeadPosition(bodyType);
 	CBodyData *pData;
 
 	GetListCtrl().DeleteAllItems();
 
 	//Regenerate the list
 
 	int nCount = 0;
 
 	while (pos)
 	{
 		pData = pCollection->GetNext(pos,bodyType);
 		AddItem(nCount++,pData);
 	}
 
 	//if list was not at beginning, scroll to its original position
 	if (topIndex)
 	{
 		CRect rect;
 		GetListCtrl().GetItemRect(topIndex,&rect,LVIR_BOUNDS);
 		CSize size;
 		size.cy = rect.Height() * topIndex;
 		GetListCtrl().Scroll(size);
 
 	}
 
 }



