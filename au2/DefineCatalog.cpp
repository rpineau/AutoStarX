// DefineCatalog.cpp : implementation file
//

#include "stdafx.h"
#include "au2.h"
#include "DefineCatalog.h"
#include "autostar\UserObjEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDefineCatalog dialog


CDefineCatalog::CDefineCatalog(CWnd* pParent /*=NULL*/)
	: CDialog(CDefineCatalog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDefineCatalog)
	//}}AFX_DATA_INIT
}


void CDefineCatalog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDefineCatalog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDefineCatalog, CDialog)
	//{{AFX_MSG_MAP(CDefineCatalog)
	ON_EN_CHANGE(IDC_EDIT_FIELD, OnChangeEditField)
	ON_BN_CLICKED(IDC_FIELD_ADD, OnFieldAdd)
	ON_BN_CLICKED(IDC_FIELD_REMOVE, OnFieldRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefineCatalog message handlers


BOOL CDefineCatalog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// limit the number of characters in the field name
	CEdit* pEdit = (CEdit *) GetDlgItem(IDC_EDIT_FIELD);
	pEdit->SetLimitText(14);

	// set the default catalog name field
	CEdit *nameEdit = (CEdit *) GetDlgItem(IDC_EDIT_CATNAME);
	nameEdit->SetWindowText(m_catName);
	nameEdit->SetLimitText(16);

	// set the default text for the list box

	CListBox* fieldList = (CListBox *) GetDlgItem(IDC_FIELD_LIST);
	fieldList->AddString("Name");
	fieldList->AddString("Right Asc.");
	fieldList->AddString("Declination");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// count the number of characters entered at each keystroke
void CDefineCatalog::OnChangeEditField() 
{
	// Count the # of character in the field name
	CEdit *pEdit = (CEdit *) GetDlgItem(IDC_EDIT_FIELD);
	CString editText;
	pEdit->GetWindowText(editText);


	// do not allow ,'s or :'s or #'s
	const int numBad =			3;
	char bad[numBad] =			{':', ',', '#'};
	CString badDesc[numBad] =	{"Colons (:)", "Commas (,)", "Number Signs (#)"};
	CString output = "";

	for (int i = 0; i < numBad; i++)
	{
		if (editText.Find(bad[i]) != -1)
		{
			output += badDesc[i];
			output += " are not allowed in the Field Name";
			MessageBox(output ,"Error",MB_ICONEXCLAMATION);
			output += "";
			editText.Remove(bad[i]);
			pEdit->SetWindowText(editText);
			pEdit->SetSel(0,-1,true);
		}
	}

	int numChar = editText.GetLength();

	// display the available characters for the field data
	CStatic* pStat = (CStatic *) GetDlgItem(IDC_STATIC_FIELD);
	char buffer[2];
	itoa(15 - numChar, buffer, 10);
	pStat->SetWindowText(buffer);


}

void CDefineCatalog::OnFieldAdd() 
{
	CEdit *input = (CEdit *) GetDlgItem(IDC_EDIT_FIELD);
	CListBox* fieldList = (CListBox *) GetDlgItem(IDC_FIELD_LIST);

	// do not add new fields if the max is already defined
	if (fieldList->GetCount() >= MAX_USEROBJEX_FIELDS)
	{
		MessageBox("Maximum number of fields reached","Error");
		return;
	}

	// get the text from the edit box
	CString inputString;
	input->GetWindowText(inputString);
	if (inputString != "" && fieldList->FindString(-1,inputString) == LB_ERR)
	{
		// add it to the list view
		fieldList->AddString(inputString);

	}

}

void CDefineCatalog::OnFieldRemove() 
{
	CListBox* fieldList = (CListBox *) GetDlgItem(IDC_FIELD_LIST);

	LPINT selIndices;

	selIndices = (LPINT) calloc(10,sizeof(LPINT));

	fieldList->GetSelItems(10,selIndices);

	for (int i=0; i < fieldList->GetSelCount(); i++)
	{
		if (selIndices[i] >= REQD_USEROBJEX_FIELDS)	// do not allow mandatory fields to be deleted
			fieldList->DeleteString(selIndices[i]);
	}

	free(selIndices);

	fieldList->RedrawWindow();

}


void CDefineCatalog::OnOK() 
{
	CEdit* edit = (CEdit *) GetDlgItem(IDC_EDIT_CATNAME);

	CString editText;
	edit->GetWindowText(editText);

	editText.TrimLeft();
	editText.TrimRight();

	if (editText != "")
		m_catName = editText.Left(16);
	else
	{
		MessageBox("Must enter a catalog name", "Error", MB_ICONEXCLAMATION);
		return;
	}

	// populate the field names array
	CListBox* fieldList = (CListBox *) GetDlgItem(IDC_FIELD_LIST);
	CString rString;
	for (int i = 3; i < fieldList->GetCount(); i++)
	{
		fieldList->GetText(i, rString);
		m_fieldNames.Add(rString);
	}
	
	CDialog::OnOK();
}


