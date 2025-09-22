/****************************************************
*  Program Name: ISpyRepairDLL.cpp                                                                                              
*  Author Name: Prajakta                                                                                                      
*  Date Of Creation: 7 Dec 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/

#include "stdafx.h"
#include "ISpyRepairDLL.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLEXPORT _declspec(dllexport)

BEGIN_MESSAGE_MAP(CISpyRepairDLLApp, CWinApp)
END_MESSAGE_MAP()

/**********************************************************************************************************                     
*  Function Name  :	CISpyRepairDLLApp                                                     
*  Description    :	C'tor
*  Author Name    :	Prajakta                                                                                          
*  Date           : 7 Dec 2013
**********************************************************************************************************/
CISpyRepairDLLApp::CISpyRepairDLLApp()
{
	m_bDBLoaded = false;
}

//CISpyRepairDLLApp object
CISpyRepairDLLApp theApp;

/**********************************************************************************************************                     
*  Function Name  :	InitInstance                                                     
*  Description    :	Instance initialization that runs each time a copy of the program runs, including the first time
*  Author Name    :	Prajakta                                                                                          
*  Date           : 7 Dec 2013
**********************************************************************************************************/
BOOL CISpyRepairDLLApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

/**********************************************************************************************************                     
*  Function Name  :	LoadISpyRepairDB                                                     
*  Description    :	Searches repair expression in ITINREPAIR.DB using SpyID returned from scanner
*  SR.NO		  : VIRUSREPAIR_0006
*  Author Name    :	Prajakta                                                                                          
*  Date           : 7 Dec 2013
**********************************************************************************************************/
bool CISpyRepairDLLApp::LoadISpyRepairDB()
{
	//bool bReturn = false;
	//FILE *pFile = NULL;
	CString csFilePath;
	csFilePath = GetWardWizPathFromRegistry();
	csFilePath.Append(L"VBDB\\VIBRANIUMAVR.DB");

	/*
	TCHAR szModulePath[MAX_PATH] = {0};
	*/

	/*if(!GetModulePath(szModulePath, MAX_PATH))
	{
		MessageBox(NULL,L"Error in GetModulePath", L"ISpy UI", MB_ICONERROR|MB_OK);
		return bReturn;
	}
	csFilePath = szModulePath;
	*/
	
	if(!PathFileExists(csFilePath))
	{
		return false;
	}
	TCHAR szTempDBFilePath[MAX_PATH] = {0};
	m_csTmpRepairDBFileName = "";

	DWORD dwVersionLength = 0x00;
	DWORD dwMajorVersion = 0x00;
	if (!m_objWrdwizEncDecManager.DecryptDBFileData(csFilePath, szTempDBFilePath, dwVersionLength, dwMajorVersion))
	{
		return false;
	}
	m_bDBLoaded = true;
	//m_csTmpRepairDBFileName = szTempDBFilePath;
	if(!m_objWrdwizEncDecManager.LoadContentFromFile(szTempDBFilePath))
	{
		return false;
	}
	if(PathFileExists(szTempDBFilePath))
	{
		DeleteFile(szTempDBFilePath);
	}
	return true;
}

bool CISpyRepairDLLApp::FetchRepairExpr(DWORD dwSpyID)
{
	bool bReturn = false;
	CString csFirstEntry = NULL, csSecondEntry = NULL;
	const ContactList& contacts = m_objWrdwizEncDecManager.m_objEncDec.GetContacts();
	// iterate over all contacts & add them to the list
	POSITION pos = contacts.GetHeadPosition();
	while(pos != NULL)
	{
		const CIspyList contact = contacts.GetNext(pos);
		
		csFirstEntry = contact.GetFirstEntry();		
		csSecondEntry =  contact.GetSecondEntry();
	
		TCHAR szidbuff[8] = {0};
		_itow(dwSpyID,szidbuff,0x10);
		DWORD dwRepairID = _wtol((LPCTSTR)csFirstEntry);
		if(dwRepairID == dwSpyID)
		{	
			//csTmp = wcsrchr(csTmp,'=');
			bReturn = ParseExpression(csSecondEntry);				
		}
	}
	
	/*pFile = _wfsopen(csFilePath, _T("r"), 0x20);

	if(!pFile)
	{
		return bReturn;
	}*/

	//char buff[5000]; 
	//CString csLine, csTmp;
	//int iPos = 0;
	//while(!feof(pFile))
//	{	
//		if(fgets(buff,5000,pFile))
//		{
			//csLine = csTmp = buff;
			//csLine = csLine.Tokenize(_T("="),iPos);
			//csLine.Trim();
			//DWORD dwLine = _wtol((LPCTSTR)csLine);
			//iPos = 0;
		//}

	//fclose(pFile);
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	ParseExpression                                                     
*  Description    :	Calls appropriate repair functions after parsing & evaluating the expression
*  SR.NO		  : VIRUSREPAIR_0007
*  Author Name    :	Prajakta                                                                                          
*  Date           : 7 Dec 2013
**********************************************************************************************************/
bool CISpyRepairDLLApp::ParseExpression(CString csLine)
{
	CString csArg = csLine;
	CString csFunID,csParam;
	int i = csArg.GetLength();
	if(csArg.GetLength() == 0)
	{
		return false;
	}
	int iPos = 0x0, iFunID = 0;
	char *pHex	= NULL;
	
	csFunID = csArg.Tokenize(_T("#"), iPos);
	iFunID = strtol(CStringA(csFunID), &pHex, 0x10);
	CStringA csResToken;
	bool bRepair = false;
	DWORD dwReturn;
	csParam = wcsrchr(csArg,'#');

	iPos = 1;
	CString csExprParam; 
	if(csParam.Trim().GetLength() != 0)
	{
		csExprParam = csParam.Tokenize(_T(","), iPos);
	}

	bool bContinue = true;
	while(csFunID != "")
	{
		csResToken = csFunID.Left(2);
		iFunID = strtol(csResToken, &pHex, 0x10);
				
		csFunID = csFunID.Mid(2);
		if(csParam.Trim().GetLength() != 0)
		{
			if(csExprParam != "")
			{
				if(!m_objISpyRepair.GetParameters(csExprParam))
				{
					return false;
				}
				csExprParam = csParam.Tokenize(_T(","), iPos);
			}
			else 
			{	
				memset(m_objISpyRepair.m_dwArgs,0,sizeof(m_objISpyRepair.m_dwArgs));
			}
		}
		
		switch(iFunID)
		{
			case 0x0://For GetParameters function
				break;
			case 0x1:
				bRepair = m_objISpyRepair.RepairDelete();
				break;
			case 0x2:
				bRepair = true;
				dwReturn = m_objISpyRepair.WriteAEP();
				if(dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				if(dwReturn != 0x02)
				{
					bContinue = false;
				}
				break;
			case 0x3:
				bRepair = m_objISpyRepair.TruncateFromAEP();
				break;
			case 0x4:
				bRepair = m_objISpyRepair.SetFileEnd();
				break;
			case 0x5:
				bRepair = m_objISpyRepair.TruncateFile();
				break;
			case 0x6:
				bRepair = m_objISpyRepair.RemoveSections();
				break;
			case 0x7:
				bRepair = m_objISpyRepair.CalculateLastSectionProperties();
				break;
			case 0x8:
				bRepair = m_objISpyRepair.CalculateImageSize();
				break;
			case 0x9:
				bRepair = m_objISpyRepair.FillWithZeros();
				break;
			case 0xA:
				bRepair = m_objISpyRepair.CalculateCheckSum();
				break;
			case 0xB:
				bRepair = true;
				dwReturn = m_objISpyRepair.ReplaceOriginalData();
				if(dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				if(dwReturn != 0x02)
				{
					bContinue = false;
				}
				break;
			case 0xC:
				bRepair = m_objISpyRepair.GetBuffer4Decryption();
				break;
			case 0xD:
				bRepair = m_objISpyRepair.DwordXOR();
				break;
			case 0xE:
				bRepair = m_objISpyRepair.RepairOptionalHeader();
				break;
			case 0xF:
				bRepair = m_objISpyRepair.ReplaceDecryptedDataInFile();
				break;
			case 0x10:
				bRepair = true;
				dwReturn = m_objISpyRepair.SearchString();
				if(dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				if(dwReturn != 0x02)
				{
					bContinue = false;
				}
				break;
			case 0x11:
				bRepair = m_objISpyRepair.ReturnDecryptedValue();
				break;
			case 0x12:
				bRepair = m_objISpyRepair.RepairRenamer();
				break;
			case 0x13:
				bRepair = true;
				dwReturn = m_objISpyRepair.RepairVirusStub();
				if(dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				if(dwReturn != 0x02)
				{
					bContinue = false;
				}
				break;
			case 0x14:
				bRepair = true;			
				dwReturn = m_objISpyRepair.GetAddrToGetEntryData();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x15:
				bRepair = true;
				dwReturn = m_objISpyRepair.GetFileEnd();
				if (dwReturn == 0xFFFFFFFF)
				{
					bRepair = FALSE;
					break;
				}
				break;
			case 0x16:
				bRepair = true;
				dwReturn = m_objISpyRepair.Runouce();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x17:
				bRepair = true;
				dwReturn = m_objISpyRepair.LuderEntryReplace();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x18:
				bRepair = true;
				dwReturn = m_objISpyRepair.Gamrue();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x19:
				bRepair = true;
				dwReturn = m_objISpyRepair.GetEntryAddr();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x1A:
				bRepair = true;
				dwReturn = m_objISpyRepair.Hidrag();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x1B:
				bRepair = true;
				dwReturn = m_objISpyRepair.QVod();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x1C:
				bRepair = true;
				dwReturn = m_objISpyRepair.Resur();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x1D:
				bRepair = true;
				dwReturn = m_objISpyRepair.Crytex();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x1E:
				bRepair = true;
				dwReturn = m_objISpyRepair.GetEntryAddr_Image();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x1F:
				bRepair = true;
				dwReturn = m_objISpyRepair.XorAla_Entry();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;		
			case 0x20:
				bRepair = true;
				dwReturn = m_objISpyRepair.ReplaceDataSize();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				if (dwReturn == 0x02)
				{
					bContinue = false;
				}
				break;
			case 0x21:
				bRepair = true;
				dwReturn = m_objISpyRepair.CalcCallAddr();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x22:
				bRepair = true;
				dwReturn = m_objISpyRepair.VirutEntryPt();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x23:
				bRepair = true;
				dwReturn = m_objISpyRepair.VirutCE();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			default:
				bRepair = false;
				break;
			case 0x24:
				bRepair = true;
				dwReturn = m_objISpyRepair.WriteElfanew();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x25:
				bRepair = true;
				dwReturn = m_objISpyRepair.JumpToOffset();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;

			case 0x26:
				bRepair = true;
				dwReturn = m_objISpyRepair.ByteXOR();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;

			case 0x27:
				bRepair = true;
				dwReturn = m_objISpyRepair.GetAEPFromLastSecName();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			
			case 0x28:
				bRepair = true;
				dwReturn = m_objISpyRepair.Savior();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x29:
				bRepair = m_objISpyRepair.FillWithNOP();
				break;

			case 0x2A:
				bRepair = true;
				dwReturn = m_objISpyRepair.ByteNOT();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x2B:
				bRepair = true;
				dwReturn = m_objISpyRepair.Ramlide();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x2C:
				bRepair = true;
				dwReturn = m_objISpyRepair.Adson();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;
			case 0x2D:
				bRepair = true;
				dwReturn = m_objISpyRepair.Priest_1521();
				if (dwReturn == 0x00)
				{
					bRepair = false;
					break;
				}
				break;

		}//end of switch

		if(!bContinue)
		{
			break;
		}

	}//end of while
	return bRepair;
}

/**********************************************************************************************************                     
*  Function Name  :	RepairFile                                                     
*  Description    :	Loads PE File & searches for expression in ISpyRepair.DB to repair the file 
*  SR.NO		  : VIRUSREPAIR_0001
*  Author Name    :	Prajakta                                                                                          
*  Date           : 7 Dec 2013
**********************************************************************************************************/
bool CISpyRepairDLLApp::RepairFile(LPCTSTR szFilePath, DWORD dwSpyID)
{
	bool bReturn = false;
	if(!m_objISpyRepair.OpenFile(szFilePath,true))
	{
		return bReturn;
	}
	if(!m_objISpyRepair.ReInitializeParam())
	{
		return bReturn;
	}

	if(!m_bDBLoaded)
	{
		if(!LoadISpyRepairDB())
		{
			return bReturn;
		}
	}
	if(FetchRepairExpr(dwSpyID))
	{
		bReturn = true;
	}
	m_objISpyRepair.CloseFile();
	
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	RepairFile                                                     
*  Description    :	EXPORT Function
*  Author Name    :	Prajakta                                                                                          
*  Date           : 7 Dec 2013
**********************************************************************************************************/
extern "C" bool DLLEXPORT RepairFile(LPCTSTR szFilePath, DWORD dwSpyID)
{
	return theApp.RepairFile(szFilePath, dwSpyID);
}

int CISpyRepairDLLApp::ExitInstance()
{
	return CWinApp::ExitInstance();
}
