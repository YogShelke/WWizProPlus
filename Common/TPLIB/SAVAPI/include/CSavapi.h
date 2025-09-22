#pragma once

#include <shlwapi.h>
#include "savapi.h"

// Define for the maximum path size
#define PATH_SIZE_MAX 4096

class CSavapi
{
public:
    CSavapi(void);
    ~CSavapi(void);

    SAVAPI_STATUS SetOption(SAVAPI_OPTION optionId, TCHAR* optionData);
    TCHAR* GetOption(int optionId);
    SAVAPI_STATUS ScanFile(CString file_path);
    SAVAPI_STATUS AbortScan();

    SAVAPI_STATUS StartSAVAPI();
    void StopSAVAPI();
    bool IsInited();
    bool ApcIsInited();

    CString		scan_ErrorReport;
	bool		m_bFound;
	bool		m_bISRepairable;
	CString		m_csThreatName;
private:
    bool inited_;
    bool apc_inited_;
    SAVAPI_FD savapi_;
    SAVAPI_INSTANCE_INIT init_data_;

};
