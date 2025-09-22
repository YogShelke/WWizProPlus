/************************************************************************************************
*  Program Name		: PtrStack.cpp
*  Description		: class definition for stack of pointers
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#pragma once
#include "BalBST.h"

typedef struct _tagPtrStack
{
	LPVOID lpPtr;
	struct _tagPtrStack* pNext;
}STACK_OF_PTRS, *LPSTACK_OF_PTRS;

class CPtrStack
{
public:
	CPtrStack();
	~CPtrStack();

	bool Push(LPVOID lpv);
	LPVOID Pop();
	bool IsEmpty();
	void RemoveAll();

private:

	LPSTACK_OF_PTRS		m_pTop;
};
