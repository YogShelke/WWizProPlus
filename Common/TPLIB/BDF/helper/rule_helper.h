#pragma once
#ifndef _RULE_HELPER_H_
#define _RULE_HELPER_H_

/** 
	File: rule_helper.h
	Description: Contains the helper functions needed for manipulating the ignis rules
*/

extern "C"
{
    #include "ptstatus.h"
}
#include "ignis_dll.h"

/**
	Populates a rule with information from the cmd parameters
*/
PTSTATUS CreateIgnisRuleFromCmd(
    __in    char**          CommandLine,
    __in    int             CommandArgs,
    __in    LONG*           ParameterIndex,
    __out   PIGNIS_RULE     Rule
);

/**
	Deletes a rule from memory
*/
PTSTATUS FreeClientRule(
    __in    PIGNIS_RULE     Rule
);

#endif // _RULE_HELPER_H_