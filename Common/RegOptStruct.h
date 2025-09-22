
#ifndef		ISPYAVREGOPT_STRUCT_
#define		ISPYAVREGOPT_STRUCT_


///////////////////////////////////////////////////////////////
//		REGOPTSCANOPTIONS structure is used while Scanning & Repairing Registry Entries
//		This job will executed by Main GUI
//		Created date	: 12-Nov-2013 10:15PM
//		Created By		: Vilas
//
//		Modified		:1. 16-Nov-2013 10:15PM, Added for repair entry for each 
///////////////////////////////////////////////////////////////



typedef struct _REGOPTSCANOPTIONS
{
	
	bool	bActiveX ;					//Check Invalid ActiveX Entries. 0->Not, 1->Scan & repair
	bool	bUninstall ;
	bool	bFont ;
	bool	bSharedLibraries ;
	bool	bApplicationPaths ;
	bool	bHelpFiles ;
	bool	bStartup ;
	bool	bServices ;
	bool	bExtensions ;
	bool	bRootKit ;
	bool	bRogueApplications ;
	bool	bWorm ;
	bool	bSpywares ;
	bool	bAdwares ;
	bool	bKeyLogger ;
	bool	bBHO ;
	bool	bExplorer ;
	bool	bIExplorer ;

	DWORD	dwStats[0x12] ;			//Count for repair for each entry

}REGOPTSCANOPTIONS, *LPREGOPTSCANOPTIONS ;

#endif