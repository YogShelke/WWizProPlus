/****************************************************************************
Program Name          : NativeLogger.h
Description           : This file contains functionality related to display and logging mechanism.
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#pragma once
#include <time.h> 
#include "Header.h"
#include "WardWizNTFile.h"

NTSTATUS RtlCliPrintString(IN PUNICODE_STRING Message);
NTSTATUS RtlCliPutChar(IN WCHAR Char);
NTSTATUS RtlClipBackspace(VOID);
NTSTATUS _cdecl RtlCliDisplayString(LOGGING_LEVEL lel, IN PWCH Message, ...);
NTSTATUS _cdecl RtlAddLogEntry(LOGGING_LEVEL lel, PWCH Message, ...);
bool RtlOpenLogFile();
bool RtlCloseLogFile();
void DisplayString(PWSTR pString, LOGGING_LEVEL lLvl = SECONDLEVEL);
bool InitializeNativeLogger();
