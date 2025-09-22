/**********************************************************************************************************
*	Program Name			: MessageQueue.cpp
*	Description				: Class which maintains Queue.
*	Author Name				: Ramkrushna Shelke
*	Date Of Creation		: 14 Jan 2017
*	Version No				: 2.6.0.121
*	Special Logic Used		:
*	Modification Log		:
***********************************************************************************************************/
#include "StdAfx.h"
#include "MessageQueue.h"

#ifdef _DEBUG

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************
Function       : CMessageQueueItem
In Parameters  : void,
Out Parameters :
Description    : constructor
Author		   : Ram Shelke
****************************************************************************/
CMessageQueueItem::CMessageQueueItem()
{
	ResetData();
}

/***************************************************************************
Function       : ResetData
In Parameters  : void,
Out Parameters :
Description    : Fucntion to reset data.
Author		   : Ram Shelke
****************************************************************************/
void CMessageQueueItem::ResetData()
{
	m_nItemType = 0;
	m_dwDownloadID = 0;
	::ZeroMemory(m_szBinLMDT,sizeof(m_szBinLMDT));
	::ZeroMemory(m_szETAG,sizeof(m_szETAG));
	::ZeroMemory(m_szDomainAddress,sizeof(m_szDomainAddress));
	m_wPriority = 0;
	m_lLastVisited = 0;
	m_wProtcolType = eProtocol_HTTP;
	m_dwContentLength = 0;
	m_dwDomainCategory = 0;
	m_dwAge = 0;
}

/***************************************************************************
Function       : operator=
In Parameters  : const CMessageQueueItem &
Out Parameters : CMessageQueueItem& 
Description    : overloaded assignment operator.
Author		   : Ram Shelke
****************************************************************************/
CMessageQueueItem& CMessageQueueItem::operator=(const CMessageQueueItem &rhs)
{
	try
	{
		// Check for self-assignment!
		if (this == &rhs)      // Same object?
			return *this;        // Yes, so skip assignment, and just return *this.

		m_strQueueItem = rhs.m_strQueueItem;
		m_nItemType = rhs.m_nItemType;
		_tcscpy(m_szBinLMDT, rhs.m_szBinLMDT);
		_tcscpy(m_szETAG, rhs.m_szETAG);
		_tcscpy(m_szDomainAddress, rhs.m_szDomainAddress);
		m_wPriority = rhs.m_wPriority;
		m_lLastVisited = rhs.m_lLastVisited;
		m_wProtcolType = rhs.m_wProtcolType;
		m_dwContentLength = rhs.m_dwContentLength;
		m_dwDomainCategory = rhs.m_dwDomainCategory;
		m_dwAge = rhs.m_dwAge;
		m_dwDownloadID = rhs.m_dwDownloadID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMessageQueueItem::operator=", 0, 0, true, SECONDLEVEL);
	}
	return *this;
}

/***************************************************************************
Function       : CMessageQueue
In Parameters  : void
Out Parameters : 
Description    : Cont'r
Author		   : Ram Shelke
****************************************************************************/
CMessageQueue::CMessageQueue(void)
{
	m_dwTaskItems = 0; 
	m_dwQueueLength = 0;
	m_hQueueEvent = NULL;
}

/***************************************************************************
Function       : CMessageQueue
In Parameters  : void
Out Parameters :
Description    : Dest'r
Author		   : Ram Shelke
****************************************************************************/
CMessageQueue::~CMessageQueue(void)
{
	DeleteQueueItem();
}

/***************************************************************************
Function       : AddQueueItem
In Parameters  : CMessageQueueItem &objQueueItem
Out Parameters : void
Description    : Function to add item in queue.
Author		   : Ram Shelke
****************************************************************************/
void CMessageQueue::AddQueueItem(CMessageQueueItem &objQueueItem)
{
	CAutoCriticalSection objTempCS(m_MsgQueueCSection);
	m_MsgQueue.push(objQueueItem);
}

/***************************************************************************
Function       : FetchQueueItem
In Parameters  : CMessageQueueItem &objQueueItem
Out Parameters : void
Description    : Function to fetch item from queue.
Author		   : Ram Shelke
****************************************************************************/
bool CMessageQueue::FetchQueueItem(CMessageQueueItem &objQueueItem)
{
	try
	{
		CAutoCriticalSection objCS(m_MsgQueueCSection);
		if (!m_MsgQueue.empty())
		{
			objQueueItem.ResetData();
			objQueueItem = m_MsgQueue.front();
			m_MsgQueue.pop();
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMessageQueue::FetchQueueItem", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function       : AddQueueItem
In Parameters  : CMessageQueueItem &objQueueItem, int nItemType
Out Parameters : void
Description    : Function to add item in queue.
Author		   : Ram Shelke
****************************************************************************/
void CMessageQueue::AddQueueItem(LPCTSTR szQueueItem,int nItemType)
{
	try
	{
		CAutoCriticalSection objTempCS(m_MsgQueueCSection);
		CMessageQueueItem objQueueItem;
		objQueueItem.m_strQueueItem = szQueueItem;
		objQueueItem.m_nItemType = nItemType;
		m_MsgQueue.push(objQueueItem);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMessageQueue::AddQueueItem", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : DeleteQueueItem
In Parameters  : void
Out Parameters : void
Description    : Function to delete item from queue.
Author		   : Ram Shelke
****************************************************************************/
void CMessageQueue::DeleteQueueItem()
{

}