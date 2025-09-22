/****************************************************************************
Program Name          : NativeLogger.h
Description           : This file contains functionality related to display and logging mechanism.
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#include "NativeLogger.h"
#include "CommonFunctions.h"

#define LOG_FILE_NAME	L"VibraniumNTLOG.LOG"

HANDLE					hHandleLogFile = NULL;
WCHAR					DisplayBuffer[1024] = { 0 };
USHORT					LinePos = 0;
WCHAR					PutChar[2] = L" ";
UNICODE_STRING			CharString = { 2, 2, PutChar };
_RTL_CRITICAL_SECTION	m_rtlCSDisplay = { 0 };

/***********************************************************************************************
*  Function Name  : RtlCliPrintString
*  Description    : The RtlCliPrintString routine display a unicode string on the display device
					@param Message
					Pointer to a unicode string containing the message to print.
					@return STATUS_SUCCESS or failure code.
					@remarks None.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
NTSTATUS
RtlCliPrintString(IN PUNICODE_STRING Message)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	__try
	{
		ULONG i;
		//
		// Loop every character
		//
		for (i = 0; i < (Message->Length / sizeof(WCHAR)); i++)
		{
			//
			// Print the character
			//
			Status = RtlCliPutChar(Message->Buffer[i]);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlCliPrintString");
	}
	//
	// Return status
	//
	return Status;
}

/***********************************************************************************************
*  Function Name  : RtlCliPutChar
*  Description    : The RtlCliPutChar routine displays a character.
					@param Char
					Character to print out.
					@return STATUS_SUCCESS or failure code.
					@remarks None.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
NTSTATUS
RtlCliPutChar(IN WCHAR Char)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	__try
	{
		//
		// Initialize the string
		//
		CharString.Buffer[0] = Char;

		//
		// Check for overflow, or simply update.
		//
#if 0
		if (LinePos++ > 80)
		{
			//
			// We'll be on a new line. Do the math and see how far.
			//
			MessageLength = NewPos - 80;
			LinePos = sizeof(WCHAR);
		}
#endif

		//
		// Make sure that this isn't backspace
		//
		if (Char != '\r')
		{
			//
			// Check if it's a new line
			//
			if (Char == '\n')
			{
				//
				// Reset the display buffer
				//
				LinePos = 0;
				DisplayBuffer[LinePos] = UNICODE_NULL;
			}
			else
			{
				//
				// Add the character in our buffer
				//
				DisplayBuffer[LinePos] = Char;
				LinePos++;
			}
		}

		//
		// Print the character
		//
		Status = NtDisplayString(&CharString);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlCliPutChar");
	}
	return Status;
}

/***********************************************************************************************
*  Function Name  : RtlClipBackspace
*  Description    : @name RtlClipBackspace
					The RtlClipBackspace routine handles a backspace command.
					@param None.
					@return STATUS_SUCCESS or failure code if printing failed.
					@remarks Backspace is handled by printing the previous string minus the last
					two characters.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
NTSTATUS
RtlClipBackspace(VOID)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	__try
	{
		if (LinePos <= 0)
			return Status;

		UNICODE_STRING BackString;

		//
		// Update the line position
		//
		LinePos--;

		//
		// Finalize this buffer and make it unicode
		//
		DisplayBuffer[LinePos] = UNICODE_NULL;
		RtlInitUnicodeString(&BackString, DisplayBuffer);

		//
		// Display the buffer
		//
		Status = NtDisplayString(&BackString);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlClipBackspace, Buffer: [%s]", DisplayBuffer);
	}
	return Status;
}

/***********************************************************************************************
*  Function Name  : RtlCliDisplayString
*  Description    : @name RtlCliDisplayString
					The RtlCliDisplayString routine FILLMEIN
					@param Message
					FILLMEIN
					@param ... (Ellipsis)
					There is no set number of parameters.
					@return NTSTATUS
					@remarks Documentation for this routine needs to be completed.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
NTSTATUS
__cdecl
RtlCliDisplayString(LOGGING_LEVEL lel, IN PWCH Message, ...)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	__try
	{
		if (lel < LOGBASE)
		{
			return Status;
		}

		RtlEnterCriticalSection(&m_rtlCSDisplay);

		va_list MessageList;
		PWCHAR MessageBuffer;
		UNICODE_STRING MessageString;

		//
		// Allocate Memory for the String Buffer
		//
		MessageBuffer = (PWCHAR)RtlAllocateHeap(RtlProcessHeap(), 0, 2048);

		//
		// First, combine the message
		//
		va_start(MessageList, Message);
		_vsnwprintf(MessageBuffer, 2048, Message, MessageList);
		va_end(MessageList);

		//
		// Now make it a unicode string
		//
		RtlCreateUnicodeString(&MessageString, MessageBuffer);

		//
		// Display it on screen
		//
		Status = RtlCliPrintString(&MessageString);

		//
		// Free Memory
		//
		RtlFreeHeap(RtlProcessHeap(), 0, MessageBuffer);
		RtlFreeUnicodeString(&MessageString);

		RtlLeaveCriticalSection(&m_rtlCSDisplay);
		//
		// Return to the caller
		//
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlLeaveCriticalSection(&m_rtlCSDisplay);
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlCliDisplayString");
	}
	return Status;
}

/***********************************************************************************************
*  Function Name  : RtlOpenLogFile
*  Description    : Function which creates log file.
					@return true if SUCCESS
					@return false if FAILED
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
bool 
RtlOpenLogFile()
{
	bool bReturn = false;
	__try
	{
		WCHAR szLogFilePath[MAX_PATH] = { 0 };
		wcscpy(szLogFilePath, szAppPath);
		wcscat(szLogFilePath, L"\\");
		wcscat(szLogFilePath, L"LOG");

		if (!FileExists(szLogFilePath))
		{
			if (!NtFileCreateDirectory(szLogFilePath))
			{
				return bReturn;
			}
		}

		wcscat(szLogFilePath, L"\\");
		wcscat(szLogFilePath, LOG_FILE_NAME);
		
		bReturn = NtFileOpenFile(&hHandleLogFile, szLogFilePath, true, true, false) ? true : false;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : RtlCloseLogFile
*  Description    : Function which close log file handle
					@return true if SUCCESS
					@return false if FAILED
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
bool
RtlCloseLogFile()
{
	bool bReturn = false;
	__try
	{
		if (hHandleLogFile != NULL)
		{
			NtFileCloseFile(hHandleLogFile);
			hHandleLogFile = NULL;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlCloseLogFile\n");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : RtlCloseLogFile
*  Description    : @name RtlCliDisplayString
					The RtlCliDisplayString routine FILLMEIN
					@param Message
					@param ... (Ellipsis)
					There is no set number of parameters.
					@return NTSTATUS
					@remarks Documentation for this routine needs to be completed.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
NTSTATUS
__cdecl
RtlAddLogEntry(LOGGING_LEVEL lel, IN PWCH Message, ...)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	__try
	{
		//@check here logging level
		if (lel < LOGBASE)
		{
			return Status;
		}

		//@If log file handle is not opened with create log file.
		if (hHandleLogFile == NULL)
		{
			if (!RtlOpenLogFile())
			{
				return Status;
			}
		}

		va_list MessageList;
		PWCHAR MessageBuffer;

		//
		// Allocate Memory for the String Buffer
		//
		MessageBuffer = (PWCHAR)RtlAllocateHeap(RtlProcessHeap(), 0, MAX_FILEPATH_LENGTH * 2);
		if (!MessageBuffer)
			return Status;

		WCHAR wszLine[MAX_FILEPATH_LENGTH * 2] = { 0 };
	
		//
		// First, combine the message
		//
		va_start(MessageList, Message);
		_vsnwprintf(MessageBuffer, MAX_FILEPATH_LENGTH * 2, Message, MessageList);
		va_end(MessageList);

		wcscat(wszLine, MessageBuffer);
		wcscat(wszLine, L"\r\n");
		
		DECLARE_UNICODE_STRING_SIZE(unicodeString, MAX_FILEPATH_LENGTH * 2);
		SetUnicodeString(&unicodeString, wszLine);

		ANSI_STRING ansString;
		RtlUnicodeStringToAnsiString(&ansString, &unicodeString, TRUE);
		
		DWORD dwWritten = 0x00;
		if (NtFileWriteFile(hHandleLogFile, ansString.Buffer, (DWORD)strlen(ansString.Buffer), &dwWritten))
		{
			Status = STATUS_SUCCESS;
		}
		
		RtlFreeAnsiString(&ansString);

		//
		// Free Memory
		//
		RtlFreeHeap(RtlProcessHeap(), 0, MessageBuffer);
		//
		// Return to the caller
		//
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return Status;
}

/***********************************************************************************************
*  Function Name  : DisplayString
*  Description    : Function which displays string to Native window console.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
void DisplayString(PWSTR pString, LOGGING_LEVEL lLvl)
{
	__try
	{
		if (lLvl < LOGBASE)
		{
			return;
		}

		UNICODE_STRING unicodeBuffer;
		RtlInitUnicodeString(&unicodeBuffer, pString);
		NtDisplayString(&unicodeBuffer);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/***********************************************************************************************
*  Function Name  : InitializeNativeLogger
*  Description    : Function which does required initialization.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  05-Dec-2017
*************************************************************************************************/
bool InitializeNativeLogger()
{
	__try
	{
		RtlInitializeCriticalSection(&m_rtlCSDisplay);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return true;
}
