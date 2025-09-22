// ////////////////////////////////////////////
// 
// Date : 2005/06/02
// Programmer : Behzad Bahjat manesh (Iranian);
//
//
// TurnOffComputer.cpp
// ////////////////////////////////////////////
#include "stdafx.h"
#include "TurnOffComputer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CTurnOffComputer::CTurnOffComputer()
{

}
CTurnOffComputer::~CTurnOffComputer()
{

}

// //////////////////////////////////////////


void CTurnOffComputer::Active(int set)
{

	//*****************

								HANDLE hToken; 
								TOKEN_PRIVILEGES tkp; 
	
	if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
	{
						LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
						tkp.PrivilegeCount = 1; 
		                tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
						if(AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
						{

						if(set==0)//Shut down
								ExitWindowsEx(EWX_SHUTDOWN|EWX_POWEROFF|EWX_FORCE,0);

							else if(set==1)//Restart
						ExitWindowsEx(EWX_REBOOT|EWX_FORCE,0);

							else if(set==2)//Log Off
								ExitWindowsEx(EWX_LOGOFF|EWX_FORCE,0);
							else if(set==3)//Stand By
							SetSystemPowerState( TRUE, TRUE);

							else if(set==4)//Hibernate
							SetSystemPowerState( FALSE, FALSE );
						}


	}
//***********************************
}