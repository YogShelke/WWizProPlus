#include "stdafx.h"
#include "WardwizOSVersion.h"


/***************************************************************************
  Function Name  : CWardWizOSversion
  Description    : C'tor
  Author Name    : Neha gharge
  Date           : 9th May 2014
****************************************************************************/
CWardWizOSversion::CWardWizOSversion(void)
{
}

/***************************************************************************
  Function Name  : ~CWardWizOSversion
  Description    : D'tor
  Author Name    : Neha gharge
  Date           : 9th May 2014
****************************************************************************/
CWardWizOSversion::~CWardWizOSversion(void)
{
}

/***************************************************************************
  Function Name  : DetectClientOSVersion
  Description    : Return client machine os version
  Author Name    : Neha gharge
  Date           : 9th May 2014
****************************************************************************/
DWORD CWardWizOSversion::DetectClientOSVersion() 
{

	OSVERSIONINFO OSversion;

	OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);

	::GetVersionEx(&OSversion);

	switch(OSversion.dwPlatformId)
	{
	case VER_PLATFORM_WIN32s: 
		m_sStr.Format(L"Windows %d.%d",OSversion.dwMajorVersion,
			OSversion.dwMinorVersion);
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		if(OSversion.dwMinorVersion==0)
			return WINOS_95;
		else
		if(OSversion.dwMinorVersion==10)  
			return WINOS_98;
		else
		if(OSversion.dwMinorVersion==90)  
			return WINOSUNKNOWN_OR_NEWEST;
     
	case VER_PLATFORM_WIN32_NT:
		if(OSversion.dwMajorVersion==5 && OSversion.dwMinorVersion==0)
			return WINOS_2000;
		else 
		if(OSversion.dwMajorVersion==5 && OSversion.dwMinorVersion == 1)  
			return WINOS_XP;
		else	
			if(OSversion.dwMajorVersion<=4) 
			return WINOS_NT;
		else	
			if(OSversion.dwMajorVersion==6 && OSversion.dwMinorVersion==0)
				return WINOS_VISTA;
		else	
			if(OSversion.dwMajorVersion==6 && OSversion.dwMinorVersion==1)
				return WINOS_WIN7;
		else	
			if(OSversion.dwMajorVersion==6 && OSversion.dwMinorVersion==2)
				return WINOS_WIN8;
		else
			if(OSversion.dwMajorVersion==6 && OSversion.dwMinorVersion==3)
				return WINOS_WIN8_1;
		else
			if(OSversion.dwMajorVersion==5 && OSversion.dwMinorVersion==2) // solved by lalit ,issue  handle the xp64 Os version
				return WINOS_XP64;
		else
			if (OSversion.dwMajorVersion == 10 && OSversion.dwMinorVersion == 0) //for Windows 10
				return WINOS_WIN10;
		else
			return WINOSUNKNOWN_OR_NEWEST;
								
	default:	
		return WINOSUNKNOWN_OR_NEWEST;


	}

return 0;

}
