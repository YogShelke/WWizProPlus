#pragma once

#include "stdafx.h"
#include <Winbase.h>
#include <afxwin.h>

#pragma warning(disable : 4996)

/***************************************************************************
  Function Name  : OS_TYPE
  Description    : enum of os version
  Author Name    : Neha gharge
  Date           : 9th May 2014
****************************************************************************/

enum __OSTYPE
{
	WINOSUNKNOWN_OR_NEWEST = 0,
	WINOS_95,
	WINOS_98,
	WINOS_2000,
	WINOS_NT,
	WINOS_XP ,
	WINOS_VISTA,
	WINOS_WIN7,
	WINOS_WIN8,
	WINOS_WIN8_1,
	WINOS_XP64,
	WINOS_WIN10,
};



class CWardWizOSversion 
{
public:
	CWardWizOSversion(void);
	~CWardWizOSversion(void);

	CString	m_sStr;

	DWORD DetectClientOSVersion();
};