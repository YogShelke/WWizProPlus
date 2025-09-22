/********************************************************************************************************** 
   Program Name          : CommonFunctions.h
   Description           : Wrapper class to use common functions in product.
						   This class helps to maintain all commonly used functions.
   Author Name           : Niranjan Deshak
   Date Of Creation      : 1/31/2015
   Version No            : 1.8.3.0
   Special Logic Used    : 
   Modification Log      :           
   1. Name -Niranjan Deshak. - 2 feb 2015.
						: Description - Member object for CWardwizLangManager is added for purpose of Compairing icon caption from INI file. 
***********************************************************************************************************/
#pragma once
#include "WardwizLangManager.h"

class CCommonFunctions
{
public:
	CCommonFunctions(void);
	virtual ~CCommonFunctions(void);
public:
	CWardwizLangManager     m_objwardwizLangManager;
	void RefreshTaskbarNotificationArea();
};
