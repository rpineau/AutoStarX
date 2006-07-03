// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Image.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define HIMETRIC_INCH 2540

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImage::CImage()
{
	m_gpPicture = NULL;
	m_outputScaleX = 1;
	m_outputScaleY = 1;
}

CImage::~CImage()
{

}

//////////////////////////////////////////////////////////////////////
// Function to load a JPEG file from disk into the CImage Object
//
// szFile = Complete filename with path
//
//////////////////////////////////////////////////////////////////////
void CImage::LoadFile(LPCTSTR szFile)
{
	//open file
	HANDLE hFile = CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return;

	//get file size
	DWORD dwFileSize = GetFileSize(hFile, NULL);

	if (dwFileSize == (DWORD)-1)
	{
		CloseHandle(hFile);
		return;
	}

	LPVOID pvData = NULL;
	//alloc memory based on file size
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);

	if (hGlobal == NULL)
	{
		CloseHandle(hFile);
		return;
	}

	pvData = GlobalLock(hGlobal);

	if (pvData == NULL)
	{
		GlobalUnlock(hGlobal);
		CloseHandle(hFile);
		return;
	}

	DWORD dwBytesRead = 0;

	//read file and store in global memory
	BOOL bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, NULL);
	GlobalUnlock(hGlobal);
	CloseHandle(hFile);

	if (!bRead)
		return;

	LPSTREAM pstm = NULL;
	//create IStream* from global memory
	HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);

	if (!(SUCCEEDED(hr)))
	{
		if (pstm != NULL)
			pstm->Release();
		return;
	}
	else if (pstm == NULL)
		return;


	//create IPicture from image file
	if (m_gpPicture)
		m_gpPicture->Release();
	hr = ::OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID *)&m_gpPicture);

	if (!(SUCCEEDED(hr)))
	{
		pstm->Release();
		return;
	}
	else if (m_gpPicture == NULL)
	{
		pstm->Release();
		return;
	}

	long tempSize;
	m_gpPicture->get_Height(&tempSize);
	m_heightHM = tempSize;
	m_gpPicture->get_Width(&tempSize);
	m_widthHM = tempSize;

	m_cropRect.SetRect(0,m_heightHM,m_widthHM,0);

	pstm->Release();

	InvalidateRect(NULL, NULL, TRUE);

	m_width = HMToPixels(m_widthHM,X);
	m_height = HMToPixels(m_heightHM,Y);

	m_fileName = szFile;

}

/////////////////////////////////////////////////////////////////
//
//Function to return the HIMETRIC width of the Original Image
//
/////////////////////////////////////////////////////////////////
long CImage::GetWidthHM()
{
/*	long width;
	m_gpPicture->get_Width(&width);
	return width;*/
	return m_widthHM;
}

/////////////////////////////////////////////////////////////////
//
//Function to return the HIMETRIC height of the Original Image
//
/////////////////////////////////////////////////////////////////
long CImage::GetHeightHM()
{
/*	long height;
	m_gpPicture->get_Height(&height);
	return height;*/
	return m_heightHM;

}

//////////////////////////////////////////////////////////////
//
// Function to check if file is properly loaded
//
/////////////////////////////////////////////////////////////
bool CImage::IsPictureValid()
{
	if (m_gpPicture) return TRUE;
	else return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Function to display and scale a previously loaded Image
//
// hdc = Current Display Context
//
// point = Point describing origin of scaled image
//
// rc = Rectangle containing GetClientRect() of Display Context
//
//////////////////////////////////////////////////////////////////////
long CImage::DrawImage(HDC hdc, CPoint *point, CRect *rc)
{

	long originX = 0,originY = 0;

	long nWidth = (long) (m_width * m_outputScaleX);
	long nHeight = (long) (m_height * m_outputScaleY);

	if (point) // when a CPoint is passed to specify the origin
	{
		originX = point->x;
		originY = point->y;
	}

	if (m_gpPicture)
		return m_gpPicture->Render(hdc,	originX,originY,nWidth,nHeight,
									m_cropRect.left,
									m_cropRect.top,
									m_cropRect.Width(),
									m_cropRect.Height(),
									rc);
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////
// Function to display and scale a previously loaded JPEG Image
//
// hdc = Current Display Context
//
// size = Rectangle describing size and location of scaled image
//
// rc = Rectangle containing GetClientRect() of Display Context
//
// NOTE: SetOutputScale does not affect this function
//
//////////////////////////////////////////////////////////////////////
long CImage::DrawImage(HDC hdc, CRect *size, CRect *rc)
{
	// calculate width and height of original image
	long hmWidth = GetWidthHM();
	long hmHeight = GetHeightHM();


	int nWidth = size->right-size->left;
	int nHeight = size->bottom-size->top;

	//draw image
	return m_gpPicture->Render(hdc,	size->left,size->top,nWidth,nHeight,
									m_cropRect.left,m_cropRect.top,
									m_cropRect.Width(),m_cropRect.Height(),rc);


}


void CImage::SetOutputScale(double scale)
{
	m_outputScaleX = scale;
	m_outputScaleY = scale;
}

double CImage::GetOutputScaleX()
{
	return m_outputScaleX;
}

double CImage::GetOutputScaleY()
{
	return m_outputScaleY;
}

void CImage::SetOutputScaleX(double scale)
{
	m_outputScaleX = scale;
}

void CImage::SetOutputScaleY(double scale)
{
	m_outputScaleY = scale;
}

///////////////////////////////////////////////////////////////
//
// Function to define cropping rectangle
// 
// crop = desired output rectangle in pixels relative to origin
//        of original picture in original scale
//
///////////////////////////////////////////////////////////////
void CImage::SetCrop(CRect *crop)	
{

	m_cropRect.right = PixelsToHM(crop->right,X);
	m_cropRect.bottom = PixelsToHM(crop->top,Y);
	m_cropRect.left = PixelsToHM(crop->left,X);
	m_cropRect.top = PixelsToHM(crop->bottom,Y);

}

long CImage::GetWidth()
{
	return m_width;
}

long CImage::GetHeight()
{
	return m_height;
}

/////////////////////////////////////////////////////////////////////////////////////
// Function to return width in pixels of output image including scale & crop
// Only meaningful when CPoint version of DrawImage() is used
////////////////////////////////////////////////////////////////////////////////////
long CImage::GetOutputWidth()
{
//	return (long) (HMToPixels(m_cropRect.Width(),X) * m_outputScaleX);
	return (long) (GetWidth() * m_outputScaleX);
}


/////////////////////////////////////////////////////////////////////////////////////
// Function to return height in pixels of output image including scale & crop
// Only meaningful when CPoint version of DrawImage() is used
////////////////////////////////////////////////////////////////////////////////////
long CImage::GetOutputHeight()
{
//	return (long) (HMToPixels(-m_cropRect.Height(),Y) * m_outputScaleY);
	return (long) (GetHeight() * m_outputScaleY);
}


/////////////////////////////////////////////////////////////////////////////////////
// Function to convert a himetric value to pixels for the screen resolution
//
// hm = himetric value
//
// a = "X" or "Y" axis
//
////////////////////////////////////////////////////////////////////////////////////
long CImage::HMToPixels(long hm, Axis a)
{
	// calculate screen resolution
	CDC cDC;
	cDC.CreateIC(_T("DISPLAY"),NULL,NULL,NULL);
	const int nLogDPI = (a == X) ? cDC.GetDeviceCaps(LOGPIXELSX) : cDC.GetDeviceCaps(LOGPIXELSY);

	return MulDiv(hm,nLogDPI,HIMETRIC_INCH);
}


/////////////////////////////////////////////////////////////////////////////////////
// Function to convert pixels to a himetric value for the screen resolution
//
// pixel = number of pixels
//
// a = "X" or "Y" axis
//
////////////////////////////////////////////////////////////////////////////////////
long CImage::PixelsToHM(long pixel, Axis a)
{
	// calculate screen resolution
	CDC cDC;
	cDC.CreateIC(_T("DISPLAY"),NULL,NULL,NULL);
	const int nLogDPI = (a == X) ? cDC.GetDeviceCaps(LOGPIXELSX) : cDC.GetDeviceCaps(LOGPIXELSY);

	return MulDiv(pixel,HIMETRIC_INCH,nLogDPI);
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Function to remove any crop settings
//
////////////////////////////////////////////////////////////////////////////////////
void CImage::ResetCrop()
{
	long tempSize;
	m_gpPicture->get_Height(&tempSize);
	m_heightHM = tempSize;
	m_gpPicture->get_Width(&tempSize);
	m_widthHM = tempSize;

	m_cropRect.SetRect(0,m_heightHM,m_widthHM,0);

}

CString CImage::GetFileName()
{
	if (IsPictureValid())
		return m_fileName;
	else
		return "ERROR";
}
