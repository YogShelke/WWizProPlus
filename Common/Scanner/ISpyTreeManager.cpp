#include "StdAfx.h"
#include "ISpyTreeManager.h"
#include <algorithm>

CISpyTreeManager::CISpyTreeManager(void)
{
	//Init the root node
	m_aRoot.pFailureNode	= NULL;
	m_aRoot.aChar			= 0;
	m_aRoot.bFinal			= false;
	m_aRoot.usDepth			= 0;
 	memset(m_szDBFilePath, 0, sizeof(m_szDBFilePath));
	m_bReScan				= false;
}

CISpyTreeManager::~CISpyTreeManager(void)
{
	//Delete the tree
	ClearTree();
}

bool CISpyTreeManager::ClearTree()
{
	return DeleteNode(&m_aRoot);
}

bool CISpyTreeManager::DeleteNode(Node* pNode)const
{
	bool bReturn = false;
	__try
	{
		bReturn = DeleteNodeSEH(pNode);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizTreeManager::DeleteNode", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

bool CISpyTreeManager::DeleteNodeSEH(Node* pNode)const
{
	try
	{
		//Make sure we have it
		if (!pNode)
			return false;

		//Iterate all its children
		for (SearchMap::iterator aIterator = pNode->aMap.begin(); aIterator != pNode->aMap.end(); aIterator++)
		{
			//Send it to deletion
			DeleteNode(aIterator->second);

			//We can assume all the children have been deleted, then delete it
			delete aIterator->second;
			aIterator->second = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTreeManager::DeleteNodeSEH", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

bool CISpyTreeManager::AddString(char * szSignature)
{
	bool bReturn = false;
	__try
	{
		bReturn = AddStringSEH(szSignature);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizTreeManager::AddString", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

bool CISpyTreeManager::AddStringSEH(char * szSignature)
{
	try
	{
		if (!szSignature)
			return false;

		char			szSig[1024] = { 0 };
		char			szChr[3] = { 0 };
		char			*szDummy = NULL;
		unsigned int	iChar = 0;
		Node			*pNode = &m_aRoot;
		Node			*pNewNode = NULL;

		strcpy_s(szSig, 1024, szSignature);
		int iSigLen = static_cast<int>(strlen(szSig));

		if (iSigLen == 0)
			return false;

		for (int iIndex = 0; iIndex < iSigLen; iIndex += 2)
		{
			szChr[0] = szSig[iIndex];
			szChr[1] = szSig[iIndex + 1];
			szChr[2] = '\0';
			iChar = strtol(szChr, &szDummy, 0x10);

			//Need to build this node
			pNewNode = new Node;
			if (pNewNode == NULL)
			{
				//Failed to allocate memory
				return false;
			}

			if (pNode == NULL)
			{
				//Reset it
				pNewNode->pFailureNode = NULL;
				pNewNode->aChar = iChar;
				pNewNode->usDepth = pNode->usDepth + 1;
				pNewNode->bFinal = false;
				//Add it
				pNode->aMap.insert(SearchMap::value_type(iChar, pNewNode));
			}
			else
			{
				//Look for the next node
				SearchMap::iterator aIterator;
				aIterator = pNode->aMap.find(iChar);

				//Do we have it?
				if (aIterator == pNode->aMap.end())
				{
					//Reset it
					pNewNode->pFailureNode = NULL;
					pNewNode->aChar = iChar;
					pNewNode->usDepth = pNode->usDepth + 1;
					pNewNode->bFinal = false;

					//Add it
					pNode->aMap.insert(SearchMap::value_type(iChar, pNewNode));
				}
				else
				{
					//TODO: Ram Shelke
					//Handled Memory leak.
					//De-Allocate the memory here as we are using already existing memory for one node .
					//Then need to remove allocated memory and use existing.
					if (pNewNode != NULL)
					{
						delete pNewNode;
						pNewNode = NULL;
					}
					//Take the node
					pNewNode = aIterator->second;
				}
			}
			pNode = pNewNode;
		}
		if (pNewNode != NULL)
		{
			pNewNode->bFinal = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTreeManager::AddStringSEH", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

bool CISpyTreeManager::BuildTreeIndex()
{
	//Build index on the root
	unsigned char szTemp[2] = "";
	return BuildIndex(szTemp, &m_aRoot);
}

bool CISpyTreeManager::BuildIndex(unsigned char * szSig, Node* pNode)
{
	try
	{
		//Sanity
		if (!pNode)
			return false;

		//Do we need to process this node?
		if (pNode->usDepth > 1)
		{
			//Clear the node first
			pNode->pFailureNode = NULL;

			unsigned char szSearchStr[1024] = { 0 };
			strcpy((char*)szSearchStr, (char*)szSig);

			unsigned char *szPtr = szSearchStr;
			szPtr += 2;

			while (strlen((char*)szPtr) != 0)
			{
				//And search
				Node* pFoundNode;
				pFoundNode = SearchNode(szPtr, &m_aRoot);

				//Did we get it?
				if (pFoundNode)
				{
					//Save it
					pNode->pFailureNode = pFoundNode;
					//Exit from this loop
					break;
				}
				szPtr += 2;
			}
		}

		unsigned char szBuildSig[1024] = { 0 };
		//Iterate all its children
		for (SearchMap::iterator aIterator = pNode->aMap.begin(); aIterator != pNode->aMap.end(); ++aIterator)
		{
			char szChar[3] = { 0 };
			unsigned char szTemp = aIterator->first;
			sprintf(szChar, "%02X", szTemp);
			strcpy((char*)szBuildSig, (char *)szSig);
			strcat((char*)szBuildSig, szChar);

			BuildIndex(szBuildSig, aIterator->second);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTreeManager::BuildIndex", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

Node* CISpyTreeManager::SearchNode(unsigned char * szString, Node* pNode)
{
	try
	{
		//Sanity check
		if (!pNode || !szString)
			return NULL;

		//The char we are looking for
		unsigned int	iChar = 0;
		char			szChr[3] = { 0 };

		szChr[0] = szString[0];
		szChr[1] = szString[1];
		szChr[2] = '\0';

		char *szDummy = NULL;
		iChar = strtol(szChr, &szDummy, 0x10);

		//Look for the next node
		SearchMap::iterator aIterator;
		aIterator = pNode->aMap.find(iChar);

		//Do we have it?
		if (aIterator != pNode->aMap.end())
		{
			//Is it the last char?
			if (strlen((char*)szString) == 2)
				//We found our string
				return aIterator->second;
			else
			{
				szString += 2;
				//Search again
				return SearchNode(szString, aIterator->second);
			}
		}
		else
			//Not found
			return NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTreeManager::SearchNode", 0, 0, true, SECONDLEVEL);
	}
	return NULL;
}

bool CISpyTreeManager::ScanBuffer(unsigned char *szBuffer, unsigned int iBufferLen, char * pszVirusName, DWORD &dwSpyID, bool bRescan)
{
	bool bReturn = false;
	__try
	{
		bReturn = ScanBufferSEH(szBuffer, iBufferLen, pszVirusName, dwSpyID, bRescan);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizTreeManager::ScanBuffer", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CISpyTreeManager::ScanBufferSEH(unsigned char *szBuffer, unsigned int iBufferLen, char * pszVirusName, DWORD &dwSpyID, bool bRescan)
{
	try
	{
		unsigned int iCnt = 0;
		MatchedStringFoundVect vtMatchedString;
		char szMatchString[1024] = {0};
		unsigned int iMatchCount = 0;

		//Our node position
		const Node* pNode;
		pNode = &m_aRoot;
		if(!pNode)
		{
			return false;
		}
		//Iterate the string
		for (unsigned int iCount = 0; iCount < iBufferLen; ++iCount)
		{
			//Did we switch node already
			bool bSwitch;
			bSwitch = false;

			//Loop while we got something
			while (1)
			{
				//Look for the char
				SearchMap::const_iterator aIterator;
				aIterator = pNode->aMap.find(szBuffer[iCount]);

				//Do we have it?
				if (aIterator == pNode->aMap.end())
				{
					//No, check if we have failure node
					if (!pNode->pFailureNode)
					{
						//No failure node, start at root again
						pNode = &m_aRoot;

						//Reset search string
						//sMatchedString=L"";
						strcpy(szMatchString, "");

						if(iMatchCount > 0)
						{
							//here is check for any match sequence breaks then need to go back to root node.
							iCount -= iMatchCount;
							iMatchCount = 0;
						}
						//Did we do a switch?
						else if (bSwitch)
							//We need to do this over
							--iCount;

						//Exit this loop
						break;
					}
					else
					{
						//What is the depth difference?
						unsigned short usDepth;
						usDepth = pNode->usDepth-pNode->pFailureNode->usDepth-1;

						//This is how many chars to remove
						//sMatchedString=sMatchedString.substr(usDepth,sMatchedString.length()-usDepth);

						//Go to the failure node
						pNode = pNode->pFailureNode;

						//Set to switch
						bSwitch = true;
					}
				}
				else
				{
					//Add the char
					//sMatchedString+=rString[iCount];
					char szHex[3] = {0};
					sprintf(szHex, "%02X", szBuffer[iCount]);
					strcat(szMatchString, szHex);

					//Save the new node
					pNode=aIterator->second;

					//make the the flag for match sequence.
					iMatchCount++;

					//Exit the loop
					break;
				}
			}

			//Is this a final node?
			if (pNode->bFinal)
			{
				//We got our data
				vtMatchedString.push_back(std::string(szMatchString));
				//strcat(szFullMatchedSignature, szMatchString);
				//strcat(szFullMatchedSignature, "*");
				//Exit
				continue;
				//return true;
			}
		}

		if(vtMatchedString.size() > 0)
		{
			//Remove duplicates
			sort( vtMatchedString.begin(), vtMatchedString.end() );
			vtMatchedString.erase( unique( vtMatchedString.begin(), vtMatchedString.end() ), vtMatchedString.end() );

			if(GetMatchedSignatureCount(vtMatchedString) >= 2)
			{
				DWORD dwSpyIdDup = 0;
				char szVirusName[100] = {0};
				if(bRescan)
				{
					strcpy(szVirusName, pszVirusName);
					dwSpyIdDup = dwSpyID;
				}
				if(IsPartsMatched(vtMatchedString, szVirusName, dwSpyIdDup, bRescan))
				{
					dwSpyID = dwSpyIdDup;
					strcpy(pszVirusName, szVirusName);
					return true;
				}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizTreeManager::ScanBufferSEH", 0, 0, true, SECONDLEVEL);
		return false;
	}
	//Nothing found
	return false;
}



bool CISpyTreeManager::IsPartsMatched(MatchedStringFoundVect &vtMatchedString, char * pszVirusName, DWORD &dwSpyID, bool bRescan)
{	
	bool bReturn = false;
	try
	{
		if(vtMatchedString.size() == 0)
			return bReturn;

		if(m_szDBFilePath && _tcslen(m_szDBFilePath) == 0)
		{
			return bReturn;
		}

		/*if(!PathFileExists(m_szDBFilePath))
		{
			return false;
		}*/
		//check for rescan entry
		
		/*Added by Prajakta: To handle file re-scanning */
		CString csVirusNameWithSpyID = L"";
		if(IsReScanEntry(pszVirusName, dwSpyID))
		{
			const size_t newsize = 100;
			size_t convertedChars = 0;
			wchar_t wcstring[newsize];
			mbstowcs_s(&convertedChars, wcstring, strlen(pszVirusName) + 1, pszVirusName, _TRUNCATE);
			csVirusNameWithSpyID.Format(L"%s_%d", wcstring, dwSpyID);
		}
		CIspyList contact;
		ContactList * pCtlRescanContacts = new ContactList();
		if(pCtlRescanContacts == NULL)
		{
			//memory allocation failed.
			return bReturn;
		}

		//Check for specific detection signature
		if(bRescan)
		{
			if(!m_objWrdwizEncDecManager->FindContact(csVirusNameWithSpyID, contact))
			{
				if(pCtlRescanContacts != NULL)
				{
					delete pCtlRescanContacts;
					pCtlRescanContacts = NULL;
				}
			 	return bReturn;
			}
			pCtlRescanContacts->AddTail(contact);
		}
		else
		{
			 const ContactList& ctlContacts = m_objWrdwizEncDecManager->GetContacts();
			 if(pCtlRescanContacts != NULL)
			 {
				delete pCtlRescanContacts;
				pCtlRescanContacts = NULL;
			 }
			 pCtlRescanContacts = (ContactList*)&ctlContacts;
		}

		ContactList& contacts = *pCtlRescanContacts;
		//const ContactList& contacts = m_objWrdwizEncDecManager->GetContacts();
		
		
		// iterate over all contacts & add them to the list
		POSITION pos = contacts.GetHeadPosition();
		while(pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
		
			CString csFirstEntry = contact.GetFirstEntry();		
			CString csSecondEntry =  contact.GetSecondEntry();

		/*FILE *pFile = NULL;
		pFile = _wfsopen(m_szDBFilePath, _T("r"), 0x20);

		if(!pFile)
		{
			return false;
		}

		char buff[5000]; 
		CString csLine, csTmp;
		int iPos = 0;*/
		//while(!feof(pFile))
		//{	
			/*if(fgets(buff,5000,pFile))
			{*/
				char	szSigLine[1024] = {0};
				//strcpy(szSigLine, buff);
				
				char szSigOnly[1024] = {0};
				char szVirusName[100] = {0};
				wcstombs(szSigOnly, csSecondEntry.GetBuffer(), sizeof(szSigOnly) + 1);
				//char *szTemp = strchr(szSigLine, '=');
				/*if(szTemp)
				{
					szTemp++;
					strcpy(szSigOnly, szTemp);
					szTemp--;
					szTemp[0] = '\0';
					*/
					//To get count of parts in signature
					char szSigTmp[1024] = {0};
					strcpy(szSigTmp,szSigOnly);
					DWORD dwSigPartsCnt = 0x0;	
					char delim[]   = "*";
					char *token = NULL;
					token = strtok( szSigTmp, delim );
						
					while(token != NULL)
					{
						trim(token, static_cast<int>(strlen(token)));
						dwSigPartsCnt++;
						token = strtok( NULL, delim );
					}
					
					char szSpyID[10] = {0};
					wcstombs(szSigLine, csFirstEntry.GetBuffer(), sizeof(szSigLine) + 1);
					char *szTempName = strrchr(szSigLine, '_');
					if(!szTempName)
					{
						dwSpyID = 0;	
						strcpy(pszVirusName, szSigLine);
					}
					else
					{
						szTempName++;
						strcpy(szSpyID, szTempName);
						szTempName--;
						szTempName[0] = '\0';
						strcpy(pszVirusName, szSigLine);

						if(strlen(szSpyID) != 0)
						{
							dwSpyID =  static_cast<DWORD>(atoi(szSpyID));
						}
					}

					DWORD dwMatchCount = 0;

					//Ram: New logic to improve performance
					char szSig2Token[1024] = { 0 };
					strcpy(szSig2Token, szSigOnly);
					token = strtok(szSig2Token, delim);

					if (token != NULL)
					{
						for (int iIndex = 0; iIndex < static_cast<int>(vtMatchedString.size()); iIndex++)
						{
							const char *szSingleString = vtMatchedString.at(iIndex).c_str();
							trim(token, static_cast<int>(strlen(token)));
							if (token[0] == szSingleString[0])
							{
								if (strcmp(token, szSingleString) == 0)
								{
									dwMatchCount++;
								}
							}
						}

						token = strtok(NULL, delim);

						while (token != NULL)
						{
							if (dwMatchCount )
							{
								for (int iIndex = 0; iIndex < static_cast<int>(vtMatchedString.size()); iIndex++)
								{
									const char *szSingleString = vtMatchedString.at(iIndex).c_str();
									trim(token, static_cast<int>(strlen(token)));
									if (token[0] == szSingleString[0])
									{
										if (strcmp(token, szSingleString) == 0)
										{
											dwMatchCount++;
										}
									}
								}
							}

							//Means second part is not matched, break with part matching.
							if (dwMatchCount == 1)
							{
								break;
							}

							token = strtok(NULL, delim);
						}
					}

					if((dwMatchCount == dwSigPartsCnt) && (dwSigPartsCnt > 2 && dwMatchCount > 2))
					{
						bReturn = true;
						break;
					}
		}

		if(pCtlRescanContacts != NULL && bRescan)
		{
			delete pCtlRescanContacts;
			pCtlRescanContacts = NULL;
		}

		//}
		//fclose(pFile);
	}
	catch(...)
	{

	}
	return bReturn;
}

int CISpyTreeManager::GetMatchedSignatureCount(MatchedStringFoundVect &vtMatchedString)
{
	return static_cast<int>(vtMatchedString.size());
	//if(!vtMatchedString.size() == 0)
	//	return 0;

	//for (int iCount=0; iCount < vtMatchedString.size(); ++iCount)
	//{
	//	printf("%S %i\n", aDataFound[iCount].sDataFound.c_str(), aDataFound[iCount].iFoundPosition);
	//}

	//unsigned int iCount = 1;
	//for(int iIndex = 0; iIndex < strlen(pszMatchedString); iIndex++)
	//{
	//	if(pszMatchedString[iIndex] == '*')
	//	{
	//		iCount++;
	//	}
	//}
	//return iCount;
}

bool CISpyTreeManager::SetPESignatureFilePath(LPTSTR pszFilePath)
{
	if(!pszFilePath)
		return false;

	_tcscpy(m_szDBFilePath, pszFilePath);

	return true;
}

bool CISpyTreeManager::CopyDataByRef(CDataManager *objrdwizEncDecManager)
{
	if(!objrdwizEncDecManager)
	{
		return false;
	}
	m_objWrdwizEncDecManager = objrdwizEncDecManager;
	return true;
}

bool CISpyTreeManager::IsReScanEntry(char *pszVirusName, DWORD dwSpyID)
{
	bool bReturn = false;
	if(strlen(pszVirusName) != 0 && dwSpyID >= 0)
	{
		bReturn = true;
	}
	return bReturn;
}