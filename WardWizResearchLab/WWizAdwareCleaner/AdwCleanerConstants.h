#ifndef _ADWARECLEANER_H
#define _ADWARECLEANER_H
#endif
#include "stdafx.h"

typedef struct __TAGSTRUCTSERVICES
{
	DWORD dwLocationCount;
	DWORD dwServicesCount;
}STRUCTSERVICES;

typedef struct __TAGSTRUCTFOLDERS
{
	DWORD dwLocationCount;
	DWORD dwFoldersCount;
}STRUCTFOLDERS;

typedef struct _TAGSTRUCTFILES
{
	DWORD dwLocationCount;
	DWORD dwFilesCount;
}STRUCTFILES;

typedef struct _TAGSTRUCTSHORTCUTS
{
	DWORD dwLocationCount;
	DWORD dwShortcutsCount;
}STRUCTSHORTCUTS;

typedef struct __TAGSTRUCTSCHEDULEDTASK
{
	DWORD dwLocationCount;
	DWORD dwScheduledTaskCount;
}STRUCTSCHEDULEDTASK;

typedef struct __TAGSTRUCTREEGISTRY
{
	DWORD dwLocationCount;
	DWORD dwRegistryCount;
}STRUCTREEGISTRY;

typedef struct __TAGSTRUCTBROWSERS
{
	DWORD dwLocationCount;
	DWORD dwBrowsersCount;
}STRUCTBROWSERS;

typedef struct __TAGSTRUCTBROWSERSREGISTRY
{
	DWORD dwLocationCount;
	DWORD dwBrowsersCount;
}STRUCTBROWSERSREGISTRY;

typedef struct __TAGSTRUCTRESERVED
{
	DWORD dwLocationCount;
	DWORD dwReserverd;
}STRUCTRESERVED;


typedef struct __TAGSTRUCTADWCLEANER
{
	STRUCTSERVICES			stServices;
	STRUCTFOLDERS			stFolders;
	STRUCTFILES				stFiles;
	STRUCTSHORTCUTS			stShortcuts;
	STRUCTSCHEDULEDTASK		stSheduledTasks;
	STRUCTREEGISTRY			stRegistry;
	STRUCTBROWSERS			stBrowsers;
	STRUCTBROWSERSREGISTRY	stBrowsersReg;
	STRUCTRESERVED			stReserved1;
	STRUCTRESERVED			stReserved2;
	STRUCTRESERVED			stReserved3;
}STRUCTADWCLEANER;
