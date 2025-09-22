#include "stdafx.h"
#include "imports.h"

HMODULE gIgnisDllModule = NULL;

// public exports
PFUNC_InitializeIgnis                    PubInitializeIgnis                     = NULL;
PFUNC_UninitializeIgnis                  PubUninitializeIgnis                   = NULL;
PFUNC_IgnisSetOpt                        PubIgnisSetOpt                         = NULL;
PFUNC_IgnisGetOpt                        PubIgnisGetOpt                         = NULL;
PFUNC_IgnisDeleteRule                    PubIgnisDeleteRule                     = NULL;
PFUNC_IgnisDeleteAllRules                PubIgnisDeleteAllRules                 = NULL;
PFUNC_IgnisCreateRuleFromTrafficInfo     PubIgnisCreateRuleFromTrafficInfo      = NULL;
PFUNC_IgnisAddRule                       PubIgnisAddRule                        = NULL;
PFUNC_IgnisReplaceRule                   PubIgnisReplaceRule                    = NULL;
PFUNC_IgnisSetLogFile                    PubIgnisSetLogFile                     = NULL;
PFUNC_IgnisSetTrafficLogFile             PubIgnisSetTrafficLogFile              = NULL;
PFUNC_IgnisGetProfileForAdapter          PubIgnisGetProfileForAdapter           = NULL;
PFUNC_IgnisSetProfileForAdapter          PubIgnisSetProfileForAdapter           = NULL;
PFUNC_IgnisGetCurrentSettings            PubIgnisGetCurrentSettings             = NULL;
PFUNC_IgnisUpdateSettings                PubIgnisUpdateSettings                 = NULL;
PFUNC_IgnisSetPortscanForAdapter         PubIgnisSetPortscanForAdapter          = NULL;
PFUNC_IgnisSetStealthForAdapter          PubIgnisSetStealthForAdapter           = NULL;
PFUNC_IgnisGetStealthForAdapter          PubIgnisGetStealthForAdapter           = NULL;
PFUNC_IgnisRegisterCallbacks             PubIgnisRegisterCallbacks              = NULL;
PFUNC_IgnisQueryCallbacks                PubIgnisQueryCallbacks                 = NULL;
PFUNC_IgnisComputeMD5                    PubIgnisComputeMD5                     = NULL;




#define GET_PROC_ADDR_AND_CHECK_ERROR(FuncImp,FuncDef,FuncName)                   \
                (FuncImp) = (FuncDef)GetProcAddress(gIgnisDllModule,(FuncName));      \
                if( NULL == (FuncImp) )                                             \
                { result = GetLastError();                                        \
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: GetProcAddress failed with result %d for function %s", result, (FuncName)); \
                  __leave; }

      

void FreeLibraryFunctions(
void
)
{
    if (NULL == gIgnisDllModule)
    {
        return;
    }

    FreeLibrary(gIgnisDllModule);
    gIgnisDllModule = NULL;
}

DWORD ResolveLibraryFunctions(
void
)
{
    DWORD result;

    result = ERROR_SUCCESS;

    __try
    {
		// Load ignis.dll from the current directory
        gIgnisDllModule = LoadLibraryW(L"ignis.dll");
        if (NULL == gIgnisDllModule)
        {
            result = GetLastError();
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: LoadLibraryW failed with result %d", result);
            __leave;
        }

        // start importing
        
        //
        // public
        //
        GET_PROC_ADDR_AND_CHECK_ERROR(PubInitializeIgnis, PFUNC_InitializeIgnis, "InitializeIgnis");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubUninitializeIgnis, PFUNC_UninitializeIgnis, "UninitializeIgnis");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisSetOpt, PFUNC_IgnisSetOpt, "IgnisSetOpt");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisGetOpt, PFUNC_IgnisGetOpt, "IgnisGetOpt");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisDeleteRule, PFUNC_IgnisDeleteRule, "IgnisDeleteRule");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisDeleteAllRules, PFUNC_IgnisDeleteAllRules, "IgnisDeleteAllRules");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisCreateRuleFromTrafficInfo, PFUNC_IgnisCreateRuleFromTrafficInfo, "IgnisCreateRuleFromTrafficInfo");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisAddRule, PFUNC_IgnisAddRule, "IgnisAddRule");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisReplaceRule, PFUNC_IgnisReplaceRule, "IgnisReplaceRule");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisSetLogFile, PFUNC_IgnisSetLogFile, "IgnisSetLogFile");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisSetTrafficLogFile, PFUNC_IgnisSetTrafficLogFile, "IgnisSetTrafficLogFile");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisGetProfileForAdapter, PFUNC_IgnisGetProfileForAdapter, "IgnisGetProfileForAdapter");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisSetProfileForAdapter, PFUNC_IgnisSetProfileForAdapter, "IgnisSetProfileForAdapter");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisGetCurrentSettings, PFUNC_IgnisGetCurrentSettings, "IgnisGetCurrentSettings");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisUpdateSettings, PFUNC_IgnisUpdateSettings, "IgnisUpdateSettings");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisSetPortscanForAdapter, PFUNC_IgnisSetPortscanForAdapter, "IgnisSetPortscanForAdapter");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisSetStealthForAdapter, PFUNC_IgnisSetStealthForAdapter, "IgnisSetStealthForAdapter");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisGetStealthForAdapter, PFUNC_IgnisGetStealthForAdapter, "IgnisGetStealthForAdapter");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisRegisterCallbacks, PFUNC_IgnisRegisterCallbacks, "IgnisRegisterCallbacks");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisQueryCallbacks, PFUNC_IgnisQueryCallbacks, "IgnisQueryCallbacks");
        GET_PROC_ADDR_AND_CHECK_ERROR(PubIgnisComputeMD5, PFUNC_IgnisComputeMD5, "IgnisComputeMD5");

    }
    __finally
    {
        if (ERROR_SUCCESS != result)
        {
            FreeLibraryFunctions();
        }
    }


    return result;
}