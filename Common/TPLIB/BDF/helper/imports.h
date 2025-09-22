#pragma once
#ifndef _IMPORTS_H_
#define _IMPORTS_H_

/** 
	File: imports.h
	Description: Contains the definitions of the functions needed for loading/unloading the API
*/


/**
	Unloads the ignis.dll 
*/
void FreeLibraryFunctions(
void
);

/**
	Loads the ignis.dll and retrieves the addresses of the exported functions
*/
DWORD ResolveLibraryFunctions(
void
);

extern HMODULE gIgnisDllModule;

// public exports
extern PFUNC_InitializeIgnis                    PubInitializeIgnis;
extern PFUNC_UninitializeIgnis                  PubUninitializeIgnis;
extern PFUNC_IgnisSetOpt                        PubIgnisSetOpt;
extern PFUNC_IgnisGetOpt                        PubIgnisGetOpt;
extern PFUNC_IgnisDeleteRule                    PubIgnisDeleteRule;
extern PFUNC_IgnisDeleteAllRules                PubIgnisDeleteAllRules;
extern PFUNC_IgnisCreateRuleFromTrafficInfo     PubIgnisCreateRuleFromTrafficInfo;
extern PFUNC_IgnisAddRule                       PubIgnisAddRule;
extern PFUNC_IgnisReplaceRule                   PubIgnisReplaceRule;
extern PFUNC_IgnisSetLogFile                    PubIgnisSetLogFile;
extern PFUNC_IgnisSetTrafficLogFile             PubIgnisSetTrafficLogFile;
extern PFUNC_IgnisGetProfileForAdapter          PubIgnisGetProfileForAdapter;
extern PFUNC_IgnisSetProfileForAdapter          PubIgnisSetProfileForAdapter;
extern PFUNC_IgnisGetCurrentSettings            PubIgnisGetCurrentSettings;
extern PFUNC_IgnisUpdateSettings                PubIgnisUpdateSettings;
extern PFUNC_IgnisSetPortscanForAdapter         PubIgnisSetPortscanForAdapter;
extern PFUNC_IgnisSetStealthForAdapter          PubIgnisSetStealthForAdapter;
extern PFUNC_IgnisGetStealthForAdapter          PubIgnisGetStealthForAdapter;
extern PFUNC_IgnisRegisterCallbacks             PubIgnisRegisterCallbacks;
extern PFUNC_IgnisQueryCallbacks                PubIgnisQueryCallbacks;
extern PFUNC_IgnisComputeMD5                    PubIgnisComputeMD5;


#endif // _IMPORTS_H_