/***************************************************************************                      
   Program Name          : JpegDialog.cpp
   Description           : To load and draw JPG to image controls
   Author Name           :                                                                         
   Date Of Creation      : 18th Oct,2013
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/
#include "stdafx.h"
#include "JpegDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************************************                    
*  Function Name  : CJpegDialog                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0128
*  Date           : 18 Oct 2013
****************************************************************************************************/
CJpegDialog::CJpegDialog(UINT nTemplateID, CWnd* pParent /*=NULL*/)
: CDialog(nTemplateID, pParent)
{
	try
	{
		m_pPicture		   = NULL;
		m_bIsInitialized   = FALSE;
		m_hBitmap          = NULL;
		m_hOldBitmap	   = NULL;
		m_hMemDC		   = NULL;
		m_nDataSize		   = 0;
		m_PictureSize.cx = m_PictureSize.cy = 0;
		SetRect(&m_PaintRect,0,0,0,0);
		m_clrBackground    = RGB(255,255,255); // white by default
	}
	catch(...)
	{
	}
}

/***************************************************************************************************                    
*  Function Name  : ~CJpegDialog                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0129
*  Date           : 18 Oct 2013
****************************************************************************************************/
CJpegDialog::~CJpegDialog()
{
	try
	{
		UnLoad();
	}
	catch(...)
	{
	}
}

/***************************************************************************************************                    
*  Function Name  : UnLoad                                                     
*  Description    : Delete or release all aloocated memory or objects.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0130
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CJpegDialog::UnLoad()
{
	try
	{
		if(m_pPicture)
		{
			m_pPicture->Release();
			m_pPicture = NULL;
		}
		if(m_hMemDC)
		{
			SelectObject(m_hMemDC,m_hOldBitmap);
			SelectObject(m_hMemDC, m_hBitmap);
			::DeleteDC(m_hMemDC);
			::DeleteObject(m_hBitmap);
			m_hMemDC  = NULL;
			m_hBitmap = NULL;
		}
	}
	catch(...)
	{
	}
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0131
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CJpegDialog, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

/***************************************************************************************************                    
*  Function Name  : OnEraseBkgnd                                                     
*  Description    : The framework calls this member function when the CWnd object background needs erasing (for example, when resized).
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0132
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CJpegDialog::OnEraseBkgnd(CDC *pDC)
{
	try
	{
		CDialog::OnEraseBkgnd(pDC);
		LONG nPaintWidth = m_PaintRect.right-m_PaintRect.left;
		if(nPaintWidth > 0)
		{
			LONG nPaintHeight = m_PaintRect.bottom - m_PaintRect.top;
			::BitBlt(pDC->m_hDC, 0, 0, nPaintWidth, nPaintHeight, m_hMemDC, m_PaintRect.left,				m_PaintRect.top, SRCCOPY);
		}
		else
		{
			::BitBlt(pDC->m_hDC, 0, 0, m_PictureSize.cx, m_PictureSize.cy, m_hMemDC, 0, 0, SRCCOPY);
		}
		return TRUE;
	}
	catch(...)
	{
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : Load                                                     
*  Description    : Load given files.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0133
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CJpegDialog::Load(LPCTSTR szFileName)
{
	try
	{
		ASSERT(szFileName);
		CFile file;
		HGLOBAL hGlobal;
		DWORD dwSize;
		if(!file.Open(szFileName, CFile::modeRead | CFile::shareDenyWrite))
			return FALSE;

		dwSize = static_cast<DWORD>(file.GetLength());
		hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD,dwSize);
		if(!hGlobal)
		{
			file.Close();
			return FALSE;
		};
		TCHAR *pData = reinterpret_cast<TCHAR*>(GlobalLock(hGlobal));
		if(!pData)
		{
			GlobalFree(hGlobal);
			file.Close();
			return FALSE;
		};
		TRY
		{
			file.Read(pData,dwSize);
		}
		CATCH(CFileException, e);
		{
			GlobalFree(hGlobal);
			e->Delete();
			file.Close();
			return FALSE;
		}
		END_CATCH
			GlobalUnlock(hGlobal);
		file.Close();
		BOOL bRetValue = Load(hGlobal,dwSize);
		GlobalFree(hGlobal);
		return bRetValue;
	}
	catch(...)
	{
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : Load                                                     
*  Description    : Load file with is resource type
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0134
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CJpegDialog::Load(LPCTSTR szResourceName, LPCTSTR szResourceType)
{
	try
	{
		ASSERT(szResourceName);
		ASSERT(szResourceType);

		HRSRC hPicture = FindResource(AfxGetResourceHandle(),szResourceName,szResourceType);
		if(!hPicture)
		{
			TRACE(_T("Find (resource): Error finding resource %s\n"),szResourceName);
			return FALSE;
		};
		HGLOBAL hResData = LoadResource(AfxGetResourceHandle(),hPicture);
		if(!hResData)
		{
			TRACE(_T("Load (resource): Error loading resource %s\n"),szResourceName);
			return FALSE;
		};
		DWORD dwSize = SizeofResource(AfxGetResourceHandle(),hPicture);

		// hResData is not the real HGLOBAL (we can't lock it)
		// let's make it real
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD,dwSize);
		if(!hGlobal)
		{
			TRACE(_T("Load (resource): Error allocating memory\n"));
			FreeResource(hResData);
			return FALSE;
		};

		TCHAR *pDest = reinterpret_cast<TCHAR *> (GlobalLock(hGlobal));
		TCHAR *pSrc = reinterpret_cast<TCHAR *> (LockResource(hResData));
		if(!pSrc || !pDest)
		{
			TRACE(_T("Load (resource): Error locking memory\n"));
			GlobalFree(hGlobal);
			FreeResource(hResData);
			return FALSE;
		};
		CopyMemory(pDest,pSrc,dwSize);
		FreeResource(hResData);
		GlobalUnlock(hGlobal);

		BOOL bRetValue = Load(hGlobal,dwSize);
		GlobalFree(hGlobal);
		hGlobal = NULL;
		return bRetValue;
	}
	catch(...)
	{
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : Load                                                     
*  Description    : Load file with its resource type from dll.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0135
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CJpegDialog::Load(HMODULE m_hResHandle, LPCTSTR szResourceName, LPCTSTR szResourceType)
{
	try
	{
		ASSERT(szResourceName);
		ASSERT(szResourceType);

		HRSRC hPicture = FindResource(m_hResHandle,szResourceName,szResourceType);
		if(!hPicture)
		{
			TRACE(_T("Find (resource): Error finding resource %s\n"),szResourceName);
			return FALSE;
		};
		HGLOBAL hResData = LoadResource(m_hResHandle,hPicture);
		if(!hResData)
		{
			TRACE(_T("Load (resource): Error loading resource %s\n"),szResourceName);
			return FALSE;
		};
		DWORD dwSize = SizeofResource(m_hResHandle,hPicture);

		// hResData is not the real HGLOBAL (we can't lock it)
		// let's make it real
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD,dwSize);
		if(!hGlobal)
		{
			TRACE(_T("Load (resource): Error allocating memory\n"));
			FreeResource(hResData);
			return FALSE;
		};

		TCHAR *pDest = reinterpret_cast<TCHAR *> (GlobalLock(hGlobal));
		TCHAR *pSrc = reinterpret_cast<TCHAR *> (LockResource(hResData));
		if(!pSrc || !pDest)
		{
			TRACE(_T("Load (resource): Error locking memory\n"));
			GlobalFree(hGlobal);
			FreeResource(hResData);
			return FALSE;
		};
		CopyMemory(pDest,pSrc,dwSize);
		FreeResource(hResData);
		GlobalUnlock(hGlobal);

		BOOL bRetValue = Load(hGlobal,dwSize);
		GlobalFree(hGlobal);
		hGlobal = NULL;
		return bRetValue;
	}
	catch(...)
	{
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : Load                                                     
*  Description    : Load files 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0136
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CJpegDialog::Load(HGLOBAL hGlobal, DWORD dwSize)
{
	try
	{

		IStream *pStream = NULL;
		m_nDataSize = dwSize;
		GlobalUnlock(hGlobal);

		// don't delete memory on object's release
		if(CreateStreamOnHGlobal(hGlobal,FALSE,&pStream) != S_OK)
			return FALSE;

		if(OleLoadPicture(pStream,dwSize,FALSE,IID_IPicture,
			reinterpret_cast<LPVOID *>(&m_pPicture)) != S_OK)
		{
			pStream->Release();
			return FALSE;
		};
		pStream->Release();

		// store picture's size
		long hmWidth;
		long hmHeight;

		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);

		HDC hDC = ::GetDC(m_hWnd);
		m_PictureSize.cx = MulDiv(hmWidth, GetDeviceCaps(hDC,LOGPIXELSX), 2540);
		m_PictureSize.cy = MulDiv(hmHeight, GetDeviceCaps(hDC,LOGPIXELSY), 2540);
		::ReleaseDC(m_hWnd,hDC);

		return PrepareDC(m_PictureSize.cx,m_PictureSize.cy);
	}
	catch(...)
	{
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : PrepareDC                                                     
*  Description    : Prepare device context
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0137
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CJpegDialog::PrepareDC(int nWidth, int nHeight)
{
	try
	{
#ifdef MASTRSCAN
		SetWindowPos(NULL, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOZORDER);
#else
		SetWindowPos(NULL, 0, 0, nWidth+3, nHeight+3, SWP_NOMOVE | SWP_NOZORDER);
#endif


		HDC hWinDC = ::GetDC(m_hWnd);
		if(!hWinDC)return FALSE;

		m_hMemDC = CreateCompatibleDC(hWinDC);
		if(!m_hMemDC)
		{
			::ReleaseDC(m_hWnd,hWinDC);
			::DeleteDC(m_hMemDC);
			::DeleteDC(hWinDC);
			return FALSE;
		};

		m_hBitmap  = CreateCompatibleBitmap(hWinDC,nWidth,nHeight);
		if(!m_hBitmap)
		{
			::ReleaseDC(m_hWnd,hWinDC);
			::DeleteDC(m_hMemDC);
			::DeleteDC(hWinDC);
			return FALSE;
		};

		m_hOldBitmap = reinterpret_cast<HBITMAP>
			(SelectObject(m_hMemDC,m_hBitmap));

		// fill the background
		m_clrBackground = GetSysColor(COLOR_3DFACE);
		RECT rect = {0,0,nWidth,nHeight};
		FillRect(m_hMemDC,&rect,(HBRUSH)(COLOR_WINDOW));

		m_bIsInitialized = TRUE;
		return TRUE;
	}
	catch(...)
	{
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : Draw                                                     
*  Description    : Draw loaded file
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0138
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CJpegDialog::Draw()
{
	try
	{
		if(!m_bIsInitialized)
		{
			TRACE(_T("Call one of the CPictureExLogo::Load()member functions before calling Draw()\n"));
			return FALSE;
		};
		if(m_pPicture)
		{
			long hmWidth;
			long hmHeight;
			m_pPicture->get_Width(&hmWidth);
			m_pPicture->get_Height(&hmHeight);
			if(m_pPicture->Render(m_hMemDC, 0, 0, m_PictureSize.cx, m_PictureSize.cy,
				0, hmHeight, hmWidth, -hmHeight, NULL) == S_OK)
			{
				Invalidate(TRUE);
				return TRUE;
			};
		};
		return FALSE;
	}
	catch(...)
	{
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : OnNcHitTest                                                     
*  Description    : The framework calls this member function for the CWnd object that contains the cursor (or the CWnd object that used the SetCapture member function to capture the mouse input)every time the mouse is moved.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0139
*  Date           : 18 Oct 2013
****************************************************************************************************/
LRESULT CJpegDialog::OnNcHitTest(CPoint point)
{
	return HTCAPTION;
}

/***************************************************************************************************                    
*  Function Name  : LoadJPEG                                                     
*  Description    : Load on JPEG for different pixel
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0140
*  Date           : 18 Oct 2013
****************************************************************************************************/
DWORD CJpegDialog::LoadJPEG(int jpgDPI_96, int jpgDPI_120, int jpgDPI_98120)
{
	try
	{
		//CCPUInfo objSystem;
		//DWORD dwValue = objSystem.GetDpiValue();
		//// code for painting the background Starts here
		//if(dwValue == 96)
			Load(MAKEINTRESOURCE(jpgDPI_96), _T("JPGTYPE"));
	/*	else if(dwValue == 120)
			Load(MAKEINTRESOURCE(jpgDPI_120), _T("JPGTYPE"));
		else if(dwValue == 98120)
		{
			Load(MAKEINTRESOURCE(jpgDPI_98120),_T("JPGTYPE"));
		}*/
		Draw();
		return 96;
		// code for painting the background Ends here
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CJpegDialog::Draw"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}