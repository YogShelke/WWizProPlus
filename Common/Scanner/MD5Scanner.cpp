#include "stdafx.h"
#include "MD5Scanner.h"


/***************************************************************************************************
*  Function Name  : CMD5Scanner
*  Description    : Cont'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
CMD5Scanner::CMD5Scanner()
{
	__try
	{
		if (!LoadReqdLibrary())
		{
			AddLogEntry(L"### DLL could not be loaded, VBMD5SCN.DLL", 0, 0, true, SECONDLEVEL);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CMD5Scanner::CMD5Scanner", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : LoadReqdLibrary
*  Description    : Function to load the required library, when the application will be loaded.
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
bool CMD5Scanner::LoadReqdLibrary()
{
	//bool bReturn = false;
	try
	{

		CString	csMD5ScanDLLPath(L"");
		csMD5ScanDLLPath = GetWardWizPathFromRegistry();

		CString	csMD5ScanDLL = L"";
#ifdef MSOUTLOOK32
		csMD5ScanDLL.Format(L"%sVibraniumMD5SCN32.DLL", csMD5ScanDLLPath);
#else
		csMD5ScanDLL.Format(L"%sVBMD5SCN.DLL", csMD5ScanDLLPath);
#endif

		if (!PathFileExists(csMD5ScanDLL))
		{
			AddLogEntry(L"### Failed to load library : %s", csMD5ScanDLL, 0, true, SECONDLEVEL);
			return false;
		}

		m_hInstLibrary = LoadLibrary(csMD5ScanDLL);

		if (!m_hInstLibrary)
		{
			//log
			AddLogEntry(L"### Failed to load library : %s", csMD5ScanDLL, 0, true, SECONDLEVEL);
			return false;
		}

		m_loadMD5Data = (LoadMD5Database)GetProcAddress(m_hInstLibrary, "LoadMD5Database");
		if (!m_loadMD5Data)
		{
			//log
			AddLogEntry(L"### DLL Function address not found, LoadMD5Database", 0, 0, true, SECONDLEVEL);
			return false;
		}

		m_pMD5Scanner = (MD5SCANNER)GetProcAddress(m_hInstLibrary, "ScanForMD5Data");
		if (!m_pMD5Scanner)
		{
			//log
			AddLogEntry(L"### DLL Function address not found, ScanForMD5Data", 0, 0, true, SECONDLEVEL);
			return false;
		}		

		m_UnloadMD5Data = (UnLoadMD5Database)GetProcAddress(m_hInstLibrary, "UnLoadSignatures");
		if (!m_UnloadMD5Data)
		{
			AddLogEntry(L"### DLL Function address not found, UnLoadSignatures", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### DLL could not be loaded, filename: %s", m_szFilePath, 0, true, SECONDLEVEL);
	}

	return true;
}


/***************************************************************************************************
*  Function Name  : ~CMD5Scanner
*  Description    : Dest'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
CMD5Scanner::~CMD5Scanner()
{
	UnloadLibrary();
}


/***************************************************************************************************
*  Function Name  : LoadMD5Data
*  Description    : Function to load MD5 database.
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
bool CMD5Scanner::LoadMD5Data(FILETYPE filetype, DWORD &dwScan)
{
	bool bReturn = false;
	__try
	{
		if (m_hInstLibrary)
		{
			if (m_loadMD5Data)
			{
				if (m_loadMD5Data(filetype, dwScan))
					bReturn = true;
				else
					bReturn = false;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CMD5Scanner::LoadMD5Data", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : UnLoadMD5Data
*  Description    : To Unload MD5 Signature Database
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
bool CMD5Scanner::UnLoadMD5Data()
{
	bool bReturn = false;
	__try
	{
		if (m_hInstLibrary)
		{
			if (m_UnloadMD5Data)
			{
				if (m_UnloadMD5Data())
					bReturn = true;
				else
					bReturn = false;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CMD5Scanner::UnLoadMD5Data", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : UnloadLibrary
*  Description    : Function to unload the library
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
void CMD5Scanner::UnloadLibrary()
{
	__try
	{
		if (m_hInstLibrary)
			FreeLibrary(m_hInstLibrary);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CMD5Scanner::UnloadLibrary", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to scan the requested file, if the library has loaded successfully
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
bool CMD5Scanner::ScanFile(LPBYTE lpBuffer, DWORD dwBufferSize, DWORD & dwVirusID, bool Rescan, FILETYPE filetype)
{
	bool bReturn = false;
	
	__try	
	{
		if (m_hInstLibrary)
		{
			if (m_pMD5Scanner)
			{
				bReturn = m_pMD5Scanner(lpBuffer, dwBufferSize, dwVirusID, Rescan, filetype);
				
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CMD5Scanner::ScanFile", 0, 0, true, SECONDLEVEL);
	}
		
	return bReturn;
}