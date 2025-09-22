// WardWizExports.cpp : Implementation of CWardWizExports

#include "stdafx.h"
#include "WardWizExports.h"
#include "Hash.h"

// CWardWizExports

STDMETHODIMP CWardWizExports::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IWardWizExports
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP CWardWizExports::FunctionGetStringHash(CHAR* strInputString, BSTR* bstrHash)
{
	try
	{
		if (!strInputString)
			return S_FALSE;

		if (!bstrHash)
			return S_FALSE;

		int iBufferSize = (int)strlen(strInputString);
		md5		alg;
		alg.Update((uchar*)strInputString, iBufferSize);
		alg.Finalize();

		char			szBufHash[64] = { 0 };
		PrintMD5(alg.Digest(), szBufHash);
		CComBSTR csHash(szBufHash);
		*bstrHash = SysAllocString(csHash);
		return S_OK;
	}
	catch (...)
	{
	}
	return S_FALSE;
}



/***************************************************************************************************
*  Function Name  :PrintMD5()
*  Description    :Converts a completed md5 digest into a char* string.
*  Author Name    :Vilas
*  Date           :4- Jul -2014 - 12 jul -2014
*  SR No          :WWIZHASH_0003
****************************************************************************************************/
void CWardWizExports::PrintMD5(BYTE md5Digest[16], char *pMd5)
{
	//char	chBuffer[256];
	char	chEach[10];
	int		nCount;

	//memset(chBuffer, 0, 256) ;
	memset(chEach, 0, sizeof(chEach));

	for (nCount = 0; nCount < 16; nCount++)
	{
		sprintf(chEach, "%02x", md5Digest[nCount]);
		strncat(pMd5, chEach, strlen(chEach));

		memset(chEach, 0, sizeof(chEach));
	}
}
