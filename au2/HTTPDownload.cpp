// HTTPDownload.cpp: implementation of the CHTTPDownload class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HTTPDownload.h"
#include "HTTPDownloadStat.h"

#include <afxwin.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define BUFFLEN 512

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Default Constructor
CHTTPDownload::CHTTPDownload()
{
	InitializeMembers();

}


// Constructor
CHTTPDownload::CHTTPDownload(CString server, CString URI, CString fileName, CString pathName)
{
	InitializeMembers();
	SetServerName(server);
	SetURIName(URI);
	if (fileName != "")
		SetSaveFileName(fileName);
	else
		SetSaveFileName(GetURINoPath(URI));
	SetSaveFilePath(pathName);

}

// Copy Constructor
CHTTPDownload::CHTTPDownload(const CHTTPDownload &cpy)
{
	InitializeMembers();
	m_downloadParams.Copy(cpy.m_downloadParams);
	m_ptrCallback = cpy.m_ptrCallback;
}

// Destructor
CHTTPDownload::~CHTTPDownload()
{
	if (m_statString)
	{
		delete m_statString;
		m_statString = NULL;
	}

	if (m_fullSavePath)
	{
		delete m_fullSavePath;
		m_fullSavePath = NULL;
	}
	
}

//////////////////////////////////////////////////////////////////////
// Member Functions
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// 
// Function to initialize member variables (called during construction)
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::InitializeMembers()
{
	m_downloadParams.RemoveAll();
	m_ptrCallback = NULL;
	m_pConnection = NULL;
	m_lastTransferLog.Empty();
	m_lastStatusCode = 0;
	m_lastStatusText = "";
	m_fileCheck = FALSE;
	m_pHttpFile = NULL;
	m_pSaveFile = NULL;
	m_cancel = FALSE;
	m_statString = new CString("");
	m_fullSavePath = new CString("");
}


//////////////////////////////////////////////////////////////////////
// 
// Function to assign one CHTTPDownload object to another
//
// Input:	Source CHTTPDownload Object
//
// Output:	Destination CHTTPDownload Object
//
//////////////////////////////////////////////////////////////////////
const CHTTPDownload& CHTTPDownload::operator=(const CHTTPDownload &right)
{
	if (&right != this)
	{
		m_downloadParams.RemoveAll();
		m_downloadParams.Copy(right.m_downloadParams);
		m_ptrCallback = right.m_ptrCallback;	
		m_ptrCallback = NULL;
		m_pConnection = NULL;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// 
// Function to set the member array of download info structures
//
// Input:	CArray of downloadParam Structures
//
// Output:	number of elements of copied Array, 0 if unsuccessful
//
//////////////////////////////////////////////////////////////////////
int CHTTPDownload::SetDownloadParams(CArray<CHTTPDownloadParams, CHTTPDownloadParams&> &dpArray)
{
	if (!dpArray.GetSize())
		return 0;

	m_downloadParams.RemoveAll();
	m_downloadParams.Copy(dpArray);

	// check to make sure that all save file names exist, if not create them
	for (int i = 0; i <= m_downloadParams.GetUpperBound(); i++)
		if (m_downloadParams[i].saveFileName == "")
			m_downloadParams[i].saveFileName = GetURINoPath(m_downloadParams[i].URIName);

	return m_downloadParams.GetSize();
}


//////////////////////////////////////////////////////////////////////
// 
// Function to get the size of the member array of download info structures
//
// Input:	none
//
// Output:	int
//
//////////////////////////////////////////////////////////////////////
int CHTTPDownload::GetParamSize()
{
		return m_downloadParams.GetSize();
}


//////////////////////////////////////////////////////////////////////
// 
// Function to set the member variable for server name
//
// Input:	CString
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::SetServerName(CString name)
{
	if (m_downloadParams.GetSize() == 0)
	{
		CHTTPDownloadParams dp;
		dp.serverName = name;
		m_downloadParams.Add(dp);
	}
	else
		m_downloadParams[0].serverName = name;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to get the member variable for server name
//
// Input:	none
//
// Output:	CString
//
//////////////////////////////////////////////////////////////////////
CString CHTTPDownload::GetServerName(int index)
{
	if (index < GetParamSize())
		return m_downloadParams[index].serverName;
	else
		return "";
}


//////////////////////////////////////////////////////////////////////
// 
// Function to set the member variable for URI name
//
// Input:	CString
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::SetURIName(CString name)
{
	if (m_downloadParams.GetSize() == 0)
	{
		CHTTPDownloadParams dp;
		dp.URIName = name;
		m_downloadParams.Add(dp);
	}
	else
		m_downloadParams[0].URIName = name;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to get the member variable for URI name
//
// Input:	none
//
// Output:	CString
//
//////////////////////////////////////////////////////////////////////
CString CHTTPDownload::GetURIName(int index)
{
	if (index < GetParamSize())
		return m_downloadParams[index].URIName;
	else
		return "";
}



//////////////////////////////////////////////////////////////////////
// 
// Function to set the member variable for the name of the file to save
//
// Input:	CString
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::SetSaveFileName(CString name)
{
	if (m_downloadParams.GetSize() == 0)
	{
		CHTTPDownloadParams dp;
		dp.saveFileName = name;
		m_downloadParams.Add(dp);
	}
	else
		m_downloadParams[0].saveFileName = name;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to get the member variable for the name of the file to save
//
// Input:	none
//
// Output:	CString
//
//////////////////////////////////////////////////////////////////////
CString CHTTPDownload::GetSaveFileName(int index)
{
	if (index < GetParamSize())
		return m_downloadParams[index].saveFileName;
	else
		return "";
}



//////////////////////////////////////////////////////////////////////
// 
// Function to set the member variable for the path of the file to save
//
// Input:	CString
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::SetSaveFilePath(CString path)
{
	if (m_downloadParams.GetSize() == 0)
	{
		CHTTPDownloadParams dp;
		dp.saveFilePath = path;
		m_downloadParams.Add(dp);
	}
	else
		m_downloadParams[0].saveFilePath = path;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to get the member variable for the path of the file to save
//
// Input:	none
//
// Output:	CString
//
//////////////////////////////////////////////////////////////////////
CString CHTTPDownload::GetSaveFilePath(int index)
{
	if (index < GetParamSize())
		return m_downloadParams[index].saveFilePath;
	else
		return "";
}


//////////////////////////////////////////////////////////////////////
// 
// Function to get the member array of download info structures
//
// Input:	&dpArray - destination of copied array
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::GetDownloadParams(CArray<CHTTPDownloadParams, CHTTPDownloadParams&> &dpArray)
{
	dpArray.Copy(m_downloadParams);
}


//////////////////////////////////////////////////////////////////////
// 
// Function to create the HttpConnection Object
//
// Input:	index of download params array containing the server name
//
// Output:	boolean success
//
//////////////////////////////////////////////////////////////////////
BOOL CHTTPDownload::Connect(int index)
{
	try
	{

		m_pConnection = GetHttpConnection(m_downloadParams[index].serverName);

		if (m_cancel) 
			Cancel();	//check if user hit cancel

	}

	catch(CInternetException* e)
	{
		if (m_cancel)	//check if user hit cancel
		{
			e->Delete();
			Cancel();	
		}
		e->ReportError();
		e->Delete();
		if (m_ptrCallback)	// check if callback pointer has been specified
			m_ptrCallback->UpdateStatus("Could not create HTTP Connection");

			return FALSE;
	}
	

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to get a pointer to the HttpFile object
//
// Input:	index of the download params array containing the URI
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::GetHttpFilePtr(int index)
{
	// default value to force while loop to execute at least once
	DWORD dwError = ERROR_INTERNET_FORCE_RETRY;

	if (m_cancel) 
		Cancel();	//check if user hit cancel

	try
	{

		// Open request for HTTP "GET" Method (1 = "GET")
		m_pHttpFile = m_pConnection->OpenRequest(1,m_downloadParams[index].URIName);

		if (m_ptrCallback && !m_cancel)
			m_ptrCallback->UpdateStatus("Attempting to connect to server...");

		if (m_cancel)
			Cancel();	//check if user hit cancel

		// Repeat Connection Attempt until user clicks cancel, or request is successful
		// Call Error Dialog if necessary to authenticate Firewall connection
		while (dwError == ERROR_INTERNET_FORCE_RETRY)
		{
			// actually send the above request
			int result = m_pHttpFile->SendRequest();

			if (m_cancel) 
				Cancel();	//check if user hit cancel
			
			// test result to see if request was successful
			DWORD dwErrorCode = result ? ERROR_SUCCESS : GetLastError();

			// launch ErrorDlg, evaluate the error, attempt authentication, and request parameters
			dwError = m_pHttpFile->ErrorDlg(AfxGetApp()->m_pMainWnd,dwErrorCode,
										FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
										FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS |
										FLAGS_ERROR_UI_FILTER_FOR_ERRORS);

			if (m_cancel) 
				Cancel();	//check if user hit cancel
		}

		// store status code in member variable
		m_pHttpFile->QueryInfoStatusCode(m_lastStatusCode);

		// store equivalent status text in member variable
		m_pHttpFile->QueryInfo(HTTP_QUERY_STATUS_TEXT,m_lastStatusText);
	}
	catch(CInternetException* e)
	{
		if (m_cancel)	//check if user hit cancel
		{
			e->Delete();
			Cancel();	
		}
		
		e->ReportError();
		e->Delete();

		if (m_ptrCallback)
			m_ptrCallback->UpdateStatus("Error connecting to server...");
		AddTransferLogEntry("Could Not Retrieve File From: " + m_downloadParams[index].serverName,
							TT_CONNECT);
		return;
	}

	if (m_ptrCallback && !m_cancel)	// send status text if callback destination has been specified
		m_ptrCallback->UpdateStatus(GetLastStatusText());	
	
	if (m_cancel) 
		Cancel();	//check if user hit cancel

	if (m_lastStatusCode == HTTP_STATUS_OK)	// if valid file pointer was obtained
	{
		// Log the successful connection
		AddTransferLogEntry("Connected to Server: " + m_downloadParams[index].serverName,
							TT_CONNECT);
	}
	else
	{
		// Log the failure to connect
		AddTransferLogEntry("Could Not Retrieve File From: " + m_downloadParams[index].serverName,
							TT_CONNECT);
		AddTransferLogEntry(GetLastStatusText());
		Sleep(1000); // pause to allow the status message to be read

		if (m_cancel) 
			Cancel();	//check if user hit cancel
	}

	return;

}


//////////////////////////////////////////////////////////////////////
// 
// Function to get a pointer to the HttpFile object
//	For external use - don't forget to garbage collect! (use CloseAll())
//
// Input:	Cstring server
//			CString uri
//
// Output:	CHttpFile pointer
//
//////////////////////////////////////////////////////////////////////
CHttpFile* CHTTPDownload::GetHttpFilePtr(CString server, CString uri)
{
	m_downloadParams.RemoveAll();	// clear all download parameters

	SetServerName(server);			// set the server name

	SetURIName(uri);				// set the URI name

	if (!Connect(0))	//Establish HTTP Connection
		return NULL;

	GetHttpFilePtr(0);

	if (m_pHttpFile)
		return m_pHttpFile;
	else
		return NULL;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to connect to the server(s), download nBytes, and return the char buffer
//
// Input:	IN number of Bytes to download
//			OUT number of Bytes actually downloaded
//
// Output:	pointer to buffer containing downloaded data
//			DO NOT FORGET TO CALL free(buffer)!
//
//////////////////////////////////////////////////////////////////////
char* CHTTPDownload::DownloadFile(IN OUT unsigned int &nBytes)
{
	ClearTransferLog();

	char* buffer = NULL;	// buffer to receive data

	if (!m_downloadParams.GetSize())	// make sure parameters have been specified
	{
		MessageBoxEx(NULL,"Missing or bad URL specification","Error",MB_OK | MB_SETFOREGROUND, LANG_ENGLISH);
		nBytes = 0;
		return buffer;
	}

	if (!Connect(0))	//Establish HTTP Connection
		return NULL;

	// Open HTTP File, Authenticating Firewall if necessary
	GetHttpFilePtr(0);

	// Check if File was successfully opened
	if (m_lastStatusCode == HTTP_STATUS_OK)
	{	
		// if so, download file to hard disk
		buffer = (char *) malloc(nBytes);
		DWORD dwRead=0;
		CString statString;

		// read nBytes from remote file
		dwRead = m_pHttpFile->Read(buffer,nBytes-1);

		statString.Format("%d Bytes Read",dwRead);
		if (m_ptrCallback && !m_cancel)	// send status text if callback destination has been specified
			m_ptrCallback->UpdateStatus(statString);
		AddTransferLogEntry(statString + " of " + m_downloadParams[0].URIName,
							TT_DOWNLOAD);		
		nBytes = dwRead;

		if (m_cancel) 
			Cancel();	//check if user hit cancel
	}
	else	// if not, log failure
	{
		AddTransferLogEntry("Could not download URI: " + m_downloadParams[0].URIName,
							TT_DOWNLOAD);
		AddTransferLogEntry(GetLastStatusText());
	}

	// record the Status Code and Text of the last operation
	m_pHttpFile->QueryInfoStatusCode(m_lastStatusCode);
	m_pHttpFile->QueryInfo(HTTP_QUERY_STATUS_TEXT,m_lastStatusText);

	// Close the remote file
	m_pHttpFile->Close();
	delete m_pHttpFile;

	// Close the connection
	m_pConnection->Close();
	delete m_pConnection;

	return buffer;
}

//////////////////////////////////////////////////////////////////////
// 
// Function to connect to the server, download one file, and save the file
//
// Input:	CString server name (or complete URL minus the method)
//			CString URI (optional if complete URL info is provided)
//			CString fileName (optional, default = URI file name)
//			CString filePath (optional, default = working directory, may 
//							  also be included in fileName)
//
// Output:	number of files successfully downloaded
//
//////////////////////////////////////////////////////////////////////
int CHTTPDownload::DownloadFile(CString server, CString URI, CString fileName, CString filePath)
{
	m_downloadParams.RemoveAll();	// clear all download parameters

	SetServerName(server);			// set the server name

	SetURIName(URI);				// set the URI name

	if (fileName != "")				// if fileName is passed,
		SetSaveFileName(fileName);	// use it,
	else							// otherwise, get it from the server or URI string
		SetSaveFileName((URI != "") ? GetURINoPath(URI) : GetURINoPath(server));

	if (filePath != "")				// if path is passed,
		SetSaveFilePath(filePath);	// use it

	 return DownloadFile();
	
}
//////////////////////////////////////////////////////////////////////
// 
// Function to connect to the server(s), download the file(s), and save the file(s)
//
// Input:	none
//
// Output:	number of files successfully downloaded
//
//////////////////////////////////////////////////////////////////////
int CHTTPDownload::DownloadFile()
{
	int count = 0; //keeps track of how many files successfully download
	bool saveSuccess = FALSE;
	ClearTransferLog();

	if (!m_downloadParams.GetSize())	// make sure parameters have been specified
	{
		MessageBoxEx(NULL,"Missing or bad URL specification","Error",MB_OK | MB_SETFOREGROUND, LANG_ENGLISH);
		return count;
	}
	// loop through each entry of the downloadParams array
	for (int index = 0; index < m_downloadParams.GetSize(); index ++)
	{
		if (m_cancel) 
			Cancel();	//check if user hit cancel

		if (!Connect(index))	//Establish HTTP Connection
			return HTTP_STATUS_BAD_REQUEST;

		*m_fullSavePath = m_downloadParams[index].saveFilePath + m_downloadParams[index].saveFileName;

		// Open HTTP File, Authenticating Firewall if necessary
		GetHttpFilePtr(index);

		// Check if File was successfully opened
		if (m_lastStatusCode == HTTP_STATUS_OK)
		{	
			// if so, download file to hard disk
			if (saveSuccess = SaveRemoteFile(index))
			{
				count++;	//if successful, increment count, log success
				AddTransferLogEntry("Downloaded URI: " + m_downloadParams[index].URIName,
									TT_DOWNLOAD);
				AddTransferLogEntry("Saved File: " + *m_fullSavePath, TT_SAVE);
				if (m_ptrCallback)
					m_ptrCallback->SaveFileComplete(m_downloadParams[index]);
			}
			else	// if not, log failure
			{
				AddTransferLogEntry("Could not download URI: " + m_downloadParams[index].URIName,
									TT_DOWNLOAD);
				AddTransferLogEntry(GetLastStatusText());
			}

		}	

		// Close the remote file
		m_pHttpFile->Close();
		delete m_pHttpFile;
		m_pHttpFile = NULL;

		// Close the connection
		m_pConnection->Close();
		delete m_pConnection;
		m_pConnection = NULL;

	}	// end for loop

	CString finalString;
	finalString.Format("%i of %i File(s) Successfully Downloaded",count,m_downloadParams.GetSize());
	if (m_ptrCallback && !m_cancel)	// send status text if callback destination has been specified
		m_ptrCallback->UpdateStatus(finalString);
	if (m_cancel) 
		Cancel();	//check if user hit cancel
	AddTransferLogEntry(finalString,TT_FINAL);

	return count;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to save the remote HTTP file to a specified local file
//
// Input:	CHTTPFile pointer
//			CString local file name
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
bool CHTTPDownload::SaveRemoteFile(int index)
{
	char buffer[BUFFLEN];
	DWORD dwRead=0;
	int complete = 0;
	CFileStatus rStatus;
	bool saveSuccess = FALSE;

	try
	{
		// check if filename exists, prompt user for action
		CheckFileName();
		if (*m_fullSavePath == "")
		{
			AddTransferLogEntry("Save filename not specified - download terminated");
			return FALSE;
		}

		// instantiate local destination file
		m_pSaveFile = new CFile(*m_fullSavePath, 
							CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
				

		// Check the size of the remote file
		DWORD dwSize;
		m_pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH,dwSize);

		while (dwRead = m_pHttpFile->Read(buffer,BUFFLEN-1))
		{
			if (dwRead == 0)
				break;

			if (m_cancel) 
				Cancel();	//check if user hit cancel

			// write buffer data to local file
			m_pSaveFile->Write(buffer,dwRead);
			complete += dwRead;

			// calculate percent complete
			int percent = (int) (complete / (float) dwSize * 100);


			m_statString->Format("%s: %i%% Complete",GetURINoPath(m_downloadParams[index].URIName),percent);
			if (m_ptrCallback && !m_cancel) // send status text if callback destination has been specified
				m_ptrCallback->UpdateStatus(*m_statString);
			if (m_cancel) 
				Cancel();	//check if user hit cancel
		}

		// check if number of downloaded bytes is equal to fileSize
		m_pSaveFile->GetStatus(rStatus);
		if (rStatus.m_size == (long) dwSize)
			saveSuccess = TRUE;

		// Record the status code and text of the last operation
		m_pHttpFile->QueryInfoStatusCode(m_lastStatusCode);
		m_pHttpFile->QueryInfo(HTTP_QUERY_STATUS_TEXT,m_lastStatusText);

		if (m_cancel) 
			Cancel();	//check if user hit cancel

	}
	catch (CFileException* e)
	{
		e->ReportError();
		char buffer[80];
		e->GetErrorMessage(buffer,79);
		CString errorText = "Error saving file: ";
		AddTransferLogEntry(errorText + buffer);
		e->Delete();
		return saveSuccess;
	}
	catch (CInternetException* e)
	{
		if (m_cancel)	//check if user hit cancel
		{
			e->Delete();
			Cancel();	
		}
		
		e->ReportError();
		e->Delete();

		if (m_ptrCallback)
			m_ptrCallback->UpdateStatus("Error retrieving remote file...");
		AddTransferLogEntry("Could Not Retrieve File From: " + m_downloadParams[index].serverName,
							TT_CONNECT);
		return saveSuccess;
	}

	m_pSaveFile->Close();
	delete m_pSaveFile;
	m_pSaveFile = NULL;

//	if (m_ptrCallback)
//		m_ptrCallback->SaveFileComplete(m_downloadParams[index]);
	return saveSuccess;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to return HTTP status code of last query
//
// Input:	none
//
// Output:	HTTP status code
//
//////////////////////////////////////////////////////////////////////
DWORD CHTTPDownload::GetLastStatusCode()
{
	return m_lastStatusCode;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to return a formatted string describing the last status query
//
// Input:	none
//
// Output:	HTTP status text
//
//////////////////////////////////////////////////////////////////////
CString CHTTPDownload::GetLastStatusText()
{
	CString text,textFormat = "HTTP Status Code: %d, Reason: %s";

	if (m_lastStatusCode)
		text.Format(textFormat,m_lastStatusCode,m_lastStatusText);
	else
		text = "HTTP Status Info Not Available: Check Internet Connection";

	return text;
}

//////////////////////////////////////////////////////////////////////
// 
// Function to remove all entries from the download parameters array
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::ClearDownloadParams()
{
	m_downloadParams.RemoveAll();
}


//////////////////////////////////////////////////////////////////////
// 
// Function to specify the destination window for status callbacks
//
// Input:	pointer to an object that inherits from CHTTPDownloadStat
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::SetCallbackPointer(CHTTPDownloadStat *stat)
{
	m_ptrCallback = stat;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to disconnect the currently open connection (if any)
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::Cancel()
{
	CloseAll();

	ExitThread(1);
}


//////////////////////////////////////////////////////////////////////
// 
// Function to close and delete all objects that may require it
//		Only call in connection with the GetHttpFilePtr function,
//		otherwise, this task is performed within the DownloadFile context
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::CloseAll()
{
	if (m_pHttpFile)
	{
		m_pHttpFile->Close();
		delete m_pHttpFile;
	}
	
	if (m_pConnection)
	{
		m_pConnection->Close();
		delete m_pConnection;
	}

	if (m_pSaveFile)
	{
		m_pSaveFile->Close();
		delete m_pSaveFile;
	}

	if (m_statString)
	{
		delete m_statString;
		m_statString = NULL;
	}

	if (m_fullSavePath)
	{
		delete m_fullSavePath;
		m_fullSavePath = NULL;
	}
}


//////////////////////////////////////////////////////////////////////
// 
// Function to return ONLY the characters to the right of the last '/'
//			in the URI string
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
CString CHTTPDownload::GetURINoPath(CString URI)
{
	int index;
	index = URI.ReverseFind('/');

	return URI.Right(URI.GetLength() - index - 1);

}

//////////////////////////////////////////////////////////////////////
// 
// Function to return the contents of the transfer log for the last transfer
//
// Input:	none
//
// Output:	CString
//
//////////////////////////////////////////////////////////////////////
CString CHTTPDownload::GetTransferLog()
{
	return m_lastTransferLog;
}


//////////////////////////////////////////////////////////////////////
// 
// Function to erase the contents of the tranfer log
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::ClearTransferLog()
{
	m_lastTransferLog.Empty();
}


//////////////////////////////////////////////////////////////////////
// 
// Function to add an entry to the Transfer Log
//
// Input:	text string
//			Special Headers:	TT_CONNECT, TT_DOWNLOAD, TT_SAVE, TT_NONE
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::AddTransferLogEntry(CString text, CHTTPDownload::TransferType type)
{
	CTime time = CTime::GetCurrentTime();
	CString timeString = time.Format("%H:%M:%S");

	switch (type)
	{
	case TT_CONNECT:
		m_lastTransferLog += "\n***CONNECTION AT " + timeString + " ***\n";
		m_lastTransferLog += text;
		break;
	case TT_DOWNLOAD:
	case TT_SAVE:
		m_lastTransferLog += text;
		m_lastTransferLog += " (" + timeString + ")";
		break;
	case TT_FINAL:
		m_lastTransferLog += "\n\n\n";
		m_lastTransferLog += text;
		break;
	default:
		m_lastTransferLog += text;
		break;
	}
	m_lastTransferLog += "\n";
}


//////////////////////////////////////////////////////////////////////
// 
// Function to check if file already exists, if so prompt user for action
//
// Input:	CString file name
//
// Output:	new filename or same filename, depending on user action
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::CheckFileName()
{
	CFileStatus rStatus;
	// if file checking is enabled and file already exists
	if (m_fileCheck && CFile::GetStatus(*m_fullSavePath,rStatus))	
	{
		CString ext = m_fullSavePath->Right(3);
		CString filters = ext + " files (*." + ext + ")|*." + ext;
		filters += "|All files (*.*)|*.*||";
		CFileDialog dlg(FALSE,ext,*m_fullSavePath,
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filters);
		dlg.m_ofn.lpstrTitle = "File Already Exists: Specify Save File Name";

		//prompt user with a "save as" dialog box
		if (dlg.DoModal() == IDOK)
		{
			// return the filename selected (even if it is the same)
			*m_fullSavePath = dlg.GetPathName();
		}
		else
		{
			// if cancel was clicked
			*m_fullSavePath = "";
			MessageBoxEx(NULL,"File will not be saved","Abort Save",MB_OK | MB_SETFOREGROUND, LANG_ENGLISH);
			AddTransferLogEntry("File Save Aborted By User");
		}
		
	}
}


//////////////////////////////////////////////////////////////////////
// 
// Function to toggle whether to check if file already exists
//
// Input:	TRUE = check file existance, prompt for action
//			FALSE = do not check file existance, overwrite files
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::EnableFileCheck(bool state)
{
	m_fileCheck = state;
}



//////////////////////////////////////////////////////////////////////
// 
// Function to set the cancel flag to TRUE, so thread will exit itself
//
// Input:	none
//
// Output:	none
//
//////////////////////////////////////////////////////////////////////
VOID CHTTPDownload::SetCancelFlag()
{
	m_cancel = TRUE;
}

//////////////////////////////////////////////////////////////////////
// 
// Function to return the cancel flag
//
// Input:	none
//
// Output:	boolean
//
//////////////////////////////////////////////////////////////////////
bool CHTTPDownload::GetCancelFlag()
{
	return m_cancel;
}

//////////////////////////////////////////////////////////////////////
// 
// Function to parse a URL into server and object strings
//
// Input:	CString URL
//			return info flag: PI_SERVER or PI_OBJECT
//
// Output:	CString
//
//////////////////////////////////////////////////////////////////////
CString CHTTPDownload::ParseURL(CString url, ParseInfo info)
{
	DWORD serviceType;
	CString server, object;
	INTERNET_PORT port;

	::AfxParseURL(url,serviceType,server,object,port);

	switch (info)
	{
	case PI_SERVER:
		return server;
		break;
	case PI_OBJECT:
		return object;
		break;
	default:
		return server;
	}
}




//////////////////////////////////////////////////////////////////////
// CHTTPDownloadParams Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTTPDownloadParams::CHTTPDownloadParams()
{

}

CHTTPDownloadParams::~CHTTPDownloadParams()
{

}
