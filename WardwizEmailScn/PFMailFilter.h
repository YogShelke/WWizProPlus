/************************************************************************************************
*  Program Name	: MailFilter
*  Description	: Incoming and Outgoing mails for POP3 service.
*  Author Name	: Amol J
*  Date Of Creation: 08 Feb 2018
*  Version No	: 3.1.0.0
*************************************************************************************************/

//  The sample adds a prefix to the subjects of incoming messages, and blocks outgoing 
//  messages having the specified prefix in subject.
#include "stdafx.h"
#include <crtdbg.h>
#include "ProtocolFilters.h"
#include "PFEventsDefault.h"
#include "WWizEmailScanner.h"
#include "WWizFirewallFilter.h"
#include "WWizParentalCntrl.h"
#include <fstream>
#include <string>
#include "MimeCode.h"
#include "MimeChar.h"
#include "Mime.h"
#include "WinHttpClient.h"
#include "ExecuteProcess.h"
#include "BufferToStructure.h"
#include "WardwizLangManager.h"

using namespace nfapi;
using namespace ProtocolFilters;
using namespace std;

// Change this string after renaming and registering the driver under different name
//#define NFDRIVER_NAME "netfilter2"
#define NFDRIVER_NAME "VBFLT"
#define NFDRIVER_NAME_07 "VBFLT07"
#define NFDRIVER_NAME_08 "VBFLT08"
#define NFDRIVER_NAME_10 "VBFLT10"

#define HTTP_RESOLVE_TIMEOUT		1000
#define HTTP_CONNECT_TIMEOUT		5000
#define HTTP_SEND_TIMEOUT			5000
#define HTTP_RECEIVE_TIMEOUT		5000

#define EMPTY_ANSI_STRING ""

typedef struct __tagAddFilter
{
	__int64 id;
	unsigned long ulFilterType;
	unsigned long ulFilterFlag;
}STRUCTFILTERFLAG;

typedef struct _tagWebAgeCategories
{
	int iId;
	int	iAgeGroupId;
	std::string strUSERNAME;
	BYTE byLastStatus;
	BYTE byAdult;
	BYTE byAdvertisement;
	BYTE byAggressive;
	BYTE byBank;
	BYTE byBitcoin;
	BYTE byChat;
	BYTE byChild;
	BYTE byDating;
	BYTE byDownload;
	BYTE byDrug;
	BYTE byFinancial;
	BYTE byFreeWebEmails;
	BYTE byGrambling;
	BYTE byGames;
	BYTE byHacking;
	BYTE byJobSearch;
	BYTE bySexEducation;
	BYTE bySocNetworking;
	BYTE bySports;
	BYTE byVideoStreamin;
	BYTE byAllOther;
}WEBAGECATEGORIES;

typedef struct _tagWWizBrowseProtection
{
	int iBrowseID;
	std::string strUSERNAME;
	BYTE byBrowse;
	BYTE byBlk_Category;
	BYTE byBlk_Website;
}WWIZBROWSEPROTECTION;

typedef struct _tagWWizSPECWebBlock
{
	std::string strUserName;
	std::string strWebsiteName;
	BYTE byIsSubDomain;

}WWizSpecWebBlock;

typedef struct _tagWWIZMANAGEEXCLIST
{
	int iId;
	std::string strUserName;
}WWIZMANAGEEXCLIST;

typedef struct _tagURLDOMAINCACHE
{
	CHAR	szUrlDomain[100];
}URLDOMAINCACHE;

typedef struct _tagBrowserSecurity{
	BYTE bIsURLProt;
	BYTE bApplyExcept;
	BYTE bApplySpecific;
	BYTE bPhishingProt;
}BROWSERSECURITYVAL;

typedef std::map<std::string, WEBAGECATEGORIES> MAPWEBCAT;
typedef std::map<std::string, WWIZBROWSEPROTECTION> MAPBROWSEPROT;
typedef std::map<nfapi::ENDPOINT_ID, std::string> MAPCONNECTEDENDPOINTS;
typedef std::map<std::string, std::vector<WWizSpecWebBlock>> MAPWWIZSPECWEBBLOCK;
typedef std::map<std::string, std::vector<std::string>> MAPWWIZMANAGEEXCLIST;
typedef std::vector<BROWSERSECURITYVAL>BROWSERSECMAP;
typedef std::vector<std::string>BROWSERSECEXCLUSION;
typedef std::vector<std::string>BROWSERSECSPECIFIC;

//
//	API events handler
//
class EventHandler : public NF_EventHandler
{
public:
	EventHandler();
	~EventHandler();
private:
	virtual void threadStart();
	virtual void threadEnd();
	virtual void tcpConnectRequest(ENDPOINT_ID id, PNF_TCP_CONN_INFO pConnInfo);
	virtual void tcpConnected(ENDPOINT_ID id, PNF_TCP_CONN_INFO pConnInfo);
	virtual void tcpClosed(ENDPOINT_ID id, PNF_TCP_CONN_INFO pConnInfo);
	virtual void tcpReceive(ENDPOINT_ID id, const char * buf, int len);
	virtual void tcpSend(ENDPOINT_ID id, const char * buf, int len);
	virtual void tcpCanReceive(ENDPOINT_ID id);
	virtual void tcpCanSend(ENDPOINT_ID id);
	virtual void udpCreated(ENDPOINT_ID id, PNF_UDP_CONN_INFO pConnInfo);
	virtual void udpConnectRequest(ENDPOINT_ID id, PNF_UDP_CONN_REQUEST pConnReq);
	virtual void udpClosed(ENDPOINT_ID id, PNF_UDP_CONN_INFO pConnInfo);
	virtual void udpReceive(ENDPOINT_ID id, const unsigned char * remoteAddress, const char * buf, int len, PNF_UDP_OPTIONS options);
	virtual void udpSend(ENDPOINT_ID id, const unsigned char * remoteAddress, const char * buf, int len, PNF_UDP_OPTIONS options);
	virtual void udpCanReceive(ENDPOINT_ID id);
	virtual void udpCanSend(ENDPOINT_ID id);
	void HandleTcpUdpRequests(ENDPOINT_ID id, LPTSTR lpszProcessName);
public:
	void SetParentalControlPtr(CWWizParentalCntrl	*pParentalCtrl);
	void SetDefaultBehaviour(DWORD bEnable);
	bool InsertApplicationsIntoDB(std::string strAppPath);
	bool CheckApplicationExists(std::string strAppPath);
public:
	CWWizFirewallFilter			m_objFirewallFilter;
	CWWizParentalCntrl			*m_pobjParentalCtrl;
	TCHAR						m_szAllUserPath[MAX_PATH];
	CString						m_csAppPath;
	int							m_iSetDefAppBehav;
	std::vector<std::string>	m_vAppList;
};

class MailFilter : public PFEventsDefault
{
private:
	TCHAR				m_szAllUserPath[MAX_PATH];
	CWWizEmailScanner	m_objScanner;
	CString				m_csAppPath;
public:
	MailFilter();
	~MailFilter();

	CWWizFirewallFilter			m_objFirewallFilter;
	CWWizParentalCntrl			*m_pobjParentalCtrl;
	CExecuteProcess				m_objExecuteProcess;
	CWardwizLangManager			m_objwardwizLangManager;
	bool ScanAttachment();
	void GetClientNames();
	void SetParentalControlPtr(CWWizParentalCntrl	*pParentalCtrl);
	virtual void tcpConnected(nfapi::ENDPOINT_ID id, nfapi::PNF_TCP_CONN_INFO pConnInfo);
	void tcpConnectedSEH(nfapi::ENDPOINT_ID id, nfapi::PNF_TCP_CONN_INFO pConnInfo);
	bool CreateDirectoryFocefully(LPTSTR lpszPath);
	bool filterOutgoingMail(nfapi::ENDPOINT_ID id, PFObject * object);
	void filterIncomingMail(nfapi::ENDPOINT_ID id, PFObject * object);
	virtual void dataAvailable(nfapi::ENDPOINT_ID id, PFObject * object);
	std::string getHttpUrl(PFObject * object);
	void postBlockHttpResponse(nfapi::ENDPOINT_ID id, std::string strLink, std::string strCategory);
	PF_DATA_PART_CHECK_RESULT dataPartAvailable(nfapi::ENDPOINT_ID id, PFObject * object);
	PF_DATA_PART_CHECK_RESULT dataPartAvailableSEH(nfapi::ENDPOINT_ID id, PFObject * object);
	bool filterHttpRequest(nfapi::ENDPOINT_ID id, PFObject * object);
	bool filterHttpResponse(nfapi::ENDPOINT_ID id, PFObject * object);
	bool IsCategoryMatched(std::string strURL, std::string &strCategory);
	bool GetCategoryFromServer(std::string strURL, std::string& strURLCategory);
	DWORD GetProxySettingsFromRegistry();
	bool GetProxyServerDetailsFromDB();
	bool ReInitializeMailFilter();
	void UninitializeWWPCWebFilter();
	bool CheckISFilterAdded(nfapi::ENDPOINT_ID id, PF_FilterType type, tPF_FilterFlags Flags);
	bool GetWebFilteringURLSFromIni();
	bool CheckForURLCache(std::string strURL, DWORD dwData = 0);
	bool AddIntoForURLCache(std::string strURL);
	bool ReloadWebSecDB();
	bool ReloadBrowseProtDB();
	bool ReloadBrowserSecDB();
	bool ReloadBrowserSecExclusionDB();
	bool ReloadBrowserSecSpecificDB();
	std::string GetUserNameFromEndPointID(nfapi::ENDPOINT_ID id);
	bool InsertUserNameandID(nfapi::ENDPOINT_ID id, std::string);
	bool CheckForDuplicates(nfapi::ENDPOINT_ID id);
	bool CheckISBrowseProtectionEnabled(std::string strUserName);
	bool CheckISBlockCategoryEnabled(std::string strUserName);
	bool CheckISBlockSpecificWebsiteEnabled(std::string strUserName);
	bool CheckUserSettingsForThisCategory(std::string strUserName, std::string strCatID, std::string &strCatName);
	bool ReloadMngExcDB();
	bool ReloadBlkSpecWebDB();
	bool ReloadURLCacheDB();
	bool SaveURLDB();
	void InsertBrowserSecReport(CString, CString, CString);
	void InsertBrowseSecurityReport(CString, CString, CString);
	INT64 InsertDataToTable(const char* szQuery);
	int GetLastBrowseSessionID();
	int InsertBrowseSessionReport();
	bool SendBlockWebData2Tray(int iMessage, CString csWebUrl, CString csWebCategory);
	bool CheckForInternetAccessForUser(std::string);
	void GetFilterExceptionList();
	bool ISFilterExcludedProcess(std::wstring strProcessName);
	bool ISApplicationListed2Filter(CString csProcessName);
	CString GetShortUrl(CString csUrl);
	bool CheckSpecificWebBlock(std::string, CString);
	bool CheckExclusionList(std::string, CString);
	CStringA GetShortBlockUrl(CStringA csBlkUrl);
	bool ClearURLCache();
	void InsertIntoParCtrlReport(CString csUserName);
	bool CheckISBrowserProtEnabled();
	bool CheckForURLProtection(std::string);
	bool CheckForSpecificURLProtection(std::string);
	bool CheckForPhishingProtection();
	bool CheckForURLException(std::string);
	bool CheckForSpecificWebsite(std::string);
public:
	vector<CString>				m_vcProcessName;
	CString						m_csServer;
	DWORD						m_dwProxySett;
	CString						m_csUserName;
	CString						m_csPassword;
	WinHttpClient				m_HttpClient;
	vector<STRUCTFILTERFLAG>	m_vEndPointID;
	vector<std::string>			m_vstrWebFilterURLS;
	CString						m_csWebFilterURL;
	vector<std::string>			m_vURLSCache;
	MAPWEBCAT					m_MapWebCategories;
	MAPBROWSEPROT				m_MapWWizBrowseProtection;
	MAPCONNECTEDENDPOINTS		m_pConnectedEndPoints;
	MAPWWIZSPECWEBBLOCK			m_MapWWizSpecWebBlcok;
	MAPWWIZMANAGEEXCLIST		m_MapWWizManageExcList;
	BROWSERSECMAP				m_vecBrowserSec;
	BROWSERSECEXCLUSION			m_vecBrowserSecExc;
	BROWSERSECSPECIFIC			m_vecBrowserSecSpec;
	CBufferToStructure			m_buf2StDB;
	CWardWizSQLiteDatabase		m_objSqlDb;
	vector<std::string>			m_vcFilterExceptionlist;
};