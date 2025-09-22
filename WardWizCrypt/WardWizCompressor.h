/**********************************************************************************************************
Program Name			: WardWizCrypter.cpp
Description				: Wrapper class to encrypt files/folders using AES 256 algorithm.
Author Name				: Ramkrushna Shelke
Date Of Creation		: 5th Jun 2015
Version No				: 1.11.0.0
Special Logic Used		:
Modification Log		:
***********************************************************************************************************/
#pragma once
//#include "stdafx.h"
#include "ZipArchive.h"

class CWardWizCompressor
{

public:
	CWardWizCompressor();
	virtual ~CWardWizCompressor();

public:

	bool CreateArchive(LPCTSTR lpszZipfilePath, LPCTSTR lpszActualFilePath);
	bool ExtractArchive(LPCTSTR lpszZipfilePath, LPCTSTR lpszActualFilePath);
	void GetStatus();
	void StopOperation();
	void ResetStatusVariable();

public:
	CZipArchive		m_zip;
	bool			m_IsExtract;
	DWORD64			m_dwFileSize;
	DWORD			m_dwBufferSize;
	DWORD			m_dwDone;
	bool			m_isbProcesStart;

};