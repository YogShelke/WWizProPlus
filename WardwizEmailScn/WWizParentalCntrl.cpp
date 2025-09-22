#include "stdafx.h"
#include "WWizParentalCntrl.h"
#include "WardWizDatabaseInterface.h"
#include "ISpyCommunicator.h"

CITinRegWrapper				g_objReg;

CWWizParentalCntrl::CWWizParentalCntrl()
{
	LoadExeBlockList();
	LoadAppBlockCategoryList();
	LoadUserListforPCtrl();
	LoadUserPermission();
}

CWWizParentalCntrl::~CWWizParentalCntrl()
{
}

/***********************************************************************************************
Function Name  : StartParentalControlThread
Description    : StartParentalControlThread
Author Name    : Jeena Mariam Saji
Date           : 08th June 2018
***********************************************************************************************/
void CWWizParentalCntrl::StartParentalControl()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		bool bIsRegistrySet = false;

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		
		dbSQlite.Open();
		//For Settings in Parental Control: Computer Usage

		PCUSERLISTMAP::iterator vUserPermissionList;
		for (vUserPermissionList = m_vecPCtrlUserList.begin(); vUserPermissionList != m_vecPCtrlUserList.end(); vUserPermissionList++)
		{
			CStringA csQuery;

			BYTE byCompUsage[8][25];
			memset(&byCompUsage, sizeof(byCompUsage), 0x00);

			CStringA csUserNameVal(vUserPermissionList->szUserName);

			csQuery.Format("Select TRACK_VALUE from WWIZ_PC_TRACK_COMP_USAGE where FK_USERNAME='%s'", csUserNameVal);

			CWardwizSQLiteTable qResult = dbSQlite.GetTable(csQuery);
			if (qResult.GetNumRows() == 0x00)
			{
				continue;
			}
			char sziStngID[340] = { 0 };
			strcpy(sziStngID, qResult.GetFieldValue(0));

			int iVal = 0;
			for (unsigned int j = 0; j <= 7; j++)
			{
				int iTemp = 0;
				for (unsigned int i = 0; i <= 46;)
				{
					byCompUsage[j][iTemp] = (byte)sziStngID[iVal];
					iTemp++;
					i += 2;
					iVal += 2;
				}
			}

			int iDay = 0x00;
			CString csCurrentHour = CTime::GetCurrentTime().Format("%H");
			int iCurrentHour = _ttoi(csCurrentHour);
			int iDayWeek = CTime::GetCurrentTime().GetDayOfWeek();
			if (iDayWeek == EN_MONDAY)
			{
				iDay = 0x01;
			}
			else if (iDayWeek == EN_TUESDAY)
			{
				iDay = 0x02;
			}
			else if (iDayWeek == EN_WEDNESDAY)
			{
				iDay = 0x03;
			}
			else if (iDayWeek == EN_THURSDAY)
			{
				iDay = 0x04;
			}
			else if (iDayWeek == EN_FRIDAY)
			{
				iDay = 0x05;
			}
			else if (iDayWeek == EN_SATURDAY)
			{
				iDay = 0x06;
			}
			else if (iDayWeek == EN_SUNDAY)
			{
				iDay = 0x07;
			}

			if (iDay != 0x00)
			{
				if (byCompUsage[iDay - 0x01][iCurrentHour] == '1' && vUserPermissionList->m_bRestrictHoursPopUp != true)
				{
					vUserPermissionList->bIsCompAccessBlocked = true;
					vUserPermissionList->m_bBlockTimePopUpShown = true;
					continue;
				}
				else
				{
					//SendParCtrlMessage2Tray(SEND_PC_TRAY_CLOSE);
					if (vUserPermissionList->m_bBlockTimePopUpShown)
					{
						vUserPermissionList->bIsCompAccessBlocked = false;
						vUserPermissionList->m_bBlockTimePopUpShown = false;
					}
				}
			}

			char szParCtrlRestrictVal[4] = { 0 };

			if (iDayWeek == 2 || iDayWeek == 3 || iDayWeek == 4 || iDayWeek == 5 || iDayWeek == 6)
			{
				csQuery.Format("Select ALLOW_ACCESS_WKDAY from PARCTRL_GENERAL where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qValResult = dbSQlite.GetTable(csQuery);
				if (qValResult.GetNumRows() == 0x00)
				{
					continue;
				}
				strcpy(szParCtrlRestrictVal, qValResult.GetFieldValue(0));
				if (strlen(szParCtrlRestrictVal) == 0)
				{
					continue;
				}
			}
			else
			{
				csQuery.Format("Select ALLOW_ACCESS_WKEND from PARCTRL_GENERAL where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qValResult = dbSQlite.GetTable(csQuery);
				if (qValResult.GetNumRows() == 0x00)
				{
					continue;
				}
				strcpy(szParCtrlRestrictVal, qValResult.GetFieldValue(0));
				if (strlen(szParCtrlRestrictVal) == 0)
				{
					continue;
				}
			}
			int iRestrictUsage = atoi(szParCtrlRestrictVal);
			if (iRestrictUsage == 0 && vUserPermissionList->m_bRestrictHoursPopUp == true)
			{
				vUserPermissionList->m_bRestrictHoursPopUp = false;
				vUserPermissionList->bIsCompAccessBlocked = false;
				continue;
			}

			csQuery.Format("Select PC_TIMER_DATE from WWIZ_TIMER_TABLE where FK_USERNAME='%s'", csUserNameVal);
			CWardwizSQLiteTable qResults = dbSQlite.GetTable(csQuery);
			if (qResults.GetNumRows() == 0x00)
			{
				continue;
			}
			char szDateParCtrl[20] = { 0 };
			strcpy(szDateParCtrl, qResults.GetFieldValue(0));

			iDay = 0;
			int iMonth = 0;
			int iYear = 0;

			char seps[] = "-";
			char *token = NULL;
			char* context = NULL;

			int iIndex = 0;
			token = strtok_s(szDateParCtrl, seps, &context);
			while (token != NULL)
			{
				if (strlen(token) > 0)
				{
					int iTokenValue = atoi(token);
					switch (iIndex)
					{
					case 0:
						iYear = iTokenValue;
						break;
					case 1:
						iMonth = iTokenValue;
						break;
					case 2:
						iDay = iTokenValue;
						break;
					}
				}
				token = strtok_s(NULL, seps, &context);
				iIndex++;
			}

			//For Settings in Parental Control: Computer Usage

			PARCTRLUSAGECHECK szParCtrlUsageList;

			char szRestrictParCtrlTime[10] = { 0 };
			int iRestrictTime;

			if (iDayWeek == 2 || iDayWeek == 3 || iDayWeek == 4 || iDayWeek == 5 || iDayWeek == 6)
			{
				csQuery.Format("Select ALLOW_ACCESS_WKDAY_TIME from PARCTRL_GENERAL where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qResultTime = dbSQlite.GetTable(csQuery);
				if (qResultTime.GetNumRows() == 0x00)
				{
					continue;
				}
				strcpy(szRestrictParCtrlTime, qResultTime.GetFieldValue(0));
				szParCtrlUsageList.iWDayCompUsage = atoi(szRestrictParCtrlTime);
				iRestrictTime = szParCtrlUsageList.iWDayCompUsage;
			}
			else
			{
				csQuery.Format("Select ALLOW_ACCESS_WKEND_TIME from PARCTRL_GENERAL where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qResultTime = dbSQlite.GetTable(csQuery);
				if (qResultTime.GetNumRows() == 0x00)
				{
					continue;
				}
				strcpy(szRestrictParCtrlTime, qResultTime.GetFieldValue(0));
				szParCtrlUsageList.iWEndCompUsage = atoi(szRestrictParCtrlTime);
				iRestrictTime = szParCtrlUsageList.iWEndCompUsage;
			}

			CString csCurrentSec = CTime::GetCurrentTime().Format("%S");
			int iCurrentSec = _ttoi(csCurrentSec);
			CString csCurrentMin = CTime::GetCurrentTime().Format("%M");
			int iCurrentMin = _ttoi(csCurrentMin);
			csCurrentHour = CTime::GetCurrentTime().Format("%H");
			iCurrentHour = _ttoi(csCurrentHour);
			int iCurrTime = iCurrentSec + (iCurrentMin * 60) + (iCurrentHour * 3600);

			CTime cTime = CTime::GetCurrentTime();
			CTime Time_DBEntry(iYear, iMonth, iDay, 0, 0, 0);
			CTimeSpan Time_Diff = cTime - Time_DBEntry;
			int iSpan = static_cast<int>(Time_Diff.GetDays());
			if (iSpan == 0)
			{
				csQuery.Format("Select PC_TIMER_TIME from WWIZ_TIMER_TABLE where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qResultTime = dbSQlite.GetTable(csQuery);
				if (qResultTime.GetNumRows() == 0x00)
				{
					continue;
				}
				char szDateParCtrlTime[20] = { 0 };
				strcpy(szDateParCtrlTime, qResultTime.GetFieldValue(0));
				int iInitialTime = atoi(szDateParCtrlTime);

				int iRestrictTimeSec = iRestrictTime * 3600;
				int iDiff = iCurrTime - iInitialTime;
				if ((iCurrTime - iInitialTime) > iRestrictTimeSec && iRestrictUsage != 0)
				{
					vUserPermissionList->bIsCompAccessBlocked = true;
					vUserPermissionList->m_bRestrictHoursPopUp = true;
				}
				else
				{
					if (vUserPermissionList->m_bRestrictHoursPopUp)
					{
						vUserPermissionList->bIsCompAccessBlocked = false;
						vUserPermissionList->m_bRestrictHoursPopUp = false;
					}
				}
			}
			else
			{
				CString csInsertQuery = _T("UPDATE WWIZ_TIMER_TABLE VALUES (null,");
				csInsertQuery.Format(_T("UPDATE WWIZ_TIMER_TABLE SET PC_TIMER_DATE = Date('now'), PC_TIMER_TIME = %d WHERE FK_USERNAME='%s'"), iCurrTime, csUserNameVal);
				CT2A asciiSession(csInsertQuery, CP_UTF8);
				INT64 iScanSessionId = InsertSQLiteDataForPC(asciiSession.m_psz);
			}
		}
		dbSQlite.Close();
		CheckForUserPermissions();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::StartParentalControl", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : StartInternetCheck
Description    : Start Internet Check
Author Name    : Jeena Mariam Saji
Date           : 07th September 2018
***********************************************************************************************/
void CWWizParentalCntrl::StartInternetCheck()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		bool bIsRegistrySet = false;

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		//For Parental Control: Internet Usage

		dbSQlite.Open();
		char sziSelectVal[4] = { 0 };
		int iDayWeek = CTime::GetCurrentTime().GetDayOfWeek();
		CStringA csQuery;

		BYTE byCompInternetUsage[8][25];

		memset(&byCompInternetUsage, sizeof(byCompInternetUsage), 0x00);

		PCUSERLISTMAP::iterator vUserPermissionList;
		for (vUserPermissionList = m_vecPCtrlUserList.begin(); vUserPermissionList != m_vecPCtrlUserList.end(); vUserPermissionList++)
		{
			CStringA csUserNameVal(vUserPermissionList->szUserName);
			csQuery.Format("Select TRACK_VALUE from WWIZ_INTERNET_USAGE where FK_USERNAME='%s'", csUserNameVal);
			CWardwizSQLiteTable qResults = dbSQlite.GetTable(csQuery);
			if (qResults.GetNumRows() == 0x00)
			{
				continue;
			}
			char sziInternetStngID[340] = { 0 };
			strcpy(sziInternetStngID, qResults.GetFieldValue(0));
			int iINetVal = 0;

			for (unsigned int j = 0; j <= 7; j++)
			{
				int iTemp = 0;
				for (unsigned int i = 0; i <= 46;)
				{
					byCompInternetUsage[j][iTemp] = (byte)sziInternetStngID[iINetVal];
					iTemp++;
					i += 2;
					iINetVal += 2;
				}
			}

			int iINetDay = 0x00;
			CString csINetCurrentHour = CTime::GetCurrentTime().Format("%H");
			int iINetCurrentHour = _ttoi(csINetCurrentHour);
			int iINetDayWeek = CTime::GetCurrentTime().GetDayOfWeek();
			if (iINetDayWeek == EN_MONDAY)
			{
				iINetDay = 0x01;
			}
			else if (iINetDayWeek == EN_TUESDAY)
			{
				iINetDay = 0x02;
			}
			else if (iINetDayWeek == EN_WEDNESDAY)
			{
				iINetDay = 0x03;
			}
			else if (iINetDayWeek == EN_THURSDAY)
			{
				iINetDay = 0x04;
			}
			else if (iINetDayWeek == EN_FRIDAY)
			{
				iINetDay = 0x05;
			}
			else if (iINetDayWeek == EN_SATURDAY)
			{
				iINetDay = 0x06;
			}
			else if (iINetDayWeek == EN_SUNDAY)
			{
				iINetDay = 0x07;
			}

			if (iINetDay != 0x00)
			{
				if (byCompInternetUsage[iINetDay - 0x01][iINetCurrentHour] == '1')
				{
					vUserPermissionList->m_bBlockInternetUsage = true;
				}
				else
				{
					vUserPermissionList->m_bBlockInternetUsage = false;
				}
			}
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::StartInternetCheck", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : SendParCtrlMessage2Tray
Description    : SendParCtrlMessage2Tray
Author Name    : Jeena Mariam Saji
Date           : 08th June 2018
***********************************************************************************************/
bool CWWizParentalCntrl::SendParCtrlMessage2Tray(int iRequest, CString csUserName)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;
		_tcscpy(szPipeData.szFirstParam, csUserName.GetBuffer());

		CISpyCommunicator objCom(TRAY_SERVER + csUserName, true, 0x03, 0x12C);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumParentalCntrl::SendMessage2Tray", 0, 0, true, ZEROLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : InsertSQLiteDataForPC
Description    : Function to insert data
Author Name    : Jeena Mariam Saji
Date           : 19th July 2018
***********************************************************************************************/
INT64 CWWizParentalCntrl::InsertSQLiteDataForPC(const char* szQuery)
{
	try
	{
		char* g_strDatabasePath = ".\\VBPARCONTROL.DB";

		CWardWizSQLiteDatabase objSqlDb(g_strDatabasePath);
		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(100);

		INT64 iLastRowId = objSqlDb.GetLastRowId();
		objSqlDb.Close();

		return iLastRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizComSrv- InsertSQLiteDataForPC. Query is : ", (TCHAR*)szQuery, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***********************************************************************************************
Function Name  : StartParCtrlINetAccessThread
Description    : StartParCtrlINetAccessThread
Author Name    : Nitin Shelar
Date           : 20th July 2018
***********************************************************************************************/
void CWWizParentalCntrl::StartParCtrlINetAccess()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		dbSQlite.Open();

		PCUSERLISTMAP::iterator vUserPermissionList;
		for (vUserPermissionList = m_vecPCtrlUserList.begin(); vUserPermissionList != m_vecPCtrlUserList.end(); vUserPermissionList++)
		{
			CStringA csUserNameVal(vUserPermissionList->szUserName);

			if (!PathFileExists(csWardWizReportsPath))
			{
				vUserPermissionList->m_bRestrictInternetUsage = false;
				continue;
			}

			CT2A dbPath(csWardWizReportsPath, CP_UTF8);
			dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

			CStringA csQuery;

			int iDayWeek = CTime::GetCurrentTime().GetDayOfWeek();
			int iRestrictCheck;
			char szRestrictINetCheck[4] = { 0 };

			if (iDayWeek == 2 || iDayWeek == 3 || iDayWeek == 4 || iDayWeek == 5 || iDayWeek == 6)
			{
				csQuery.Format("Select RSTRCT_INET_WDAY from PARCTRL_INTRNET where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qResultTime = dbSQlite.GetTable(csQuery);
				if (qResultTime.GetNumRows() == 0x00)
				{
					vUserPermissionList->m_bRestrictInternetUsage = false;
					continue;
				}
				strcpy(szRestrictINetCheck, qResultTime.GetFieldValue(0));
				iRestrictCheck = atoi(szRestrictINetCheck);
			}
			else
			{
				csQuery.Format("Select RSTRCT_INET_WEND from PARCTRL_INTRNET where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qResultTime = dbSQlite.GetTable(csQuery);
				if (qResultTime.GetNumRows() == 0x00)
				{
					vUserPermissionList->m_bRestrictInternetUsage = false;
					continue;
				}
				strcpy(szRestrictINetCheck, qResultTime.GetFieldValue(0));
				iRestrictCheck = atoi(szRestrictINetCheck);
			}

			if (iRestrictCheck == 0)
			{
				vUserPermissionList->m_bRestrictInternetUsage = false;
				continue;
			}

			csQuery.Format("Select INET_TIMER_DATE from WWIZ_INET_TIMER_TABLE where FK_USERNAME='%s'", csUserNameVal);
			CWardwizSQLiteTable qResults = dbSQlite.GetTable(csQuery);
			if (qResults.GetNumRows() == 0x00)
			{
				vUserPermissionList->m_bRestrictInternetUsage = false;
				continue;
			}
			char szDateParCtrl[20] = { 0 };
			strcpy(szDateParCtrl, qResults.GetFieldValue(0));

			int iDay = 0;
			int iMonth = 0;
			int iYear = 0;

			char seps[] = "-";
			char *token = NULL;
			char* context = NULL;

			int iIndex = 0;
			token = strtok_s(szDateParCtrl, seps, &context);
			while (token != NULL)
			{
				if (strlen(token) > 0)
				{
					int iTokenValue = atoi(token);
					switch (iIndex)
					{
					case 0:
						iYear = iTokenValue;
						break;
					case 1:
						iMonth = iTokenValue;
						break;
					case 2:
						iDay = iTokenValue;
						break;
					}
				}
				token = strtok_s(NULL, seps, &context);
				iIndex++;
			}

			//For Settings in Parental Control: Internet access

			PARCTRLINETUSAGECHECK szParCtrlInetUsageList;

			int iRestrictTime;
			char szRestrictINetTime[4] = { 0 };

			if (iDayWeek == 2 || iDayWeek == 3 || iDayWeek == 4 || iDayWeek == 5 || iDayWeek == 6)
			{
				csQuery.Format("Select RSTRCT_INET_TIME_WDAY from PARCTRL_INTRNET where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qResultTime = dbSQlite.GetTable(csQuery);
				if (qResultTime.GetNumRows() == 0x00)
				{
					vUserPermissionList->m_bRestrictInternetUsage = false;
					continue;
				}
				strcpy(szRestrictINetTime, qResultTime.GetFieldValue(0));
				iRestrictTime = atoi(szRestrictINetTime);
			}
			else
			{
				csQuery.Format("Select RSTRCT_INET_TIME_WEND from PARCTRL_INTRNET where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qResultTime = dbSQlite.GetTable(csQuery);
				if (qResultTime.GetNumRows() == 0x00)
				{
					vUserPermissionList->m_bRestrictInternetUsage = false;
					continue;
				}
				strcpy(szRestrictINetTime, qResultTime.GetFieldValue(0));
				iRestrictTime = atoi(szRestrictINetTime);
			}
			szParCtrlInetUsageList.iWDayINetUsage = iRestrictTime;

			CString csCurrentSec = CTime::GetCurrentTime().Format("%S");
			int iCurrentSec = _ttoi(csCurrentSec);
			CString csCurrentMin = CTime::GetCurrentTime().Format("%M");
			int iCurrentMin = _ttoi(csCurrentMin);
			CString csCurrentHour = CTime::GetCurrentTime().Format("%H");
			int iCurrentHour = _ttoi(csCurrentHour);
			int iCurrTime = iCurrentSec + (iCurrentMin * 60) + (iCurrentHour * 3600);

			CTime cTime = CTime::GetCurrentTime();
			CTime Time_DBEntry(iYear, iMonth, iDay, 0, 0, 0);
			CTimeSpan Time_Diff = cTime - Time_DBEntry;
			int iSpan = static_cast<int>(Time_Diff.GetDays());
			if (iSpan == 0)
			{
				csQuery.Format("Select INET_TIMER_TIME from WWIZ_INET_TIMER_TABLE where FK_USERNAME='%s'", csUserNameVal);
				CWardwizSQLiteTable qResultTime = dbSQlite.GetTable(csQuery);
				if (qResultTime.GetNumRows() == 0x00)
				{
					continue;
				}
				char szDateParCtrlTime[20] = { 0 };
				strcpy(szDateParCtrlTime, qResultTime.GetFieldValue(0));

				int iInitialTime = atoi(szDateParCtrlTime);

				int iRestrictTimeSec = iRestrictTime * 3600;
				int iDiff = iCurrTime - iInitialTime;

				if ((iDiff > iRestrictTimeSec))
				{
					vUserPermissionList->m_bRestrictInternetUsage = true;
				}
				else
				{
					vUserPermissionList->m_bRestrictInternetUsage = false;
				}
			}
			else
			{
				CString csInsertQuery = _T("UPDATE WWIZ_INET_TIMER_TABLE VALUES (null,");
				csInsertQuery.Format(_T("UPDATE WWIZ_INET_TIMER_TABLE SET INET_TIMER_DATE = Date('now'), INET_TIMER_TIME = %d WHERE FK_USERNAME='"), iCurrTime, csUserNameVal);
				CT2A asciiSession(csInsertQuery, CP_UTF8);
				INT64 iScanSessionId = InsertSQLiteDataForPC(asciiSession.m_psz);
			}
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::StartParCtrlINetAccessThread", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckIfUserActive
Description    : Check if User is active
Author Name    : Jeena Mariam Saji
Date           : 17th August 2018
***********************************************************************************************/
bool CWWizParentalCntrl::CheckIfUserActive()
{
	bool bReturn = false;
	try
	{
		CString csRegKeyPath;
		csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		DWORD dwParCtrlValue = 0x00;

		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParCtrlValue) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CheckIfUserActive KeyPath: %s", csRegKeyPath, 0, true, ZEROLEVEL);;
		}

		if (dwParCtrlValue != 0x01)
		{
			bReturn = false;
			return bReturn;
		}

		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			bReturn = false;
			return bReturn;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();
		CString csUserName = GetCurrentUsrName();
		csUserName.MakeLower();
		CStringA csQuery;
		csQuery.Format("select ISCONTROL from WWiz_PC_UserList where USERNAME ='");
		csQuery.AppendFormat(CStringA(csUserName.GetBuffer()));
		csQuery.AppendFormat("'");
		CWardwizSQLiteTable qValResult = dbSQlite.GetTable(csQuery);
		if (qValResult.GetNumRows() == 0x00)
		{
			bReturn = false;
			return bReturn;
		}

		char szParCtrlTogglerVal[4] = { 0 };
		strcpy(szParCtrlTogglerVal, qValResult.GetFieldValue(0));
		if (strlen(szParCtrlTogglerVal) == 0)
		{
			bReturn = false;
			return bReturn;
		}
		int iRestrictUser = atoi(szParCtrlTogglerVal);
		if (iRestrictUser == 0)
		{
			bReturn = false;
		}
		else
		{
			bReturn = true;
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckIfUserActive ", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : CheckIsEXEBlockedbyPC
Description    : Check if EXE is blocked
Author Name    : Jeena Mariam Saji
Date           : 20th July 2018
***********************************************************************************************/
void CWWizParentalCntrl::LoadExeBlockList()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();
		CString csUserName = GetCurrentUsrName();
		csUserName.MakeLower();
		CStringA csQuery;

		csQuery.Format("select APP_PATH from WWIZ_RESTRICT_USE_OF_APP where WWIZ_RESTRICT_USE_OF_APP.FK_USERNAME = '");
		csQuery.AppendFormat(CStringA(csUserName.GetBuffer()));
		csQuery.AppendFormat("'");
		CWardwizSQLiteTable qResult = dbSQlite.GetTable(csQuery);
		if (qResult.GetNumRows() == 0x00)
		{
			return;
		}

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);	
			
			//to read unicode string from db
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(0), -1, NULL, 0);
			wchar_t *wstrDbData = new wchar_t[wchars_num];
			if (wstrDbData == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CVibraniumParentalCntrl::LoadExeBlockList", 0, 0, true, SECONDLEVEL);
				continue;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(0), -1, wstrDbData, wchars_num);
			std::wstring strPathVal = wstrDbData;
			m_vecExeList.push_back(strPathVal);
			delete[] wstrDbData;
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::LoadExeBlockList ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckIsEXEBlockedbyPC
Description    : Unload EXE Block List
Author Name    : Jeena Mariam Saji
Date           : 20th July 2018
***********************************************************************************************/
bool CWWizParentalCntrl::CheckEXEBlockList(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	try
	{
		VECEXEBLOCKLIST::iterator vBLockListIter;
		std::wstring strFilePath;
		strFilePath = lpszFilePath;
		for (vBLockListIter = m_vecExeList.begin(); vBLockListIter != m_vecExeList.end(); vBLockListIter++)
		{
			if (strFilePath == (*vBLockListIter))
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckEXEBlockList ", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReLoadBlockedAppList
Description    : ReLoad Blocked App List
Author Name    : Jeena Mariam Saji
Date           : 20th July 2018
***********************************************************************************************/
bool CWWizParentalCntrl::ReLoadBlockedAppList()
{
	try
	{
		m_vecExeList.clear();
		m_vecAppCategoryList.clear();
		LoadExeBlockList();
		LoadAppBlockCategoryList();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::ReLoadBlockedApplicationList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : LoadAppBlockCategoryList
Description    : Load App Block Category List
Author Name    : Jeena Mariam Saji
Date           : 23th July 2018
***********************************************************************************************/
void CWWizParentalCntrl::LoadAppBlockCategoryList()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		CWardWizSQLiteDatabase dbSQlites;
		CString	csWardWizModulePaths = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPaths = L"";
		csWardWizReportsPaths.Format(L"%sVibraniumPARBLK.DB", csWardWizModulePaths);

		if (!PathFileExists(csWardWizReportsPaths))
		{
			return;
		}

		CT2A dbPaths(csWardWizReportsPaths, CP_UTF8);
		dbSQlites.SetDataBaseFilePath(dbPaths.m_psz);

		dbSQlite.Open();
		dbSQlites.Open();

		CString csUserName = GetCurrentUsrName();
		csUserName.MakeLower();
		CStringA csQuery;

		csQuery.Format("select ID from WWIZ_MANAGE_RESTRICT_APP where SELECTED = 1 AND WWIZ_MANAGE_RESTRICT_APP.FK_USERNAME = '");
		csQuery.AppendFormat(CStringA(csUserName.GetBuffer()));
		csQuery.AppendFormat("'");
		CWardwizSQLiteTable qResult = dbSQlite.GetTable(csQuery);
		if (qResult.GetNumRows() == 0x00)
		{
			return;
		}

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			char szListCategory[100] = { 0 };
			qResult.SetRow(iRow);
			strcpy_s(szListCategory, qResult.GetFieldValue(0));
			int iVal = atoi(szListCategory);

			CStringA csQuery;
			csQuery.Format("select APP_NAME from PARCTRL_BLOCK_APP_LIST where BLOCK_CATEGORY = %d", iVal);
			CWardwizSQLiteTable qResults = dbSQlites.GetTable(csQuery.GetBuffer());
			if (qResults.GetNumRows() == 0x00)
			{
				return;
			}

			for (int iRow = 0; iRow < qResults.GetNumRows(); iRow++)
			{
				char szListExeFile[200] = { 0 };
				qResults.SetRow(iRow);
				strcpy_s(szListExeFile, qResults.GetFieldValue(0));
				std::wstring strPathVal = CA2W(szListExeFile);
				m_vecAppCategoryList.push_back(strPathVal);
			}
		}
		dbSQlites.Close();
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::LoadAppBlockCategoryList ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : UnloadAppBlockList
Description    : UnLoad App Block Category List
Author Name    : Jeena Mariam Saji
Date           : 23th July 2018
***********************************************************************************************/
bool CWWizParentalCntrl::CheckAppBlockList(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	try
	{
		VECAPPBLOCKLIST::iterator vAppBLockListIter;
		std::wstring strFilePath;
		strFilePath = lpszFilePath;
		CString csHiddenTxt = L"\\";

		int iPos = static_cast<int>(strFilePath.rfind(csHiddenTxt));
		strFilePath.erase(0,iPos+1);
		strFilePath.erase(strFilePath.find('.'));

		for (vAppBLockListIter = m_vecAppCategoryList.begin(); vAppBLockListIter != m_vecAppCategoryList.end(); vAppBLockListIter++)
		{
			if (strFilePath == (*vAppBLockListIter))
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckAppBlockList ", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : GetCurrentUsrName
Description    : To get Current Usr Name from productsetting.ini file
Author Name    : NITIN SHELAR
Date           : 30 July 2018
***********************************************************************************************/
CString CWWizParentalCntrl::GetCurrentUsrName()
{
	TCHAR szUsrName[MAX_PATH] = { 0 };
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		GetPrivateProfileString(L"VBSETTINGS", L"CurrentUserName", L"", szUsrName, MAX_PATH, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::GetCurrentUsrName ", 0, 0, true, SECONDLEVEL);
	}
	return szUsrName;
}

/***********************************************************************************************
Function Name  : SendMessage2UI
Description    : Send Message to UI
Author Name    : Jeena Mariam Saji
Date           : 03 August 2018
***********************************************************************************************/
bool CWWizParentalCntrl::SendMessage2UI(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(UI_SERVER, false);

		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumParentalCntrl::SendMessage2UI", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::SendMessage2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : CheckEXEBlocked
Description    : Check if EXE is blocked
Author Name    : Jeena Mariam Saji
Date           : 13 August 2018
***********************************************************************************************/
bool CWWizParentalCntrl::CheckEXEBlocked(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	try
	{
		if (!IsWardwizPath(lpszFilePath))
		{
			return bReturn;
		}
		if (!CheckIfUserActive())
		{
			return bReturn;
		}
		
		if (CheckEXEBlockList(lpszFilePath))
		{
			bReturn = true;
			return bReturn;
		}

		if (CheckAppBlockList(lpszFilePath))
		{
			bReturn = true;
			return bReturn;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckEXEBlocked ", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : IsWardwizPath
Description    : Check for WardWiz Path
Author Name    : Jeena Mariam Saji
Date           : 14 August 2018
***********************************************************************************************/
bool CWWizParentalCntrl::IsWardwizPath(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	try
	{
		CString	m_csAppPath;
		m_csAppPath = GetWardWizPathFromRegistry();
		CString csProcessPath{ lpszFilePath };
		CString csHiddenTxt = L"\\";
		int iPos = csProcessPath.ReverseFind('\\');
		csProcessPath = csProcessPath.Left(iPos + 1);
		CString csWardWizPath = m_csAppPath.MakeLower();
		csProcessPath.MakeLower();
		if (csProcessPath == csWardWizPath)
		{
			bReturn = false;
			return bReturn;
		}
		else
		{
			bReturn = true;
			return bReturn;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::IsWardwizPath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
/***********************************************************************************************
Function Name  : ReLoadBlockedApplication
Description    : Reload blocked application
Author Name    : Jeena Mariam Saji
Date           : 13 August 2018
***********************************************************************************************/
void CWWizParentalCntrl::ReLoadBlockedApplication()
{
	try
	{
		m_vecExeList.clear();
		m_vecAppCategoryList.clear();
		LoadExeBlockList();
		LoadAppBlockCategoryList();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::ReLoadBlockedApplication ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : LoadUserListforPCtrl
Description    : Load user control list
Author Name    : Jeena Mariam Saji
Date           : 24 June 2019
***********************************************************************************************/
void CWWizParentalCntrl::LoadUserListforPCtrl()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		m_vecPCtrlUserList.clear();

		dbSQlite.Open();
		CWardwizSQLiteTable qResultUser = dbSQlite.GetTable("Select USERNAME from WWiz_PC_UserList;");
		Sleep(20);

		for (int iRow = 0; iRow < qResultUser.GetNumRows(); iRow++)
		{
			PCTRLUSERPERMISSION szParCtrlPermission;

			qResultUser.SetRow(iRow);
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResultUser.GetFieldValue(0), -1, NULL, 0);
			wchar_t *wstrDbData = new wchar_t[wchars_num];
			if (wstrDbData == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CVibraniumParentalCntrl::LoadExeBlockList", 0, 0, true, SECONDLEVEL);
				continue;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResultUser.GetFieldValue(0), -1, wstrDbData, wchars_num);
			CString csDataVal = wstrDbData;
			strcpy_s(szParCtrlPermission.szUserName, CStringA(csDataVal).GetString());
			CString csVal;
			csVal = szParCtrlPermission.szUserName;
			m_vecPCtrlUserList.push_back(szParCtrlPermission);
			delete[] wstrDbData;
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::LoadUserListforPCtrl ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckInternetUsagePermission
Description    : To check internet usage permission
Author Name    : Jeena Mariam Saji
Date           : 24 June 2019
***********************************************************************************************/
bool CWWizParentalCntrl::CheckInternetUsagePermission(CString csUserName)
{
	bool bReturn = false;
	try
	{
		PCUSERLISTMAP::iterator vUserPermissionList;
		csUserName.MakeLower();
		for (vUserPermissionList = m_vecPCtrlUserList.begin(); vUserPermissionList != m_vecPCtrlUserList.end(); vUserPermissionList++)
		{
			CString csUserNameValue = vUserPermissionList->szUserName;
			if (csUserNameValue == csUserName)
			{
				if (vUserPermissionList->m_bBlockInternetUsage == true || vUserPermissionList->m_bRestrictInternetUsage == true)
				{
					bReturn = false;
					break;
				}
				else
				{
					bReturn = true;
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckInternetUsagePermission ", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : CheckForUserPermissions
Description    : To check for user permission
Author Name    : Jeena Mariam Saji
Date           : 24 June 2019
***********************************************************************************************/
void CWWizParentalCntrl::CheckForUserPermissions()
{
	try
	{
		PCUSERLISTMAP::iterator vUserPermissionList;
		for (vUserPermissionList = m_vecPCtrlUserList.begin(); vUserPermissionList != m_vecPCtrlUserList.end(); vUserPermissionList++)
		{
			CString csUserName = vUserPermissionList->szUserName;
			CT2CA pszConvertedAnsiString(csUserName);
			std::string strUserName(pszConvertedAnsiString);
			if (CheckUserAllowed(strUserName))
			{
				if (vUserPermissionList->bIsCompAccessBlocked == true)
				{
					SendParCtrlMessage2Tray(SEND_PC_TRAY_LOCK_WND, csUserName);
				}
				else
				{
					SendParCtrlMessage2Tray(SEND_PC_TRAY_CLOSE, csUserName);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckForUserPermissions ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckUserAllowed
Description    : To check for user permission
Author Name    : Jeena Mariam Saji
Date           : 24 June 2019
***********************************************************************************************/
bool CWWizParentalCntrl::CheckUserAllowed(std::string strUserName)
{
	bool bReturn = false;
	try
	{
		MAPUSERLISTCHECK::iterator it = m_mapParCtrlUserAccess.find(strUserName);
		if (it != m_mapParCtrlUserAccess.end())
		{
			bReturn = it->second;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckUserAllowed ", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadUserPermission
Description    : To load user permission
Author Name    : Jeena Mariam Saji
Date           : 16 July 2019
***********************************************************************************************/
void CWWizParentalCntrl::LoadUserPermission()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		
		m_mapParCtrlUserAccess.clear();
		
		dbSQlite.Open();
		CWardwizSQLiteTable qResultUser = dbSQlite.GetTable("Select USERNAME from WWiz_PC_UserList;");
		Sleep(20);

		for (int iRow = 0; iRow < qResultUser.GetNumRows(); iRow++)
		{
			qResultUser.SetRow(iRow);
			if (qResultUser.GetFieldIsNull(0))
			{
				continue;
			}

			char szUserNameVal[50] = { 0 };
			bool bUserVal = false;
			strcpy_s(szUserNameVal, qResultUser.GetFieldValue(0));

			CStringA csQuery;
			csQuery.Format("select ISCONTROL from WWiz_PC_UserList where USERNAME = '%s';", szUserNameVal);
			CWardwizSQLiteTable qResult = dbSQlite.GetTable(csQuery);
			for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
			{
				qResult.SetRow(iRow);

				if (qResult.GetFieldIsNull(0))
				{
					continue;
				}
				char szUserVal[10] = { 0 };
				strcpy_s(szUserVal, qResult.GetFieldValue(0));
				if (strlen(szUserVal) == 0)
				{
					continue;
				}
				int iRestrictUser = atoi(szUserVal);
				bUserVal = iRestrictUser ? true : false;
			}
			m_mapParCtrlUserAccess.insert(make_pair(szUserNameVal, bUserVal));
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::LoadUserPermission ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckInternetAccessBlock
Description    : To check whether internet access is blocked
Author Name    : Jeena Mariam Saji
Date           : 10 Dec 2019
***********************************************************************************************/
bool CWWizParentalCntrl::CheckInternetAccessBlock()
{
	bool bReturn = false;
	try
	{
		TCHAR szUsrName[MAX_PATH] = { 0 };
		CString csUserName;

		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		GetPrivateProfileString(L"VBSETTINGS", L"CurrentUserName", L"", szUsrName, MAX_PATH, csIniFilePath);
		CT2CA pszConvertedAnsiString(szUsrName);
		std::string strUserName(pszConvertedAnsiString);

		if (strUserName.c_str() == "")
		{
			return bReturn;
		}
		if (CheckUserAllowed(strUserName))
		{
			LoadAppBlockCategoryList();
			if (CheckAppBlockList(L"\\iexplore.exe"))
			{
				bReturn = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckInternetAccessBlock ", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}