/**********************************************************************************************************
Program Name			: WWizIndexer.cpp
Description				: This class is written to load/Unload and get function pointers of WRDWIZINDEXER.DLL
Author Name				: Ramkrushna Shelke
Date Of Creation		: 04/06/2016
Version No				: 1.14.1.10
Special Logic Used		:
Modification Log		:
1. Name					: Description
***********************************************************************************************************/
#pragma once
#include "stdafx.h"

//================================ FUNCTION PROTOTYPE ================================

typedef bool(*INSERTINTOWHITEDB) (TCHAR pDrive, UINT64 &uiCRCValue);
typedef bool(*ISWHITELISTED)	 (PBYTE pbData, size_t len, UINT64 &uiCRCValue);
typedef bool(*LOADLOCALDATABASE)	 ();
typedef bool(*SAVELOCALDATABASE)	 (); 
typedef bool(*CLEARINDEXES)	 ();

//==================================================================================== 

class CWWizIndexer
{
public:
	CWWizIndexer();
	virtual ~CWWizIndexer();
	void LoadIndexLibrary();
	bool InsertIntoWhiteDB(TCHAR pDrive, UINT64 &uiCRCValue);
	bool ISWhiteListed(PBYTE pbData, size_t len, UINT64 &uiCRCValue);
	bool LoadLocalDatabase();
	bool LoadLocalDatabaseSEH();
	bool SaveLocalDatabase();
	bool ClearIndedexes();
public:
	HMODULE				m_hModule;
	ISWHITELISTED		m_pIsWhiteListed;
	INSERTINTOWHITEDB	m_pInserIntoWhiteDB;
	LOADLOCALDATABASE	m_pLoadLocalDB;
	SAVELOCALDATABASE	m_pSaveLocalDB;
	CLEARINDEXES		m_pClearIndexes;
};

