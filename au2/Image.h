// Image.h: interface for the CImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGE_H__A8DF0A29_C1F6_4C57_8B31_4B442AB53ADF__INCLUDED_)
#define AFX_IMAGE_H__A8DF0A29_C1F6_4C57_8B31_4B442AB53ADF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef enum {X,Y} Axis;

class CImage  
{
public:
	CString GetFileName();
	CString m_fileName;
	void ResetCrop();
	long GetOutputHeight();
	long GetOutputWidth();
	long GetHeight();
	long GetWidth();
	long DrawImage(HDC hdc, CRect *size, CRect *rc = NULL);
	void SetOutputScaleY(double scale);
	void SetOutputScaleX(double scale);
	double GetOutputScaleY();
	double GetOutputScaleX();
	void SetCrop(CRect *crop);
	void SetOutputScale(double);
	long DrawImage(HDC hdc, CPoint* point = NULL, CRect* rc = NULL);
	bool IsPictureValid();
	long GetHeightHM();
	long GetWidthHM();
	void LoadFile(LPCTSTR szFile);
	CImage();
	virtual ~CImage();

private:
	long PixelsToHM(long pixel, Axis a);
	long HMToPixels(long hm, Axis a);
	long m_width;
	long m_height;
	long m_widthHM;
	long m_heightHM;
	double m_outputScaleY;
	double m_outputScaleX;
	CRect m_cropRect;
	LPPICTURE m_gpPicture;
};

#endif // !defined(AFX_IMAGE_H__A8DF0A29_C1F6_4C57_8B31_4B442AB53ADF__INCLUDED_)
