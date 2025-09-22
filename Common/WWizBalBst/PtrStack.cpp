/************************************************************************************************
*  Program Name		: PtrStack.cpp
*  Description		: class definition for stack of pointers
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#include "stdafx.h"
#include "PtrStack.h"

/****************************************************************************************************
Function       : CPtrStack
Description    : constructor
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
CPtrStack::CPtrStack()
{
	m_pTop = NULL;
}

/****************************************************************************************************
Function       : CPtrStack
Description    : destructor
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
CPtrStack::~CPtrStack()
{
	RemoveAll();
}

/****************************************************************************************************
Function       : Push
Description    : push one item on stack
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CPtrStack::Push(LPVOID lpv)
{
	LPSTACK_OF_PTRS pNew = NULL;

	pNew = (LPSTACK_OF_PTRS) Allocate(sizeof(STACK_OF_PTRS));
	if(!pNew)
	{
		return false;
	}

	pNew->lpPtr = lpv;
	pNew->pNext = m_pTop;
	m_pTop = pNew;
	return true;
}

/****************************************************************************************************
Function       : Push
Description    : pop one item from stack
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
LPVOID CPtrStack::Pop()
{
	LPVOID lpv = NULL;

	if(m_pTop)
	{
		LPSTACK_OF_PTRS pHold = m_pTop;
		lpv = m_pTop->lpPtr;
		m_pTop = m_pTop->pNext;
		Release((LPVOID&)pHold);
	}

	return lpv;
}

/****************************************************************************************************
Function       : IsEmpty
Description    : return true if stack is empty else false
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CPtrStack::IsEmpty()
{
	return (m_pTop == NULL);
}

/****************************************************************************************************
Function       : IsEmpty
Description    : cleanup all stack occupied memory
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void CPtrStack::RemoveAll()
{
	LPSTACK_OF_PTRS pHold = m_pTop;

	while(m_pTop)
	{
		pHold = m_pTop -> pNext;
		Release((LPVOID&)m_pTop);
		m_pTop = pHold;
	}
}

