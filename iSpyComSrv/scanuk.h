/*++

Copyright (c) 1999-2002  Microsoft Corporation

Module Name:

    scanuk.h

Abstract:

    Header file which contains the structures, type definitions,
    constants, global variables and function prototypes that are
    shared between kernel and user mode.

Environment:

    Kernel & user mode

--*/

#ifndef __SCANUK_H__
#define __SCANUK_H__

//
//  Name of port used to communicate
//
const PWSTR ScannerPortName = L"\\ScannerPort";


#define WLSRV_READ_BUFFER_SIZE   1024

typedef struct _WLSRV_NOTIFICATION {
    ULONG ulCallerProcessId;
    ULONG ulCallType;
    WCHAR wcAccessingProcessName[WLSRV_READ_BUFFER_SIZE];
    WCHAR usAccessedFileName[WLSRV_READ_BUFFER_SIZE];
} WLSRV_NOTIFICATION, *PWLSRV_NOTIFICATION;

typedef struct _SCANNER_REPLY {

    BOOLEAN SafeToOpen;
    
} SCANNER_REPLY, *PSCANNER_REPLY;

#endif //  __SCANUK_H__


