
#pragma once


#include <Windows.h>
#include <StrSafe.h>

//#include "CleanMalwareRegEntry.h"	//Commented on 19-02-2019 by Vilas to avoid slow down issue

#pragma comment( lib, "Version.lib" )

class WWHeuristic
{
public:
	WWHeuristic();
	virtual ~WWHeuristic();

	DWORD GetVirusID();
	LPTSTR GetVirusName();

	DWORD GetPEFileInfo(LPCTSTR lpszFileName);

	//Modified on 07 / 08 / 2015 by Vilas
	bool						m_IS64Bit;
	bool						m_ISPEFile;

	//Added by Pradip on 01/11/2018 to remove the registry entries created by malware
	//Commented on 18-12-2018 by Vilas to avoid slow down issue
	//CCleanMalwareRegEntry m_CleanMalwareRegEntry;

private:
	DWORD m_dwVirusID;
	TCHAR m_szVirusName[128];

	DWORD	m_dwFileSize;
	DWORD	m_iBufferOffset;
	HANDLE	m_hFileHandle;

	WORD						m_NumberOfSections;
	WORD						m_PECharacteristics;
	//bool						m_IS64Bit;
	//bool						m_ISPEFile;
	DWORD						m_dwRVA[100] ;
	DWORD						m_dwVS[100] ;
	DWORD						m_dwPRD[100] ;
	DWORD						m_dwSRD[100] ;

	IMAGE_SECTION_HEADER		m_stSectionHeader[100];
	IMAGE_OPTIONAL_HEADER32		m_stImageOptionalHeader32;
	IMAGE_OPTIONAL_HEADER64		m_stImageOptionalHeader64;

	struct VbHeader
	{
		char szVbMagic[0x04];
		WORD wRuntimeBuild;
		char szLangDll[0x0D];
		char szSecLangDll[0x0D];
		WORD wRuntimeRevision;
		DWORD dwLCID;
		DWORD dwSecLCID;
		DWORD lpSubMain;
		DWORD lpProjectData;
		DWORD fMdlIntCtls;
		DWORD fMdlIntCtls2;
		DWORD dwThreadFlags;
		DWORD dwThreadCount;
		WORD wFormCount;
		WORD wExternalCount;
		WORD dwThunkCount;
		DWORD lpGuiTable;
		DWORD lpExternalTable;
		DWORD lpComRegisterData;
		LONG bSZProjectDescription;
		LONG bSZProjectExeName;
		LONG bSZProjectHelpFile;
		LONG bSZProjectName;
	};

	struct Proj_Info
	{
		DWORD dwVersion;
		LONG lpObjectTbl;
		DWORD dwNULL;
		LONG lpCodeStart;
		LONG lpCodeEnd;
		DWORD dwDataSize;
		LONG lpThreadSpace;
		LONG lpaVbaSeh;
		LONG lpNativeCode;
		char szPathInfo[0x210];
		LONG lpExtTbl;
		LONG lpExtCnt;
	};

	struct ComRegData
	{
		DWORD bRegInfo;
		LONG bSZProjectName;
		LONG bSZHelpDirectory;
		LONG bSZProjectDescription;
		char uuidProjectClsId[0x10];
		DWORD dwTlbLcid;
		WORD wUnknown;
		WORD wTlbVerMajor;
		WORD wTlbVerMinor;
	};

	struct ComRegInfo
	{
		LONG bNextObject;
		LONG bObjectName;
		LONG bObjectDescription;
		DWORD dwInstancing;
		DWORD dwObjectId;
		char uuidObject[0x10];
		DWORD fIsInterface;
		LONG bUuidObjectIFace;
		LONG bUuidEventsIFace;
		DWORD fHasEvents;
		DWORD dwMiscStatus;
		DWORD ClassType;
		DWORD fObjectType;
		WORD wToolboxBitmap32;
		WORD wDefaultIcon;
		DWORD fIsDesigner;
		LONG bDesignerData;

	};

	struct Obj_Tbl
	{
		LONG lpHeapLink;
		LONG lpExecProj;
		LONG lpProjInfo2;
		DWORD dwReserved;
		DWORD dwNull;
		LONG lpProjObject;
		BYTE uuidObj[0x10];
		WORD fCompileState;
		WORD TotObj;
		WORD wObjInUse;
		LONG lpObjInArray;
		LONG fIdeFlag;
		LONG lpIdeData;
		LONG lpIdeData2;
		DWORD lpszProjName;
		DWORD dwLcid;
		DWORD dwLcid2;
		LONG lpIdeData3;
		DWORD dwIdentifier;
	};

	VbHeader m_VBHeader;
	ComRegData m_ComRegData;
	ComRegInfo m_ComRegInfo;
	Proj_Info m_Proj_Info;
	Obj_Tbl m_Obj_Tbl;

	LPCTSTR		lpszFilePath;
	DWORD RVA2FileOffset(DWORD dwRva, WORD nSections ,DWORD *pdwFileOff);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwBytesToRead, DWORD * pdwBytesRead);	
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool NewSearchFor(BYTE *pbReadBuffer, DWORD dwBufferSize, BYTE *pszSearchString, DWORD dwSearchStrSize);
	bool Rc4Decryption(BYTE *pbBuffer, DWORD dwBufSize, BYTE *pbKey, DWORD dwKeySize);
	bool GetProductName(LPCTSTR lpszFilePath,  LPTSTR strProductName);
	bool GetResourceDataEntryAddr(DWORD dwEntryId, DWORD dwSubEntryNo, DWORD *pdwAddr);
	bool GetResourceDataEntryAddrEx(BYTE *pszNameEntry, DWORD dwSubEntryNo, DWORD *pdwAddr, DWORD dwSize);
	long hex2dec_c(const char*s, int length);
	DWORD ScanFileForHeuristic();

	bool GetEntropy(BYTE *pbReadBuffer, DWORD dwBufferSize, double *pEntropy);
	bool GetFileBufferEx(LPVOID pbReadBuffer, DWORD dwAddress, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL, bool boType = TRUE);
	bool NewSearchForEx(BYTE *pbReadBuffer, DWORD dwBufferSize, BYTE *pszSearchString, DWORD dwSearchStrSize, DWORD *pdwBytesRead = NULL);
	DWORD GetCntlPtSecInd();

	//Modified for Scan speed by Jyotsna on 11 / 08 / 2015
	//DWORD GetParticularSection(char *pszSecName);
	DWORD GetParticularSection(DWORD dwSecName);
	bool GetExportEntry(IMAGE_EXPORT_DIRECTORY *ExpDirEntry);
	bool GetExportEntryAddr(char *szExpName, DWORD dwExpInd, DWORD *pdwAddress);
	DWORD ror(DWORD dwValue, DWORD dwPlaces);
	DWORD rol(DWORD dwValue, DWORD dwPlaces);
	/////Heuristic detection for Viruses

	DWORD CheckForDaws();
	//DWORD CheckForKedebe();
	DWORD CheckForZBot();
	DWORD CheckForZBotB();
	DWORD CheckForAllaple();
	DWORD CheckForSixer();
	DWORD CheckForSixerStub();
	DWORD CheckForGamarue();
	DWORD CheckForStaser();
	DWORD CheckForSytroO();
	DWORD CheckForSytroP();
	DWORD CheckForAutorunFnc();
	DWORD CheckForDavobevixB();
	DWORD CheckForWBNAIPI();
	DWORD CheckForWBNABUL();
	DWORD CheckForVBNABFUZ();
	DWORD CheckForVobfusBVU();
	DWORD CheckForVobfusVNK();
	DWORD CheckForAntinny();
	DWORD CheckForMSILAgent();
	DWORD CheckForSocksA();
	DWORD CheckForSocksPEQ();
	DWORD CheckForVikingBPMN();//Cleaning
	DWORD CheckForVikingBYMCZ();
	DWORD CheckForHlux();
	DWORD CheckForSwizzor();
	DWORD CheckForFiseriaA();
	DWORD CheckForFiseriaB();
	DWORD CheckForFiseriaC();
	DWORD CheckForFiseriaD();
	DWORD CheckForGepysA();
	DWORD CheckForGepysAA();
	DWORD CheckForUpatreA();
	DWORD CheckForZbotY();
	DWORD CheckForOnlineGamesP();
	DWORD CheckForOnlineGamesNK();
	DWORD CheckForiBryteA();
	DWORD CheckForEggnogA();
	DWORD CheckForOnlineGamesJ();
	DWORD CheckForLollipop();
	DWORD CheckForDelfA();
	DWORD CheckForPicSysC();
	DWORD CheckForJusched();
	DWORD CheckForSocksB();
	DWORD CheckForQukartA();
	DWORD CheckForWenperA();
	DWORD CheckForGepysD();
	DWORD CheckForCryptUU();
	DWORD CheckForKrypting();
	DWORD CheckForKryptingAA();
	DWORD CheckForGepysB();
	DWORD CheckForKedebeB();
	DWORD CheckForKykymberB();
	DWORD CheckForRunouce();//Cleaning
	DWORD CheckForMSILAgentA();
	DWORD CheckForMSILBladabindiP();
	DWORD CheckForBrontokA();
	DWORD CheckForICLoaderA();
	DWORD CheckForKryptingB();

	DWORD GetPEFileVersion(LPCTSTR lpFileName, LPCTSTR lpszItem, LPTSTR lpVersionField);
	DWORD GetVersionInfoBySearch(LPBYTE lpData, DWORD dwlpDataSize, LPCTSTR lpszItem, LPTSTR lpVersionField);

	DWORD CheckForString(LPBYTE lpData, DWORD dwlpDataSize, LPCTSTR lpszSearchString, DWORD dwSearchStringLen);
	DWORD CheckForString(LPBYTE lpData, DWORD dwlpDataSize, LPBYTE lpszSearchString, DWORD dwSearchStringLen);

	DWORD CheckForWogueCABO();
	DWORD CheckForArchSMS();

	DWORD CheckForRootkitTinyAC();


	DWORD CheckForFearsoA();
	DWORD CheckForFearsoB();
	DWORD CheckForStarterY();
	DWORD CheckForWabotA();
	DWORD CheckForMabezatA();//Cleaning
	DWORD CheckForMabezatN();
	DWORD CheckForMabezatB();
	DWORD CheckForWizPopA();
	DWORD CheckForStaserA();
	DWORD CheckForNimdaA();
	DWORD CheckForMortoA();
	DWORD CheckForGipperA();
	DWORD CheckForGipperB();
	DWORD CheckForDowquieA();
	DWORD CheckForFannyA();
	DWORD CheckForQVodA();//Cleaning
	DWORD CheckForMultiplug();
	DWORD CheckForLlacA();
	DWORD CheckForPcClient();
	DWORD CheckForAutorun();
	DWORD CheckForLuderA();//Cleaning
	DWORD CheckForLamechiA();
	DWORD CheckForStartpage();
	DWORD CheckForGameup();
	DWORD CheckForNitol();
	DWORD CheckForNitolA();
	DWORD CheckForAmonetize();
	DWORD CheckForLethicA();
	DWORD CheckForOgimantA();
	DWORD CheckForAspxor();
	DWORD GetWinMainOffset();
	DWORD CheckForAgentA();
	DWORD CheckForResur();//Cleaning
	DWORD CheckForCrytex();//Cleaning
	DWORD CheckForKlez();
	DWORD CheckForShodi();//Cleaning
	DWORD CheckForVelost();//Cleaning
	DWORD CheckForLamer();//Cleaning
	DWORD CheckForDeTroie();//Cleaning
	DWORD CheckForPioneerBr();//Cleaning
	DWORD CheckForExpiroW();//Cleaning
	DWORD CheckForExpiroAI(); // Cleaning
	DWORD CheckForExpiroQ();//cleaning
	DWORD CheckForExpiroB();//Cleaning
	DWORD CheckForExpiroN();//Cleaning
	DWORD CheckForExpiroNr();//Cleaning
	DWORD CheckForRebhipA();
	DWORD CheckForChyodoA();
	DWORD CheckForReconycA();
	DWORD CheckForRuskkillA();
	DWORD CheckForHijackerC();
	DWORD CheckForVBln();
	DWORD CheckForEstiwirA();
	DWORD CheckForPE13FA();

	DWORD CheckForKillMBRU();
	DWORD CheckForAgentCUUA();
	DWORD CheckForDynamerAA();
	DWORD CheckForDynamerTFW();
	DWORD CheckForInjectBBYO();
	DWORD CheckForAmmyA();
	DWORD CheckForCyclerA();

	DWORD CheckForInjectTAX();
	DWORD CheckForRootkitDuquA();

	bool EstiwirA(DWORD dwaddr);
	DWORD CheckForEstiwirB();
	DWORD CheckForLovGateA();
	DWORD CheckForVirutCE();//cleaning
	DWORD CheckForGamehuck();
	DWORD CheckForRubinburd();
	DWORD CheckForTinyCM();
	DWORD CheckForGatorA();
	DWORD CheckForVopack();
	DWORD CheckForTurkojanA();
	DWORD CheckForGhosterA();
	DWORD CheckForFarfliO();
	DWORD CheckForFarfliE();
	DWORD CheckForBinderB();
	DWORD CheckForDelfInjectA();
	DWORD CheckForHupigonFI();
	DWORD CheckForJewdoA();
	DWORD CheckForhdRootA();
	DWORD CheckForKlevateA();
	DWORD CheckForNbddA();

	DWORD CheckForLinkury();
	DWORD CheckForAgentAVW();
	DWORD CheckForInstallCoreBVUE();
	DWORD CheckForGBotAH();
	DWORD CheckForTirrip();
	
	//After 1st july
	DWORD CheckForQuinchSpy();
	DWORD CheckForBlockerA();
	DWORD CheckForDumpyA();
	DWORD CheckForAutorunA();
	DWORD CheckForComameA();
	DWORD CheckForPykspaC();
	DWORD CheckForMsPoserA();
	DWORD CheckForGoldRv();
	DWORD CheckForGamarueUSB();
	DWORD CheckForDirtyCrypt();
	DWORD CheckForTexel();//Cleaning

	DWORD CheckForRefroso();
	DWORD CheckForZBotTVar();
	DWORD CheckForGamarueDLLInUSB();

	DWORD CheckForInstallMateCJ();

	DWORD CheckForJolee();
	
	//12-aug
	DWORD CheckForVBcz();
	DWORD CheckForSalityStub();//Cleaning
	DWORD CheckForFunlove();//Cleaning
	DWORD CheckForDzanC();//Cleaning
	DWORD CheckForPrekAgent();
	DWORD CheckForDuncoA();
	DWORD CheckForSveklaA();
	DWORD CheckForAgentDjcr();
	DWORD CheckForAgentFnfl();
	DWORD CheckForTabMsgSql();
	DWORD CheckForBagleA();
	DWORD CheckForInstBar();
	DWORD CheckForAgentCpyi();
	DWORD CheckForKeyloggerAlok();
	DWORD CheckForPariteB();//Cleaning
	DWORD CheckForWinfetcherG();
	DWORD CheckForDuduA();
	DWORD CheckForNetPassCF();
	DWORD CheckForKloneAF();
	DWORD CheckForAgentgz();
	DWORD CheckForVBEnm();
	DWORD CheckForSwisyn();
	DWORD CheckForDropperNui();
	DWORD CheckForKeylog();
	DWORD CheckForAgentbiox();
	DWORD CheckForAgentAltd();
	DWORD CheckForAgentQef();
	DWORD CheckForRofinA();
	DWORD CheckForAgentAB();
	DWORD CheckForAgentAN();

	//Added on 01 / 10 / 2015 by Jyotsna
	//DWORD CheckForAgentAji();
	//DWORD CheckForAgentDU();
	DWORD CheckForKranetC();
	DWORD CheckForDelfCst();
	DWORD CheckForMurloIdd();
	DWORD CheckForMapleA();
	DWORD CheckForAliBar();
	DWORD CheckForZenoSearchA();
	DWORD CheckForCraagleA();
	DWORD CheckForPexA();
	DWORD CheckForAgentAcb();
	DWORD CheckForLydraJe();
	DWORD CheckForAgentAd();
	DWORD CheckForAgentACAS();

	//Added for detection of Trojan.W64.BitCoinMiner.SX by Vilas on 02 / 11 / 2014
	DWORD CheckForBitCoinMinerSX64();
	DWORD CheckForVilselBQRC();
	DWORD CheckForBrowseFoxK();
	DWORD CheckForBrowseFoxN();
	DWORD CheckForGamarueB();
	DWORD CheckForAgentOCA();
	DWORD CheckForAdwareMultiPlugAOB();
	DWORD CheckForCryptorQF();
	DWORD DownwareSoft32E();
	DWORD AdwareBundloreL();
	DWORD AdwareSoftonicA();
	DWORD DownwareMaxigetHWS();
	

	//Added on 20-04-2017 by Vilas
	DWORD CheckForGamarueAmjul();

	//Added on 01-08-2017 by Nihar
	DWORD CheckForLom();

	//Added on 01-08-2017 by Nihar
	DWORD CheckForWanna_M();
	DWORD CheckForTrojanRansomer();
	DWORD CheckForZerber();
	DWORD CheckForSageCrypt();
	DWORD CheckForLyposit();
	DWORD CheckForMydoom();
	DWORD CheckForShodi_I();
	DWORD CheckForSpyAgent_dfbc();
	DWORD CheckForTrojanAgentb_IP();
	DWORD CheckForCoinMiner_rlo();
	DWORD CheckForCoinMiner_sdi();
	DWORD CheckForCoinMiner_shi();

	//Added on 20-04-2017 by Sagar
	DWORD CheckForDelfGen();
	DWORD CheckForAndromA();
	DWORD CheckForAndromB();
	DWORD CheckForBestaFera();
	DWORD CheckForBestaFeraB();
	DWORD CheckForBestaFeraC();
	DWORD CheckForBestaFeraD();
	DWORD CheckForBestaFeraE();
	DWORD CheckForBestaFeraF();
	DWORD CheckForBestaFeraG();
	DWORD CheckForEkstak();
	DWORD CheckForAdload();
	DWORD CheckForAdloadBx();
	DWORD CheckForDovs();
	DWORD CheckForDovsA();

	//Added on 31-07-2017 by Pradip 
	DWORD CheckForSpyGate();
	//Added on 1-08-2017 by Pradip 
	DWORD CheckForUpantix();
	//Added on 2-08-2017 by Pradip 
	DWORD CheckForTpyn();
	//Added on 3-8-2017
	DWORD CheckForScarOetk();
	//Added on 7-8-2017
    DWORD CheckForScarOhfn();
	//Added on 9-8-2017
	DWORD CheckForPoweliks();
	//Added on 9-8-2017
	DWORD CheckForWBNA();
	//Added on 15-8-2017 by Pradip 
	DWORD CheckForForeignImkmXqx();
	
	//Added on 18-8-2017 by Pradip 
	DWORD CheckForForeign();
	
	//Added on 24-8-2017 by Pradip 
	DWORD CheckForSram();
	
	//Added on 5-9-2017 by Pradip 
	DWORD CheckForGimemo();
	
	//Added on 8-9-2017 by Pradip 
	DWORD CheckForOnion(); 
	
    //Added on 12-9-2017 by Pradip 
	DWORD CheckForSpora();
	
	//Added on 15-9-2017 by Pradip
	DWORD CheckForCrypmod();
	
	//Added on 18-9-2017 by Pradip
	DWORD CheckForMyxaH();
	
	//Added on 25-9-2017 by Pradip
	DWORD CheckForRakhni();

	//Added on 25-9-2017 by Pradip
	DWORD CheckForShipUp();
	
	//Added on 29-9-2017 by Pradip
	DWORD CheckForTakBum();
	
	//Added on 3-10-2017 by Pradip
	DWORD CheckForPihun();

	//Added on 3-10-2017 by Pradip
	DWORD CheckForCryptoWall();

	//Added on 4-10-2017 by Pradip
	DWORD CheckForCryptoWall3();

	//Added on 5-10-2017 by Pradip
	DWORD CheckForFury();
	
	//Added on 9-10-2017 by Pradip
	DWORD CheckForFSWarning();

	//Added on 9-10-2017 by Pradip
	DWORD CheckForCryptoff();
	
	//Added on 10-10-2017 by Pradip
	DWORD CheckForRotor();

	//Added on 10-10-2017 by Pradip
	DWORD CheckForTimer_HCN();

	//Added on 10-10-2017 by Pradip
	DWORD CheckForWLock();

	//Added on 10-10-2017 by Pradip
	DWORD CheckForIshtar();

	//Added on 10-10-2017 by Pradip
	DWORD CheckForPurgen();

	//Added on 11-10-2017 by Pradip
	DWORD CheckForMbro_RV();

	//Added on 11-10-2017 by Pradip
	DWORD CheckForHmBlocker1();

	//Added on 11-10-2017 by Pradip
	DWORD CheckForHmBlocker_WUXD();

	//Added on 11-10-2017 by Pradip
	DWORD CheckForHmBlocker_FBA();

	//Added on 11-10-2017 by Pradip
	DWORD CheckForHmBlocker_ANOU();

	//Added on 12-10-2017 by Pradip
	DWORD CheckForHmBlocker_CADY();
	DWORD CheckForForeign_NCAK();
	DWORD CheckForForeign_NBTP();
	DWORD CheckForFakeInstaller_ALVA();
	
	//Added on 13-10-2017 by Pradip
	DWORD CheckForCryakl_AKV();
	DWORD CheckForCrypren_ACSM();
	DWORD CheckForCrypmod_YIMNO();
	
	//Added on 16-10-2017 by Pradip
	DWORD CheckForShade_KOV();
	DWORD CheckForNSISOnion_YABFG();
	DWORD CheckForNSISOnion_UMX();
	DWORD CheckForRack_GPZI();
		
	//Added on 17-10-2017 by Pradip
	DWORD CheckForNSISOnion_XHO();
	DWORD CheckForNSISOnion_VXY();
	DWORD CheckForNSISOnion_MJSW();
	
	//Added on 18-10-2017 by Pradip
	DWORD CheckForNSISOnion_AEAD();
	DWORD CheckForForeign_NA();
	DWORD CheckForTakbum_HI();
	
	//Added on 23-10-2017 by Pradip
	DWORD CheckForXorist_LR();
	DWORD CheckForSwed_E();
	DWORD CheckForSpora_BC();
	DWORD CheckForSpora_BJ();
	DWORD CheckForSpora_BN();
	DWORD CheckForSpora_BQ();
	DWORD CheckForSpora_BY();
	DWORD CheckForSpora_BW();

	//Added on 24-10-2017 by Pradip
	DWORD CheckForSpora_BM();
	DWORD CheckForSpora_BL();
	DWORD CheckForSpora_BX();
	DWORD CheckForSpora_BV();
	DWORD CheckForSpora_BR();
	
	//Added on 25-10-2017 by Pradip
	DWORD CheckForSpora_BP();
	DWORD CheckForSpora_BO();
	DWORD CheckForSpora_BS();
	
	//Added on 26-10-2017 by Pradip
	DWORD CheckForSpora_BZ();
	DWORD CheckForCryakl_VE();
	
	//Added on 27-10-2017 by Pradip
	DWORD CheckForDorifel_PEK();
	DWORD CheckForXBlocker_BRP();
	DWORD CheckForMatrix_CD();
	DWORD CheckForMatrix_HPQ();	

	//Added on 30-10-2017 by Pradip
	DWORD CheckForMatrix_CVU();
	DWORD CheckForMatrix_GUW();
	DWORD CheckForGenkryptik();
	DWORD CheckForGenkryptik_A();
	DWORD CheckForGenkryptik_B();
	DWORD CheckForGenkryptik_C();
	DWORD CheckForGenkryptik_D();
	DWORD CheckForGenkryptik_E();
		
	//Added on 31-10-2017 by Pradip
	DWORD CheckForGenkryptik_F();

	//Added on 1-11-2017 by Pradip
	DWORD CheckForBitcovarC();
	DWORD CheckForBitcovar_CQR();
	DWORD CheckForBitcovar_CIX();
	DWORD CheckForBitcovar_SZ();
	DWORD CheckForBitcovar_TUW();
	
	//Added on 2-11-2017 by Pradip
	DWORD CheckForSpora_DI();
	DWORD CheckForSpora_DN();
	DWORD CheckForSpora_DC();
	DWORD CheckForSpora_DB();
	DWORD CheckForSpora_DD();
	DWORD CheckForSpora_CT();
	
	//Added on 3-11-2017 by Pradip
	DWORD CheckForSpora_AQ();
	DWORD CheckForSpora_BD();
    DWORD CheckForSpora_AV();
	DWORD CheckForSpora_BA();
	DWORD CheckForSpora_CI();
	DWORD CheckForSpora_BT();
	DWORD CheckForSpora_BLM();
	DWORD CheckForSpora_CF();

	//Added on 6-11-2017 by Pradip
	DWORD CheckForSpora_AU();
	DWORD CheckForSpora_BKL();
	DWORD CheckForSpora_BZC();
	DWORD CheckForSpora_CJ();

	//Added on 7-11-2017 by Pradip
	DWORD CheckForSpora_DJK();
	DWORD CheckForSpora_BFW();	
	
	//Added on 8-11-2017 by Pradip
	DWORD CheckForSpora_AVW();
	DWORD CheckForSpora_BAB();

	//Added on 9-11-2017 by Pradip
	DWORD CheckForCryFile_ZBE();
	DWORD CheckForCrypren_ADRS();

	//Added on 10-11-2017 by Pradip
	DWORD CheckForCrypren_ADS();
	DWORD CheckForAutoRunFNC();

	//Added on 13-11-2017 by Pradip
	DWORD CheckForBadRabbit();
	DWORD CheckForCrypmodadvXH();

	//Added on 14-11-2017 by Pradip
	DWORD CheckForCerber();
	DWORD CheckForXamyhOX();

	//Added on 16-11-2017 by Pradip
	DWORD CheckForCrusisTO();
	DWORD CheckForCerberB();

	//Added on 20-11-2017 by Pradip
	DWORD CheckForSatan();
	DWORD CheckForNasanB();
	DWORD CheckForMaktub();
	DWORD CheckForCryptoDefense();
	DWORD CheckForCritroni();
	DWORD CheckForCritroniA();

	//Added on 21-11-2017 by Pradip
	DWORD CheckForCryptodefYXT();
	DWORD CheckForFareitCPBS();
	DWORD CheckForCryptodefYYP();
	DWORD CheckForCrypVaultL();
	DWORD CheckForZCryptor();

	//Added on 22-11-2017 by Pradip
	DWORD CheckForTorrentLocker();
	DWORD CheckForCryptoLocker();
	DWORD CheckForTelecrypt();
	DWORD CheckForJobCrypter();

	//Added on 23-11-2017 by Pradip
	DWORD CheckForAsterope();
	DWORD CheckForDircrypt();
	DWORD CheckForRadamant();
	DWORD CheckForRokku();

	//Added on 27-11-2017 by Pradip
	DWORD CheckForAutoitDXX();
	DWORD CheckForAutoitDR();
	DWORD CheckForBeaconA();
	DWORD CheckForVBSRunnerXX();

	//Added on 28-11-2017 by Pradip
	DWORD CheckForFilecoderNHQ();
	DWORD CheckForSageCryptMI();
	DWORD CheckForBLC();
	DWORD CheckForCarberpCX();
	DWORD CheckForPaprasFG();

	//Added on 29-11-2017 by Pradip
	DWORD CheckForLockyEMC();
	DWORD CheckForNymaimBA();
	DWORD CheckForNymaimB();

	//Added on 31-07-2017 by Shodhan 
	DWORD CheckForMepaow();
	DWORD CheckForFlyStudioGEN();
	DWORD CheckForKryptikKZ();
	DWORD CheckForDarkKometAACOHOZA();
	DWORD CheckForPincavBLZG();
	DWORD CheckForNeurevtANC();
	DWORD CheckForShifuIU();
	DWORD CheckForSwisynBNER();
	DWORD CheckForAgent();
	DWORD CheckForVBDAIJ();
	DWORD CheckForVobfus();
	DWORD CheckForWaldek();
	DWORD CheckForCosmicDuke();
	DWORD CheckForInject();
	DWORD CheckForHupigon();
	DWORD CheckForPlite();
	DWORD CheckForInjector();
	DWORD CheckForReconyc();
	DWORD CheckForAdLoadWE();
	DWORD CheckForNymaim();
	DWORD CheckForFareit();
	DWORD CheckForYakes();
	DWORD CheckForRegsup();
	DWORD CheckForBlocker();
	DWORD CheckForLocky();	
	DWORD CheckForSysdrop();
	DWORD CheckForDinwod();
	DWORD CheckForDorgam();
	DWORD CheckForMagania();
	DWORD CheckForBublik();
	DWORD CheckForBlueWushu();
	DWORD CheckForCorrempa();
	DWORD CheckForReadsAugur();
	DWORD CheckForVemply();
	DWORD CheckForBlackv();
	DWORD CheckForPornoAsset();
	DWORD CheckForPetya();
	DWORD CheckForBitmanQMF();
	DWORD CheckForCryptorGW();
	DWORD CheckForPurgenDT();
	DWORD CheckForSnocryDCA();
	DWORD CheckForMbro();
	DWORD CheckForCryaklAIV();
	DWORD CheckForRakhniVI();
	DWORD CheckForScatterUB();
	DWORD CheckForCryFile();
	DWORD CheckForBlockerKAAY();
	DWORD CheckForBlockerJZTU();
	DWORD CheckForClicker();
	DWORD CheckForHiddenTear();
	DWORD CheckForInject3();
	DWORD CheckForFakealert();
	DWORD CheckForTobfy();
	DWORD CheckForLmir();
	DWORD CheckForIxeshe();
	DWORD CheckForEmotetEQ();
	DWORD CheckForGupboot();
	DWORD CheckForMbrlock();
	DWORD CheckForPterodo();
	DWORD CheckForQqware();
	DWORD CheckForBho();
	DWORD CheckForDofoil();
	DWORD CheckForKhalesiSP();
	DWORD CheckForMansaboUR();
	DWORD CheckForMansaboVL();
	DWORD CheckForMiner();
	DWORD CheckForMuccEZY();
	DWORD CheckForPakesAVQM();
	DWORD CheckForShipUpBNL();
	DWORD CheckForShipUpBOE();
	DWORD CheckForSnojanBUVF();
	DWORD CheckForSwisynDDFK();
	DWORD CheckForWakmeB();
	DWORD CheckForWecodALL();
	DWORD CheckForGozNymYD();
	DWORD CheckForTofsee();
	DWORD CheckForBlouiroetDU();
	DWORD CheckForBoaxxe();
	DWORD CheckForNeurevt();
	DWORD CheckForSkill_VQV();
	DWORD CheckForSkill_VRE();
	DWORD CheckForTrojan_HILOTI();
	DWORD CheckForBlouiroet_B();
	DWORD CheckForBuzus_NOJX();
	DWORD CheckForHosts2_GEN();
	DWORD CheckForScarsi_APGF();
	DWORD CheckForWin32_TROJANPROXY();
	DWORD CheckForPini_AB();
	DWORD CheckForTrojan_CHINKY();
	DWORD CheckForEncpk_GEN();
	DWORD CheckForTrojan_MIDGARE();
	DWORD CheckForTrojan_MINGGY();
	DWORD CheckForPacked_AUTOIT();
	DWORD CheckForTrojan_RUNDAS();
	DWORD CheckForBublik_BDJB();
	DWORD CheckForWin32_GLUPTEBA();
	DWORD CheckForGozi_A();
	DWORD CheckForWin32_HOUDRAT();
	DWORD CheckForWin32_JORIK();
	DWORD CheckForLlac_KAMP();
	DWORD CheckForWin32_POWP();
	DWORD CheckForWin32_SEARCHPAGE();
	DWORD CheckForSiscos_WHR();
	DWORD CheckForWin32_SPAMMER();
	DWORD CheckForWin32_TINBA();
	DWORD CheckForWecod_ALL();
	DWORD CheckForTrojan_WINLOCK();
	DWORD CheckForWin32_VUNDO();
	
	//Added on 12=03-18
	DWORD CheckForWin32_SNARASITE();
	DWORD CheckForHupigon_TIPV();
	DWORD CheckForPcClient_GEHP();
	DWORD CheckForProrat_NPV();
	DWORD CheckForRbot_GEN();
	DWORD CheckForRansom_HEXZONE();
	DWORD CheckForAgent_NEVPVS();
	DWORD CheckForAntiFW_A();
	DWORD CheckForAntiFW_B();
	DWORD CheckForInject_AHRQN();
	DWORD CheckForInject_AHSUE();
	DWORD CheckForInject_AIDZL();
	DWORD CheckForInject_AIEBG();
	DWORD CheckForInject_AHQTX();
	DWORD CheckForInject_AIEEB();
	DWORD CheckForInject_AIEFC();
	DWORD CheckForInject_AIEFP();
	DWORD CheckForInject_AIEIE();
	DWORD CheckForInject_AIFLU();
	DWORD CheckForPoweliks_DSA();
	DWORD CheckForReconyc_GSKC();
	DWORD CheckForScar_EHCC();
	DWORD CheckForScar_PIZZ();
	DWORD CheckForSwisyn_DDFK();
	DWORD CheckForWin32_ZACCL();
	DWORD CheckForAndromeda_DEX();
	DWORD CheckForBjlog_AABZ();
	DWORD CheckForBlocker_KPUP();
	DWORD CheckForBlocker_KPUQ();
	DWORD CheckForCarberp_APR();
	DWORD CheckForBackdoor_DOUBLEPULSAR();
	DWORD CheckForWin32_KOUTODOOR();
	DWORD CheckForRbot_AEU();
	DWORD CheckForWin32_SHIZ();
	DWORD CheckForSinowal_PML();
	DWORD CheckForArchSMS_HEUR();
	DWORD CheckForNymaim_BDPG();
	DWORD CheckForWin32_PCCLIENT();
	DWORD CheckForWin32_SERVSTART();
	DWORD CheckForWin32_SHYAPE();
	DWORD CheckForWin32_TONMYE();
	DWORD CheckForWin32_CUTWAIL();
	DWORD CheckForWin32_WLORD();
	DWORD CheckForTrojan_Ransom_MBRO();
	
	DWORD CheckForWin32_NOON();
	DWORD CheckForNgrbot_BDIW();
	DWORD CheckForWin32_SHAKBLADES();
	//DWORD CheckForTrojan_SKORIK();
	DWORD CheckForAgentb_BPML();
	DWORD CheckForAgentb_IXBF();
	DWORD CheckForWin32_ANTIFW();
	DWORD CheckForAutoit_CBH();
	DWORD CheckForWin32_BANCTEIAN();
	DWORD CheckForWin32_DIPLE();
	DWORD CheckForWin32_DYNAMER();
	DWORD CheckForInject_AIGUG();
	DWORD CheckForInject_AIICO();
	DWORD CheckForInject_AIMLA();
	DWORD CheckForInject_AIPQG();
	DWORD CheckForInject_AIPVN();
	DWORD CheckForWin32_NEOP();
	DWORD CheckForNymaim_BDPD();
	DWORD CheckForVBKrypt_XABO();
	DWORD CheckForStantinko_BCT();
	DWORD CheckForStantinko_CEH();
	DWORD CheckForStantinko_CER();
	DWORD CheckForTovkater_DBYH();
	DWORD CheckForPornoAsset_CWJB();
	DWORD CheckForAntavmu_ABOW();
	DWORD CheckForWin32_EXPLODER();
	DWORD CheckForWin32_FLOODER();
	DWORD CheckForWin32_HANGPOKE();
	DWORD CheckForWin32_IDSOHTU();
	DWORD CheckForInject_AHZGM();
	DWORD CheckForInject_AHZHI();
	DWORD CheckForAD_ADPOSHEL();
	DWORD CheckForBackdoor_GDOOR();
	DWORD CheckForTrojan_CHANGELING();
	DWORD CheckForTrojan_DNSCHANGE();
	DWORD CheckForTrojan_GENDAL();
	DWORD CheckForTrojan_GOLROTED();
	DWORD CheckForTrojan_GRAFTOR();
	DWORD CheckForTrojan_JORD();
	DWORD CheckForTrojan_JORIK();
	DWORD CheckForInject_AIFBC();
	DWORD CheckForInject_AIFRR();
	DWORD CheckForWakme_MC();
	DWORD CheckForGandCrypt_BU();
	DWORD CheckForGandCrypt_CD();
	DWORD CheckForVobfus_ETSJ();
	DWORD CheckForTrojan_KILLPROC();
	DWORD CheckForTrojan_SCRIBBLE();
	DWORD CheckForTrojan_MINERBOT();
	DWORD CheckForTrojan_PAKES();
	DWORD CheckForPCK_KATUSHA();
	DWORD CheckForPowerShell_ROZENA();
	DWORD CheckForTrojan_RINCUX();
	DWORD CheckForTrojan_SAMCA();
	DWORD CheckForTrojan_SCHOOLGIRL();

	 //Added on 26-09-17 by Nihar
	DWORD CheckForTrojanLuman_A();
	DWORD CheckForLlac_JTOO();
	DWORD CheckForLlac_KZQR();
	DWORD CheckForScar_EXXU();
	DWORD CheckForScar_QBBI();
	DWORD CheckForShelma();
	DWORD CheckForBcex_AVIL();
	DWORD CheckForBcex_AVDY();
	DWORD CheckForStantinko();
	DWORD CheckForLlac_LHOV();
	DWORD CheckForLlac_LHUJ();
	DWORD CheckForAgent_AAGBM();
	DWORD CheckForAgent_ACMCL();
	DWORD CheckForAgent_HUWX();
	DWORD CheckForAgent_ICGH();
	DWORD CheckForAgent_HWGS();
	DWORD CheckForAgent_HWGS_NetPasser();
	DWORD CheckForAgent_IKRL();
	DWORD CheckForAgent_ACBOB();
	DWORD CheckForAgent_AFJCW();
	DWORD CheckForAgent_AGLFJ();
	DWORD CheckForAgent_ALCRO();
	DWORD CheckForTemcac();
	DWORD CheckForAgent_ALDTE();
	DWORD CheckForAgent_HWGW();
	DWORD CheckForAgent_IHGB();
	DWORD CheckForAgent_IKPE();
	DWORD CheckForAgent_NEYAAI();
	DWORD CheckForAgent_NEYXTS();
	DWORD CheckForAgent_NEZHLB();

	//Nihar Trojan.Spy.Zbot
	DWORD CheckForZbot_DJRM();
	DWORD CheckForZbot_HYUS();
	DWORD CheckForZbot_QLGW();
	DWORD CheckForZbot_QSQD();
	
	//Nihar
	DWORD CheckForDomino();
	DWORD CheckForKasidet();
	DWORD CheckForKasidet_BQY();
	DWORD CheckForKasidet_BRA();
	DWORD CheckForKasidet_BRE();
	DWORD CheckForKasidet_BRF();
	DWORD CheckForKasidet_BRI();
	DWORD CheckForKasidet_BRJ();
	DWORD CheckForKasidet_BRK();
	DWORD CheckForKasidet_BRM();
	DWORD CheckForKasidet_BRN();
	DWORD CheckForKasidet_BRP();
	DWORD CheckForKasidet_BRR();
	DWORD CheckForKasidet_BRS();
	DWORD CheckForKasidet_BTX();
	DWORD CheckForKasidet_BUI();
	DWORD CheckForKasidet_BUK();
	DWORD CheckForKasidet_BUL();
	DWORD CheckForKasidet_BUM();
	DWORD CheckForKasidet_BUO();
	DWORD CheckForKasidet_BUS();
	DWORD CheckForKasidet_BUU();
	DWORD CheckForKasidet_BUW();
	DWORD CheckForKasidet_BUZ();
	//DWORD CheckForKasidet_BVB();
	DWORD CheckForKasidet_BVC();
	DWORD CheckForKasidet_BVQ();
	DWORD CheckForKasidet_BVR();
	DWORD CheckForKasidet_BVS();
	DWORD CheckForKasidet_BVT();
	DWORD CheckForKasidet_BVU();
	
	
	//Nihar Trojan.Win32.Agent.nfadbf
	DWORD CheckForAgent_NFADBF();
	DWORD CheckForAgent_NFADBF_DLL();

	DWORD CheckForSchoolGirl();
	
	
	//Nihar
	DWORD CheckForBoht_LV();
	DWORD CheckForBroskod_RB();
	DWORD CheckForAntiFw_B();
	DWORD CheckForBublik_BKFJ();
	DWORD CheckForBublik_CRED();
	DWORD CheckForBuzus_XYCX();
	DWORD CheckForCrypt_CYM();
	DWORD CheckForEmager_NGB();
	DWORD CheckForInject_AZGW();
	DWORD CheckForJorikShakblades_DQL();
	DWORD CheckForJorikVobfus_AIJW();
	DWORD CheckForLunam_A();
	DWORD CheckForPatched_PJ();
	DWORD CheckForStarter_YY();
	DWORD CheckForVBKryjetor_ALPX();
	DWORD CheckForPoison_IJWT();
	DWORD CheckForPoison_IJYN();
	DWORD CheckForLoskad_IGM();
	//Sagar
	DWORD CheckForRanTest();
	DWORD CheckForAndrom_Qxe();
	DWORD CheckForAndrom_Ihaf();
	DWORD CheckForAndrom_Nxzb();
	DWORD CheckForBitman_Aciv();
	
	//Nihar -26 Variants
	DWORD CheckForPoison_IJZS();
	DWORD CheckForPoison_IKCC();
	DWORD CheckForPoison_IKVV();
	DWORD CheckForVBKrypt_YCFV();

	//Nihar 
	DWORD CheckForBanbra_VSJF();
	DWORD CheckForShifu_EPA();
	DWORD CheckForGenome_VRLA();
	DWORD CheckForAgent_FQVK();
	//Nihar23-10-17
	DWORD CheckForAgent_FQVK_GO();
	DWORD CheckForAgent_FQVK_GO2();
	DWORD CheckForAgent_FQVK_INSTALL();
	DWORD CheckForAgent_FQVK_SETUP();
	DWORD CheckForAgent_FQVK_PLAYER();
	DWORD CheckForKasidet_BZE();
	DWORD CheckForKasidet_CAX();
	DWORD CheckForKasidet_CDI();
	DWORD CheckForKasidet_CDO();
	DWORD CheckForKasidet_CDQ();
	DWORD CheckForKasidet_CDU();
	DWORD CheckForKasidet_CDV();
	DWORD CheckForKasidet_CDY();
	DWORD CheckForKasidet_CDZ();
	DWORD CheckForKasidet_CEA();
	DWORD CheckForKasidet_CED();
	DWORD CheckForKasidet_CEL();
	DWORD CheckForKasidet_CEM();
	DWORD CheckForDisfa_MFQQ();
	DWORD CheckForDisfa_MFSJ();
	DWORD CheckForDisfa_MFYM();
	DWORD CheckForDisfa_MGAX();
	DWORD CheckForStarter_D();
	DWORD CheckForStarter_D_REPLAY();
	DWORD CheckForStarter_D_RINST();
	DWORD CheckForBuzus_YBIB();
	DWORD CheckForFsysna_WL();
	DWORD CheckForScar_QHVH();
	DWORD CheckForSchoolBoy_ATH();
	DWORD CheckForBitman_ACZZ();
	DWORD CheckForDisfa_LAQN();
	DWORD CheckForDisfa_LRXP();
	DWORD CheckForDisfa_MFJP();
	DWORD CheckForDisfa_MFPU();
	DWORD CheckForDisfa_MFUB();
	DWORD CheckForDisfa_MFUZ();
	DWORD CheckForDisfa_MFZC();
	DWORD CheckForDisfa_MFZO();
	DWORD CheckForDisfa_MFZQ();
	DWORD CheckForDisfa_MGAH();
	DWORD CheckForDisfa_MGAJ();
	DWORD CheckForDisfa_MGAM();
	DWORD CheckForDisfa_MGAT();
	DWORD CheckForDisfa_MGBB();
	DWORD CheckForDisfa_MGBK();
	DWORD CheckForDisfa_MGBL();
	DWORD CheckForDisfa_MGBY();
	DWORD CheckForDisfa_MGCG();
	DWORD CheckForDisfa_MGCI();
	DWORD CheckForDisfa_MGDH();
	DWORD CheckForDisfa_MGEM();
	
	
	//25-10-17
	DWORD CheckForDisfa_MGFC();
	DWORD CheckForDisfa_MGFD();
	DWORD CheckForDisfa_MGFO();
	DWORD CheckForDisfa_MGFQ();
	DWORD CheckForDisfa_MGIZ();
	DWORD CheckForDisfa_MGJD();
	DWORD CheckForDisfa_MGJR();
	DWORD CheckForDisfa_MGJS();
	DWORD CheckForDisfa_MGJX();
	DWORD CheckForCrypt_CYUQ();
	DWORD CheckForCrypt_EMLQ();

	//26-10-17
	DWORD CheckForCrypt_EZEI();
	DWORD CheckForCrypt_FARP();
	DWORD CheckForCrypt_FBCV();
	DWORD CheckForCrypt_FBEM();
	DWORD CheckForCrypt_FBWE();
	DWORD CheckForCoinminer_TBN();
	DWORD CheckForNanoBot_YEG();
	DWORD CheckForNanoBot_ZON();
	DWORD CheckForNanoBot_ZQI();
	DWORD CheckForNanoBot_ZQV();
	DWORD CheckForNanoBot_ZRB();
	DWORD CheckForNanoBot_ZRN();
	DWORD CheckForNanoBot_ZSR();
	DWORD CheckForNanoBot_ZTF();
	DWORD CheckForNanoBot_ZTU();
	DWORD CheckForNanoBot_ZTV();
	DWORD CheckForNanoBot_ZUA();
	DWORD CheckForAgent_BUA();
	DWORD CheckForAutorun_BUT();
	DWORD CheckForAutorun_ICP();
	DWORD CheckForAutorun_VX();
	DWORD CheckForDebris();
	DWORD CheckForMabezat_B();
	DWORD CheckForPondFull_A();
	DWORD CheckForRemoh_AH();
	DWORD CheckForVBNA_ALXM();
	DWORD CheckForVebad_A();
	DWORD CheckForVobfus_DHTQ();
	DWORD CheckForVobfus_EHRW();
	DWORD CheckForNsisAgent_EA();
	DWORD CheckForAgent_HNQS();
	DWORD CheckForCoinminer_TBN_BOT();
	
	//Sagar
	DWORD CheckForLunam();
	DWORD CheckForShiz();
	DWORD CheckForNgrbot_Ou();
	DWORD CheckFoNgrbot_DHX();
	DWORD CheckFoShade_Yn();
	DWORD CheckFoShade_Mwa();
	DWORD CheckFoTovkater();
	DWORD CheckForDarkKomet_Alkk();
	DWORD CheckForBandok();

	DWORD CheckForVB_Fer();
	DWORD CheckForFarfli_W();
	DWORD CheckForFsysna_Eoco();
	DWORD CheckForVilselCumm();
	DWORD CheckForPadodor();
	DWORD CheckForQuchiSpy();
	DWORD CheckForPophot();
	DWORD CheckForEmotet();
	DWORD CheckForShiFu();
	DWORD CheckForShiFu_Epa();
	
	
	//Client issue -HemantPatel 
	//Added by Nihar
	//Date-18-01-18
	DWORD CheckForAgent_HESF();

	//Added on 30-11-2017 by Pradip
	DWORD CheckForZurgopC();
	DWORD CheckForNymaimEFUL();
	DWORD CheckForRuKometa();

	//Added on 1-12-2017 by Pradip
	DWORD CheckForLockyCF();
	DWORD CheckForLockyA();
	DWORD CheckForLockyB();
	DWORD CheckForSporaGEN();

	//Added on 4-12-2017 by Pradip
	DWORD CheckForLockyAC();
	DWORD CheckForLockyAD();
	DWORD CheckForLockyAE();
	DWORD CheckForSporaCP();
	DWORD CheckForSporaCR();
	DWORD CheckForSporaCM();

	//Added on 5-12-2017 by Pradip
	DWORD CheckForFuryEW();
	DWORD CheckForForeignNPRC();
	DWORD CheckForBlockerD();
	DWORD CheckForSageCryptJC();
	DWORD CheckForForeignNPW();
	DWORD CheckForForeignNPY();

	//Added on 6-12-2017 by Pradip
	DWORD CheckForFuryH();
	DWORD CheckForFuryJ();
	DWORD CheckForPurgenFS();
	DWORD CheckForPurgenFK();
	DWORD CheckForLockyXT();
	DWORD CheckForLockyAB();

	//Added on 7-12-2017 by Pradip
	DWORD CheckForLockyABD();
	DWORD CheckForFuryI();
	DWORD CheckForShadeOB();
	DWORD CheckForShadeNY();
	DWORD CheckForLockyDMR();

	//Added on 8-12-2017 by Pradip
	DWORD CheckForPurgenGC();
	DWORD CheckForPurgenGD();

	//Added on 13-12-2017 by Pradip
	DWORD CheckForBlueScreenNA();
	DWORD CheckForMatrixF();
	DWORD CheckForMatrixG();
	DWORD CheckForMatrixE();
	DWORD CheckForMatrixFC();
	DWORD CheckForMatrixEN();
	DWORD CheckForCrusisQH();
	DWORD CheckForCrusisSE();
	DWORD CheckForGimemoCDQU();
	DWORD CheckForSomhoveranC();

	//Added on 14-12-2017 by Pradip
	DWORD CheckForPurgaAF();
	DWORD CheckForShadeLPY();
	DWORD CheckForShadeLQC();
	DWORD CheckForBadRabbitE();

	//Added on 15-12-2017 by Pradip
	DWORD CheckForShadeNUW();
	DWORD CheckForWinlock();
	DWORD CheckForDelfSE();

	//Added on 18-12-2017 by Pradip
	DWORD CheckForShadeMR();
	DWORD CheckForPetYa();

	//Added on 19-12-2017 by Pradip
//	DWORD CheckForCryFile();
	DWORD CheckForForeignNJIQ();

	//Added on 20-12-2017 by Pradip
	DWORD CheckForSporaDGK();
	DWORD CheckForSporaDNT();
	DWORD CheckForSporaDOJ();
	DWORD CheckForSporaDQW();
	DWORD CheckForSporaDQZ();
	DWORD CheckForSporaDRD();
	DWORD CheckForSnocryDAJ();
	DWORD CheckForSnocryDGI();

	//Added on 22-12-2017 by Pradip
	DWORD CheckForLockyAFT();
	DWORD CheckForLockyADH();

	//Added on 23-12-2017 by Pradip
	DWORD CheckForSporaEMU();
	DWORD CheckForSporaELG();
	DWORD CheckForSporaDZB();

	//Added on 25-12-2017 by Pradip
	DWORD CheckForSporaD();
	DWORD CheckForSporaBY();

	//Added on 26-12-2017 by Pradip
	DWORD CheckForShadeMWA();
	DWORD CheckForShadeMQK();
	DWORD CheckForPurgenLD();

	//Added on 2-1-2018 by Pradip
	DWORD CheckForPurgenFA();
	DWORD CheckForShadeNDJ();

	//Added on 3-1-2018 by Pradip
	DWORD CheckForShadeNGJ();
	DWORD CheckForShadeNSD();

	//Added on 4-1-2018 by Pradip
	DWORD CheckForPurgenFP();
	DWORD CheckForPurgenFW();

	//Added on 5-1-2018 by Pradip
	DWORD CheckForPurgenJM();
	DWORD CheckForPurgenMY();

	//Added on 08-1-2018 by Pradip
	DWORD CheckForPurgaK();
	DWORD CheckForShadeLNO();
	DWORD CheckForPurgenHD();
	DWORD CheckForSporaEFY();
	DWORD CheckForSporaDYT();

	//Added on 09-1-2018 by Pradip
	DWORD CheckForSporaDQQ();
	DWORD CheckForSporaBBQ();
	DWORD CheckForSporaDQE();

	//Added on 10-1-2018 by Pradip
	DWORD CheckForShadeMFR();
	DWORD CheckForShadeMII();
	DWORD CheckForSporaDRO();
	DWORD CheckForSporaCCM();
	DWORD CheckForSporaCCT();

	//Added on 11-1-2018 by Pradip
	DWORD CheckForSporaCDR();
	DWORD CheckForSporaCDU();
	DWORD CheckForSporaCEG();
	DWORD CheckForSporaCGG();
	DWORD CheckForSporaCJP();
	DWORD CheckForSporaCJ();

	//Added on 12-1-2018 by Pradip
	DWORD CheckForShadeLTL();
	DWORD CheckForShadeLSE();

	//Added on 15-1-2018 by Pradip
	DWORD CheckForGenomeCHS(); 
	DWORD CheckForScarMULV();
	DWORD CheckForSysnBQYM();
	DWORD CheckForScarOICH(); 
	DWORD CheckForSramlerJ();

	//Added on 16-1-2018 by Pradip
	DWORD CheckForPurgenACD();
	DWORD CheckForPurgenAEM();

	//Added on 17-1-2018 by Pradip
	DWORD CheckForAgentABQK();
	DWORD CheckForGenGSV();
	DWORD CheckForRackGSV();

	//Added on 22-1-2018 by Pradip
	DWORD CheckForForeignNXL();
	DWORD CheckForLockyABEB();
	DWORD CheckForLockyAOC();
	
	//Added on 31-1-2018 by Pradip
	DWORD CheckForAutoRunHFP();
	DWORD CheckForPatchedLW();
	
	
	
	//Nihar -Merging on 23-01-18
	//6-11-17
	DWORD CheckForDapato_OHLP();
	DWORD CheckForDapato_OZLB();
	DWORD CheckForFraudrop();
	
	//7-11-17
	DWORD CheckForDisfa_BOP();//2
	DWORD CheckForDisfa_LYZO();
	DWORD CheckForDisfa_MHNH();
	DWORD CheckForDisfa_MHNP();
	DWORD CheckForDisfa_MHNS();
	DWORD CheckForDisfa_MHNX();
	DWORD CheckForDisfa_MHPD();
	DWORD CheckForDiple_EOYN();
	DWORD CheckForDiztakun_BCLT();
	DWORD CheckForInject_BBYO();
	
	//9-11-17
	DWORD CheckForRegrun_PKE();
	
	//13-11-17
	//51 variants in total 13-14-15 Poison family
	DWORD CheckForPoison_ITWS();
	DWORD CheckForPoison_ITXT();
	DWORD CheckForPoison_IUCV();
	DWORD CheckForPoison_IUHX();
	DWORD CheckForPoison_IUIN();
	DWORD CheckForPoison_IUIQ();
	DWORD CheckForPoison_IUIS();
	DWORD CheckForPoison_IUJM();
	DWORD CheckForPoison_IUKM();	
	DWORD CheckForPoison_IUNX();
	
	//14-11-17
	DWORD CheckForPoison_CJBB();
	DWORD CheckForPoison_FQNI();
	
	//15-11-17
	DWORD CheckForPosion_GGRF();
	DWORD CheckForPoison_ITOB();
	DWORD CheckForPoison_ITPI();
	DWORD CheckForPoison_ITTY();
	DWORD CheckForPoison_ITUB();//3
	DWORD CheckForPoison_ITWI();//9
	DWORD CheckForPoison_UN();
	DWORD CheckForUrsnif_UKF();
	DWORD CheckForUrsnif_UKG();
	DWORD CheckForUrsnif_UKH();
	
	//16-11-17
	DWORD CheckForUrsnif_UKI();
	DWORD CheckForUrsnif_UKJ();
	DWORD CheckForUrsnif_UKK();
	DWORD CheckForUrsnif_UKL();
	DWORD CheckForUrsnif_UKM();
	DWORD CheckForCoinMiner_SZJ();//2
	DWORD CheckForCoinMiner_SZJ_BOT();
	DWORD CheckForCoinMiner_SZJ_WINHOST();
	
	
	//20-11-17
	DWORD CheckForCoinMiner_TIX();
	DWORD CheckForCoinMiner_TIX_WINSVC();//2
	DWORD CheckForCoinMiner_TJK_BOT();
	DWORD CheckForCoinMiner_TJK_WINHOST();
	DWORD CheckForCoinMiner_TPT();
	DWORD CheckForCoinMiner_TRP();
	DWORD CheckForCoinMiner_TVP();//2
	DWORD CheckForAgent_BJTEBZ();
	DWORD CheckForAgent_BJTEBZ_HEX();
	DWORD CheckForAgent_BJTLIX();
	
	//21-11-17
	DWORD CheckForAgent_NOCY_MAL();
	DWORD CheckForAgent_NOCY_DLL();
	DWORD CheckForAgent_NOCY_MSSUP();
	DWORD CheckForAgent_SBHL();
	
	//22-11-17
	DWORD CheckForAutoIt_ABCFAZ();
	DWORD CheckForAutoIt_BVA();
	
	//28-11-17
	DWORD CheckForSysn_BFNW();
	DWORD CheckForSysn_BFNW_KOXP();
	
	
	//29-11-17
	DWORD CheckForDaws_BGQV();
	DWORD CheckForDinwod_AEMU();
	DWORD CheckForDinwod_AEMV();
	DWORD CheckForDorgam_WZK();
	DWORD CheckForFrauDrop_ALXXR();//2
	DWORD CheckForFrauDrop_ALXXT();
	
	//30-11-17
	DWORD CheckForInjector_TESZ();
	DWORD CheckForInjector_TEVF();
	DWORD CheckForInjector_TEYK();
	DWORD CheckForInjector_TFPA();
	DWORD CheckForInjector_TFPD();
	DWORD CheckForInjector_TFQS();
	DWORD CheckForNecrus_AANH();
	DWORD CheckForNSIS_AUPI();
	DWORD CheckForScrop_CCM();
	DWORD CheckForScrop_CZY();
	DWORD CheckForScrop_DAX();
	DWORD CheckForScrop_DCI();
	DWORD CheckForScrop_DDK();
	
	//1-12-17
	DWORD CheckForSysn_CGZC();
	
	
	//4-12-2017
	DWORD CheckForSysn_CHAL();
	DWORD CheckForSysn_CHAT();
	DWORD CheckForSysn_CHAZ();
	DWORD CheckForXPAntivirus_WPM();
	DWORD CheckForGamethief_DLG();
	DWORD CheckForLethic_AFCC();
	DWORD CheckForLethic_AFCL();
	
	//5-12-2017
	DWORD CheckForChisburg_ADIJ();
	DWORD CheckForChisburg_AHUE();
	DWORD CheckForChisburg_AHUF();
	DWORD CheckForChisburg_AHXD();
	DWORD CheckForChisburg_AHYD();
	DWORD CheckForChisburg_AHYW();
	
	DWORD CheckForFareit_DEJV();
	DWORD CheckForFareit_DESM();
	DWORD CheckForFareit_DITO();
	DWORD CheckForFareit_DIYJ();
	DWORD CheckForFareit_DJAY();
	DWORD CheckForFareit_DJKO();
	DWORD CheckForFareit_DJKQ();
	DWORD CheckForFareit_DJKZ();//5
	DWORD CheckForFareit_DJLY();
	
	
	//8/12/17
	DWORD CheckForInject_AAEAK();
	DWORD CheckForInject_AEFFA();
	
	//12-12-17
	DWORD CheckForAgent_IFDX();
	DWORD CheckForAgent_IHFK();
	DWORD CheckForAgent_NETXLE();
	DWORD ChekForAgent_NEVPVS();
	DWORD CheckForAgent_NEZOED();
	DWORD CheckForAgent_NEZUSZ();
	DWORD CheckForAgent_NEZVFI();
	
	
	//13-12-17
	DWORD CheckForAgent_XJGJ();
	DWORD CheckForAgent_BQYR();
	
	
	//27-12-17
	DWORD CheckForAgent_BJTHKF();
	DWORD CheckForAgent_BJTOMN();
	DWORD ChecKForAgent_BJTOMT();
	DWORD CheckForAgent_BJTQUM();
	DWORD CheckForAgent_IASN();
	
	//28-12-17
	DWORD CheckForAgent_SBMC();
	DWORD CheckForAutoit_BTG();
	DWORD CheckForSysn_CHLH();
	DWORD CheckForSysn_CHLZ();//18
	DWORD CheckForInject_WNME();
	
	
	//2-01-18
	DWORD CheckForInject_AHXCK();
	DWORD CheckForAutoIT_FJB();
	DWORD CheckForAutoIT_FFZ();
	DWORD CheckForAPosT_CMG();
	DWORD ChecKForAgentb_bvhd();
	DWORD ChecKForAgent_NFAFND();
	DWORD CheckForAgent_NEUDHK();
	DWORD CheckForQhost_BFR();
	DWORD CheckForDisfa_MPVY();
	
	//3-01-18
	DWORD CheckForDisfa_BOP_CHEAT();
	DWORD CheckForDisfa_BOP_EXPLORE();
	DWORD CheckForAgent_NEVUBF();
	DWORD CheckForAgent_QWFEQM();
	DWORD CheckForAgentb_JAN();
	DWORD CheckForScar_OKVT();
	DWORD CheckForZonidel_CDP();
	DWORD CheckForNgrbot_DHX();
	DWORD CheckForNgrbot_OU();
	DWORD CheckForRecyl_AFR();
	DWORD CheckForSkor_BEST();
	DWORD CheckForVbna_AXWF();
		
	//15-01-18 30 variants
	DWORD CheckForVobfus_AIGR();
	DWORD CheckForWBNA_OAV();
	DWORD CheckForPoison_IZJB();
	DWORD CheckForLlac_CJDS();
	DWORD CheckForLlac_KXLI();
	DWORD CheckForLlac_LJXT();
	DWORD CheckForLoader_O();
	DWORD CheckForMenti_GEN();
	DWORD CheckForMucc_CED();
	DWORD CheckForNeurevt_ANC();
	DWORD CheckForNymaim_BDJI();
	DWORD CheckForNymaim_BDJK();
	DWORD CheckForPirminay_BAFY();
	DWORD CheckForScar_QMYI();

	//16-01-18
	DWORD CheckForGozNym_CBU();
	DWORD CheckForGozNym_CBY();
	DWORD CheckForGozNym_CCE();
	DWORD CheckForGozNym_CCG();
	DWORD CheckForGozNym_CCJ();
	DWORD CheckForGozNym_CQQ();
	DWORD CheckForGozNym_CQR();
	DWORD CheckForGozNym_CQU();
	
	//19-01-18
	DWORD CheckForGozNym_CQW();
	DWORD CheckForGozNym_CQX();
	DWORD CheckForGozNym_CQY();
	DWORD CheckForGozNym_CQZ();
	DWORD CheckForGozNym_CRA();
	DWORD CheckForGozNym_CRB();
	DWORD CheckForGozNym_CRC();
	DWORD CheckForGozNym_CRD();
	DWORD CheckForGozNym_CRE();
	DWORD CheckForGozNym_CRF();
	//20-01-18
	DWORD CheckForGozNym_CRH();
	DWORD CheckForGozNym_CRI();
	DWORD CheckForGozNym_CRJ();
	DWORD CheckForGozNym_CRK();
	DWORD CheckForGozNym_CRL();
	DWORD CheckForGozNym_CRM();
	//21-01-18
	DWORD CheckForGozNym_CRO();
	DWORD CheckForGozNym_CRQ();
	DWORD CheckForGozNym_CZW();
	DWORD CheckForGozNym_DEI();
	DWORD CheckForTpyn_CHU();
	DWORD CheckForRevenge_BOC();
	DWORD CheckForRevenge_BC();
	DWORD CheckForVBKrypt_CDKR();
	//22-01-18
	DWORD CheckForVBKrypt_CDSK();
	DWORD CheckForVBKrypt_CIRK();
	DWORD CheckForVBKrypt_CIXE();
	DWORD CheckForVBKrypt_CJCC();
	DWORD CheckForVBKrypt_DRHE();
	DWORD CheckForVBKrypt_JGW();
	DWORD CheckForVBKrypt_NRWW();
	
	
	
	//Sagar Merged 23-01-18
	//6-11-17
	DWORD CheckForArdurk();
	DWORD CheckForLethic();
	DWORD CheckForLethicB();
	DWORD CheckForTaraytAn();
	//08-11-17
	DWORD CheckForTaraytAo();
	DWORD CheckForTaraytT();
	DWORD CheckFoZbot_svfs();
	DWORD CheckForAutoIt_Aea();
	DWORD CheckForAutoIt_dn();
	DWORD CheckForAutoIt_sl();
	DWORD CheckForAutoIt_sv();

	//09-11-17
	DWORD CheckForNaKocTbA();
	DWORD CheckForNaKocTbB();

	DWORD CheckForCarberp();
	DWORD CheckForTepfer();
	DWORD CheckForQQPass();
	DWORD CheckForLolojan();
	DWORD CheckForSecuritySphere();
	DWORD CheckForDapato_OHQC();
	DWORD CheckForVB_ZQS();
	DWORD CheckForPincav();
    DWORD CheckForRefinkaA();
	DWORD CheckForRefinkaB();
	
	//16-11-2017
	DWORD CheckForRefinkaC();
	DWORD CheckForRefinkaD();
	DWORD CheckForRefinkaE();
	DWORD CheckForRefinkaF();
	DWORD CheckForRefinkaG();
	DWORD CheckForRefinkaH();
	DWORD CheckForRefinkaI();
	DWORD CheckForNeurevt_ZWJ();
	DWORD CheckForNeurevt_ZVT();
	DWORD CheckForNeurevt_ZWS();
	DWORD CheckForNeurevt_ZWU();
	DWORD CheckForNeurevt_ZWV();
	DWORD CheckForNeurevt_ZWW();
	
	//23-11-2017
	DWORD CheckForFeljina();
	DWORD CheckForGTbot();
	DWORD CheckForPushbot();
	DWORD CheckForJuched_FIE();
	DWORD CheckForJuched_FIA();
	DWORD CheckForJuched_FJA();
	DWORD CheckForMydoom_M();
	DWORD CheckForMydoom_I();
	DWORD CheckForBrontok_N();
	DWORD CheckForBloored_D();
	DWORD CheckForHoundhack();

	//22-11-17
	DWORD CheckForOmaneat_AWA();
	DWORD CheckForOmaneat_BSZ();
	DWORD CheckForPlite_bhuz();
	DWORD CheckForCabby_zipxi();
	DWORD CheckForCabby_zipzp();
	DWORD CheckForCabby_zipzq();
	DWORD CheckForDagozill_mh();
	DWORD CheckForDagozill_md();
	DWORD CheckForGeral_bpu();
	DWORD CheckForGootkit_bhf();
	DWORD CheckForGootkit_bhj();
	DWORD CheckForGootkit_bhl();
	DWORD CheckForDorifel_atgs();

	//29-11-17
	DWORD CheckForDorifel_atgu();
	DWORD CheckForDorifel_atha();
	DWORD CheckForDorifel_athc();
	DWORD CheckForDorifel_atvs();
	DWORD CheckForDorifel_azqc();
	DWORD CheckForDorifel_azqe();
	DWORD CheckForIcefog_AS();
	DWORD CheckForKonus_dq();
	DWORD CheckForOmaneat_aaw();
	DWORD CheckForOmaneat_kg();
	DWORD CheckForOmaneat_act();
	DWORD CheckForOmaneat_alm();
	//06-12-2018
	DWORD CheckForOmaneat_axc();
	DWORD CheckForOmaneat_bfg();
	DWORD CheckForOmaneat_btx();
	DWORD CheckForOmaneat_epc();
	DWORD CheckForOmaneat_eyc();
	DWORD CheckForOmaneat_eyg();
	DWORD CheckForCitadel();
	DWORD CheckForTinba_lyb();

	////13-12-17
	DWORD CheckForTurkojan_zwh();
	DWORD CheckForVawtrak_acx();
	DWORD CheckForXtreme_bqj();
	DWORD CheckForQhost_bfn();
	DWORD CheckForStarter_du();
	DWORD CheckForVobfus_escx();
	DWORD CheckForVobfus_ekal();
	DWORD CheckForVobfus_eyyc();
	DWORD CheckForVobfus_fcga();
	DWORD CheckForVobfus_fdit();
	DWORD CheckForVobfus_fdja();
	DWORD CheckForSelfDel_arha();
	DWORD CheckForShipUp_boe();
	DWORD CheckForShipUp_bqa();
	DWORD CheckForSnovir_eer();

	//18-12-17
	DWORD CheckForVilsel_bpxe();
    DWORD CheckForCayu_eos();
	DWORD CheckForExtenBro_c();
	DWORD CheckForCtms_os();
	DWORD CheckForSmall_sh();
	DWORD CheckForStaser_bokg();
	DWORD CheckForTrickster_bhk();
	DWORD CheckForTvt_ll();
	DWORD CheckForAndromeda_dex();
	DWORD CheckForAutoHK_cd();
	DWORD CheckForFipp_a();
	DWORD CheckForBlouiroet_GEN();
	DWORD CheckForKillAV_JI();
	DWORD CheckForMiner_TICS();
	DWORD CheckForSnarasite_D();
	DWORD CheckForGhoul_GX();
	DWORD CheckForGhoul_HI();
	DWORD CheckForGhoul_HO();
	DWORD CheckForGhoul_LJ();

	//3-01-2018
	DWORD CheckForGhoul_ML();
	DWORD CheckForGhoul_MM();
	DWORD CheckForGhoul_MO();
	DWORD CheckForGhoul_MP();
	DWORD CheckForGhoul_MQ();
	DWORD CheckForGhoul_MR();
	DWORD CheckForGhoul_NH();
	DWORD CheckForGhoul_ZV();
	DWORD CheckForQukart_VIK();
	DWORD CheckForQukart_VJH();
	DWORD CheckForDetnat_E();
	DWORD CheckForPondfull_A();

  //15-01-18
	DWORD CheckForShakblades_XFP();
	DWORD CheckForSkor_BEQC();
	DWORD CheckForNjrat_PO();
	DWORD CheckForNjrat_PU();
	//16-01-18
	DWORD CheckForAndrom_OVMT();
	DWORD CheckForAndrom_OVQH();
	DWORD CheckForAndrom_OVQR();
	DWORD CheckForAndrom_OWDS();
	//17-01-18
	DWORD CheckForAndrom_OWDT();
	DWORD CheckForAndrom_OWDU();
	DWORD CheckForAndrom_OWDV();
	//18-01-18
	DWORD CheckForAndrom_OWDX();
	DWORD CheckForAndrom_OWEG();
	DWORD CheckForMucc_FIL();
	//19-01-18
	DWORD CheckForWauchos_A();
	DWORD CheckForWdfload_BVG();
	DWORD CheckForDinwod_AEWB();

	
	//Nihar post 23-01-18 merging
	DWORD CheckForTurkojan_ZWH();
	DWORD CheckForVawtrak_QU();
	DWORD CheckForWinnti_OV();
	DWORD CheckForZAccess_FJYS();
	DWORD CheckForZAccess_FJYT();
	DWORD CheckForZegost_MTGAO();
	DWORD CheckForZegost_MTGAP();
	DWORD CheckForVemply_GEN();
	DWORD CheckForAgent_GEN();
	DWORD CheckForBlueWushu_GEN();
	DWORD CheckForCrypt_GEN();

	DWORD CheckForAndrom_A();
	DWORD CheckForAndrom_OECH();
	DWORD CheckForDreambot_BW();
	DWORD CheckForBypassUAC_DOM();
	DWORD CheckForAgent_XXDGUT();
	DWORD CheckForMultiPlug_NBJA();
	

	DWORD CheckForDiple_GXFN();
	DWORD CheckForInject_ACERR();
	DWORD CheckForUrsnif_ULV();
	DWORD CheckForUrsnif_URI();
	DWORD CheckForMulti_GENERIC();

	//12-02-18
	DWORD CheckForAdposhel();//114variants
	
	//12-02-18 -Nihar- Customer samples
	DWORD CheckForWWIZ_AUTOSPY();


	DWORD CheckForFarfli_AXLI();
	DWORD CheckForFarfli_AZNX();
	DWORD CheckForAgent_AAASX();
	DWORD CheckForAgentb_IOFT();
	DWORD CheckForBlamon_AZE();
	DWORD CheckForBlamon_AZX();
	DWORD CheckForChistudi_VZA();
	DWORD CheckForKolovorot_CHR();

	//15-02-18
	DWORD CheckForOpasoft_PAC();
	DWORD CheckForDico_GEN();
	DWORD CheckForKrap_JC();
	DWORD CheckForObfuscated_EN();
	DWORD CheckForSmall_CPL();
	DWORD CheckForSwisyn_DBYQ();
	DWORD CheckForRanky_AN();
	DWORD CheckForBlocker_KPUO();
	DWORD CheckForGandCrypt_BQ();
	DWORD CheckForWin32_GENERIC();
	DWORD CheckForSly_AFW();
	DWORD CheckForSly_AFX();
	DWORD CheckForSly_AGB();
	DWORD CheckForSly_BOQ();
	DWORD CheckForSly_BPG();
	DWORD CheckForSly_BPP();
	DWORD CheckForSly_BQC();
	DWORD CheckForSly_BSC();
	DWORD CheckForSly_BUG();
	DWORD CheckForSly_BUR();
	DWORD CheckForSly_CEU();
	DWORD CheckForSly_CFQ();
	DWORD CheckForSly_CKO();
	DWORD CheckForInjector_SB();

	
	//21-2-18-CustomerSamples
	DWORD CheckForSysn_BQYM();
	DWORD CheckForAgent_VRBR();
	DWORD CheckForWin32_KOMODIA();
	DWORD CheckForAgent_BTDR();
	DWORD CheckForWin32_STRICTOR();
	DWORD CheckForWin32_LOGGER();
	DWORD CheckForAutorun_HAAG();
	DWORD CheckForMiner_AYC();
	DWORD CheckForAgent_CAC();
	DWORD CheckForLauncher_A();

	//26-02-18
	DWORD CheckForCossta_SHU();
	DWORD CheckForScar_OZZU();
	DWORD CheckForWBNA_ROC();

	//27-02-18
	DWORD CheckForBlueh_AB();
	DWORD CheckForBlueh_ABSCVHOST();
	DWORD CheckForDynamer_ASBE();
	DWORD CheckForBrontok_BI();
	DWORD CheckForLodbak_WR5();

	//6-03-18
	DWORD CheckForSlugin_A();
	DWORD CheckForSlugin_ADLL();

	//8-03-18
	DWORD CheckForZusy_DLL_GENERIC();
	DWORD CheckForAshlynBrook_A();
	DWORD CheckForXpaj_GEN();

	//Added by Sagar on 12-03-18
	DWORD CheckForAxiombase_GEN();
    DWORD CheckForCometer_GEN();
    DWORD CheckForMiner_SXEF();
    DWORD CheckForMiner_SXEK();
    DWORD CheckForMsht_P();
    DWORD CheckForSasfis_ELRO();
    DWORD CheckForSasfis_ELRR();
    DWORD CheckForSasfis_ELRS();
    DWORD CheckForStaser_AXBN();
    DWORD CheckForSwisyn_FMIU();
    DWORD CheckForSwisyn_FNRV();
    DWORD CheckForTofsee_PN();
    DWORD CheckForTrickster_AAV();
    DWORD CheckForTrickster_AFX();
    DWORD CheckForTrickster_AGC();
    DWORD CheckForTrickster_AGI();
    DWORD CheckForTrickster_AGL();
    DWORD CheckForTrickster_AGN();
    DWORD CheckForTrickster_AGO();
    DWORD CheckForTrickster_AGQ();
    DWORD CheckForTrickster_AGT();
    DWORD CheckForTrickster_AGU();
    DWORD CheckForGoo_XQT();
    DWORD CheckForWin32_KEENVAL();
    DWORD CheckForNSIS_MD();
    DWORD CheckForChisburg_AHNL();
    DWORD CheckForSnorm_GPX();
    DWORD CheckForLoskad_HJH();
    DWORD CheckForLoskad_HLS();
    DWORD CheckForLoskad_HOR();
    DWORD CheckForLoskad_HPK();
    DWORD CheckForLoskad_HPR();
    DWORD CheckForLoskad_HRJ();
    DWORD CheckForLoskad_HRT();
    DWORD CheckForLoskad_HST();
    DWORD CheckForLoskad_HSW();
    DWORD CheckForLoskad_HSX();
    DWORD CheckForLoskad_HSY();
    DWORD CheckForLoskad_HSZ();
    DWORD CheckForLoskad_HTB();
    DWORD CheckForLoskad_HTC();
    DWORD CheckForLoskad_HTD();
    DWORD CheckForAgent_ABXW();
    DWORD CheckForAlreay_GEN();
    DWORD CheckForBanz_WKH();
    DWORD CheckForBitWallet_R();
    DWORD CheckForCarder_A();
    DWORD CheckForChePro_MNRL();
    DWORD CheckForChePro_MZLX();
    DWORD CheckForChePro_MZNI();
    DWORD CheckForChePro_NDBS();
    DWORD CheckForChePro_NDBT();
    DWORD CheckForChePro_NDBZ();
    DWORD CheckForChePro_NDCB();
    DWORD CheckForChePro_NDCG();
    DWORD CheckForChePro_NDCH();
    DWORD CheckForChePro_NDCI();
    DWORD CheckForChePro_NDCM();
    DWORD CheckForChePro_NDCN();
    DWORD CheckForChePro_NDCR();
    DWORD CheckForChthonic_IE();
    DWORD CheckForChthonic_IG();
    DWORD CheckForCridex_BH();
    DWORD CheckForNeutrinoPOS_EH();
    DWORD CheckForNeutrinoPOS_GE();
    DWORD CheckForPalibu_FJ();
    DWORD CheckForPalibu_GEN();
    DWORD CheckForRTM_MQ();
    DWORD CheckForShiotob_WJI();
    DWORD CheckForSnojan_U();
    DWORD CheckForTinba_LYB();
    DWORD CheckForTinba_MOO();
    DWORD CheckForTinyNuke_FZ();
    DWORD CheckForPhny_A();
    DWORD CheckForBanker_SLS();
    DWORD CheckForFasong_C();
    DWORD CheckForHlubea_A();
    DWORD CheckForRunfer_WRI();
	DWORD CheckForInject_OWPANJEIB();
	DWORD CheckForGraftor_134912();
	DWORD CheckForStrictor_21549();
	DWORD CheckForBronotek_GENERIC();
	DWORD CheckForDzanVirut_N();
	DWORD CheckForDelf_ZI();
	DWORD CheckForVB_BEM();

	//Added on 29-1-2018 to 8-3-2018 by pradip
	DWORD  CheckForAndromAA();
	DWORD  CheckForAndromOECH();
	DWORD  CheckForDreambotBW();
	DWORD  CheckForDipleGXFN();
	DWORD  CheckForLethicAFWE();
	DWORD  CheckForInjectAIDNB();
	DWORD  CheckForGenericBT();
	DWORD  CheckForInjectACERR();
    DWORD  CheckForYakesVNTM();
	DWORD  CheckForYakesVNUQ();
	DWORD  CheckForYakesVNWL();
	DWORD  CheckForYakesVOJO();
	DWORD  CheckForPrekDQ();
	DWORD  CheckForScarQIPW();
	DWORD  CheckForSiscosXBV();
	DWORD  CheckForSkillisBLPL();
	DWORD  CheckForVBKryjetorAJYZ();
	DWORD  CheckForVBKryjetorAMBK();
	DWORD  CheckForVBKryptYHHT();
	DWORD  CheckForVBKryptYYNE();
	DWORD  CheckForYakesURKM();
    DWORD  CheckForDelfDUY();
	DWORD  CheckForTepferGEN();
    DWORD  CheckForZusyD();
	DWORD  CheckForTepferSBIY();
	DWORD  CheckForKeyLoggerKX();
	DWORD  CheckForRecamAHOY();
    DWORD  CheckForSramAA();
	DWORD  CheckForSramAB();
	DWORD  CheckForSramAC();
    DWORD  CheckForSramAD();
	DWORD  CheckForSramAE();
	DWORD  CheckForSramAG();
	DWORD  CheckForSramAL();
	DWORD  CheckForSramAX();
	DWORD  CheckForSramAY();
	DWORD  CheckForSramBA();
	DWORD  CheckForSramBB();
	DWORD  CheckForSramBU();
	DWORD  CheckForSramCA();
	DWORD  CheckForSramF();
	DWORD  CheckForSramI();
	DWORD  CheckForSramJ();
	DWORD  CheckForSramK();
	DWORD  CheckForSramN();
	DWORD  CheckForSramP();
	DWORD  CheckForSramV();
	DWORD  CheckForSramW();
    DWORD  CheckForGandCryptB();
	DWORD  CheckForGenDLE();
	DWORD  CheckForKangarA();
	DWORD  CheckForGenHFM();
	DWORD  CheckForGenQ();
    DWORD  CheckForGandCryptAG();
	DWORD  CheckForGandCryptAL();
	DWORD  CheckForGandCryptAN();
	DWORD  CheckForGandCryptBB();
    DWORD  CheckForLockyAPW();
	DWORD  CheckForLockyCDM();
	DWORD  CheckForLockyCRV();
	DWORD  CheckForLockyCTC();
	DWORD  CheckForLockyDLY();
	DWORD  CheckForLockyWLS();
	DWORD  CheckForLockyWLY();
	DWORD  CheckForLockyXHB();
	DWORD  CheckForBlockerKQZO();
	DWORD  CheckForGandCryptFN();
	DWORD  CheckForGandCryptGI();
	DWORD  CheckForGimemoCGIZ();
    DWORD  CheckForXamyhLCC();
	DWORD  CheckForXamyhLHJ();
	DWORD  CheckForXamyhMBM();
	DWORD  CheckForGenericCryptorJJQ();
	DWORD  CheckForGenericCryptorJWI();
	DWORD  CheckForGenericCryptorKUU();
	DWORD  CheckForGenericCryptorKVG();
    DWORD  CheckForCrusisBZY();
	DWORD  CheckForMbroBAXZ();
	DWORD  CheckForPetrA();
    DWORD  CheckForGenericCryptorCYS();
	DWORD  CheckForGenericCryptorCZT();
	DWORD  CheckForGenericCryptorCZU();
	DWORD  CheckForGenericCryptorCZX();
	DWORD  CheckForPornoAssetCFBJ();
	DWORD  CheckForPornoAssetCIJU();
    DWORD  CheckForAtomZL();
	DWORD  CheckForGandCryptHQ();
	DWORD  CheckForHitlerA();
	DWORD  CheckForHitlerB();
	DWORD  CheckForScreenLockerA();
	DWORD  CheckForGrandCrabA();
	DWORD  CheckForSageA();
    DWORD  CheckForBlockerGNYT();
	DWORD  CheckForBlockerHGYP();
	DWORD  CheckForBlockerHRFT();
	DWORD  CheckForBlockerIKYQ();
	DWORD  CheckForBlockerIQMC();
	DWORD  CheckForBlockerIVCC();
    DWORD  CheckForBlockerIWLD();
	DWORD  CheckForBlockerJAGV();
	DWORD  CheckForBlockerJAIC();
	DWORD  CheckForBlockerJCZM();
	DWORD  CheckForBlockerJMDI();
	DWORD  CheckForCryptoffABL();
	DWORD  CheckForSageCryptDHZ();
	DWORD  CheckForSageCryptDPO();
	DWORD  CheckForChimeraD();
    DWORD  CheckForCryaklABP();
	DWORD  CheckForCryFileBMQ();
	DWORD  CheckForCryptodefCKU();
	DWORD  CheckForCrusisBWN();
	DWORD  CheckForCrusisBXA();
	DWORD  CheckForCrusisNT();
	
	
	//15-03-18
	DWORD CheckForXmrigCoinMiner();

	//19-03-18
	DWORD CheckForSlugin_B();
	//23-03-18
	DWORD CheckForRansomware_GlobeImposter();
	DWORD CheckForPhoto_GENERIC();
	DWORD CheckForAgent_BWB();
	DWORD CheckForAutoRun_FTC();
	
	DWORD CheckForMinerX64_QN();
	DWORD CheckForMinerX64_WW();
	DWORD CheckForBitMinerX64_GEN();
	DWORD CheckForLinkuryX64_O();
	DWORD CheckForPhoto_PACKED();
	
	//Added on 14-05-2018 by pradip
	DWORD  CheckForKMSAutoO64();
	DWORD  CheckForKMSAutoC64();

	//Added on 14-05-2018 by pradip
	DWORD  CheckForVBobfusFH64();

	//Added on 16-05-2018 by pradip
	DWORD  CheckForMikey64();
	DWORD  CheckForScribbleB64();

	//Added on 17-05-2018 by pradip
	DWORD  CheckForAnnabelle64();

	//Added on 18-05-2018 by pradip
	//For customer Mr. Abhishek Vaidya 
	DWORD  CheckForJawegoG64();
	DWORD  CheckForYTBlockJH64();
	DWORD  CheckForWajam64();

	//For Sanjay Ghalani 
	DWORD  CheckForNimdaB64();
	DWORD  CheckForNimdaO64();
	DWORD  CheckForNimdaC64();
	DWORD  CheckForNimdaD64();
	DWORD  CheckForNimdaE64();
	DWORD  CheckForNimdaF64();
   
	//Added on 19-05-2018 and 20-5-2018 by pradip
	DWORD  CheckForNimdaH64();
	DWORD  CheckForNimdaI64();
	DWORD  CheckForNimdaM64();
	DWORD  CheckForNimdaN64();
	DWORD  CheckForNimdaP64();

	//Added on 21-05-2018 by pradip
	DWORD  CheckForNimdaW64();
	DWORD  CheckForNimdaX64();
	DWORD  CheckForNimdaY64();
	DWORD  CheckForNimdaZ64();
    DWORD  CheckForRPCHookA();
	
	//Added on 23-5-2018 by pradip
	DWORD  CheckForElex();
	DWORD  CheckForCrossRider();

	//Added on 24-5-2018 by pradip
	DWORD  CheckForSnaraAD();
	DWORD  CheckForNetFilterE();
	DWORD  CheckForSwiftBrowseCH();
	DWORD  CheckForInstallMonsterTC();
	DWORD  CheckForWajamEDCSZ();

	//Added on 25-5-2018 by pradip
	DWORD  CheckForElexHPA();
	DWORD  CheckForInstallMonsterGC();
	DWORD  CheckForNeoreklami();
	DWORD  CheckForWajamROATV();

	//Added on 4-6-2018 by pradip
	DWORD  CheckForSpyLokiBot();
	//Sveta given sample
	DWORD  CheckForAntiAVCRAD();

	//Added on 5-6-2018 by pradip
	//Sveta given sample
	DWORD  CheckForShadowBrokerAO();
	//Gujrat samples 
	DWORD  CheckForInjectorA();
	DWORD  CheckForDealPlyPRMUY();
	DWORD  CheckForZbotPatchedB();
	DWORD  CheckForCsdiMonetizeNJWVE();	

	//Added on 6-6-2018 by pradip
	//Customer Dilwar Bhai
	DWORD  CheckForWizzMonetize();
	DWORD  CheckForWizrem();
	DWORD  CheckForInjectorDYKM();
	//Customer reliable shevgaon
	DWORD  CheckForComisproc();
	
	//Added on 7-6-2018 by pradip
	//Customer Mr. Dilavar
	DWORD  CheckForCsdiMonetize();
	DWORD  CheckForZippyLoaderCUI();
	DWORD  CheckForBitMinerRBA();
	DWORD  CheckForMinerUBLQ();
	DWORD  CheckForYahooChrome();
	DWORD  CheckForZeroAccessJC();

	//Added on 8-6-2018 by pradip
	//Customer Mr. Dilavar
	DWORD  CheckForLinkuryZUSY();
	DWORD  CheckForDelphiUMWGG();
	DWORD  CheckForLinkuryCV();
	//Customer Reliable Computer
	DWORD  CheckForDropperUYL();
	DWORD  CheckForAgentVRBR();

	//Added on 11-6-2018 by pradip
	DWORD  CheckForGandCryptRI();
	DWORD  CheckForBrResMon();
	DWORD  CheckForGandCryptPF();
	DWORD  CheckForGandCryptPG();
	DWORD  CheckForStalkA();
    DWORD  CheckForKriskynote();
	DWORD  CheckForShizKLPT();

	//Added on 12-6-2018 by pradip
	DWORD  CheckForGandCryptTPIA();
	DWORD  CheckForCrusisCKA();
	DWORD  CheckForAXF();
	


	//Added by Pradip on date 5-6-2018
	DWORD  CheckForCryaklAPC();
	DWORD  CheckForCryaklDQ();
	DWORD  CheckForCryFileBMT();
	DWORD  CheckForPhpwGP();
	DWORD  CheckForAgentQWFQND();
	DWORD  CheckForChapakML();
	DWORD  CheckForChapakPO();
	DWORD  CheckForPastaVRI();
	DWORD  CheckForTemrBK();
	DWORD  CheckForChifraxA();
	DWORD  CheckForFlyStudioVSD();
	DWORD  CheckForFsysnaEJTL();
	DWORD  CheckForFsysnaEJTN();
	DWORD  CheckForKolovorotVR();
	DWORD  CheckForMinerTOKN();
	DWORD  CheckForNymaimBEAU();
	DWORD  CheckForPastaSYM();
    DWORD  CheckForSiscosXEH();
	DWORD  CheckForTemrVJO();
	DWORD  CheckForAgentGOW();
	DWORD  CheckForCosmuAIGH();
	DWORD  CheckForBitMinD();
	DWORD  CheckForAutoRunFNCA();
	DWORD  CheckForBnfQVS();
	DWORD  CheckForJorikRMU();
	DWORD  CheckForMinerTHRT();
	DWORD  CheckForSnojanCCGB();
	DWORD  CheckForStavaDHY();
	DWORD  CheckForVehidisXIS();
	DWORD  CheckForWauchosEX();
	DWORD  CheckForBublikAELD();
	DWORD  CheckForChapakDAV();
	DWORD  CheckForChapakDAW();
	DWORD  CheckForChapakDFT();
	DWORD  CheckForServStartAKH();
	DWORD  CheckForVilselHUS();
	DWORD  CheckForDovsMEN();
	DWORD  CheckForFlyStudioWPE();
	DWORD  CheckForInjectAIXCM();
	DWORD  CheckForInjectAIXEF();
	DWORD  CheckForLebagAELU();
	DWORD  CheckForMinerTBJX();
	DWORD  CheckForMinerTUKZ();
	DWORD  CheckForYakesVTOM();
	DWORD  CheckForAgentFPAR();
	DWORD  CheckForCryptERRR();
	DWORD  CheckForAgentNFAIRU();
	DWORD  CheckForAgentbBVRG();
	DWORD  CheckForAgentQWFDEQ();
	DWORD  CheckForAgentCNRA();
	DWORD  CheckForMinerBAP();
	DWORD  CheckForMinerTTKW();
	DWORD  CheckForPhpwGLL();
	DWORD  CheckForAdloadIQVO();
	DWORD  CheckForTovkaterCILJ();
	DWORD  CheckForTovkaterCILP();
	DWORD  CheckForChapakEUQ();
	DWORD  CheckForChistudiVTX();
	DWORD  CheckForCometerAJ();
	DWORD  CheckForDiztakunBAVJ();
	DWORD  CheckForDynamerIES();
	DWORD  CheckForPholdiconA();
	DWORD  CheckForAgentbBSOT();
	DWORD  CheckForAgentbIFIA();
	DWORD  CheckForBicololoBHTE();
	DWORD  CheckForBlamonBUA();
	DWORD  CheckForCobaltA();
	DWORD  CheckForGimemoCGNO();
	DWORD  CheckForGlobimBA();
	DWORD  CheckForGlobimBM();
	DWORD  CheckForGlobimCA();
	DWORD  CheckForGlobimCF();
	DWORD  CheckForGlobimCH();
	DWORD  CheckForGlobimCI();
	DWORD  CheckForGlobimCM();
	DWORD  CheckForGlobimCO();
	DWORD  CheckForGlobimCP();
	DWORD  CheckForLeshiyA();
	DWORD  CheckForMbroBAXQ();
	DWORD  CheckForGandCryptIM();
	DWORD  CheckForGandCryptIO();
	DWORD  CheckForGandCryptIY();
	DWORD  CheckForGandCryptJO();
	DWORD  CheckForGandCryptNF();
	DWORD  CheckForGenHQJ();
	DWORD  CheckForGenHQK();
	DWORD  CheckForGenHQR();
	DWORD  CheckForGenHQU();
	DWORD  CheckForGenHRD();
	DWORD  CheckForGenHRE();
	DWORD  CheckForAgentFQJJ();
	DWORD  CheckForAgentWG();
	DWORD  CheckForGeographDL();
	DWORD  CheckForXamyhOID();
	DWORD  CheckForKeRangerA();
	DWORD  CheckForAgentAATF();
	DWORD  CheckForBireleAJDN();
	DWORD  CheckForBireleAIUE();
	DWORD  CheckForBlockerJZEC();
	DWORD  CheckForCryptorBRJ();
	DWORD  CheckForForeignNYBA();
	DWORD  CheckForHosts2XXY();
	DWORD  CheckForHosts2YCQ();
	DWORD  CheckForHosts2YDT();
	DWORD  CheckForKolovorotBCF();
	DWORD  CheckForKolovorotBQM();
	DWORD  CheckForKolovorotPD();
	DWORD  CheckForMianchaGWY();
	DWORD  CheckForSnojanCAPN();
	DWORD  CheckForSnojanCEFS();
	DWORD  CheckForStaserBQPF();
	DWORD  CheckForReconycIPYM();
	DWORD  CheckForScarRIJJ();
	DWORD  CheckForBuzusXYAC();
	DWORD  CheckForChapakMWT();
	DWORD  CheckForDiztakunAWVR();
	DWORD  CheckForDovsNCX();
	DWORD  CheckForIRCbotAVTW();
	DWORD  CheckForPhpwESG();
	DWORD  CheckForPhpwHUH();
	DWORD  CheckForReconycDUUU();
    DWORD  CheckForAndromPFYJ();
	DWORD  CheckForAndromPIXF();
	DWORD  CheckForBumerBJ();
	DWORD  CheckForDarkKometACEG();
	DWORD  CheckForFarfliBACE();
	DWORD  CheckForPoisonGGRF();
	DWORD  CheckForZegostMTHIK();
    DWORD  CheckForCrusisAZI();
	DWORD  CheckForCrusisAZN();
	DWORD  CheckForCrusisAZU();
	DWORD  CheckForCrusisBDI();
	DWORD  CheckForCryptorBAE();
	DWORD  CheckForCryptorBAM();
	DWORD  CheckForGotangoGILE();
	DWORD  CheckForHesvCTVL();
	DWORD  CheckForHosts2XLV();
	DWORD  CheckForHosts2YIY();
	DWORD  CheckForHosts2YKW();
	DWORD  CheckForHosts2YLH();
	DWORD  CheckForKhalesiECJ();
	DWORD  CheckForKhalesiECL();
	DWORD  CheckForKhalesiECN();
	DWORD  CheckForKhalesiEIM();
	DWORD  CheckForKolovorotBEB();
	DWORD  CheckForLlacLKNO();
	DWORD  CheckForLoskadACWC();
	DWORD  CheckForMianchaHUE();
    DWORD  CheckForCosmuAIGS();


	//Nihar
	DWORD CheckForAgent_FPAR();
	DWORD CheckForCrypt_GHLX();
	DWORD CheckForDisfa_BQG();
	DWORD CheckForDisfa_MZBP();
	DWORD CheckForDisfa_MZBZ();
	DWORD CheckForDisfa_MZCM();
	DWORD CheckForDisfa_MZCS();
	DWORD CheckForDisfa_MZCT();
	DWORD CheckForDisfa_MZCV();
	DWORD CheckForDisfa_MZCW();
	DWORD CheckForDisfa_MZCZ();
	DWORD CheckForDisfa_MZDA();
	DWORD CheckForDisfa_MZDR();
	DWORD CheckForDisfa_MZEA();
	DWORD CheckForAgent_ACKDL();
	DWORD CheckForAgent_HWHR();
	DWORD CheckForAgent_NESSHM();
	DWORD CheckForAgent_QWFRSN();
	DWORD CheckForAgent_VEFB();
	DWORD CheckForAgentb_BQYR();
	DWORD CheckForAgentb_IXUT();
	DWORD CheckForAutoit_CKC();
	DWORD CheckForAutoit_FFZ();
	DWORD CheckForAutoit_ZU();
	DWORD CheckForBitCoinMiner_ALN();
	DWORD CheckForChapak_EF();
	DWORD CheckForChapak_EG();
	DWORD CheckForChapak_FH();
	DWORD CheckForChapak_FO();
	DWORD CheckForChapak_FX();
	DWORD CheckForChapak_LL();
	DWORD CheckForVobfus_RKU();
	
	
	DWORD CheckForFakeAv_TEMS();
	DWORD CheckForFakeAv_TEOJ();
	DWORD CheckForFakeAv_TEOS();
	DWORD CheckForFakeAv_TEOU();
	DWORD CheckForFakeAv_TEOW();
	DWORD CheckForFakeAv_TEOY();
	DWORD CheckForFakeAv_TEOZ();
	DWORD CheckForFakeAv_TEPC();
	DWORD CheckForFakeAv_TEPG();
	DWORD CheckForFakeAv_TEPH();
	DWORD CheckForFakeAv_TEPI();
	DWORD CheckForFakeAv_TEPJ();
	DWORD CheckForFakeAv_TEPL();
	DWORD CheckForFakeAv_TEPM();
	DWORD CheckForFakeAv_TEPN();
	DWORD CheckForFakeAv_TEPP();
	DWORD CheckForFakeAv_TEPQ();
	DWORD CheckForFakeAv_TEPR();
	DWORD CheckForFakeAv_TEPU();

	DWORD CheckForFlyStudio_WLW();
	DWORD CheckForFsysna_EUIC();
	DWORD CheckForFsysna_EUJD();
	DWORD CheckForFsysna_EUJI();
	DWORD CheckForFsysna_EUJK();
	DWORD CheckForFsysna_EUJX();
	DWORD CheckForGofot_IXY();
	DWORD CheckForGotango_GJCK();
	DWORD CheckForHesv_CYUI();
	DWORD CheckForHesv_CYZG();
	DWORD CheckForHesv_CZAC();
	DWORD CheckForHosts2_YEB();
	DWORD CheckForHosts2_YEG();
	DWORD CheckForHosts2_YEH();
	DWORD CheckForInformer_I();
	DWORD CheckForInject_AHRAK();
	DWORD CheckForInject_AISXQ();
	DWORD CheckForInject_AIWIA();
	DWORD CheckForInject_AJATW();
	
	DWORD CheckForInject_AJAVB();
	DWORD CheckForInject_AJAVT();
	DWORD CheckForInject_AJAWC();
	DWORD CheckForInject_AJAXT();
	DWORD CheckForInject_AJAYA();
	DWORD CheckForInject_AJBAB();
	DWORD CheckForInject_AJBAW();
	DWORD CheckForInject_AJBBW();
	DWORD CheckForInject_AJBBX();
	DWORD CheckForInject_AJBCP();
	DWORD CheckForInject_AJBEO();
	DWORD CheckForInject_AJBEP();
	DWORD CheckForInject_AJBGN();
	DWORD CheckForInject_AJBGO();
	DWORD CheckForInject_AJBGP();
	DWORD CheckForInject_AJBGY();
	DWORD CheckForInject_AJBIL();
	DWORD CheckForInject_AJBIN();
	DWORD CheckForInject_AJBIV();
	DWORD CheckForInject_AJBIW();

	DWORD CheckForInject_AJBJC();
	DWORD CheckForInject_AJBJF();
	DWORD CheckForInject_AJBJG();
	DWORD CheckForInject_AJBJY();
	DWORD CheckForInject_AJBKL();
	DWORD CheckForKasidet_MVT();
	DWORD CheckForKasidet_MVZ();
	DWORD CheckForKasidet_MWZ();
	DWORD CheckForKasidet_MXH();
	DWORD CheckForKasidet_MXQ();
	DWORD CheckForKhalesi_DFB();
	DWORD CheckForKhalesi_DFO();
	DWORD CheckForKhalesi_DHG();
	DWORD CheckForKhalesi_DHI();
	DWORD CheckForKhalesi_DJM();
	DWORD CheckForKhalesi_DJP();
	DWORD CheckForKhalesi_DJT();
	DWORD CheckForKhalesi_DJU();
	DWORD CheckForKhalesi_DKB();
	DWORD CheckForKhalesi_DKH();

	DWORD CheckForKhalesi_DKT();
	DWORD CheckForKhalesi_DKU();
	DWORD CheckForKovter_BEP();
	DWORD CheckForKovter_QXY();
	DWORD CheckForKriskynote_GN();
	DWORD CheckForKriskynote_GO();
	DWORD CheckForKriskynote_GP();
	DWORD CheckForKriskynote_GQ();
	DWORD CheckForKriskynote_GR();
	DWORD CheckForLebag_AENB();
	DWORD CheckForLebag_FAD();
	DWORD CheckForLlac_LLGB();
	DWORD CheckForLlac_LLGD();
	DWORD CheckForLlac_LLGL();
	DWORD CheckForLlac_LLGT();
	DWORD CheckForLlac_LLGU();
	DWORD CheckForLlac_LLHA();
	DWORD CheckForLlac_LLHD();
	DWORD CheckForLlac_LLKG();
	DWORD CheckForLlac_LLKJ();
	
	DWORD CheckForLoskad_AAOA();
	DWORD CheckForLoskad_AAVL();
	DWORD CheckForLoskad_ABAC();
	DWORD CheckForMansabo_AWS();
	DWORD CheckForMansabo_AWU();
	DWORD CheckForMansabo_AWV();
	DWORD CheckForMiner_TECN();
	DWORD CheckForMiner_TNYY();
	DWORD CheckForMiner_TTQY();
	DWORD CheckForMiner_TTZQ();
	DWORD CheckForMiner_TUXZ();
	DWORD CheckForMiner_TUZY();
	DWORD CheckForMiner_TVEF();
	DWORD CheckForMiner_TVHF();
	DWORD CheckForMiner_TVIK();
	DWORD CheckForMiner_TVJO();
	DWORD CheckForMiner_TVKA();
	DWORD CheckForMiner_TVKG();
	DWORD CheckForMiner_TVKH();

	DWORD CheckForMiner_TVKY();
	DWORD CheckForMiner_TVKZ();
	DWORD CheckForMiner_TVLB();
	DWORD CheckForMiner_TVLC();
	DWORD CheckForMiner_TVLH();
	DWORD CheckForMiner_TVLJ();
	DWORD CheckForMiner_TVLS();
	DWORD CheckForMiner_TVLX();
	DWORD CheckForMiner_TVMB();
	DWORD CheckForMiner_TVMP();
	DWORD CheckForMiner_TVMQ();
	DWORD CheckForMiner_TVMW();
	DWORD CheckForMiner_TVMY();
	DWORD CheckForMiner_TVNG();

	//shodhan 13-06-2018
	DWORD CheckForAndrom_PDLX();
	DWORD CheckForDreambot_FA();
	DWORD CheckForKasidet_FYS();
	DWORD CheckForRemoteManipulator_KBL();
	DWORD CheckForMetla_A();
	DWORD CheckForDOTHETUK_RXA();
	DWORD CheckForWin32_WEBDOWN();
	DWORD CheckForWin32_DROOPTROOP();
	DWORD CheckForWin32_WOOZLIST();
	DWORD CheckForWin32_LAGER();
	DWORD CheckForWin32_BRONTOK();
	DWORD CheckForWin32_BUNDPIL();
	DWORD CheckForWin32_GRIFOUT();
	DWORD CheckForWin32_PHORPIEX();
	DWORD CheckForWin32_SOHANAD();
	DWORD CheckForMSIL_PACOREDI();
	DWORD CheckForShopBot_COZ();
	DWORD CheckForTrojan_PEED();
	DWORD CheckForTrojan_TRADOWN();
	DWORD CheckForBancos_R();
	DWORD CheckForBlamon_BTC();
	DWORD CheckForWin32_DAPATO();
	DWORD CheckForDovs_MCX();
	DWORD CheckForDovs_MDN();
	DWORD CheckForWin32_EMOTET();
	DWORD CheckForBanker_CITADEL();
	DWORD CheckForWin32_PYNAMER();
	DWORD CheckForWin32_INJECTED();
	DWORD CheckForLlac_WBQ();
	DWORD CheckForLebag_AEJU();
	DWORD CheckForBackdoor_PONTOEB();
	DWORD CheckForWin32_RSCRT();
	DWORD CheckForBackdoor_ZEGOST();
	DWORD CheckForTrojan_BROWSEBAN();
	DWORD CheckForTrojan_DETPLOCK();
	DWORD CheckForTrojan_SISPROC();
	DWORD CheckForWin32_ADDUSER();
	DWORD CheckForWin32_BOAXXE();
	DWORD CheckForDialer_AY();
	DWORD CheckForDialer_DU();
	DWORD CheckForDialer_U();
	DWORD CheckForWin32_EXESCRIPT();
	DWORD CheckForWin32_RECONYC();
	DWORD CheckForWin32_SHELMA();
	DWORD CheckForWin32_WOOOL();
	DWORD CheckForWin32_ZELEFFO();
	DWORD CheckForNjrat_UJ();
	DWORD CheckForMiner_TPKM();
	DWORD CheckForMiner_TPLR();
	DWORD CheckForScarsi_PXO();
	DWORD CheckForWin32_VFLOODER();
	DWORD CheckForYoddos_WMJ();
	DWORD CheckForEvital_A();
	DWORD CheckForTinyNuke_JL();
	DWORD CheckForDluca_GEN();
	DWORD CheckForQDown_F();
	DWORD CheckForFarfli_AZPO();
	DWORD CheckForKapucen_B();
	DWORD CheckForTrojan_OFFEND();
	DWORD CheckForTrojan_RANAPAMA();
	DWORD CheckForWin32_MINIDUKE();
	DWORD CheckForWin32_BUZUS();
	DWORD CheckForCnopa_BKW();
	DWORD CheckForCutwail_WYW();
	DWORD CheckForWin32_DRIDEX();
	DWORD CheckForWin32_INDILOADZ();
	DWORD CheckForWin32_NUKESPED();
	DWORD CheckForNymaim_BEEE();
	DWORD CheckForNymaim_BEJZ();
	DWORD CheckForWin32_PATPOOPY();
	DWORD CheckForWin32_REDCONTROLE();
	DWORD CheckForWin32_SASFIS();
	DWORD CheckForWin32_TIGGRE();
	DWORD CheckForTrickster_COF();
	DWORD CheckForZonidel_CTK();
	DWORD CheckForAutoItScript_A();
	DWORD CheckForFrauDrop_AMASF();
	DWORD CheckForWanna_AMJJ();
	DWORD CheckForShiz_EUXX();
	DWORD CheckForShelma_GEN();
	DWORD CheckForTrojan_ATROS4();
	DWORD CheckForWin32_ADDSHARE();
	DWORD CheckForBuzus_XURS();
	DWORD CheckForDovs_NCZ();
	DWORD CheckForFakeAv_TEQV();
	DWORD CheckForPakes_AXDG();
	DWORD CheckForRefroso_BSP();
	DWORD CheckForWin32_RODECAP();
	DWORD CheckForSelfDel_GHFQ();
	DWORD CheckForXcaon_R();
	DWORD CheckForBanbra_WKRI();
	DWORD CheckForNeutrinoPOS_CDS();
	DWORD CheckForBitmin_VYH();
	DWORD CheckForSteam_QX();
	DWORD CheckForCarberp_ASGH();
	DWORD CheckForNoon_KLK();
	DWORD CheckForNoon_KLR();
	DWORD CheckForPhds_B();
	DWORD CheckForUrsnif_YNS();
	DWORD CheckForWin32_BIRELE();
	DWORD CheckForTrojan_Spy_HAWKEYE();
	DWORD CheckForReptile_GEN();
	DWORD CheckForAntavmu_DOENA();
	DWORD CheckForATRAPS_PMKRU();
	DWORD CheckFor629_JH();
	DWORD CheckFor7_901();
	DWORD CheckForEPACK_JUKLM();
	DWORD CheckForXpack_BILYO();
	DWORD CheckForXpack_CRYHW();
	DWORD CheckForXpack_EEHBB();
	DWORD CheckForHijacker_GEN();
	DWORD CheckForMogoogwi_QIFA();
	DWORD CheckForOccamy_LYFHD();
	DWORD CheckForA_800();
	DWORD CheckForOtran_AC();
	DWORD CheckForOtran_AIOB();
	DWORD CheckForSmall_BHOOUM();
	DWORD CheckForSmall_BHOUMB();
	DWORD CheckForSmall_BHOUMD();
	DWORD CheckForDiple_EPDI();
	DWORD CheckForTinba_MHP();
	DWORD CheckForNanoBot_AELA();
	DWORD CheckForWin32_CECKNO();
	DWORD CheckForEnfal_KD();
	DWORD CheckForEnfal_KF();
	DWORD CheckForFarfli_BCAJ();
	DWORD CheckForMokes_XIP();
	DWORD CheckForProsti_AG();
	DWORD CheckForRA_based_GE();
	DWORD CheckForSalgorea_C();
	DWORD CheckForZegost_MTFIQ();
	DWORD CheckForZegost_VGS();
	DWORD CheckForVermin_GEN();
	DWORD CheckForCorrempa_GEN();
	DWORD CheckForScarCruft_GEN();
	DWORD CheckForBackDoor_HUPIGON6();
	
	//Added on 13-6-2018 by pradip
	DWORD  CheckForDexelA();
	DWORD  CheckForBancteianD();
	DWORD  CheckForCerberSMFE();
	DWORD  CheckForVesenlosowA();
	DWORD  CheckForBrontokBI();

	//Added on 14-6-2018 by pradip
	DWORD  CheckForReconycCDBQ();
	DWORD  CheckForDumpyAA();
	DWORD  CheckForGasonenA();
	
	//Added on 15-6-2018 by pradip
	DWORD  CheckForGandCryptUG();
	DWORD  CheckForGandCryptUK();
	DWORD  CheckForGandCryptUQ();
	DWORD  CheckForGandCryptVQ();
	DWORD  CheckForGandCryptVY();
	DWORD  CheckForGandCryptWA();
	DWORD  CheckForGandCryptWC();
	DWORD  CheckForGandCryptWJ();
	DWORD  CheckForGandCryptWR();
	DWORD  CheckForGandCryptXB();

	//Added on 16-6-2018 by pradip
	DWORD  CheckForGandCryptXD();
	DWORD  CheckForGandCryptXE();
	DWORD  CheckForGandCryptXF();
	DWORD  CheckForGandCryptXN();
	DWORD  CheckForGandCryptXW();
	DWORD  CheckForGandCryptXX();
	DWORD  CheckForGandCryptYA();
	DWORD  CheckForGandCryptYF();

	//Added on 17-6-2018 by pradip
	DWORD  CheckForGandCryptYP();
	DWORD  CheckForGandCryptYS();
	DWORD  CheckForGandCryptYW();
	DWORD  CheckForGandCryptYX();
	DWORD  CheckForGenHEF();
	DWORD  CheckForGenHNT();
	DWORD  CheckForGenHRQ();
	DWORD  CheckForGenICT();

	//Added on 19-6-2018 by pradip
	DWORD  CheckForGandCryptYZ();
	DWORD  CheckForGenericCryptorCY();
	DWORD  CheckForGenericCryptorCZP();
	DWORD  CheckForHermezH();
	DWORD  CheckForGpcodeAO();
	DWORD  CheckForHermezR();
	DWORD  CheckForLockyABRZ();

	//Added on 20-6-2018 by pradip
	DWORD  CheckForOccamyC();
	DWORD  CheckForCoinMinerBW();
	DWORD  CheckForAgentJPQA();
	DWORD  CheckForTiggre();

	//Added on 25-6-2018 by pradip
	DWORD  CheckForGandCryptAGN();
	DWORD  CheckForGandCryptAHW();
	DWORD  CheckForGandCryptAIN();
	DWORD  CheckForGandCryptAIS();

	//Added on 26-6-2018 by pradip
    DWORD  CheckForGandCryptAJH();
	DWORD  CheckForGandCryptAKN();
	DWORD  CheckForGandCryptALG();
	DWORD  CheckForGandCryptAPL();

	//Added on 27-6-2018 by pradip
	DWORD  CheckForGandCryptAPP();
	DWORD  CheckForGandCryptAPR();
	DWORD  CheckForGandCryptAPZ();
	DWORD  CheckForGandCryptAQF();
	DWORD  CheckForGandCryptAQH();

	//Added on 28-6-2018 by pradip
	DWORD  CheckForDawsBKBB();
	DWORD  CheckForGandCryptAQM();
	DWORD  CheckForGandCryptASG();
	DWORD  CheckForWannaAMIF();
	DWORD  CheckForWannaAMIH();
	
	//Added by Sagar
	DWORD CheckForEvital_DH();
	DWORD CheckForEvital_DI();
	DWORD CheckForEvital_GEN();
	DWORD CheckForMarwal_BZ();
	DWORD CheckForMarwal_CA();
	DWORD CheckForMarwal_CC();
	DWORD CheckForMarwal_CE();
	DWORD CheckForMarwal_CI();
	DWORD CheckForMarwal_CJ();
	DWORD CheckForTrojan_Banker_TERDO();
	DWORD CheckForTrojan_Banker_URSNIF();
	DWORD CheckForWin32_AGENT();
	DWORD CheckForBitWallet_T();
	DWORD CheckForCapper_AAAF();
	DWORD CheckForCapper_AAAG();
	DWORD CheckForCapper_AAAI();
	DWORD CheckForCapper_AAAJ();
	DWORD CheckForCapper_AABJ();
	DWORD CheckForCapper_AABV();
	DWORD CheckForChePro_KIS();
	DWORD CheckForChePro_NEEM();
	DWORD CheckForChePro_NEET();
	DWORD CheckForChePro_NEFY();
	DWORD CheckForChePro_NEQH();
	DWORD CheckForChePro_VUD();
	DWORD CheckForChthonic_AD();
	DWORD CheckForChthonic_HG();
	DWORD CheckForChthonic_IM();
	DWORD CheckForChthonic_IS();
	DWORD CheckForChthonic_IY();
	DWORD CheckForChthonic_IZ();
	DWORD CheckForCoinMiner_BF();
	DWORD CheckForCoinMiner_CF();
	DWORD CheckForCoinMiner_DH();
	DWORD CheckForCoinMiner_EF();
	DWORD CheckForCoreBot_FU();
	DWORD CheckForCoreBot_FZ();
	DWORD CheckForCoreBot_GH();
	DWORD CheckForCoreBot_GI();
	DWORD CheckForCoreBot_GQ();
	DWORD CheckForCoreBot_GR();
	DWORD CheckForCridex_BX();
	DWORD CheckForJackpot_GEN();
	DWORD CheckForMetel_CAI();
	DWORD CheckForMetel_CGE();
	DWORD CheckForMultiBanker_BZS();
	DWORD CheckForNimnul_GUJ();
	DWORD CheckForPhpw_DC();
	DWORD CheckForPhpw_DJ();
	DWORD CheckForPhpw_DX();
	DWORD CheckForRTM_MK();
	DWORD CheckForRTM_OD();
	DWORD CheckForRTM_OG();
	DWORD CheckForRTM_OH();
	DWORD CheckForRTM_OJ();
	DWORD CheckForRTM_OL();
	DWORD CheckForRTM_ON();
	DWORD CheckForRTM_OU();
	DWORD CheckForRTM_PF();
	DWORD CheckForRTM_PJ();
	DWORD CheckForRTM_PK();
	DWORD CheckForRTM_PM();
	DWORD CheckForRTM_PO();
	DWORD CheckForRTM_PS();
	DWORD CheckForRTM_PU();
	DWORD CheckForRTM_PX();
	DWORD CheckForShiotob_WKI();
	DWORD CheckForShiotob_WMD();
	DWORD CheckForTuhkit_AAA();
	DWORD CheckForTuhkit_AAX();
	DWORD CheckForTuhkit_AAY();
	DWORD CheckForTuhkit_ZM();
	DWORD CheckForTuhkit_ZN();
	
	//Added by Nihar
	DWORD CheckForTuhkit_ZQ();
	DWORD CheckForTuhkit_ZX();
	DWORD CheckForCVE2_AQ();
	DWORD CheckForMSIL_GENERIC();
	DWORD CheckForStartSurf_AUI();
	DWORD CheckForWin32_GENERIC2();
	DWORD CheckForPetr_L();
	DWORD CheckForPetr_XW();
	DWORD CheckForPurgen_AHP();
	DWORD CheckForNoon_GYP();
	DWORD CheckForNoon_GYS();
	DWORD CheckForNoon_GYT();
	DWORD CheckForRombertik_A();
	DWORD CheckForInject_AIPBV();
	DWORD CheckForInject_AIRRX();
	DWORD CheckForInject_VEWU();
	DWORD CheckForNeurevt_AN();
	DWORD CheckForWinCred_DG();
	DWORD CheckForYakes_VSXG();
	DWORD CheckForElex64_Hpa();

	//Added on 29-6-2018 by pradip
	DWORD  CheckForWannaAMIL();
	DWORD  CheckForGandCryptPQ();
	DWORD  CheckForGandCryptPR();
	DWORD  CheckForWannaAMKP();

	//Added on 02-7-2018 by pradip
	DWORD  CheckForGandCryptPY();
	DWORD  CheckForGandCryptQD();
	DWORD  CheckForGandCryptQT();
	DWORD  CheckForGandCryptRH();
	DWORD  CheckForGandCryptSU();
	DWORD  CheckForGandCryptTE();

	//Added on 03-7-2018 by pradip
	DWORD  CheckForGandCryptUU();
	DWORD  CheckForGandCryptVA();
	DWORD  CheckForGandCryptZD();

	//Added on 04-7-2018 by pradip
	DWORD  CheckForAgentAAFJ();
	DWORD  CheckForAgentABHY();
	DWORD  CheckForAgentABJR();
	DWORD  CheckForAgentGMR();
	DWORD  CheckForAgentZH();
	
	//Added on 05-7-2018 by pradip
	DWORD  CheckForUpatreGXBK();
	DWORD  CheckForCryprenACMD();
	DWORD  CheckForGandCrabB01C02D7();
	DWORD  CheckForGandCrabE948FAC8();

	//Added on 06-7-2018 by pradip
	DWORD  CheckForGandCrabG4();
	DWORD  CheckForChapakAGGT();
	DWORD  CheckForGandCrabGen2();
	DWORD  CheckForGandCrabGen4();
	DWORD  CheckForGandCrabG3();
    DWORD  CheckForChapakAGFR();
	DWORD  CheckForGandCryptBWH();
	DWORD  CheckForGandCrabD8();
	DWORD  CheckForGandCrab3C();
 
	//Added by Sagar on date 4-07-2018
	DWORD CheckForDompie_a();
	DWORD CheckForAtom_AA();
	DWORD CheckForAtom_AB();
	DWORD CheckForAtom_AD();
	DWORD CheckForAtom_AE();
	DWORD CheckForAtom_AF();
	DWORD CheckForAtom_AG();
	DWORD CheckForAtom_AH();
	DWORD CheckForAtom_AI();
	DWORD CheckForAtom_AJ();
	DWORD CheckForAtom_AK();
	DWORD CheckForAtom_AL();
	DWORD CheckForAtom_AM();
	DWORD CheckForAtom_AN();
	DWORD CheckForAtom_AO();
	DWORD CheckForAtom_AP();
	DWORD CheckForAtom_AQ();
	DWORD CheckForAtom_AR();
	DWORD CheckForAtom_AS();
	DWORD CheckForAtom_AT();
	DWORD CheckForAtom_AU();
	DWORD CheckForHesv_DGJY();
	DWORD CheckForHesv_DGUQ();
	DWORD CheckForShelma_ABPD();
	DWORD CheckForSnojan_CIWE();
	DWORD CheckForSnojan_CIWF();
	DWORD CheckForWaldek_BBIV();
	DWORD CheckForBriss_C();
	DWORD CheckForCrimson_QX();
	DWORD CheckForCrimson_QY();
	
	//shodhan 04/07/2018
	//DWORD CheckForTrojan_HILOTI();
	DWORD CheckForTrojan_HOBLIG();
	DWORD CheckForDisfa_NDXS();
	DWORD CheckForDisfa_NHFJ();
	DWORD CheckForRevenge_CFZ();
	DWORD CheckForTpyn_FHG();
	DWORD CheckForTrojan_RUND();
	DWORD CheckForTrojan_SKORIK();
	DWORD CheckForTrojan_SWIZZOR();
	DWORD CheckForWin32_ASPROTECT();
	DWORD CheckForWin32_BLUEH();
	DWORD CheckForBoht_VTQ();
	DWORD CheckForWin32_CAB();
	DWORD CheckForWin32_DDOS();

	//Added on 09-7-2018 by pradip
	DWORD  CheckForAgentGJP();
	DWORD  CheckForAgentILW();
	DWORD  CheckForAgentIZQ();
	DWORD  CheckForAgentJAJ();

	//Added on 10-7-2018 by pradip
	DWORD  CheckForFloxifE();
    DWORD  CheckForBlockerGDOR();
	DWORD  CheckForBlockerJYQU();
	DWORD  CheckForPornoBlockerEJTX();
	DWORD  CheckForSnocryCXU();
	
	//Added on 11-7-2018 by pradip
	DWORD  CheckForBlockerJWZQ();
	DWORD  CheckForBlockerJXWY();
	DWORD  CheckForBlockerJZED();
	DWORD  CheckForZerberBZAI();

	//Added on 12-7-2018 by pradip
	DWORD  CheckForPornoBlockerEKJT();
	DWORD  CheckForPornoBlockerEKKM();
	DWORD  CheckForPornoBlockerEKLE();
	DWORD  CheckForPornoBlockerEKQD();

	//Added on 13-7-2018 by pradip
	DWORD  checkforWannaAMAP();
	DWORD  checkforWannaAMDY();
	DWORD  checkforWannaAMEV();
	DWORD  checkforWannaAMSH();
	
	//Shodhan 13-07-2018
	DWORD CheckForFakeoff_BRS();
	DWORD CheckForFsysna_EUYC();
	DWORD CheckForWin32_INEXSMAR();
	DWORD CheckForMucc_GPB();
	DWORD CheckForWin32_NOPLEMENTO();
	DWORD CheckForPhpw_IHQ();
	DWORD CheckForPovertel_LX();
	DWORD CheckForWin32_ROPEST();
	DWORD CheckForSnojan_CHDL();
	DWORD CheckForSnojan_CHDM();
	DWORD CheckForWin32_TDSS();
	DWORD CheckForWin32_TURLA();
	DWORD CheckForWin32_WAKME();
	DWORD CheckForBanbra_WDHU();

	//Sagar 13-07-2018
	DWORD CheckForAndrom_NSTH();
	DWORD CheckForAndrom_NSTK();
	DWORD CheckForAndrom_NSUZ();
	DWORD CheckForAndrom_NSZS();
	DWORD CheckForAndrom_NSZT();
	DWORD CheckForAndrom_NTAH();
	DWORD CheckForAndrom_NTAV();
	DWORD CheckForAndrom_NTAZ();
	DWORD CheckForAndrom_NTBJ();
	DWORD CheckForAtom_AV();
	DWORD CheckForAtom_AW();
	DWORD CheckForAtom_AX();
	DWORD CheckForAtom_AY();
	DWORD CheckForAtom_AZ();
	DWORD CheckForAtom_BA();
	DWORD CheckForAtom_BB();
	DWORD CheckForAtom_BC();
	DWORD CheckForAtom_BE();
	DWORD  checkforFakonMWF();
	 //Added on 16-7-2018 by pradip
	DWORD  CheckforCerber247();
	DWORD  CheckforCerberA4();
	DWORD  CheckforJigSawA();
	DWORD  CheckforHfsAutoBD50E();
	
    //Added on 17-7-2018 by pradip
	DWORD  checkforTemondeA();
	DWORD  checkforAgentCNGZ();
	DWORD  checkforKhalesiA();
	DWORD  checkforKhalesiB();
	DWORD  checkforAutoItAKE();

	//Added on 18-7-2018 by pradip
	DWORD  checkforWizzMonetizeA();
	DWORD  CheckforSkeeyahB();
	DWORD  checkforWizremA();
	DWORD  checkforWizremB();

	//Added on 19-7-2018 by pradip
    DWORD  checkforAgentOH64();
	DWORD  checkforEquationDrugLE();
	DWORD  checkforShadowBrokersAA();
	DWORD  checkforShadowBrokersT();
	DWORD  checkforShadowBrokersZ();
    
   //Added on 20-7-2018 by pradip
	DWORD  checkforBlockerJJGL();
	DWORD  checkforBlockerJLPP(); 
	DWORD  checkforSillyDC();

	//Added on 23-7-2018 by pradip
	DWORD  checkforBlockerKPUO();
	DWORD  checkforBlockerKXPV();
	DWORD  checkforGlobeimposterA();
	DWORD  checkforGlobeimposterB();
	
   //Added on 24-7-2018 by pradip
	DWORD  checkforGlobeimposterC();
	DWORD  checkforGlobeimposterD();
	DWORD  checkforGlobeimposterE();
	DWORD  checkforGlobeimposterF();

	//Added on 25-7-2018 by pradip
	DWORD  checkforAgentGOS();
	DWORD  checkforAgentGOV();
	DWORD  checkforForeignKUOT();
	DWORD  checkforForeignNVXL();

	//Added on 26-7-2018 by pradip
	DWORD  checkforForeignNVXU();
	DWORD  checkforForeignNVXV();
	DWORD  checkforForeignNVYN();
	DWORD  checkforForeignNVYV();

	//Added on 27-7-2018 by pradip
	DWORD  checkforPornoAssetDBNC();
	DWORD  checkforPornoAssetDBNV();
	DWORD  checkforShadeOLW();
	DWORD  checkforShadeOMA();
	
	//Added on 28-7-2018 by pradip
	DWORD  checkforGandCryptBJV();
	DWORD  checkforGandCryptBJZ();
	DWORD  checkforGandCryptBKS();
	DWORD  checkforGandCryptBKU();
	DWORD  checkforGandCryptAWR();
	DWORD  checkforGandCryptOY();

    //Added on 30-7-2018 by pradiP
	DWORD  checkforGandCryptPH();
	DWORD  checkforGandCryptQG();
	DWORD  checkforScriptGeneric();
	DWORD  checkforWBNAROC();

	//Added on 31-07-2018 by shodhan
	DWORD CheckForDimnie_UT();
	DWORD CheckForDimnie_VT();
	DWORD CheckForWin32_FLEERCIVET();
	DWORD CheckForWin32_FSYSNA();
	DWORD CheckForHosts2_YQP();
	DWORD CheckForWin32_KOVTER();
	DWORD CheckForLlac_LMKU();
	DWORD CheckForNisloder_GET();
	DWORD CheckForWin32_POISON();
	DWORD CheckForWin32_QADARS();
	DWORD CheckForWin32_SKEEYAH();
	DWORD CheckForSnojan_CHYG();
	DWORD CheckForWin32_TANATOS();

	DWORD CheckForBanbra_WLCI();
	DWORD CheckForChthonic_JC();
	DWORD CheckForIcedID_DNV();

	DWORD CheckForNeutrinoPOS_CGD();
	DWORD CheckForShiotob_XFK();
	DWORD CheckForAutit_ARO();
	DWORD CheckForMudrop_YPR();
	DWORD CheckForChisburg_AJOD();

	DWORD CheckForCabby_ZIPXI();
	DWORD CheckForOmaneat_AMY();
	DWORD CheckForKeyLogger_BFLW();
	DWORD CheckForPanda_AWE();

	//Added By Sagar 01-08-2018
	DWORD CheckForAtom_BF();
	DWORD CheckForAtom_BH();
	DWORD CheckForAtom_BJ();
	DWORD CheckForAtom_BL();
	DWORD CheckForAtom_BM();
	DWORD CheckForAtom_DU();
	DWORD CheckForAtom_DV();
	DWORD CheckForAtom_DW();
	DWORD CheckForAtom_DZ();
	DWORD CheckForAtom_EO();
	DWORD CheckForAtom_EQ();
	DWORD CheckForAtom_FC();
	DWORD CheckForAtom_FO();
	DWORD CheckForAtom_FP();
	DWORD CheckForAtom_FQ();
	DWORD CheckForAtom_FS();
	DWORD CheckForAtom_FT();
	DWORD CheckForAtom_IY();
	DWORD CheckForAtom_JT();
	DWORD CheckForAtom_JU();
	DWORD CheckForAndrom_NLYX();
	DWORD CheckForAndrom_NMOO();
	DWORD CheckForAndrom_NMYP();
	DWORD CheckForAndrom_NNJA();
	DWORD CheckForAndrom_NNTL();
	DWORD CheckForAndrom_NNWH();
	DWORD CheckForAndrom_NNWQ();
	DWORD CheckForAndrom_NOBC();
	DWORD CheckForAndrom_NOCD();
	DWORD CheckForAndrom_NOCI();
	DWORD CheckForAndrom_NODX();
	DWORD CheckForAndrom_NOKG();
	DWORD CheckForAndrom_NOLS();
	DWORD CheckForAndrom_NOSN();
	DWORD CheckForAndrom_NOYK();
	DWORD CheckForAndrom_NOYN();
	DWORD CheckForAndrom_NPCJ();
	DWORD CheckForAndrom_NPDT();

	//Added on 03-08-2018 by pradip
	//Wajam
	DWORD CheckForAdwareWajam();
	DWORD CheckForAdwareWajamA();
	DWORD CheckForAdwareWajamB();

   //Added on 06-08-2018 by pradip
	DWORD CheckForBackswap();
	DWORD CheckForBandarchor();
	DWORD CheckForBrowlock();
	DWORD CheckForTeleGrab();
   
	//Added on 07-08-2018 by pradip
	DWORD CheckForVeilev();
    DWORD  CheckForGandCryptBSV();
	DWORD  CheckForGandCryptBTF();
	DWORD  CheckForGandCryptBTQ();

	//Added on 10-08-2018 by pradip
	DWORD  CheckForGandCryptBUJ();
	DWORD  CheckForGandCryptBWP();
	DWORD  CheckForGandCryptBWS();
	DWORD  CheckForGandCryptBWZ();

	//Added on 11-08-2018 by pradip
	DWORD  CheckForGandCryptBXV();
	DWORD  CheckForGandCryptBYP();
	DWORD  CheckForGandCryptBZL();
	DWORD  CheckForGandCryptBZR();

	//Added on 13-08-2018 by pradip
	DWORD  CheckForGandCryptCAH();
	DWORD  CheckForGandCryptCAQ();
	DWORD  CheckForGandCryptCAR();
	DWORD  CheckForGandCryptCBT();
	DWORD  CheckForGandCryptCBX();

	//Added on 14-08-2018 by pradip
	DWORD  CheckForGandCryptCCB();
	DWORD  CheckForGandCryptCCY();
	DWORD  CheckForGandCryptCDA();
	DWORD  CheckForGandCryptCDC();

	//Added on 15-08-2018 by pradip
	DWORD  CheckForGandCryptCDG();
	DWORD  CheckForGandCryptCDT();
	DWORD  CheckForGandCryptCEC();
	DWORD  CheckForGandCryptCEG();

	//Added on 16-08-2018 by pradip
	DWORD  CheckForGandCryptCEH();
	DWORD  CheckForGandCryptCEJ();
	DWORD  CheckForGandCryptCEO();
	DWORD  CheckForGandCryptCEV();

	//Shodhn on 16-08-2018
	DWORD CheckForWajamA();
	DWORD CheckForWajamB();
	DWORD CheckForWajamC();
	DWORD CheckForWajamD();
	DWORD CheckForWajamE();
	DWORD CheckForWajamF();
	DWORD CheckForWajamG();
	DWORD CheckForWajamH();
	DWORD CheckForRansom_GANDCRAB();
	DWORD CheckForRansom_GANDC();
	DWORD CheckForAutoit_LC();
	DWORD CheckForBlocker_IWKZ();
	DWORD CheckForBlocker_LALU();
	DWORD CheckForBlocker_LAMX();
	DWORD CheckForBlocker_LANL();
	DWORD CheckForBlocker_LANW();
	DWORD CheckForBlocker_LAOB();
	DWORD CheckForBlocker_LAPK();
	DWORD CheckForBlueScreen_NA();
	DWORD CheckForChameleonUnlicence_GC();
	DWORD CheckForGandCrypt_AAA();
	DWORD CheckForGandCrypt_AAS();
	DWORD CheckForGandCrypt_AAY();

    //64bit
	DWORD CheckForPidief_we();
	DWORD CheckForPidief_ww();

    //Added on 16-08-2018 by Sagar
	DWORD CheckForBrowseFox_V();
	DWORD CheckForMplug_DC();
	DWORD CheckForChapak_AMMD();
	DWORD CheckForKillAV_RJY();
	DWORD CheckForAndrom_NIQE();
	DWORD CheckForAndrom_NJDA();
	DWORD CheckForAndrom_NJWB();
	DWORD CheckForAndrom_NJYK();
	DWORD CheckForAndrom_NKYX();
	DWORD CheckForAndrom_NPYO();
	DWORD CheckForProrat_KCM();
	DWORD CheckForUdr_A();
	DWORD CheckForDelfiDelfi_DJX();
	DWORD CheckForGatak_GEN();
	DWORD CheckForReconyc_FLAD();
	DWORD CheckForSelfDel_GHFC();
	DWORD CheckForRanky_GEN();

	//Added on 18-08-2018 and 20-8-2018 by pradip
	DWORD  CheckForIcePol();
	DWORD  CheckForSpyAzorUlt();
	DWORD  CheckForBTCWare();
	DWORD  CheckForChapakJDJ();
	DWORD  checkforAutoRunENUF();
	DWORD  checkforHesvAVGR();
	DWORD  checkforAdwareTuto4PC();
	DWORD  checkforDNSCleanerGen();
	DWORD  checkforPowzip();
	DWORD  checkforAgentQWHDIZ();
	DWORD  checkforBitMinerBEK();
	DWORD  checkforZippyLoaderDBS();
	DWORD  checkforAdwareInstallcore();
	DWORD  checkforBrontok();
	DWORD  checkforBrontokB();
	
	//Added by Sagar 24-08 
	
	DWORD CheckForRansom_AASTG();
	DWORD CheckForRansom_BDFTC();
	DWORD CheckForRansom_DTPEZ();
	DWORD CheckForRansom_HEFHP();
	DWORD CheckForRansom_IRPIR();
	DWORD CheckForRansom_IWWIQ();
	DWORD CheckForRansom_NJJTI();
	DWORD CheckForRansom_QBUMR();
	DWORD CheckForRansom_QQKEV();
	DWORD CheckForRansom_1203201();
	DWORD CheckForRansom_203776();
	DWORD CheckForRansom_46976();
	DWORD CheckForRansom_79872();
	DWORD CheckForRansom_ADZPM();
	DWORD CheckForBTCware_KDVJF();
	DWORD CheckForCerber_BWBGV();
	DWORD CheckForCerber_HLWRN();
	DWORD CheckForCerber_OTIJX();
	DWORD CheckForRansom_AYZFU();
	DWORD CheckForRansom_HGYUQ();
	
	//shodhan on 24-08-18
	DWORD CheckForCrannbestfoxmail_AWD();
	DWORD CheckForCrannbestfoxmail_FIO();
	DWORD CheckForCrannbestfoxmail_IUD();
	DWORD CheckForCrannbestfoxmail_JYU();
	DWORD CheckForCrannbestfoxmail_KJUI();
	DWORD CheckForCrannbestfoxmail_LKE();
	DWORD CheckForCrannbestfoxmail_OLU();
	DWORD CheckForCrannbestfoxmail_PLOI();
	DWORD CheckForDanabot_DANABOT();
	DWORD CheckForIcePol_ICEPOL();
	DWORD CheckForKillDisk_KILLDISK();
	DWORD CheckForKronos_KRONOS();
	DWORD CheckForAgentTesla_AGENTTESLA();
	DWORD CheckForDrixed_DRIXED();
	DWORD CheckForEmotet_EMOTET();
	DWORD CheckForHancitor_HANCITOR();
	DWORD CheckForKayPass_KAYPASS();
	DWORD CheckForZeusPanda_ZEUSPANDA();
	DWORD CheckForNewJigsaw_NEWJI();
	DWORD CheckForNewJigsaw_NEWJIG();
	DWORD CheckForNewJigsaw_JIGS();
	DWORD CheckForNewJigsaw_WJI();
	DWORD CheckForNewJigsaw_IGSAW();
	DWORD CheckForNewJigsaw_SAW();
	DWORD CheckForRuftar_BNMK();
	DWORD CheckForShade_OSG();
	DWORD CheckForDowneks_BSX();
	DWORD CheckForPerez_A();
	DWORD CheckForWin32_RUSSOTURISTO();
	DWORD CheckForWin32_TRILISSA();
	DWORD CheckForWin32_ZOEK();
	DWORD CheckForBladabindi_AQLX();
	DWORD CheckForCardinal_AIN();
	DWORD CheckForBitman_ADUG();
	DWORD CheckForCidox_ACHM();
	DWORD CheckForCrusis_TQ();
	DWORD CheckForGen_AK();

	DWORD CheckGenericTKA_17();

	//Shodhan 29-08-2018 
	DWORD CheckForDarkComet_27082018();
	DWORD CheckForEmotet_28082018();
	DWORD CheckForFareit_27082018();
	DWORD CheckForFareitNew_27082018();
	DWORD CheckForHancitor_28082018();
	DWORD CheckForKronos_28082018();
	DWORD CheckForTepfer_27082018();
	DWORD CheckForSmokeLoader();
	DWORD CheckForTrickBot();
	DWORD CheckForTrickBot_2018();
	DWORD CheckForUrsnif_28082018();
	DWORD CheckForEmotet_AC();
	DWORD CheckForEmotet_ACBIT();
	DWORD CheckForFuerboos_ACL();
	DWORD CheckForGandcrab_AF();
	DWORD CheckForNocturnal_A();
	DWORD CheckForTiggreirfn();
	DWORD CheckForVigorf_A();
	DWORD CheckForZeurS_27082018();
	//Sagar 31-08-2018
	DWORD CheckForAtom_OV();
	DWORD CheckForAtom_OW();
	DWORD CheckForAtom_UF();
	DWORD CheckForAtom_UG();
	DWORD CheckForAtom_UI();
	DWORD CheckForAtom_UJ();
	DWORD CheckForAtom_UU();
	DWORD CheckForAtom_UX();
	DWORD CheckForAtom_UY();
	DWORD CheckForAtom_UZ();
	DWORD CheckForAtom_V();
	DWORD CheckForGandCrypt_BTCWARE();
	DWORD CheckForGandCrypt_CGS();
	DWORD CheckForGandCrypt_CGT();
	DWORD CheckForGandCrypt_CGU();
	DWORD CheckForGandCrypt_CGV();
	DWORD CheckForGandCrypt_CGW();
	DWORD CheckForGandCrypt_MAKTUBLOCKER();
	DWORD CheckForGandCrypt_RAPID();

	////Added by pradip
	DWORD  CheckForBredolabZJF();
	DWORD  CheckForPlocustKHUD();
	DWORD  CheckForAgentIFWB();
	DWORD  CheckForAgentIGTV();
	DWORD  CheckForBadurKPUS();
	DWORD  CheckForGandCryptCFJ();
	DWORD  CheckForGandCryptCFK();
	DWORD  CheckForGandCryptCFP();
	DWORD  CheckForGandCryptCGZ();
	DWORD  CheckForGandCryptCHI();
	DWORD  CheckForGandCryptCHJ();
    DWORD  CheckForCrysisA();
	DWORD  CheckForFilecoderD();
    DWORD  CheckForForeignGG();
	DWORD  CheckForGandcrabF();
	DWORD  CheckForGandCrabR();
    DWORD  CheckForDhramaBIP();
	DWORD  CheckForDhramaBIG();
	//Shodhan 06-09-18
	DWORD CheckForCobalt_A();
	DWORD CheckForDofoil_AC();
	DWORD CheckForFuerboos_C();
	DWORD CheckForGandcrab_ER();
	DWORD CheckForHermes_ERT();
	DWORD CheckForHermes_ET();
	DWORD CheckForScarab_AC();
	DWORD CheckForScarab_BE();
	DWORD CheckForScarab_RW();
	DWORD CheckForScarab_AD();
	DWORD CheckForVigorf_FR();
	DWORD CheckForTrojan_AUTOITDOWNLOADER();
	DWORD CheckForAutophyte_AIDHA();
	DWORD CheckForDofoil_ACD();
	DWORD CheckForTrojan_DRIDEX();
	DWORD CheckForMereTam_A();
	DWORD CheckForVigorf_S();
	DWORD CheckForXorist();
	DWORD CheckForXorist_RF();
	DWORD CheckForXorist_RT();
	DWORD CheckForXorist_RY();
	DWORD CheckForXorist_RI();
	DWORD CheckForXorist_RO();

	//Added By pradip on date 7-09-2018
	DWORD  CheckForBlockerLBGH();
	DWORD  CheckForBlockerLBGI();
	DWORD  CheckForBlockerLBIH();
	DWORD  CheckForBlockerLBLU();
	DWORD  CheckForCrusisCLR();
	DWORD  CheckForCrypmodZIM();
	DWORD  CheckForCryprenADKU();
	DWORD  CheckForForeignOACW();
	DWORD  CheckForGandCryptACC();
	DWORD  CheckForGandCryptAFY();
	DWORD  CheckForGandCryptACN();
	DWORD  CheckForGandCryptAGI();
	DWORD  CheckForGandCryptAHG();
	DWORD  CheckForGandCryptAHY();
	DWORD  CheckForGandCryptAIB();
	DWORD  CheckForGandCryptAKH();
	DWORD  CheckForGandCryptAKV();
	DWORD  CheckForGandCryptALD();
	DWORD  CheckForGandCryptAMU();
	DWORD  CheckForGandCryptAPN();
	DWORD  CheckForGandCryptAQK();
	DWORD  CheckForGandCryptAQO();
	DWORD  CheckForGandCryptASO();
	DWORD  CheckForGandCryptASQ();
	DWORD  CheckForGandCryptASY();
	DWORD  CheckForGandCryptATE();
    DWORD  CheckForGandCryptATJ();
	DWORD  CheckForGandCryptATO();
	DWORD  CheckForGandCryptATQ();
	DWORD  CheckForGandCryptATS();
	DWORD  CheckForAgentGenVUNDO();
	DWORD  CheckForAndromTMH();
	DWORD  CheckForKelihosRFN();
	DWORD  CheckForForeignCGIC();
	DWORD  CheckForCeeInjectA();

	//Sagar 07-09
	DWORD CheckForAgent_FQIB();
	DWORD CheckForAgent_GOE();
	DWORD CheckForMyxaH_PFO();
	DWORD CheckForBlocker_JRDO();
	DWORD CheckForBlocker_JTDU();
	DWORD CheckForBlocker_JTSH();
	DWORD CheckForBlocker_KKFZ();
	DWORD CheckForForeign_NNZZ();
	DWORD CheckForForeign_NOAB();
	DWORD CheckForForeign_NPCR();
	DWORD CheckForGen_HEH();
	DWORD CheckForGeneric_AB();
	DWORD CheckForBredolab_ZJF();
	DWORD CheckForAgent_BLKX();
	DWORD CheckForAgent_BNAH();
	DWORD CheckForBitWallet_X();
	DWORD CheckForTrickster_DL();
	DWORD CheckForGenericKD_30();
	DWORD CheckForGenericKD_A();
	DWORD CheckForCoins_DRJ();
	DWORD CheckForCoins_JV();
	DWORD CheckForBlocker_LCMQ();
	DWORD CheckForRansomKD_12();
	DWORD CheckForRazy_21();
	DWORD CheckForAgent_QWGWAH();
	DWORD CheckForGeneric_AP();
	DWORD CheckForNisloder_GGR();
	DWORD CheckForOccamy_C();
	DWORD CheckForReconyc_IWKK();
	DWORD CheckForYakes_TBW();
	DWORD CheckForMulti_GENERIC_A();

   //Added By pradip on date 11-9-2018
	DWORD  CheckForZACCESS();
	DWORD  CheckForInjectAUTO();
	DWORD  CheckForMaxplusCWINQC();
	DWORD  CheckForMiurefTC();
	DWORD  CheckForOccamyB();
	DWORD  CheckForOccamyD();
	DWORD  CheckForSakuraED64();
	DWORD  CheckForSirefefCK();
	DWORD  CheckForSirefefP();
	DWORD  CheckForSkeeyahA();
	DWORD  CheckForTiggreRFN();
	DWORD  CheckForUrausyE();
	DWORD  CheckForWLDCRC();
	DWORD  CheckForZAccessBA();
	DWORD  checkforPoisonJBIU();
	DWORD  checkforUpatreAIKR();
	DWORD  checkforGenericLJKG();
	DWORD  checkforKryptikEXTJMK();
	DWORD  checkforNymaimBFDV();
	DWORD  checkforVBKryptESZQTA();
	DWORD  checkforCerberSMALY0A();

	//Nihar 11-09-18
	DWORD CheckForTR_RANSOMKD();
	DWORD CheckForTrojanRansom_CERBER();
	DWORD CheckForTrojanRansom_CRYPTOLOCKER();
	DWORD CheckForTrojanRansom_LOCKY();
	DWORD CheckForTrojanRansom_ROKKU();
	DWORD CheckForTrojanRansom_WANNACRY();
	//DWORD CheckForWin32_BIRELE();
	DWORD CheckForWin32_BLOCKER();
	DWORD CheckForWin32_GIMEMO();
//	DWORD CheckForTrojanRansomer();

	//Nihar
	DWORD CheckForCrusis_AEJ();
	DWORD CheckForZerber_ENIY();
	DWORD CheckForZerber_ENKV();
	DWORD CheckForZerber_ENPK();
	DWORD CheckForWin32_WANNACRYPT();
	DWORD CheckForRansomHeur_IJLVJ();
	DWORD CheckForRansomHeur_ILYDA();
	DWORD CheckForRansomHeur_JPLDU();
	DWORD CheckForRansomHeur_MPBPI();
	DWORD CheckForRansomHeur_YSGPS();
	DWORD CheckForRansom207360_1();
	DWORD CheckForRansom_550912();
	DWORD CheckForRansom_BHLPG();
	DWORD CheckForRansom_CCIAL();
	DWORD CheckForCerber_FBQVP();
	DWORD CheckForCerber_LQMHG();
	DWORD CheckForCerber_QQMPG();

	//Sagar
	DWORD CheckForEmotet_BDDS();
	DWORD CheckForTrickster_KK();
	DWORD CheckForGeral_BQIN();
	DWORD CheckForMSIL_POSH();
	DWORD CheckForWin32_CRYPMOD();
	DWORD CheckForGen_EAG();
	DWORD CheckForGen_FUQ();
	DWORD CheckForLocky_ACFL();
	DWORD CheckForPornoAsset_CRPC();
	DWORD CheckForPornoAsset_CYLS();
	DWORD CheckForSpora_FGP();
	DWORD CheckForZerber_FGXS();
	DWORD CheckForWLock_AH();
	DWORD CheckForWLock_BG();
	DWORD CheckForPanda_BWW();
	DWORD CheckForKazy_72();
	DWORD CheckForAgentb_JEJK();
	DWORD CheckForJohnnie_11();
	DWORD CheckForIcedID_13();
	DWORD CheckForZusy_28();

	//Added by pradip on date 12-9-2018 to 21-9-2018
	DWORD  CheckForBryteA();
	DWORD  CheckForBredolabAGBW();
	DWORD  CheckForBredolabO();
	DWORD  CheckForBredolabFI();
	DWORD  CheckForFKMFBLJSM();
	DWORD  CheckForPlocustDHXMLR();
	DWORD  CheckForSpyLLGY();
	DWORD  CheckForTogaRFN();
	DWORD  CheckForXPACKVKFPV();
	DWORD  CheckForYakesTBW();
	//64 bit
	DWORD  CheckForAdwareNEO();
	DWORD  CheckForAgentQWHHOW();
	DWORD  CheckForBitCoinMinerJSUC();
	DWORD  CheckForXMRigMinerA();
	
	DWORD  CheckForGandCryptAQG();
	DWORD  CheckForGandCryptAQQ();
	DWORD  CheckForGandCryptAQS();
	DWORD  CheckForGandCryptAQU();
	DWORD  CheckForGandCryptAQV();
	DWORD  CheckForGandCryptAQW();
	DWORD  CheckForGandCryptAQY();
	DWORD  CheckForGandCryptAQZ();
	DWORD  CheckForGandCryptARC();
	DWORD  CheckForGandCryptARF();
	DWORD  CheckForGandCryptARN();
	DWORD  CheckForGandCryptARO();
	DWORD  CheckForGandCryptARS();
	DWORD  CheckForGandCryptARV();
	DWORD  CheckForGandCryptARW();
	DWORD  CheckForGandCryptARY();
	DWORD  CheckForGandCryptARZ();
	DWORD  CheckForGandCryptASD();
	DWORD  CheckForGandCryptASK();
	DWORD  CheckForGandCryptASN();
	DWORD  CheckForGandCryptATP();
	DWORD  CheckForGandCryptAUO();
    DWORD  CheckForGandCryptAUS();
	DWORD  CheckForGandCryptAUV();
	DWORD  CheckForGandCryptAUY();
	DWORD  CheckForGandCryptAVA();
	DWORD  CheckForGandCryptAVC();
	DWORD  CheckForGandCryptAVE();
	DWORD  CheckForGandCryptAVF();
	DWORD  CheckForGandCryptAVG();
	DWORD  CheckForGandCryptAVH();
	DWORD  CheckForGandCryptAVJ();
	DWORD  CheckForGandCryptAVK();


	//Nihar
	DWORD CheckForCerber_UDTUK();
	DWORD CheckForCerber_UZMZU();
	DWORD CheckForRansom_CEUHW();
	DWORD CheckForRansom_COEKE();
	DWORD CheckForCrysis_EAIAI();
	DWORD CheckForRansom_DNEFB();
	DWORD CheckForEB_23();
	DWORD CheckForRansom_JKTSO();
	DWORD CheckForRansom_JOXBF();
	DWORD CheckForJQ_7();
	DWORD CheckForRansom_KXKJU();
	DWORD CheckForRansom_LKOFD();
	DWORD CheckForRansom_NPWDW();
	DWORD CheckForRansom_OAFWJ();
	DWORD CheckForRansom_OFRGU();
	DWORD CheckForRansom_OKRPX();
	DWORD CheckForRansom_OOVYH();
	DWORD CheckForRansom_OQPQZ();
	DWORD CheckForRansom_PXIOD();
	DWORD CheckForRansom_QCIQD();
	DWORD CheckForRansom_QHFNH();
	DWORD CheckForRansom_RFWAB();
	DWORD CheckForRansom_RLUEQ();
	DWORD CheckForRansom_RRFDS();
	DWORD CheckForRansom_SOTBF();
	DWORD CheckForRansom_SVFGQ();
	DWORD CheckForRansom_TIXKL();
	DWORD CheckForRansom_UIZIA();
	DWORD CheckForRansom_UL();
	DWORD CheckForRansom_UZZXO();


	DWORD CheckForRansom_EWEQE();
	DWORD CheckForRansom_FCRCC();
	DWORD CheckForRansom_FEKPL();
	DWORD CheckForRansom_FGOUC();
	DWORD CheckForRansom_GEN();
	DWORD CheckForRansom_GZFZN();
	DWORD CheckForRansom_IJYTP();
	DWORD CheckForRansom_JAINZ();
	DWORD CheckForJigsawLocker_CZNHS();
	DWORD CheckForJigsawLocker_TZRGZ();
	DWORD CheckForRansom_XORZU();
	DWORD CheckForRansom_XXLEU();
	DWORD CheckForRansom_YOTKG();
	DWORD CheckForRansom_ZLVLQ();
	DWORD CheckForBirele_AJDN();
	DWORD CheckForBlocker_CGYD();
	
	DWORD CheckForBlocker_KXBS();
	DWORD CheckForBlocker_KXGQ();
	DWORD CheckForBlocker_KXNL();
	DWORD CheckForBlocker_KXNT();
	DWORD CheckForBlocker_KXNY();
	DWORD CheckForBlocker_KXPV();
	DWORD CheckForCrusis_CDW();
	DWORD CheckForCrusis_CEI();
	DWORD CheckForCrusis_CEK();
	DWORD CheckForCrusis_CEL();
	DWORD CheckForCrusis_CEN();
	DWORD CheckForCrypmod_YIY();
	DWORD CheckForCrypmodadv_XQW();
	DWORD CheckForCryptodef_BYS();
	DWORD CheckForCryptodef_CKU();
	DWORD CheckForCryptor_BRK();
	DWORD CheckForCryrar_HAP();
	DWORD CheckForForeign_NZEC();
	DWORD CheckForGandCrypt_DP();
	DWORD CheckForGandCrypt_EU();
	DWORD CheckForGandCrypt_FM();
	DWORD CheckForGandCrypt_GD();
	DWORD CheckForGandCrypt_GO();
	DWORD CheckForGenericCryptor_CYS();
	DWORD CheckForPornoAsset_CSAK();


	DWORD CheckForBlocker_BSHV();
	DWORD CheckForBlocker_CJYK();
	DWORD CheckForBlocker_HASO();
	DWORD CheckForBlocker_HRFT();
	DWORD CheckForBlocker_IYJG();
	DWORD CheckForBlocker_JCEN();
	DWORD CheckForBlocker_JKRW();
	DWORD CheckForBlocker_JZKE();
	DWORD CheckForBlocker_KNBP();
	DWORD CheckForBlocker_KNHJ();
	
	
	DWORD CheckForCrusis_BSD();
	DWORD CheckForCrusis_OD();
	DWORD CheckForCrusis_TO();

	DWORD CheckForCryrar_GWS();
	DWORD CheckForCryrar_GWT();

	//shodhan 24/09/18
	DWORD CheckForTrogen_AGENTTESLA();
	DWORD CheckForTrogen_CHANITOR();
	DWORD CheckForTrogen_EMOTET();
	DWORD CheckForGandCrab_NEW();
	DWORD CheckForTrogen_HAWKEYE();
	DWORD CheckForTrogen_KRAKEN();
	DWORD CheckForRansom_CG();
	DWORD CheckForRansom_EW();
	DWORD CheckForRansom_HJ();
	DWORD CheckForRansom_N();
	DWORD CheckForRansom_NE();
	DWORD CheckForRansom_NEW();
	DWORD CheckForRansom_RY();
	DWORD CheckForRansom_VD();
	DWORD CheckForRansom_WS();
	DWORD CheckForRansom_YU();
	DWORD CheckForTrickBot_YI();
	DWORD CheckForTrogen_TRICKSTER();
	DWORD CheckForUnwaders_C();
	DWORD CheckForUpatre_OI();
	DWORD CheckForZbot_PANDA();
	DWORD CheckForFuery_B();
	DWORD CheckForLocky_RY();
	DWORD CheckForLocky_TR();
	DWORD CheckForLocky_WQ();
	DWORD CheckForPropagate_UT();
	DWORD CheckForRapid_A();
	DWORD CheckForBirele_AIMJ();
	DWORD CheckForBitman_QMF();
	DWORD CheckForBlocker_DAFN();
	DWORD CheckForBlocker_DVJN();
	DWORD CheckForBlocker_EABS();
	DWORD CheckForBlocker_FRTJ();
	DWORD CheckForBlocker_GFHU();
	DWORD CheckForBlocker_GFVP();
	DWORD CheckForBlocker_JCZM();
	DWORD CheckForBlocker_JQVT();
	DWORD CheckForBlocker_JUEH();
	DWORD CheckForBlocker_JVVN();
	DWORD CheckForBlocker_JXBH();
	DWORD CheckForBlocker_JXEQ();
	DWORD CheckForBlocker_JXKZ();
	DWORD CheckForBlocker_JYEH();
	DWORD CheckForBlocker_JZEC();
	DWORD CheckForBlocker_JZSF();
	DWORD CheckForBlocker_JZTT();
	DWORD CheckForBlocker_KAXU();

	//Added by pradip 
	DWORD  CheckForGandCryptAIP();
	DWORD  CheckForGandCryptAKD();
    DWORD  CheckForChapakAVEX();
	DWORD  CheckForGenericV2();
	DWORD  CheckForInjectAKKOO();
	DWORD  CheckForInjectAKLBY();
	DWORD  CheckForPhorpiexFIBTJA();
	DWORD  CheckForVigorfA();

	//Nihar28-09-18
	DWORD CheckForAgent_ABEK();
	DWORD CheckForGen_Q();
	DWORD CheckForGenericCryptor_CZU();
	DWORD CheckForGenericCryptor_CZX();
	DWORD CheckForLocky_CDM();
	DWORD CheckForLocky_CRV();
	DWORD CheckForLocky_CTC();
	DWORD CheckForLocky_WLS();
	DWORD CheckForLocky_WLY();
	DWORD CheckForPornoAsset_CWHQ();
	DWORD CheckForPornoAsset_CWKW();
	DWORD CheckForPornoAsset_CWLG();
	DWORD CheckForPornoAsset_CYEB();
	DWORD CheckForPornoAsset_CZCO();
	DWORD CheckForPornoBlocker_EJTX();
	DWORD CheckForPornoBlocker_EJWO();
	DWORD CheckForPornoBlocker_EKJT();
	DWORD CheckForPornoBlocker_EKLE();
	DWORD CheckForPurga_H();
	DWORD CheckForPurga_K();
	DWORD CheckForPurgen_AGD();
	DWORD CheckForPurgen_IB();
	DWORD CheckForPurgen_RE();
	DWORD CheckForRack_GUN();
	DWORD CheckForRakhni_VJ();
	DWORD CheckForSageCrypt_JE();
	DWORD CheckForSageCrypt_QF();
	DWORD CheckForShade_NRV();
	DWORD CheckForWanna_AMGF();
	DWORD CheckForWanna_AMGJ();
	DWORD CheckForWanna_AMGL();
	DWORD CheckForWanna_ZBU();
	DWORD CheckForXorist_LK();
	DWORD CheckForXpan_C();
	DWORD CheckForZedoPoo_PRX();
	DWORD CheckForZerber_AUFX();
	DWORD CheckForZerber_BEKC();
	DWORD CheckForZerber_COPN();
	DWORD CheckForZerber_DHFG();
	DWORD CheckForZerber_DHNH();
	DWORD CheckForZerber_DYXR();
	DWORD CheckForZerber_DYYC();
	DWORD CheckForZerber_ECQJ();
	DWORD CheckForZerber_EFCS();
	DWORD CheckForZerber_EHUA();
	DWORD CheckForZerber_EITR();
	DWORD CheckForZerber_EKEX();
	DWORD CheckForZerber_ERXH();
	DWORD CheckForZerber_ERXW();

	//Shodhan 28-09-2018
	DWORD CheckForBlocker_KBJU();
	DWORD CheckForBlocker_KDWP();
	DWORD CheckForBlocker_KFDT();
	DWORD CheckForBlocker_KFGI();
	DWORD CheckForBlocker_KFGK();
	DWORD CheckForBlocker_KKMT();
	DWORD CheckForBlocker_KKXA();
	DWORD CheckForBlocker_KQQE();
	DWORD CheckForBlocker_OOW();
	DWORD CheckForBlocker_TRK();
	DWORD CheckForCryakl_ASE();
	DWORD CheckForEmotet_VGT();
	DWORD CheckForMagniber_ASW();
	DWORD CheckForMagniber_BGR();
	DWORD CheckForMagniber_OPI();
	DWORD CheckForMagniber_PLM();
	DWORD CheckForMagniber_RGY();
	DWORD CheckForMagniber_RVN();
	DWORD CheckForMagniber_TGV();
	DWORD CheckForMagniber_TYE();
	DWORD CheckForMagniber_UIO();
	DWORD CheckForMagniber_XDR();
	DWORD CheckForMagniber_YHB();
	DWORD CheckForMagniber_ZAQ();
	DWORD CheckForNanocore_BGT();
	DWORD CheckForNanocore_BNH();
	DWORD CheckForNanocore_CDR();
	DWORD CheckForNanocore_CFB();
	DWORD CheckForNanocore_EDF();
	DWORD CheckForNanocore_FGV();
	DWORD CheckForNanocore_GV();
	DWORD CheckForNanocore_IOP();
	DWORD CheckForNanocore_KJN();
	DWORD CheckForNanocore_MJK();
	DWORD CheckForNanocore_TGY();
	DWORD CheckForNanocore_TUJ();
	DWORD CheckForNanocore_VFG();
	DWORD CheckForNanocore_WRT();
	DWORD CheckForNanocore_XSC();
	DWORD CheckForNanocore_XSD();
	DWORD CheckForRamnit_LIP();
	DWORD CheckForRedyms_DFR();
	DWORD CheckForRedyms_FRT();
	DWORD CheckForRedyms_GYJ();
	DWORD CheckForSamSam_BYM();
	DWORD CheckForSamSam_VYM();
	DWORD CheckForTrojenLoki_XDR();

	//Sagar - 28-09-2018
	DWORD CheckForAndrom_GEN();
	DWORD CheckForGeneric_DD();
	DWORD CheckForPhpw_ANQ();
	DWORD CheckForFlooder_GEN();
	DWORD CheckForFlyStudio_GEN();
	DWORD CheckForOmniTweak_GEN();
	DWORD CheckForUniblue_GEN();
	DWORD CheckForAgent_XXDPBN();
	DWORD CheckForAgent_AEPH();
	DWORD CheckForAgent_GENA();
	DWORD CheckForAgent_JKZP();
	DWORD CheckForAmonetize_AMQQ();
	DWORD CheckForAmonetize_BDXA();
	DWORD CheckForBHO_AXTO();
	DWORD CheckForEorezo_EMOM();
	DWORD CheckForFileFinder_GEN();
	DWORD CheckForFunshion_GU();
	DWORD CheckForKraddare_AMS();
	DWORD CheckForKuaiba_A();
	DWORD CheckForKuaiba_AEY();
	DWORD CheckForKuaiba_AGM();
	DWORD CheckForMediaMagnet_JI();
	DWORD CheckForZvuZona_B();
	DWORD CheckForConduit_GEN();
	DWORD CheckForEverFresh_GEN();
	DWORD CheckForFunshion_GEN();
	DWORD CheckForICLoader_GEN();
	DWORD CheckForKuaiZip_GEN();
	DWORD CheckForLinkury_GEN();
	DWORD CheckForMediaMagnet_GEN();
	DWORD CheckForWindapp_GEN();
	DWORD CheckForAmmyy_AQM();
	DWORD CheckForAmmyy_XMR();
	DWORD CheckForAmmyy_XZX();
	DWORD CheckForSogou_WI();
	DWORD CheckForMultiPacked_GEN();
	DWORD CheckForBlack_D();
	DWORD CheckForCrypt_GENA();
	DWORD CheckForStartPage_W();
	DWORD CheckForVobfus_AYIX();

	//Added By pradip on 1-10-2018
    DWORD  CheckForGandCryptAKK();
	DWORD  CheckForXamyhAEO();
	DWORD  CheckForAgentJAW();
	DWORD  CheckForHermezRR();
	DWORD  CheckForlnhyeA();
	DWORD  CheckForPornoAssetA();
	DWORD  CheckForPornoBlockerEKLD();
	DWORD  CheckForPornoBlockerELMJ();
	DWORD  CheckForPornoBlockerELXE();
	DWORD  CheckForPornoBlockerEMBR();
	DWORD  CheckForPurgenVP();
	DWORD  CheckForPurgenVQ();
	DWORD  CheckForSporaESE();
	DWORD  CheckForZerberABVV();
	DWORD  CheckForBlockerHJXN();
	DWORD  CheckForBlockerIELN();
	DWORD  CheckForBlockerIFPD();
	DWORD  CheckForBlockerIFPG();
	DWORD  CheckForBlockerIFPM();
	DWORD  CheckForBlockerIFPV();
	DWORD  CheckForBlockerIFPY();
	DWORD  CheckForBlockerIFPZ();
	DWORD  CheckForCryprenADDQ();
	DWORD  CheckForCryrarGMC();
	DWORD  CheckForForeignNKWL();
	DWORD  CheckForForeignNMEI();
	DWORD  CheckForGenCGS();
	DWORD  CheckForGenCYT();
	DWORD  CheckForGenDNQ();
	DWORD  CheckForGenDNU();
	DWORD  CheckForGenDOJ();
	DWORD  CheckForGenericBS();
	DWORD  CheckForInject1FEVHTY();
	DWORD  CheckForPMaxA();
	DWORD  CheckForSirefefDA();
	DWORD  CheckForAimbotBN();
	DWORD  CheckForKirtsA();
	DWORD  CheckForRBotYCX();
	DWORD  CheckForBetisryptB();
	DWORD  CheckForGenericACCLK();
	DWORD  CheckForGenericBN();
	DWORD  CheckForRbotKPS();
	DWORD  CheckForBTCWare44();
	DWORD  CheckForCryptAS();

	//Shodhan-05-10-2018
	DWORD CheckForAgent_PUQNG();
	DWORD CheckForTrojan_AKANEM();
	DWORD CheckForTrojan_ANGPULA();
	DWORD CheckForTrojan_BAYROB();
	DWORD CheckForTrojan_CAIEGI();
	DWORD CheckForTrojan_CATIROS();
	DWORD CheckForChapak_A();
	DWORD CheckForChapk_RR();
	DWORD CheckForTrojan_COPERAS();
	DWORD CheckForTrojan_ECONE();
	DWORD CheckForEcone_LOK();
	DWORD CheckForFareit_GI();
	DWORD CheckForTrojan_FILECODER();
	DWORD CheckForTrojan_FRANKERS();
	DWORD CheckForTrojan_FUERBOOS();
	DWORD CheckForFuerboos_EDF();
	DWORD CheckForTrojan_GUSTAKIP();
	DWORD CheckForGustakip_LTL();
	DWORD CheckForTrojan_MALPACK();
	DWORD CheckForTrojan_MIDIE();
	DWORD CheckForTrojan_MIKEY();
	DWORD CheckForMikey_JNB();
	DWORD CheckForTrojan_PALITURAS();
	DWORD CheckForTrojan_PHORPIEX();
	DWORD CheckForTrojan_PRAMRAH();
	DWORD CheckForPWS_PANDA();
	DWORD CheckForPWS_STEALER();
	DWORD CheckForTrojan_RAPRUA();
	DWORD CheckForTrojan_RIMANRAO();
	DWORD CheckForTrojan_RIMANRAOI();
	DWORD CheckForTrojan_SIGMAL();
	DWORD CheckForTrojan_CAVEUI();
	DWORD CheckForTrojan_EMOTETFUERBOOS();
	DWORD CheckForTrojan_FRIED();
	DWORD CheckForTrojan_TRICKBOTTRICKSTER();
	DWORD CheckForCryptoff_BMC();
	DWORD CheckForCrypmod_XRL();
	DWORD CheckForCrypren_ADOF();
	DWORD CheckForCryptodef_WUJ();
	DWORD CheckForCryptodef_ZKU();
	DWORD CheckForCryptoff_XE();
	DWORD CheckForCryptor_FP();
	DWORD CheckForCryptor_IQ();
	DWORD CheckForCryptor_PX();
	DWORD CheckForForeign_EHJY();
	DWORD CheckForForeign_NIPZ();
	DWORD CheckForForeign_NJQA();

	//Added by pradip on 4-10-2018
	DWORD  CheckForAgentCXCE();
	DWORD  CheckForChapakASKB();
	DWORD  CheckForShadowBrokersAL();
	DWORD  CheckForShadowBrokersY();
	DWORD  CheckForXMRigMINER();
	DWORD  CheckForEquationDrugA();
	DWORD  CheckForEquationDrugJF();
	DWORD  CheckForShadowBrokersAC();
	DWORD  CheckForShadowBrokersAM();
	DWORD  CheckForShadowBrokersAQ();
	DWORD  CheckForAgentCXCD();
	DWORD  CheckForShadowBrokersATN();
	DWORD  CheckForShadowBrokersAX();
	DWORD  CheckForShadowBrokersB();
	DWORD  CheckForShadowBrokersAE();
	DWORD  CheckForShadowBrokersMC();
	DWORD  CheckForAntiAVCRNF();
	DWORD  CheckForGluptebaFIINFJ();
	DWORD  CheckForShadowBrokersEQCYHW();
	DWORD  CheckForAgentDBZX();
	DWORD  CheckForChapakAWYM();
	DWORD  CheckForVBKryptDYZVRB();
	DWORD  CheckForGandcrabAF();
	DWORD  CheckForGlobimAC();
	DWORD  CheckForBlockerDVJN();
	DWORD  CheckForBlockerJLPP();
	DWORD  CheckForBlockerJMJH();
	DWORD  CheckForBlockerJRCJ();
	DWORD  CheckForBlockerJTSH();
	DWORD  CheckForBlockerKMJD();
	DWORD  CheckForBlockerKSGQ();
	DWORD  CheckForCrypmodZKW();
	DWORD  CheckForCrypmodZKX();
	DWORD  CheckForCryprenADWG();
	DWORD  CheckForCryptorBSM();
	DWORD  CheckForCryptorBTM();
	DWORD  CheckForGandCryptBFT();
	DWORD  CheckForGandCryptBFX();
	DWORD  CheckForGandCryptBGK();
	DWORD  CheckForGandCryptBGM();
	DWORD  CheckForGandCryptBGR();
	DWORD  CheckForGandCryptBCO();
	DWORD  CheckForGandCryptBCR();
	DWORD  CheckForGandCryptBDH();
	DWORD  CheckForGandCryptBDP();
	DWORD  CheckForGandCryptBEP();
	DWORD  CheckForGandCryptBFD();
	DWORD  CheckForGandCryptBFM();
	DWORD  CheckForGandCryptBFN();
	DWORD  CheckForGandCryptBGU();
	DWORD  CheckForGandCryptBGX();

	//Sagar-05-10-2018
	DWORD CheckForSMYd_4E();
	DWORD CheckForMidie_50491();
	DWORD CheckForGeneric_ADA();
	DWORD CheckForGeneric_ACB();
	DWORD CheckForPhorpiex_I();
	DWORD CheckForBanbra_WLA();
	DWORD CheckForAdload_AB();
	DWORD CheckForAdload_TC();
	DWORD CheckForGenome_STA();
	DWORD CheckForQQHelper_VA();
	DWORD CheckForAgent_HNMS();
	DWORD CheckForSysn_BAMW();
	DWORD CheckForCoins_ISA();
	DWORD CheckForPosh_ACB();
	DWORD CheckForOnion_GEN();
	DWORD CheckForBlocker_HRSC();
	DWORD CheckForCrypmod_AB();
	DWORD CheckForForeign_OATJ();
	DWORD CheckForGen_DFE();
	DWORD CheckForGen_FCCQ();
	DWORD CheckForLocky_ACEW();
	DWORD CheckForPornoAsset_CRPQ();
	DWORD CheckForPornoAsset_CYT();
	DWORD CheckForZerber_FX();
	DWORD CheckForWLock_AS();
	DWORD CheckForWLock_B();
	DWORD CheckForReptile_AC();
	DWORD CheckForBlackv_GEN();
	DWORD CheckForGenome_QHGC();
	DWORD CheckForChapak_SB();
	DWORD CheckForHosts2_AB();
	DWORD CheckForgen_HOAX();
	DWORD CheckForFlyStudio_PEF();
	DWORD  CheckForGandCryptAZL();
	DWORD  CheckForGandCryptBAB();
	DWORD  CheckForGandCryptBAH();
	DWORD  CheckForGandCryptBAJ();
	DWORD  CheckForGandCryptBAK();
	DWORD  CheckForGandCryptBAW();
	DWORD  CheckForGandCryptBAZ();
	DWORD  CheckForGandCryptBBY();
	DWORD  CheckForGandCryptBCG();
	DWORD  CheckForGandCryptBHC();

	//Nihar 5-10-18
	DWORD CheckForZerber_ESCM();
	DWORD CheckForZerber_ESYD();
	DWORD CheckForZerber_ETBY();
	DWORD CheckForZerber_EVHU();
	DWORD CheckForZerber_EVXC();
	DWORD CheckForZerber_EYDG();
	DWORD CheckForZerber_JXI();
	DWORD CheckForMyxaH_MYY();
	DWORD CheckForMyxaH_NAL();
	DWORD CheckForForeign_SB();
	DWORD CheckForPornoAsset_CYOB();
	DWORD CheckForZerber_EDQZ();
	DWORD CheckForZerber_KSQ();
	DWORD CheckForCerber_AOX();
	DWORD CheckForRansom32_A();
	DWORD CheckForRansom32_B();
	DWORD CheckForMyxaH_OFI();
	DWORD CheckForOnion_UMY();
	DWORD CheckForBlocker_IWMW();
	DWORD CheckForBlocker_JTPO();
	DWORD CheckForBlocker_KAJT();
	DWORD CheckForCrusis_AAG();
	DWORD CheckForCrusis_YF();
	DWORD CheckForFullscreen_VQO();
	DWORD CheckForGen_CG();
	DWORD CheckForGen_CYT();
	DWORD CheckForGen_DPJ();
	DWORD CheckForGimemo_CDQU();
	DWORD CheckForGimemo_CFDC();
	DWORD CheckForWin32_PORNOBLOCKER();
	DWORD CheckForSageCrypt_C();
	DWORD CheckForShade_LPX();
	DWORD CheckForShade_NDH();
	DWORD CheckForShade_NDV();
	DWORD CheckForShade_NDW();
	DWORD CheckForShade_NEN();
	DWORD CheckForWin32_TIMER();
	DWORD CheckForWanna_AK();
	DWORD CheckForWanna_C();
	DWORD CheckForWanna_ZBV();
	DWORD CheckForZerber_EDEM();
	DWORD CheckForZerber_EDWD();
	DWORD CheckForZerber_EEET();
	DWORD CheckForZerber_EEEZ();
	DWORD CheckForZerber_EEFH();
	//DWORD CheckForBlocker_JJGL();
	DWORD CheckForBirele_AIMD();
	DWORD CheckForBitcovar_GX();
	DWORD CheckForBitman_ACIV();
	DWORD CheckForBitman_QLG();
	DWORD CheckForWin32_ROKKU();
	
	//Added by Pradip on 16-10-2018
	DWORD  CheckForForeignOADO();
	DWORD  CheckForForeignNOIB();
	DWORD  CheckForGandCryptAUK();
	DWORD  CheckForGandCryptAVU();
	DWORD  CheckForGandCryptAWD();
	DWORD  CheckForGandCryptAWF();
	DWORD  CheckForGandCryptAXH();
	DWORD  CheckForGandCryptAXW();
	DWORD  CheckForGandCryptAYM();
	DWORD  CheckForGandCryptAYO();
	DWORD  CheckForGandCryptAZB();
	DWORD  CheckForGandCryptAZD();
	DWORD  CheckForCrusisTX();
	DWORD  CheckForForeignNOHZ();
	DWORD  CheckForForeignOADF();
	DWORD  CheckForForeignOADK();
	DWORD  CheckForGandCryptAGD();
	DWORD  CheckForGandCryptAHC();
	DWORD  CheckForGandCryptAID();
	DWORD  CheckForLobzikX();
	DWORD  CheckForZerberDTWC();
	DWORD  CheckForZerberDTWG();
	DWORD  CheckForZerberDTWK();
	DWORD  CheckForZerberDTWN();
	DWORD  CheckForZerberDTWO();
	DWORD  CheckForZerberDTWP();
	DWORD  CheckForZerberDTWZ();
	DWORD  CheckForZerberDTXB();
	DWORD  CheckForZerberDTYV();
	DWORD  CheckForZerberDTGX();
	DWORD  CheckForZerberDTGY();
	DWORD  CheckForZerberDTHE();
	DWORD  CheckForZerberDTHR();
	DWORD  CheckForZerberDTIC();
	DWORD  CheckForZerberDTJH();
	DWORD  CheckForZerberDTJL();
	DWORD  CheckForZerberDTTB();
	DWORD  CheckForZerberDTWA();
	DWORD  CheckForZerberDTWB();
	DWORD  CheckForZerberCGVY();
	DWORD  CheckForZerberCGXB();
	DWORD  CheckForZerberCGXD();
	DWORD  CheckForZerberCGXF();
	DWORD  CheckForZerberDAYO();
	DWORD  CheckForZerberDEDG();
	DWORD  CheckForZerberDEQD();
	DWORD  CheckForZerberDESE();
	DWORD  CheckForZerberDNKZ();
	DWORD  CheckForZerberDNYX();
	DWORD  CheckForBlockerJZES();
	DWORD  CheckForBlockerJZEI();
	DWORD  CheckForBlockerJZFJ();
	DWORD  CheckForBlockerJZFY();
	DWORD  CheckForBlockerJZGD();
	DWORD  CheckForBlockerJZGI();
	DWORD  CheckForGenericCryptorEMM();
	DWORD  CheckForGenericCryptorESW();
	DWORD  CheckForSageCryptDBO();
	DWORD  CheckForSageCryptDBP();
	DWORD  CheckForChapakRZE();
	DWORD  CheckForFuerboosE();
	DWORD  CheckForNeutrinoPOSCFB();
	DWORD  CheckForAgentGYKW();
	DWORD  CheckForCerberCS();
	DWORD  CheckForHermesGF();
	DWORD  CheckForInfinitetearAB();
	DWORD  CheckForSigmaBZ();
	DWORD  CheckForSynackA();
	DWORD  CheckForZerberDSOR();
	DWORD  CheckForZerberDSOS();
	DWORD  CheckForZerberDSOU();
	DWORD  CheckForZerberDSOV();
	DWORD  CheckForZerberDSTC();
	DWORD  CheckForZerberDTFW();
	DWORD  CheckForZerberDTGV();
	DWORD  CheckForZerberDSEC();
	DWORD  CheckForZerberDSOI();
	DWORD  CheckForZerberDSFO();
	DWORD  CheckForZerberDSFZ();
	DWORD  CheckForZerberDSHS();
	DWORD  CheckForBireleCV();
	DWORD  CheckForFileCoderAD();
	DWORD  CheckForGandCrabGDS();
	DWORD  CheckForGimemoDX();
	DWORD  CheckForLockyAX();
	DWORD  CheckForPetYaD();
	DWORD  CheckForRantestA();
	DWORD  CheckForZerberDSDO();
	DWORD  CheckForZerberDSDW();
	DWORD  CheckForZerberDOIO();
	DWORD  CheckForZerberDPQS();
	DWORD  CheckForZerberDRJH();
	DWORD  CheckForZerberDRQC();
	DWORD  CheckForZerberDSCL();
	DWORD  CheckForZerberDSCZ();
	DWORD  CheckForShadeMOF();
	DWORD  CheckForSporaCSD();
	DWORD  CheckForSporaCTP();
	DWORD  CheckForSporaCUK();
	DWORD  CheckForSporaCUT();
	DWORD  CheckForSporaCUX();
	DWORD  CheckForSporaCUY();
	DWORD  CheckForZerberDQBN();

	//Added on 16-10-2018 by Shodhan
	DWORD CheckForTrojan_AGENTTESLA();
	DWORD CheckForAgentTesla_TG();
	DWORD CheckForAgenttesla_V();
	DWORD CheckForAzden_A();
	DWORD CheckForBanker_SV();
	DWORD CheckForTrojan_BSYMEM();
	DWORD CheckForCerbu_X();
	DWORD CheckForCMYU_RC();
	DWORD CheckForCoins_AW();
	DWORD CheckForCoins_DR();
	DWORD CheckForCoins_H();
	DWORD CheckForCoins_HB();
	DWORD CheckForCoins_MR();
	DWORD CheckForCoins_VG();
	DWORD CheckForDarkKomet_KL();
	DWORD CheckForTrojan_DINWOD();
	DWORD CheckForEmotet_E();
	DWORD CheckForEmotet_FGH();
	DWORD CheckForEmotet_Q();
	DWORD CheckForEmotet_R();
	DWORD CheckForEmotet_RFV();
	DWORD CheckForEmotet_TR();
	DWORD CheckForFuerboos_EFC8();
	DWORD CheckForGandCrab_CD();
	DWORD CheckForGandCrab_DCF();
	DWORD CheckForInject_DS();
	DWORD CheckForKronos_HBK();
	DWORD CheckForLimpopo_JH();
	DWORD CheckForLokiBot_DD();
	DWORD CheckForLokiBot_DIJ();
	DWORD CheckForLokiBot_DOD();
	DWORD CheckForLokiBot_DRB();
	DWORD CheckForLokiBot_DXD();
	DWORD CheckForLokiBot_FG();
	DWORD CheckForLokiBot_N();
	DWORD CheckForLokiBot_Y();
	DWORD CheckForLokiBot_YG();
	DWORD CheckForLokiBot_YGR();
	DWORD CheckForLokiBot_YGT();
	DWORD CheckForNanoBot_GV();
	DWORD CheckForNymaim_SD();
	DWORD CheckForPony_F();
	DWORD CheckForAgent_IZA();
	DWORD CheckForAtom_PY();
	DWORD CheckForForeign_NOLY();
	DWORD CheckForForeign_NXL();
	DWORD CheckForForeign_NXOM();
	DWORD CheckForForeign_NXQU();
	DWORD CheckForForeign_NXRS();
	DWORD CheckForForeign_NXUC();
	DWORD CheckForForeign_NXUG();
	DWORD CheckForForeign_NXWN();
	DWORD CheckForForeign_NXYB();
	DWORD CheckForForeign_NXYM();
	DWORD CheckForFury_MU();
	DWORD CheckForGen_DIE();
	DWORD CheckForScarab_Purga();		//Scarab.AC variant
	DWORD CheckForScarab_G();
	DWORD CheckForScarab_R();
	DWORD CheckForScarab_WE();
	DWORD CheckForSmokeLoader_E();
	DWORD CheckForTrojan_URSU();
	DWORD CheckForUrsu_LJ();
	DWORD CheckForForeign_NXUA();
	DWORD CheckForTrojanTrickBot_YU();

	//Nihar 11-10-18
	DWORD CheckForZerber_DBNG();
	DWORD CheckForZerber_DBNY();
	DWORD CheckForZerber_DCFT();
	DWORD CheckForZerber_DCSJ();
	DWORD CheckForZerber_DCSO();
	DWORD CheckForZerber_DDFN();
	DWORD CheckForZerber_DDPU();
	DWORD CheckForZerber_DEDW();
	DWORD CheckForZerber_DEDX();
	DWORD CheckForZerber_DEEZ();
	DWORD CheckForZerber_DEKU();
	DWORD CheckForZerber_DELB();
	DWORD CheckForZerber_DEVB();
	DWORD CheckForZerber_DGJF();
	DWORD CheckForZerber_DGJW();
	DWORD CheckForZerber_DGKQ();
	DWORD CheckForZerber_DGZK();
	DWORD CheckForZerber_DHEM();
	DWORD CheckForZerber_DHET();
	DWORD CheckForZerber_DIDW();
	DWORD CheckForZerber_DIFP();
	DWORD CheckForZerber_DIUP();
	DWORD CheckForZerber_DIWI();
	DWORD CheckForZerber_DIWJ();
	DWORD CheckForZerber_DIWR();
	DWORD CheckForZerber_DIWU();
	DWORD CheckForZerber_DIWZ();
	DWORD CheckForZerber_DIXP();
	DWORD CheckForZerber_DIXV();
	DWORD CheckForZerber_DIZD();
	DWORD CheckForZerber_DIZE();
	DWORD CheckForZerber_DJAC();
	DWORD CheckForZerber_DJBE();
	DWORD CheckForZerber_DJCS();
	DWORD CheckForZerber_DJST();
	DWORD CheckForZerber_DKNS();
	DWORD CheckForZerber_DLBX();
	DWORD CheckForZerber_DLWC();
	DWORD CheckForZerber_DLWK();
	DWORD CheckForZerber_DLYN();
	DWORD CheckForZerber_DLYX();
	DWORD CheckForZerber_DLZT();
	DWORD CheckForZerber_DMAL();
	DWORD CheckForZerber_DMBH();
	DWORD CheckForZerber_DMCF();
	DWORD CheckForZerber_DOTX();
	DWORD CheckForZerber_DPDJ();
	DWORD CheckForZerber_DPLF();
	DWORD CheckForZerber_DPTX();
	DWORD CheckForZerber_DPUB();
	DWORD CheckForZerber_DPUM();
	DWORD CheckForZerber_DPWQ();
	DWORD CheckForZerber_DPWR();
	DWORD CheckForZerber_DPWY();
	DWORD CheckForZerber_DPXH();
	DWORD CheckForZerber_DPXR();
	DWORD CheckForZerber_DPXU();
	DWORD CheckForZerber_DPYK();
	DWORD CheckForZerber_DPYO();
	DWORD CheckForZerber_DPYV();
	DWORD CheckForRansom_HIDDENTEAR();
	DWORD CheckForRansom_LOCKY();
	DWORD CheckForMyxaH_BKY();
	DWORD CheckForMyxaH_LIW();
	DWORD CheckForAutoIt_VNA();
	DWORD CheckForBitman_IUE();
	DWORD CheckForBlocker_HRFX();
	DWORD CheckForForeign_NJJZ();
	DWORD CheckForRakhni_TG();
	DWORD CheckForSageCrypt_BIE();
	DWORD CheckForSageCrypt_CCK();
	DWORD CheckForSageCrypt_CMC();
	DWORD CheckForSageCrypt_DDP();
	DWORD CheckForSageCrypt_DFD();
	DWORD CheckForSageCrypt_DIQ();
	DWORD CheckForSageCrypt_JB();
	DWORD CheckForSpora_AX();
	DWORD CheckForZerber_DPZA();
	DWORD CheckForZerber_DQBH();
	DWORD CheckForZerber_DQMF();
	DWORD CheckForZerber_DQMU();
	DWORD CheckForZerber_DQNV();
	DWORD CheckForZerber_DQNZ();
	DWORD CheckForZerber_DQOC();
	DWORD CheckForZerber_DQYC();
	DWORD CheckForZerber_DQZD();
	DWORD CheckForZerber_DQZX();
	DWORD CheckForZerber_DRBK();
	DWORD CheckForZerber_DRBN();
	DWORD CheckForZerber_DRCC();
	DWORD CheckForZerber_DRCZ();
	DWORD CheckForZerber_DRDW();
	DWORD CheckForZerber_DRFA();
	DWORD CheckForZerber_DRFK();
	DWORD CheckForZerber_DRRI();
	DWORD CheckForZerber_DRUR();
	DWORD CheckForZerber_DRWK();

	//Added by Sagar on 16-10-2018
	DWORD CheckForGeneric_BKD();
	DWORD CheckForAndrom_MVML();
	DWORD CheckForAndrom_MZBW();
	DWORD CheckForAndrom_MZFO();
	DWORD CheckForAndrom_MZIV();
	DWORD CheckForAndrom_MZJR();
	DWORD CheckForAndrom_MZJS();
	DWORD CheckForAndrom_MZKN();
	DWORD CheckForAndrom_NCFS();
	DWORD CheckForAndrom_NDBQ();
	DWORD CheckForAndrom_NDFL();
	DWORD CheckForAndrom_NUNE();
	DWORD CheckForBadJoke_AN();
	DWORD CheckForBadJoke_AO();
	DWORD CheckForBlack_F();
	DWORD CheckForDico_AN();
	DWORD CheckForOccamy_BA();
	DWORD CheckForUpatre_GCNO();
	DWORD CheckForAgent_FPWS();
	DWORD CheckForAgent_PCA();
	DWORD CheckForFasem_D();
	DWORD CheckForOnion_CC();
	DWORD CheckForRansomKD_6013();
	DWORD CheckForRansomKD_D562();
	DWORD CheckForAdware_GEN();
	DWORD CheckForGeneric_ART();
	DWORD CheckForKhalesi_SB();
	DWORD CheckForRansom_GENERIC();
	DWORD CheckForRansomKD_GEN();
	DWORD CheckForShelma_SB();
	DWORD CheckForYakes_SB();
	DWORD CheckForFrauDrop_ALUV();
	DWORD CheckForAndrom_NRRE();
	DWORD CheckForAndrom_NSAB();
	DWORD CheckForAndrom_NSCE();
	DWORD CheckForAndrom_NSCK();
	DWORD CheckForAndrom_NSFG();
	DWORD CheckForGhoul_ACL();
	DWORD CheckForGhoul_OG();
	DWORD CheckForGozNym_AED();
	DWORD CheckForGozNym_AEG();
	DWORD CheckForGozNym_AIP();
	DWORD CheckForGozNym_CXR();
	DWORD CheckForGozNym_CYQ();
	DWORD CheckForGozNym_CYR();
	DWORD CheckForGozNym_CZL();
	DWORD CheckForNeutrinoPOS_BW();
	DWORD CheckForNeutrinoPOS_XU();
	DWORD CheckForNeutrinoPOS_XV();
	DWORD CheckForNeutrinoPOS_XX();
	DWORD CheckForBanker_LQ();
	DWORD CheckForBanker_LY();
	DWORD CheckForBanker_ME();
	DWORD CheckForAtom_OX();
	DWORD CheckForAtom_OZ();
	DWORD CheckForAtom_PA();
	DWORD CheckForAtom_PB();
	DWORD CheckForAtom_PD();
	DWORD CheckForAtom_PE();
	DWORD CheckForShade_NZP();
	DWORD CheckForShade_NZR();
	DWORD CheckForShade_NZS();
	DWORD CheckForShade_NZV();
	DWORD CheckForRegsup_YWF();
	DWORD CheckForRegsup_YWM();
	DWORD CheckForRegsup_YWN();
	DWORD CheckForRegsup_YWO();
	DWORD CheckForRegsup_YWQ();
	DWORD CheckForRegsup_YWR();

	//Added By pradip on date 19-10-2018
	DWORD  CheckForSporaCVW();
	DWORD  CheckForSporaCWB();
	DWORD  CheckForSporaCWN();
	DWORD  CheckForSporaCWV();
	DWORD  CheckForSporaCWW();
	DWORD  CheckForSporaCWY();
	DWORD  CheckForSporaCWZ();
	DWORD  CheckForSporaCXA();
	DWORD  CheckForXpanC();
	DWORD  CheckForDealplyLORT();
	DWORD  CheckForPolyRansomDPZFCR();
	DWORD  CheckForPresenokerA();
	DWORD  CheckForNoonLightB();
	DWORD  CheckForWizremGEN7();
	DWORD  CheckForRansomB48();
	DWORD  CheckForLinkuryCD();
	DWORD  CheckForWajam654();
	DWORD  CheckForCoinMinerFCVCHR();
	DWORD  CheckForMinerUBLD();
	DWORD  CheckForStalkAB();
	DWORD  CheckForEsaprofA();
	DWORD  CheckForAddropA();
	DWORD  CheckForAddropCD();
	DWORD  CheckForAndromENEUSI();
	DWORD  CheckForCoinMinerD();
	DWORD  CheckForGenericKDZFHFGRE();
	DWORD  CheckForKryptikENDYFF();   //dll
	DWORD  CheckForLaziokEVCIGS();
	DWORD  CheckForNetWireFHSFPE();
	DWORD  CheckForScarLPCO();
	DWORD  CheckForGandCrabV3();
	DWORD  CheckForGandCrabV4();
	DWORD  CheckForNoblisA();
	DWORD  CheckForAndromABNK();
	DWORD  CheckForAndromDXDK();
	DWORD  CheckForSatan6();
	DWORD  CheckForFareit403();
	DWORD  CheckForFareitBC();
	DWORD  CheckForBnfAEKU();
	DWORD  CheckForEmotet2IAMS0();
	DWORD  CheckForStealerAUK();
	DWORD  CheckForKryptikFQRO();
	DWORD  CheckForRemcosE();
	DWORD  CheckForUrsnifMLJEJ();

	//Nihar 19-10-18
	DWORD CheckForZerber_DRWV();
	DWORD CheckForZerber_DRXX();
	DWORD CheckForZerber_DSBC();
	DWORD CheckForZerber_DSJW();
	DWORD CheckForZerber_DSJZ();
	DWORD CheckForZerber_DSKY();
	DWORD CheckForZerber_DSLP();
	DWORD CheckForZerber_DSLY();
	DWORD CheckForZerber_DSNH();
	DWORD CheckForZerber_DSVF();
	DWORD CheckForZerber_DSXC();
	DWORD CheckForZerber_DSXI();
	DWORD CheckForZerber_DSXN();
	DWORD CheckForZerber_DSYC();
	DWORD CheckForZerber_DSZK();
	DWORD CheckForZerber_DSZL();
	DWORD CheckForZerber_DSZT();
	DWORD CheckForZerber_DTAG();
	DWORD CheckForZerber_DTBD();
	DWORD CheckForZerber_DTBE();
	DWORD CheckForZerber_DTIF();
	DWORD CheckForZerber_DTMA();
	DWORD CheckForZerber_DTMD();
	DWORD CheckForZerber_DTMM();
	DWORD CheckForZerber_DTNH();
	DWORD CheckForZerber_DTNR();
	DWORD CheckForZerber_DTNX();
	DWORD CheckForZerber_DTPH();
	DWORD CheckForZerber_DTPR();
	DWORD CheckForZerber_DTQC();
	DWORD CheckForZerber_DTQI();
	DWORD CheckForZerber_DTQV();
	DWORD CheckForZerber_DTRM();
	DWORD CheckForZerber_DTSA();
	DWORD CheckForZerber_DTSC();
	DWORD CheckForZerber_DTSO();
	DWORD CheckForZerber_DTSQ();
	DWORD CheckForZerber_DTSR();
	DWORD CheckForZerber_DTTF();
	DWORD CheckForZerber_DTTQ();
	DWORD CheckForZerber_DTTT();
	DWORD CheckForZerber_DTTV();
	DWORD CheckForZerber_DUCC();
	DWORD CheckForZerber_DUDK();
	DWORD CheckForZerber_DUFL();
	DWORD CheckForZerber_DUJP();
	DWORD CheckForZerber_DULX();
	DWORD CheckForZerber_DUME();
	DWORD CheckForZerber_DUNT();
	DWORD CheckForZerber_DUOJ();
	DWORD CheckForZerber_DUPE();
	DWORD CheckForZerber_DUYZ();
	DWORD CheckForZerber_DVIE();

	//Sagar 22-10-18
	DWORD CheckForSector_30();
	DWORD CheckForHotbar_19();
	DWORD CheckForHotbar_999();
	DWORD CheckForScreenSaver_E();
	DWORD CheckForScreenSaver_I();
	DWORD CheckForQuasar_AB();
	DWORD CheckForArtemis_AS();
	DWORD CheckForGeneric_2272();
	DWORD CheckForGenericKD_AS();
	DWORD CheckForAgent_FOQX();
	DWORD CheckForQuasar_EBF();
	DWORD CheckForQuasar_EDS();
	DWORD CheckForQuasar_EQW();
	DWORD CheckForQuasar_EQX();
	DWORD CheckForQuasar_ERD();
	DWORD CheckForQuasar_ERG();
	DWORD CheckForQuasar_ESJ();
	DWORD CheckForQuasar_EST();
	DWORD CheckForSymmi_AB();
	DWORD CheckForSymmi_AC();
	DWORD CheckForZusy_90451();
	DWORD CheckForHotbhar_AB();
	DWORD CheckForScar_AG();

	//Nihar 25-10-18
	DWORD CheckForGancrab_V503();
	//Nihar 30-10-18
	DWORD CheckForZerber_DVIU();
	DWORD CheckForZerber_DVJD();
	DWORD CheckForZerber_DVJX();
	DWORD CheckForZerber_DVKI();
	DWORD CheckForZerber_DVSA();
	DWORD CheckForZerber_DVSV();
	DWORD CheckForZerber_DVTC();
	DWORD CheckForZerber_DVTJ();
	DWORD CheckForZerber_DVUF();
	DWORD CheckForZerber_DVUZ();
	DWORD CheckForZerber_DVVF();
	DWORD CheckForZerber_DVWC();
	DWORD CheckForZerber_DVZO();
	DWORD CheckForZerber_DWJI();
	DWORD CheckForZerber_DWKH();
	DWORD CheckForZerber_DWPB();
	DWORD CheckForZerber_DWTQ();
	DWORD CheckForZerber_DWWR();
	DWORD CheckForZerber_DWWY();
	DWORD CheckForZerber_DXFF();
	DWORD CheckForZerber_DXGA();
	DWORD CheckForZerber_DXGF();
	DWORD CheckForZerber_DXGH();
	DWORD CheckForZerber_DXGO();
	DWORD CheckForZerber_DXGQ();
	DWORD CheckForZerber_DXHI();
	DWORD CheckForZerber_DXHJ();
	DWORD CheckForZerber_DXHR();
	DWORD CheckForZerber_DXHS();
	DWORD CheckForZerber_DXHT();
	DWORD CheckForZerber_DXPP();
	DWORD CheckForZerber_DXPR();
	DWORD CheckForZerber_DXQP();
	DWORD CheckForZerber_DXVF();
	DWORD CheckForZerber_DXZR();
	DWORD CheckForZerber_DYAZ();
	DWORD CheckForZerber_DYBT();
	DWORD CheckForZerber_DYBU();
	DWORD CheckForZerber_DYCE();
	DWORD CheckForZerber_DYIL();
	DWORD CheckForZerber_DYVR();
	DWORD CheckForZerber_DYVT();
	DWORD CheckForZerber_DYVU();
	DWORD CheckForZerber_DYVV();
	DWORD CheckForZerber_CGQP();
	DWORD CheckForZerber_CPRB();
	DWORD CheckForZerber_DAZY();
	DWORD CheckForZerber_DBTS();
	DWORD CheckForZerber_DCZA();
	DWORD CheckForZerber_DCZR();
	DWORD CheckForZerber_DCZS();
	DWORD CheckForZerber_DCZV();
	DWORD CheckForZerber_DDAD();
	DWORD CheckForZerber_DDII();
	DWORD CheckForZerber_DFHT();
	DWORD CheckForZerber_DGJI();
	DWORD CheckForZerber_DGNN();
	DWORD CheckForZerber_DGQC();

	DWORD CheckForSaveFiles_A();
	DWORD CheckForSaveFiles_B();
	DWORD CheckForSaveFiles_C();

	DWORD CheckForSpora_BZY();

	//Shodhan 30-10-2018
	DWORD CheckForBlackShades_WLK();
	DWORD CheckForChapak_LP();
	DWORD CheckForDropper_UIO();
	DWORD CheckForEmotet_ATN();
	DWORD CheckForFareit_BUE();
	DWORD CheckForFuerboos_HYB();
	DWORD CheckForGandcrab_CQA();
	DWORD CheckForLokiBot_DEY();
	DWORD CheckForLokiBot_HYT();
	DWORD CheckForMalware_ZEL();
	DWORD CheckForNanoCore_FET();
	DWORD CheckForUrsnif_IPS();
	DWORD CheckForUrsu_U();
	DWORD CheckForTrojan_AGENTTESLABASED();
	DWORD CheckForGandCrab_DKJ();
	DWORD CheckForGarrun_CE();
	DWORD CheckForGarrun_YUC();
	DWORD CheckForGarrun_YUD();
	DWORD CheckForImminent_KAW();
	DWORD CheckForInjector_KUYR();
	DWORD CheckForKasidet_TBJ();
	DWORD CheckForLethic_IAZ();
	DWORD CheckForMalware_UTX();
	DWORD CheckForMalware_UTY();
	DWORD CheckForRedlonam_A();
	DWORD CheckForRuftar_XEA();
	DWORD CheckForUrsnif_EQV();
	DWORD CheckForUrsnif_GBA();
	DWORD CheckForVenik_S();
	DWORD CheckForPlimrost_B();
	DWORD CheckForPony_TWE();
	DWORD CheckForBlackShades_JORIK();
	DWORD CheckForTrojan_BLADABINDI();
	DWORD CheckForBladabindi_A();
	DWORD CheckForGandCrab_IO();
	DWORD CheckForGen_WOQC();
	DWORD CheckForLokiBot_KI();
	DWORD CheckForLokiBot_KU();
	DWORD CheckForBlocker_ASEK();
	DWORD CheckForBlocker_KIUO();
	DWORD CheckForBlocker_LAS();
	DWORD CheckForBlocker_LKIE();
	DWORD CheckForBlocker_MERC();
	DWORD CheckForBlocker_OLED();
	DWORD CheckForGandCrab_JDB();
	DWORD CheckForGandCrab_RDG();
	DWORD CheckForLoki_VOP();
	DWORD CheckForSatan_BVM();
	DWORD CheckForShrug_PAZ();
	DWORD CheckForChapak_AZRO();
	DWORD CheckForFareit_FAM();
	DWORD CheckForGandCrab_VURW();
	DWORD CheckForLokiBot_UCTQ();
	DWORD CheckForNanoCore_LOPW();
	DWORD CheckForOlympicDestroyer_CTU();
	DWORD CheckForPony_TOY();
	DWORD CheckForGenasom_RCT();
	DWORD CheckForRecmos_LJE();
	DWORD CheckForUnwaders_CQE();

	//Sagar 30-10-2018

	DWORD CheckForHotbar_AB();
	DWORD CheckForHotbar_AC();
	DWORD CheckForAndrom_NQVZ();
	DWORD CheckForAndrom_NQXR();
	DWORD CheckForAndrom_NQZE();
	DWORD CheckForAndrom_NQZF();
	DWORD CheckForAndrom_NRAG();
	DWORD CheckForAndrom_NRLK();
	DWORD CheckForArtemis_ABS();
	DWORD CheckForGhoul_ACQ();
	DWORD CheckForGhoul_OAG();
	DWORD CheckForGozNym_ADQ();
	DWORD CheckForGozNym_AJQ();
	DWORD CheckForGozNym_AKK();
	DWORD CheckForGozNym_CZQ();
	DWORD CheckForGozNym_GZT();
	DWORD CheckForGozNym_GZU1();
	DWORD CheckForNeutrinoPOS_AAQ();
	DWORD CheckForNeutrinoPOS_ABS();
	DWORD CheckForRTM_1WQ();
	DWORD CheckForTrickster_KA();
	DWORD CheckForGeral_BQW();
	DWORD CheckForBanker_LE();
	DWORD CheckForAtom_PN();
	DWORD CheckForAtom_PP();
	DWORD CheckForAtom_PR();
	DWORD CheckForRansomKD_ADQ();
	DWORD CheckForRansomKD_D565();
	DWORD CheckForZusy_A284();
	DWORD CheckForZusy_B90451();
	DWORD CheckForFuerboos_ACQ();
	DWORD CheckForHawkEye_ACS();
	DWORD CheckForNocturnal_AA();
	DWORD CheckForRansomKD_A564();
	DWORD CheckForRazy_ABQ();
	DWORD CheckForRyzerlo_AB();
	DWORD CheckForSpora_FGQ();
	DWORD CheckForStealer_ABA();
	DWORD CheckForTiggre_AV();
	DWORD CheckForMidie_ABS();
	DWORD CheckForVigorf_AQ();
	DWORD CheckForYakes_AAYC();
	DWORD CheckForRansom_B44();
	DWORD CheckForCrysis_AS();
	DWORD CheckForBitman_QME();
	DWORD CheckForGupboot_GC();
	DWORD CheckForRansom_EVZTOC();
	DWORD CheckForSymmi_ABC();

	//Added By pradip on 26-10-2018
	DWORD  CheckForCsdiMonetizeA();
	DWORD  CheckForAndromGEN();
	DWORD  CheckForRemcosGENERIC();
	DWORD  CheckForDarkCometB();
	DWORD  CheckForFareitFEMYKA();
	DWORD  CheckForLebagA();
	DWORD  CheckForQuervarB();
	DWORD  CheckForStealerFEMGNP();
	DWORD  CheckForVBDXUTYX();
	DWORD  CheckForGandCryptSI();
	DWORD  CheckForGandCryptTG();
	DWORD  CheckForGandCryptTX();
	DWORD  CheckForGandCryptUE();
	DWORD  CheckForGandCryptUH();
	DWORD  CheckForGandCryptUJ();
	DWORD  CheckForGandCryptVJ();
	DWORD  CheckForGandCryptWQ();
	DWORD  CheckForGandCryptYL();
	DWORD  CheckForGandCryptZK();
	DWORD  CheckForBlockerJTXQ();
	DWORD  CheckForBlockerJVTP();
	DWORD  CheckForBlockerJYQS();
	DWORD  CheckForBlockerJZEZ();
	DWORD  CheckForBlockerKSGD();
	DWORD  CheckForBlockerKWOM();
	DWORD  CheckForBlockerKYGG();
	DWORD  CheckForBlockerKYQU();
	DWORD  CheckForBlockerLADZ();
	DWORD  CheckForBlockerLAHV();
	DWORD  CheckForBlockerLAHX();
	DWORD  CheckForBlockerLAHZ();
	DWORD  CheckForBlockerLAIA();
	DWORD  CheckForBlockerLAJB();
	DWORD  CheckForBlockerLAJK();
	DWORD  CheckForGandCryptRN();
	DWORD  CheckForGandCryptRS();
	DWORD  CheckForGandCryptRX();
	DWORD  CheckForBlockerAZAD();
	DWORD  CheckForGandCryptSG();
	DWORD  CheckForGandCryptUV();
	DWORD  CheckForGandCryptUY();
	DWORD  CheckForBlockerJLPZZ();
	DWORD  CheckForGandCryptALI();
	DWORD  CheckForGandCryptALQ();
	DWORD  CheckForGandCryptAMF();
	DWORD  CheckForGandCryptAMK();
	DWORD  CheckForGandCryptAMM();
	DWORD  CheckForGandCryptAOI();
	DWORD  CheckForGandCryptAOK();
	DWORD  CheckForGandCryptAON();

	//Pradip Added date on 02-11-2018 
	DWORD  CheckForGandCryptAAB();
	DWORD  CheckForGandCryptAAX();
	DWORD  CheckForGandCryptABT();
	DWORD  CheckForGandCryptACD();
	DWORD  CheckForGandCryptACF();
	DWORD  CheckForGandCryptACG();
	DWORD  CheckForGandCryptACH();
	DWORD  CheckForGandCryptACK();
	DWORD  CheckForGandCryptACO();
	DWORD  CheckForGandCryptAFF();
	DWORD  CheckForGandCryptAFS();
	DWORD  CheckForGandCryptAFX();
	DWORD  CheckForGandCryptAFZ();
	DWORD  CheckForGandCryptAGB();
	DWORD  CheckForGandCryptAGF();
	DWORD  CheckForGandCryptAGL();
	DWORD  CheckForGandCryptAGT();
	DWORD  CheckForGandCryptAHA();
	DWORD  CheckForGandCryptAHF();
	DWORD  CheckForGandCryptAHO();
	DWORD  CheckForGandCryptAHT();
	DWORD  CheckForGandCryptAHV();
	DWORD  CheckForGandCryptAIA();
	DWORD  CheckForGandCryptAIE();
	DWORD  CheckForGandCryptAIM();
	DWORD  CheckForGandCryptAINA();
	DWORD  CheckForGandCryptAIO();
	DWORD  CheckForGandCryptAIPA();
	DWORD  CheckForGandCryptAISA();
	DWORD  CheckForGandCryptAIX();
	DWORD  CheckForGandCryptAJB();
	DWORD  CheckForGandCryptAJC();
	DWORD  CheckForGandCryptAJF();
	DWORD  CheckForGandCryptAJG();
	DWORD  CheckForGandCryptAJP();
	DWORD  CheckForGandCryptAJY();
	DWORD  CheckForGandCryptAKC();
	DWORD  CheckForGandCryptAKHA();
	DWORD  CheckForGandCryptAKKA();
	DWORD  CheckForGandCryptAKM();
	DWORD  CheckForGandCryptAKNA();
	DWORD  CheckForGandCryptANX();
	DWORD  CheckForGandCryptAOX();
	DWORD  CheckForGandCryptAOZ();
	DWORD  CheckForGandCryptNK();
	DWORD  CheckForGandCryptOO();
	DWORD  CheckForGandCryptOS();
	DWORD  CheckForGandCryptOT();
	DWORD  CheckForGandCryptQV();
	DWORD  CheckForGandCryptRB();

	//Shodhan added on 02-11-2018
	DWORD CheckForLokiBot_JUE();
	DWORD CheckForNanoCore_VUT();
	DWORD CheckForTroldesh_BCDO();
	DWORD CheckForTrojanPony_DFBU();
	DWORD CheckForAgenttesla_TCE();
	DWORD CheckForAgenttesla_YEG();
	DWORD CheckForChapak_BAOM();
	DWORD CheckForCoinMiner_TUYV();
	DWORD CheckForHawkEye_WYH();
	DWORD CheckForLockScreen_JET();
	DWORD CheckForLokiBot_BBVY();
	DWORD CheckForLokiBot_MSIL();
	DWORD CheckForNjRat_RXQ();
	DWORD CheckForRemcos_GWQ();
	DWORD CheckForUrsnif_YUX();
	DWORD CheckForYakes_UCQ();
	DWORD CheckForZeusPanda_ARFN();
	DWORD CheckForAgentteslaBased_IHT();
	DWORD CheckForHawkEye_OPU();
	DWORD CheckForLoki_FJZ();
	DWORD CheckForLoki_FUX();
	DWORD CheckForLoki_HTC();
	DWORD CheckForLoki_KEU();
	DWORD CheckForLokiBased_LQX();
	DWORD CheckForLokiBot_KEF();
	DWORD CheckForNanoCore_UOX();
	DWORD CheckForCrypmod_OGH();
	DWORD CheckForRemcos_HGI();
	DWORD CheckForSmokeLoader_VUN();
	DWORD CheckForTrickBotBased_JUF();
	DWORD CheckForTrickBotBased_JUG();
	DWORD CheckForUrsnif_YHV();
	DWORD CheckForXtremeRat_HUN();
	DWORD CheckForAgenttesla_OPQ();
	DWORD CheckForHiddenTear_GWC();
	DWORD CheckForHiddenTear_YUE();
	DWORD CheckForLokiBot_JKQ();
	DWORD CheckForLokiBot_LPEC();
	DWORD CheckForNanoCore_OPAS();
	DWORD CheckForPony_DEQT();
	DWORD CheckForRansom_HCZ();
	DWORD CheckForPornoBlocker_ELIJ();
	DWORD CheckForRansom_RANSOMKD();
	DWORD CheckForGen_AJ();
	DWORD CheckForGen_IQJ();
	DWORD CheckForZerber_EIXT();
	DWORD CheckForZerber_EJVM();
	DWORD CheckForRecmos_LWVT();
	DWORD CheckForTrickBot_IUT();
	DWORD CheckForXtremeRat_DTB();
	DWORD CheckForZenpak_AQW();

	//Nihar added on 02-11-2018
	DWORD CheckForShade_LRL();
	DWORD CheckForShade_LRT();
	DWORD CheckForShade_LSC();
	DWORD CheckForShade_LSG();
	DWORD CheckForShade_LSI();
	DWORD CheckForShade_LSJ();
	DWORD CheckForShade_LSL();
	DWORD CheckForShade_LSM();
	DWORD CheckForShade_LSO();
	DWORD CheckForShade_LSU();
	DWORD CheckForShade_LTC();

	//Sagar added on 02-11-2018
	DWORD CheckForNanoBot_AFCH();
	DWORD CheckForNjrat_ZG();
	DWORD CheckForEnfal_KP();
	DWORD CheckForFarfli_ABTW();
	DWORD CheckForFarfli_ADHI();
	DWORD CheckForFarfli_AVNE();
	DWORD CheckForFarfli_BCKV();
	DWORD CheckForHupigon_VBEI();
	DWORD CheckForInfecDoor_14();
	DWORD CheckForMokes_XII();
	DWORD CheckForMokes_XIX();
	DWORD CheckForMokes_XJC();
	DWORD CheckForMokes_XJE();
	DWORD CheckForMokes_XKT();
	DWORD CheckForMoonPie_13();
	DWORD CheckForNewRest_Z();
	DWORD CheckForNhopro_AOU();
	DWORD CheckForNova_B();
	DWORD CheckForPcClient_ABC();
	DWORD CheckForWin32_PIPES();
	DWORD CheckForQBot_BAT();
	DWORD CheckForRUX30_E();
	DWORD CheckForTurkojan_ZWA();
	DWORD CheckForZegost_MTIQA();
	DWORD CheckForDico_AS();
	DWORD CheckForEmotet_ASQX();
	DWORD CheckForGozNym_CZR();
	DWORD CheckForNeutrinoPOS_YH();
	DWORD CheckForUpatre_GXDS();
	DWORD CheckForBanker_LN();
	DWORD CheckForCoins_BKW();
	DWORD CheckForCoins_BKY();
	DWORD CheckForCoins_CBO();
	DWORD CheckForNoon_NGO();
	DWORD CheckForAntavmu_ARPK();
	DWORD CheckForAPosT_DSI();
	DWORD CheckForBicololo_BIGN();
	DWORD CheckForBicololo_BIHF();
	DWORD CheckForHesv_DGUU();
	DWORD CheckForHesv_DGXA();
	DWORD CheckForInject_AJWQR();
	DWORD CheckForInject_AJWRK();
	DWORD CheckForInject_EYEW();
	DWORD CheckForKhalesi_FHU();
	DWORD CheckForMiner_GEN();
	DWORD CheckForPyme_D();
	DWORD CheckForSchoolBoy_XI();
	DWORD CheckForSiscos_XTX();
	DWORD CheckForSiscos_XUH();
	DWORD CheckForSnojan_BNJU();

	//Pradip Added on 06-11-2018
	DWORD  CheckForGandCryptAVN();
	DWORD  CheckForGandCryptAVS();
	DWORD  CheckForGandCryptAVZ();
	DWORD  CheckForGandCryptAWI();
	DWORD  CheckForGandCryptAWQ();
	DWORD  CheckForGandCryptAWZ();
	DWORD  CheckForGandCryptAYA();
	DWORD  CheckForGandCryptAYC();
	DWORD  CheckForGandCryptAYD();
	DWORD  CheckForGandCryptAYF();
	DWORD  CheckForGandCryptAZA();
	DWORD  CheckForGandCryptAZR();
	DWORD  CheckForFileCoderAF();
	DWORD  CheckForGenIWF();
	DWORD  CheckForGandCryptAWJ();
	DWORD  CheckForGandCryptAWN();
	DWORD  CheckForShadeOQU();
	DWORD  CheckForHermesAD();
	DWORD  CheckForGandCryptAS();
	DWORD  CheckForSigmaAB();

	//Shodhan - 06-11-2018
	DWORD CheckForAgenttesla_FBKK();
	DWORD CheckForAgenttesla_JISK();
	DWORD CheckForAgenttesla_MGYH();
	DWORD CheckForGoodKit_VHVV();
	DWORD CheckForLokiBot_KOH();
	DWORD CheckForLokiBot_KOL();
	DWORD CheckForNanoCore_JUH();
	DWORD CheckForNjRat_MUS();
	DWORD CheckForPony_DFG();
	DWORD CheckForRecmos_WFU();
	DWORD CheckForRemcos_KJUO();
	DWORD CheckForTrickBot_HYJ();
	DWORD CheckForUrsnif_LOP();

	//Nihar - 06-11-2018
	DWORD CheckForSnocry_CWY();
	DWORD CheckForSnocry_CXC();
	DWORD CheckForSnocry_CXG();
	DWORD CheckForTakbum_H();
	DWORD CheckForTakbum_I();

	//Sagar 12-11-2018
	DWORD CheckForNanoBot_AFCB();
	DWORD CheckForNanoBot_AFCJ();
	DWORD CheckForSpyGate_ACPI();
	DWORD CheckForEnfal_KO();
	DWORD CheckForFarfli_BCGF();
	DWORD CheckForHlux_GCLP();
	DWORD CheckForLiondoor_03();
	DWORD CheckForMokes_XIL();
	DWORD CheckForMokes_XIW();
	DWORD CheckForMokes_XIZ();
	DWORD CheckForMokes_XKU();
	DWORD CheckForMokes_XKV();
	DWORD CheckForMokes_XKW();
	DWORD CheckForMokes_XLQ();
	DWORD CheckForHesv_DGXC();
	DWORD CheckForInject_NQZX();

	//Nihar 13-11-2018
	DWORD CheckForCrusis_QH();
	DWORD CheckForCrusis_SG();
	DWORD CheckForCrusis_UB();
	DWORD CheckForForeign_NLON();
	DWORD CheckForForeign_NLPN();
	DWORD CheckForForeign_NLPO();
	DWORD CheckForForeign_NLPP();
	DWORD CheckForForeign_NLQS();
	DWORD CheckForForeign_NLSF();
	DWORD CheckForForeign_NLSI();
	DWORD CheckForForeign_NLSR();
	DWORD CheckForForeign_NLSS();
	DWORD CheckForForeign_NLSV();
	DWORD CheckForForeign_NLSW();
	DWORD CheckForForeign_NLTJ();
	DWORD CheckForMyxaH_HSU();
	DWORD CheckForMyxaH_NYM();
	DWORD CheckForMyxaH_OAC();
	DWORD CheckForMyxaH_OAJ();
	DWORD CheckForMyxaH_OAO();
	DWORD CheckForMyxaH_OAV();
	DWORD CheckForSageCrypt_BXI();
	DWORD CheckForSageCrypt_BYN();
	DWORD CheckForSageCrypt_BYO();
	DWORD CheckForSageCrypt_BYP();
	DWORD CheckForSageCrypt_BYR();
	DWORD CheckForSageCrypt_BYS();
	DWORD CheckForSageCrypt_BYT();
	DWORD CheckForSageCrypt_BYV();
	DWORD CheckForSageCrypt_BZT();
	DWORD CheckForSageCrypt_CAD();
	DWORD CheckForSageCrypt_CAI();
	DWORD CheckForSageCrypt_CEH();
	DWORD CheckForSageCrypt_CEL();
	DWORD CheckForSageCrypt_CEM();
	DWORD CheckForSageCrypt_CEQ();
	DWORD CheckForSageCrypt_CEC();
	DWORD CheckForSageCrypt_CER();
	DWORD CheckForSageCrypt_CET();
	DWORD CheckForSageCrypt_CEU();
	DWORD CheckForSageCrypt_CEV();
	DWORD CheckForSageCrypt_CEW();
	DWORD CheckForSageCrypt_CEZ();
	DWORD CheckForSageCrypt_CFA();
	DWORD CheckForSageCrypt_CFB();
	DWORD CheckForSageCrypt_CFC();
	DWORD CheckForSageCrypt_CFF();
	DWORD CheckForSageCrypt_CFI();
	DWORD CheckForSageCrypt_CFJ();
	DWORD CheckForSageCrypt_CFK();
	DWORD CheckForSpora_CBO();
	DWORD CheckForSpora_CJF();
	DWORD CheckForSpora_CJO();
	DWORD CheckForSpora_CJP();
	DWORD CheckForSpora_CKI();
	DWORD CheckForSageCrypt_CFL();
	DWORD CheckForSageCrypt_CFM();
	DWORD CheckForSageCrypt_CFN();
	DWORD CheckForSageCrypt_CFO();
	DWORD CheckForSageCrypt_CFP();
	DWORD CheckForSageCrypt_CFR();
	DWORD CheckForSageCrypt_CIR();
	DWORD CheckForSageCrypt_CJE();
	DWORD CheckForSageCrypt_CJF();
	DWORD CheckForSageCrypt_CJG();
	DWORD CheckForSageCrypt_CKE();
	DWORD CheckForSageCrypt_CKH();
	DWORD CheckForSageCrypt_CKI();
	DWORD CheckForSageCrypt_CKJ();
	DWORD CheckForSageCrypt_CKK();
	DWORD CheckForSageCrypt_CKM();
	DWORD CheckForSageCrypt_JN();
	DWORD CheckForSageCrypt_JW();
	DWORD CheckForSageCrypt_OQ();
	
	//Shodhan 15-11-2018
	DWORD CheckForBlocker_DUYW();
	DWORD CheckForBlocker_FYRE();
	DWORD CheckForBlocker_GNUR();
	DWORD CheckForBlocker_GTCE();
	DWORD CheckForBlocker_GUHW();
	DWORD CheckForBlocker_HUQS();
	DWORD CheckForBlocker_HUTE();
	DWORD CheckForBlocker_HYRT();
	DWORD CheckForBlocker_JUWT();
	DWORD CheckForBlocker_KILQ();
	DWORD CheckForBlocker_KOWE();
	DWORD CheckForBlocker_KUTC();
	DWORD CheckForADWAREDEALPLY();
	DWORD CheckForADWARESLIMWARE();
	DWORD CheckForAgentteslaBased_LPAS();
	DWORD CheckForArtemis_JUL();
	DWORD CheckForChapak_BFQE();
	DWORD CheckForChapak_BFTH();
	DWORD CheckForEmotet_KELF();
	DWORD CheckForGandCrab_JUYB();
	DWORD CheckForPhorpiex_NURG();
	DWORD CheckForPropagate_VYWD();
	DWORD CheckForSymmi_KJ();
	DWORD CheckForZpevdo_KOP();
	DWORD CheckForAdware_AFG();
	DWORD CheckForAdware_AFP();
	DWORD CheckForAdware_AGB();
	DWORD CheckForAdware_AGD();
	DWORD CheckForAdwareCryakl_AAZ();
	DWORD CheckForEmotet_SDVB();
	DWORD CheckForFareit_GVNW();
	DWORD CheckForLokiBot_KGH();
	DWORD CheckForLokiBot_KWA();
	DWORD CheckForRemcos_GHQ();
	DWORD CheckForRemcos_GJB();
	DWORD CheckForRemcos_GJC();
	DWORD CheckForUrsnif_HUJI();
	DWORD CheckForUrsnif_HUMK();
	DWORD CheckForDunihi_DHUB();
	DWORD CheckForEmotet_KLXF();
	DWORD CheckForEmotet_KLZC();
	DWORD CheckForJENXCUS_JUKE();
	DWORD CheckForJENXCUS_UKDR();
	DWORD CheckForLokiBot_KDVB();
	DWORD CheckForLokiBot_NUME();
	DWORD CheckForLokiBot_NYJK();
	DWORD CheckForMiner_IODT();
	DWORD CheckForTrickBot_UYRH();
	DWORD CheckForTroldesh_KOPQ();
	DWORD CheckForBlocker_JRSN();

	//Nihar 15-11-2018
	DWORD CheckForShade_KUW();
	DWORD CheckForShade_KUX();
	DWORD CheckForShade_KVE();
	DWORD CheckForShade_KVF();
	DWORD CheckForShade_KVG();
	DWORD CheckForShade_KXD();
	DWORD CheckForShade_LJV();
	DWORD CheckForShade_LOI();
	DWORD CheckForShade_LOJ();
	DWORD CheckForShade_LOK();
	DWORD CheckForShade_MSP();
	DWORD CheckForShade_MTD();
	DWORD CheckForShade_MTE();
	DWORD CheckForShade_MTG();
	DWORD CheckForShade_MTK();
	DWORD CheckForShade_MTP();
	DWORD CheckForShade_MTX();
	DWORD CheckForShade_MUA();
	DWORD CheckForShade_MUG();
	DWORD CheckForShade_MUJ();
	DWORD CheckForShade_MUK();
	DWORD CheckForShade_MUL();
	DWORD CheckForShade_MUN();
	DWORD CheckForShade_MUP();
	DWORD CheckForShade_MUR();
	DWORD CheckForShade_MUS();
	DWORD CheckForShade_MUT();
	DWORD CheckForShade_MUU();
	DWORD CheckForShade_MUV();
	DWORD CheckForShade_MUW();
	DWORD CheckForShade_MUX();
	DWORD CheckForShade_MUY();
	DWORD CheckForShade_MVC();
	DWORD CheckForShade_MVE();

	//Added by pradip on date 16-11-2018
	DWORD  CheckForGandCryptA();
	DWORD  CheckForGandCryptAFYA();
	DWORD  CheckForGandCryptBTT();
	DWORD  CheckForGandCryptBTV();
	DWORD  CheckForGandCryptYB();
	DWORD  CheckForGandCryptYC();
	DWORD  CheckForGandCryptYQ();
	DWORD  CheckForGandCryptYY();
	DWORD  CheckForGandCryptZA();
	DWORD  CheckForGandCryptZC();
	DWORD  CheckForGandCryptZT();
	DWORD  CheckForGandCryptZW();
	DWORD  CheckForGandCryptZY();
	DWORD  CheckForGandCryptDIT();
	DWORD  CheckForGandCryptFKD();
	DWORD  CheckForGandCryptSB();
	DWORD  CheckForGandCryptWF();
	DWORD  CheckForGandCryptWI();
	DWORD  CheckForGandCryptWL();
	DWORD  CheckForGandCryptWT();
	DWORD  CheckForGandCryptWW();
	DWORD  CheckForGandCryptWY();
	DWORD  CheckForGandCryptWZ();
	DWORD  CheckForGandCryptXA();
	DWORD  CheckForGandCryptXT();
	DWORD  CheckForGANDCRABSMALY3();
	DWORD  CheckForChapakRKV();
	DWORD  CheckForChapakYDG();
	DWORD  CheckForChapakYEL();
	DWORD  CheckForChapakEYXOBA();
	DWORD  CheckForChapakRZG();
	DWORD  CheckForGandCryptGENV2();
	DWORD  CheckForGandCrabGEN2();
	DWORD  CheckForGandCryptAZC();
	DWORD  CheckForGandCryptBDA();
	DWORD  CheckForGandCryptBET();
	DWORD  CheckForGandCryptBFA();//dll
	DWORD  CheckForGandCryptBOL();
	DWORD  CheckForGandCryptCAB();
	DWORD  CheckForKryptikFAQYOJ();
	DWORD  CheckForGandCryptABW();
	DWORD  CheckForGandCryptBKB();
	DWORD  CheckForGandCryptBKQ();
	DWORD  CheckForGandcrabAKOS();
	DWORD  CheckForGandCryptBXF();
	DWORD  CheckForGandCryptBZS();
	DWORD  CheckForGandCryptCBO();
	DWORD  CheckForGandcryptHTBT();
	DWORD  CheckForGandcryptPDMH();

	//Sagar - 16-11-2018
	DWORD CheckForEnfal_KN();
	DWORD CheckForFarfli_BCBH();
	DWORD CheckForFarfli_BCJY();
	DWORD CheckForFarfli_BCKR();
	DWORD CheckForGunbot_F();
	DWORD CheckForMokes_XIC();
	DWORD CheckForMokes_XIG();
	DWORD CheckForMokes_XIV();
	DWORD CheckForMokes_XMY();
	DWORD CheckForMokes_XND();
	DWORD CheckForMoSucker_N();
	DWORD CheckForPhpw_ARK();
	DWORD CheckForSinowal_XBK();
	DWORD CheckForZegost_MTIQB();
	DWORD CheckForEmotet_ASVP();
	DWORD CheckForGozNym_AKH();
	DWORD CheckForBetload_CNB();
	DWORD CheckForBitmin_WCG();
	DWORD CheckForCoins_CDP();
	DWORD CheckForCoins_KG();
	DWORD CheckForAgent_GJW();
	DWORD CheckForBitman_YNE();
	DWORD CheckForBlocker_DKKD();
	DWORD CheckForBlocker_HRTF();
	DWORD CheckForBlocker_JXWY();
	DWORD CheckForCryrar_GRH();
	DWORD CheckForFury_EY();
	DWORD CheckForXorist_LN();
	DWORD CheckForZerber_DZPX();
	DWORD CheckForZerber_KWM();
	DWORD CheckForNoon_NBA();
	DWORD CheckForNoon_NHK();
	DWORD CheckForNoon_NHN();
	DWORD CheckForBicololo_BIGP();
	DWORD CheckForHesv_DGWE();
	DWORD CheckForHesv_DGWX();
	DWORD CheckForHesv_DGWY();
	DWORD CheckForInject_AJWTW();
	DWORD CheckForInject_VCFZ();
	DWORD CheckForKasidet_OKJ();
	DWORD CheckForSchoolBoy_WD();
	DWORD CheckForSiscos_XLH();
	DWORD CheckForSiscos_XUG();
	DWORD CheckForSnojan_CIAX();
	DWORD CheckForSwisyn_FNPH();
	DWORD CheckForSwisyn_FOPQ();
	DWORD CheckForBundpil_AXH();
	DWORD CheckForVB_CZAS();
	DWORD CheckForVBNA_CB();
	DWORD CheckForWBNA_ROCA();

	//Pradip 22 / 11 / 2018
	DWORD  CheckForGandCrabAE();
	DWORD  CheckForGandCrabAES();
	DWORD  CheckForChapakFAORIU();
	DWORD  CheckForChapakTPNK();
	DWORD  CheckForEncoderEYKTIQ();
	DWORD  CheckForEncoderEYRUOW();
	DWORD  CheckForGandCrabAC();
	DWORD  CheckForGandCrypt181();
	DWORD  CheckForGandCryptEM();
	DWORD  CheckForGandCryptFEEYGA();
	DWORD  CheckForQuicdyA();
	DWORD  CheckForChapakIJA();
	DWORD  CheckForGandCrab143();
	DWORD  CheckForGandCryptBLA();
	DWORD  CheckForGandCryptBLK();
	DWORD  CheckForGandCryptBLS();
	DWORD  CheckForGandCryptBVD();
	DWORD  CheckForGandCryptBVR();
	DWORD  CheckForGandCryptFDTEPH();
	DWORD  CheckForGrandCrabCNYZY();
	DWORD  CheckForGandCryptAAC();
	DWORD  CheckForGandCryptAAL();
	DWORD  CheckForGandCryptAAN();
	DWORD  CheckForGandCryptAAR();
	DWORD  CheckForGandCryptAAT();
	DWORD  CheckForGandCryptAAV();
	DWORD  CheckForGandCryptABX();
	DWORD  CheckForGandCryptABY();
	DWORD  CheckForGandCryptALN();
	DWORD  CheckForGandCryptALP();
	DWORD  CheckForGandCryptABZ();
	DWORD  CheckForGandCryptACB();
	DWORD  CheckForGandCryptACE();
	DWORD  CheckForGandCryptACI();
	DWORD  CheckForGandCryptADL();
	DWORD  CheckForGandCryptAFC();//dll
	DWORD  CheckForGandCryptAFE();
	DWORD  CheckForGandCryptAFG();
	DWORD  CheckForGandCryptAFJ();
	DWORD  CheckForGandCryptAFK();
	DWORD  CheckForGandCryptAFW();
	DWORD  CheckForGandCryptAGA();
	DWORD  CheckForGandCryptAGC();
	DWORD  CheckForGandCryptAGG();
	DWORD  CheckForGandCryptAGJ();
	DWORD  CheckForGandCryptAGP();
	DWORD  CheckForGandCryptAGZ();
	DWORD  CheckForGandCryptAHB();
	DWORD  CheckForGandCryptAHE();
	DWORD  CheckForGandCryptAKB();
	DWORD  CheckForGandCryptAJR();
	DWORD  CheckForGandCryptAKF();
	DWORD  CheckForGandCryptAKG();
	DWORD  CheckForGandCryptAKHB();
	DWORD  CheckForGandCryptAKL();
	DWORD  CheckForGandCryptAKO();
	DWORD  CheckForGandCryptAKOS();
	DWORD  CheckForGandCryptAKP();
	DWORD  CheckForGandCryptAKQ();
	DWORD  CheckForGandCryptAKS();
	DWORD  CheckForGandCryptAKX();

	//Shodhan 23-11-2018
	DWORD CheckForCoins_PWS();
	DWORD CheckForEmotet_GHJIY();
	DWORD CheckForFareit_KOW();
	DWORD CheckForFareit_KTH();
	DWORD CheckForGandCrab_LIS();
	DWORD CheckForNanoCore_OPV();
	DWORD CheckForNanoCore_OPZ();
	DWORD CheckForNoon_WIW();
	DWORD CheckForShade_PBR();
	DWORD CheckForSmokeLoader_FGVA();
	DWORD CheckForSmokeLoader_LPD();
	DWORD CheckForTrickBot_CUK();
	DWORD CheckForYakes_YFMF();
	DWORD CheckForTroldesh_KIC();
	DWORD CheckForAgenttesla_MULGEN();
	DWORD CheckForCyberGate_IKLW();
	DWORD CheckForEmotet_LSER();
	DWORD CheckForFareit_POSTW();
	DWORD CheckForLokiBot_HYEF();
	DWORD CheckForLokiBot_HYEG();
	DWORD CheckForNanoCore_CUDE();
	DWORD CheckForPonyBased_JUFT();
	DWORD CheckForPonyBased_JUFZ();
	DWORD CheckForRemcos_OKLE();
	DWORD CheckForSmokeLoader_LOJR();
	DWORD CheckForTrickBotBased_GBMW();
	DWORD CheckForUrsnif_AVEO();
	DWORD CheckForAgent_JDT();
	DWORD CheckForBladabindi_GXB();
	DWORD CheckForBladabindi_JOG();
	DWORD CheckForBladabindi_RAN();
	DWORD CheckForBladabindi_RWX();
	DWORD CheckForBladabindi_SWK();
	DWORD CheckForNanoBot_OZH();
	DWORD CheckForNanoBot_QGD();
	DWORD CheckForNanoBot_VFF();
	DWORD CheckForNanoBot_XNS();
	DWORD CheckForNanoBot_XPH();
	DWORD CheckForSpyGate_UZ();
	DWORD CheckForSpyGate_VK();
	DWORD CheckForAgent_CBDD();
	DWORD CheckForAgent_CJXG();
	DWORD CheckForGbot_ANEN();
	DWORD CheckForGbot_POR();
	DWORD CheckForHlux_FUWL();
	DWORD CheckForIRCBot_GEN();
	DWORD CheckForIRCBot_GOU();
	DWORD CheckForJaan_S();
	DWORD CheckForTrickBot_JIER();
	DWORD CheckForTrickBot_JIRT();

	//Sagar 23-11-2018
	DWORD CheckForLyposit_AE();
	DWORD CheckForCerber_AS();
	DWORD CheckForFarfli_BCGK();
	DWORD CheckForMokes_XMF();
	DWORD CheckForMokes_XMR();
	DWORD CheckForMokes_XMS();
	DWORD CheckForAgent_AQAX();
	DWORD CheckForCerber_AA();
	DWORD CheckForCerber_AB();
	DWORD CheckForRansom_HIDDENTEARA();
	DWORD CheckForMbro_AC();
	DWORD CheckForMyxaH_OIF();
	DWORD CheckForMyxaH_OFN();
	DWORD CheckForOnion_VHW();
	DWORD CheckForAgent_AAGG();
	DWORD CheckForBlocker_ASD();
	DWORD CheckForBlocker_IFPD();
	DWORD CheckForBlocker_JZJB();
	DWORD CheckForBlocker_JZOO();
	DWORD CheckForBlocker_JZQQ();
	DWORD CheckForBlocker_JZRK();
	DWORD CheckForCryrar_GHR();
	DWORD CheckForForeign_FR();
	DWORD CheckForForeign_NCHZ();
	DWORD CheckForForeign_NMLH();
	DWORD CheckForGen_ADNU();
	DWORD CheckForGimemo_CDUQ();
	DWORD CheckForGimemo_CDQV();
	DWORD CheckForGimemo_GM();
	DWORD CheckForGimemo_GN();
	DWORD CheckForGimemo_GS();
	DWORD CheckForHexzone_HX();
	DWORD CheckForLocky_XOV();
	DWORD CheckForWin32_MBROMB();
	DWORD CheckForPornoAsset_CIFR();
	DWORD CheckForPornoAsset_CPBB();
	DWORD CheckForPornoAsset_PA();
	DWORD CheckForPornoBlocker_EJXT();
	DWORD CheckForPornoBlocker_EK();
	DWORD CheckForPornoBlocker_PB();
	DWORD CheckForSageCrypt_CYT();
	DWORD CheckForZerber_DHIV();
	DWORD CheckForZerber_DIYU();
	DWORD CheckForZerber_DZJZ();
	DWORD CheckForNoon_NHI();
	DWORD CheckForBicololo_BIHB();
	DWORD CheckForBicololo_BIHD();
	DWORD CheckForHesv_SLM();
	DWORD CheckForSiscos_XUJ();
	DWORD CheckForSiscos_XWB();
	DWORD CheckForEmotet_MULTIGENERIC();
	DWORD CheckForScar_FKGH();

	//Nihar 23-11-2018
	DWORD CheckForBitman_ACRT();
	DWORD CheckForBitman_ACWB();
	DWORD CheckForBitman_ACWD();
	DWORD CheckForBitman_ACWE();
	DWORD CheckForBitman_ACWH();
	DWORD CheckForBitman_ACWN();
	DWORD CheckForBitman_ACWO();
	DWORD CheckForBitman_ACYC();
	DWORD CheckForBitman_ACYD();
	DWORD CheckForBitman_ACYP();
	DWORD CheckForBitman_ACZH();
	DWORD CheckForBitman_ADAC();
	DWORD CheckForBitman_DZJ();
	DWORD CheckForBlocker_AQGJ();
	DWORD CheckForBlocker_ARFH();
	DWORD CheckForBlocker_HQEQ();
	DWORD CheckForBlocker_IELN();
	DWORD CheckForBlocker_JJWZ();
	DWORD CheckForBlocker_JKZC();
	DWORD CheckForBlocker_JLMY();
	DWORD CheckForBlocker_JLPP();
	DWORD CheckForBlocker_JZSK();
	DWORD CheckForBlocker_SAF();
	DWORD CheckForForeign_NAEW();
	DWORD CheckForForeign_NGFS();
	DWORD CheckForForeign_NHFN();
	DWORD CheckForForeign_NHNJ();
	DWORD CheckForForeign_NLVM();
	DWORD CheckForForeign_NMIZ();
	DWORD CheckForForeign_NMLY();
	DWORD CheckForFury_FG();
	DWORD CheckForFury_FH();
	DWORD CheckForFury_FJ();
	DWORD CheckForOnion_AFTX();
	DWORD CheckForOnion_IVC();
	DWORD CheckForOnion_NHX();
	DWORD CheckForOnion_NMT();
	DWORD CheckForZerber_AWPZ();
	DWORD CheckForZerber_AWQB();
	DWORD CheckForZerber_AWQV();
	DWORD CheckForZerber_AWRF();
	DWORD CheckForZerber_AWRR();
	DWORD CheckForZerber_AXFG();
	DWORD CheckForZerber_AXHY();
	DWORD CheckForZerber_AXME();
	DWORD CheckForZerber_AXPG();
	DWORD CheckForZerber_AXPM();
	DWORD CheckForZerber_AXQP();
	DWORD CheckForZerber_AXWF();
	DWORD CheckForZerber_AYER();
	DWORD CheckForZerber_BZG();
	DWORD CheckForZerber_DBVK();
	DWORD CheckForZerber_DCVI();
	DWORD CheckForZerber_DCVO();
	DWORD CheckForGandCrab_V504();
	DWORD CheckForGandCrab_V503B();

	//Pradip 26-11-2018
	DWORD CheckMaliciousFilePaths();

	//Pradip - 12-02-2019
	DWORD CheckForMinerJPNA();
	DWORD CheckForMinerJPNB();

};