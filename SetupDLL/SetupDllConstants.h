//Setup Dll Constants
//Constants are declared as per entries in .ISS files 
// Need to maintain the sequence for make it work correctly
//e.g. In ISS Custom messagebox entry is  
//CM_0_WWSetupDllAppRunningUnInstall = Some of the application of WardWiz is running, Do you want close to continue Uninstallation?
// Need to add same entry for better understanding and use it accodingly
typedef enum _SETUPDLLISSCONST
{
	CM_0_WWSetupDllAppRunningUnInstall
	,CM_1_WWSetupDllAppRunningReInstall
	,CM_2_WWSetupDllOutlookCloseInstall
	,CM_3_WWSetupDllOutlookCloseUnInstall
	,CM_4_WWSetupDllInstallPatches
	,CM_5_WWSetupDllClose
	,CM_6_WWSetupDllUnInstallContinue
	,CM_7_WWSetupDllLatestInstalled
	,CM_8_WWSetupDllLatestInstalled
	,CM_9_WWSetupDllCustomMsgBoxButtonYes
	,CM_10_WWSetupDllCustomMsgBoxButtonYesToAll
	,CM_11_WWSetupDllCustomMsgBoxButtonNo
	,CM_12_WWSetupDllCustomMsgBoxButtonNoToAll
	,CM_13_WWSetupDllCustomMsgBoxButtonCancel
	,CM_14_WWSetupDllOtherAV
	,CM_15_WWSetupDllOtherAV
}SETUPDLLISSCONST;
