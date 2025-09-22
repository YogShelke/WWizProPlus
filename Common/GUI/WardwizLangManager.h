/**********************************************************************************************************              	  Program Name          : WardwizLangManager.h
	  Description           : Wrapper class to handle languages, and provide specific string from ini files.
							  
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 22nd Apr 2014
	  Version No            : 1.0.0.4
	  Special Logic Used    : Need to maintain last seletected language DWORD which is stopred in ini file
							  when user launches UI, it shows string from selected languages and shows on UI.
	  Modification Log      :           
	  1. Ramkrushna           Created Wrapper class for language support.
***********************************************************************************************************/
#pragma once

#define EMPTY_STRING _T("")

enum
{
	ENGLISH = 0,
	HINDI,
	GERMAN,
	CHINESE,
	SPANISH,
	FRENCH,
};

enum
{
	ESSENTIAL = 1,
	PRO,
	ELITE,
	BASIC,
	ESSENTIALPLUS,
};

class CWardwizLangManager
{
public:
	CWardwizLangManager(void);
	virtual ~CWardwizLangManager(void);
	CString GetString(CString csID);
	void InitializeVariables();
	void SetSelectedLanguage(DWORD dwLangID);
	DWORD GetSelectedLanguage();
	CString GetWardWizPathFromRegistry();
	void SetSelectedProductID(DWORD dwProductID);
	DWORD GetSelectedProductID();
public:
	CString		m_csiniFilePath;
	CString		m_csRegKeyPath;
};
