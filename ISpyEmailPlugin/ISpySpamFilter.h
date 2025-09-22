/**********************************************************************************************************      		Program Name          : ISpySpamFilter.h
	Description           : This class which provides functionality for spam filter functionality.
	Author Name			  : Ramkrushna Shelke                                                                  	  
	Date Of Creation      : 20th Aug 2014
	Version No            : 1.0.0.8
	Special Logic Used    : 
	Modification Log      :           
***********************************************************************************************************/
#pragma once
#include "stdafx.h"
#include "ISpyDataManager.h"

class CISpySpamFilter
{
public:
	CISpySpamFilter(void);
	virtual ~CISpySpamFilter(void);
	bool LoadSendersEmailAddressDB(CString csPathName);
	DWORD FilterSenderEmailAddress(CString csSenderEmailAddress);
public:
	bool									m_bSpamFilterSettingLoaded;
private:
	CDataManager							m_objSpamFilterdb;
	std::map<CString, bool>					m_objSendersAddresses;
};
