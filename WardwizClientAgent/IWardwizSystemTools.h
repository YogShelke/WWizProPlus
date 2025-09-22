#pragma once
//Contains Definitions for the Functions to be supported by the Adapter class.

#include <afxcoll.h>
#include "PipeConstants.h"

class IWardwizSystemTools
{
public:
	// Forbid copying
	IWardwizSystemTools(IWardwizSystemTools const &) = delete;
	IWardwizSystemTools & operator=(IWardwizSystemTools const &) = delete;

	//virtual destructor.
	virtual ~IWardwizSystemTools() = default;

	virtual BOOL StartRegistryOptimizer() = 0;
	
	virtual BOOL StopRegistryOptimizer() = 0;

	virtual BOOL PauseRegistryOptimizer() = 0;

	virtual DWORD Encrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus) = 0;
	
	virtual DWORD Decrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus) = 0;

	virtual BOOL EncryptSingleFile(CString csFilePath, CString csOutputPath, CString csOrgFileName) = 0;

	virtual BOOL DecryptSingleFile(CString csFilePath, CString csTmpPath, bool bTempRequired) = 0;

protected:
	// allow construction for child classes only
	IWardwizSystemTools()=default;

};