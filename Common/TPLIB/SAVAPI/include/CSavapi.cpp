#include "StdAfx.h"
#include "CSavapi.h"

#define LICENSE_KEY    _T("HBEDV.KEY")

CSavapi::CSavapi(void)
{
    this->inited_ = false;
    this->apc_inited_ = false;
    this->savapi_ = NULL;
	m_bFound = false;
	m_bISRepairable = false;
	m_csThreatName = _T("");
}

int CC StatusCallback(SAVAPI_CALLBACK_DATA *data);
int CC ErrorCallback(SAVAPI_CALLBACK_DATA *data);

TCHAR* sys_temp_dir_get()
{
    TCHAR *temp = NULL;
    int os_temp_dir_size = 0;

    TCHAR os_temp_dir[PATH_SIZE_MAX] = { 0 };

    GetTempPath(PATH_SIZE_MAX, os_temp_dir);
    os_temp_dir_size = PATH_SIZE_MAX;

    temp = (TCHAR *)calloc(PATH_SIZE_MAX, sizeof(TCHAR));
    memcpy(temp, os_temp_dir, os_temp_dir_size);

    return temp;
}

void savapi_apc_global_init_prepare(SAVAPI_APC_GLOBAL_INIT *apc_global_init, TCHAR *module_path)
{
    // Get the system's temporary directory
    apc_global_init->temp_dir = sys_temp_dir_get();

    // Set the APC options to the default ones used in the SAVAPI Service
    apc_global_init->apc_mode = SAVAPI_APC_SCAN_MODE_FULL;  // Full functionality of APC
    apc_global_init->cache_size = 5242880;                  // Cache size of 5MB
    apc_global_init->dump_cache_file = 0;                   // No cache file dump
    apc_global_init->cache_file_path = NULL;                // Default temporary directory
    apc_global_init->blackout_retries = 5;                  // 5 APC scan fails until APC will be declared unavailable
    apc_global_init->blackout_timeout = 300;                // 300 seconds after which a new connection will be tried to APC, if APC was declared unavailable
    apc_global_init->proxy = NULL;         // Use the system proxy or the one configured in the env (HTTPS_PROXY, HTTP_PROXY, etc)
    apc_global_init->lib_dir = module_path;
    apc_global_init->cert_dir = module_path;
}

void savapi_apc_global_init_release(SAVAPI_APC_GLOBAL_INIT *apc_global_init)
{
    free(apc_global_init->temp_dir);
    memset(apc_global_init, 0, sizeof(SAVAPI_APC_GLOBAL_INIT));
}

SAVAPI_STATUS CSavapi::StartSAVAPI()
{
    // Initialize SAVAPI library if not already initialized  
    if (inited_)
    {
        return SAVAPI_E_ALREADY_INITIALIZED;
    }

    SAVAPI_STATUS ret_code = SAVAPI_S_OK;
    SAVAPI_GLOBAL_INIT global_init = { 0 };

    TCHAR module_path[_MAX_PATH] = { 0 };

    global_init.api_major_version = SAVAPI_API_MAJOR_VERSION;
    global_init.api_minor_version = SAVAPI_API_MINOR_VERSION;

    // Get current path  
    if(0 != GetModuleFileName(NULL, module_path, _MAX_PATH))
    {
        if(PathRemoveFileSpec(module_path))
        {
            TCHAR keyFile[_MAX_PATH] = { 0 };
            _sntprintf_s(keyFile, sizeof(keyFile), _T("%s\\SVDB\\%s"), module_path, LICENSE_KEY);
            global_init.key_file_name = keyFile;

            global_init.engine_dirpath = module_path;

			TCHAR vdfs_dirpath[_MAX_PATH] = { 0 };
			_sntprintf_s(vdfs_dirpath, sizeof(vdfs_dirpath), _T("%s\\%s"), module_path, L"SVDB");
			global_init.vdfs_dirpath = vdfs_dirpath;

            ret_code = SAVAPI_initialize(&global_init);
        }
    }

    if (ret_code == SAVAPI_S_OK)
    {
        SAVAPI_APC_GLOBAL_INIT apc_global_init = { 0 };

        // Prepare the data for APC Library global initialization
        savapi_apc_global_init_prepare(&apc_global_init, module_path);

        // Actually call global initialization on APC
        // If no license for APC is present in the key file. SAVAPI will not use APC for scanning
        ret_code = SAVAPI_APC_initialize(&apc_global_init);

        // Release the data for APC Library global initialization
        savapi_apc_global_init_release(&apc_global_init);

        if (ret_code != SAVAPI_S_OK && ret_code != SAVAPI_E_APC_NO_LICENSE)
        {
            // Stop and uninitialize SAVAPI on error  
            StopSAVAPI();
            return ret_code;
        }

        if (ret_code == SAVAPI_S_OK)
        {
            apc_inited_ = true;
        }
        else
        {
            // Continue initialization even if SAVAPI found no license for APC
            ret_code = SAVAPI_S_OK;
        }
    }

    // Get a library instance if the library is initialized  
    if (ret_code == SAVAPI_S_OK)
    {
        ZeroMemory(&init_data_, sizeof(init_data_));
        ret_code = SAVAPI_create_instance(&init_data_, &savapi_);
    }

    // Register the callbacks  
    if (ret_code == SAVAPI_S_OK)
    {
        ret_code = SAVAPI_register_callback(savapi_, SAVAPI_CALLBACK_REPORT_FILE_STATUS, StatusCallback);
    }

    if (ret_code == SAVAPI_S_OK)
    {
        ret_code = SAVAPI_register_callback(savapi_, SAVAPI_CALLBACK_REPORT_ERROR, ErrorCallback);
    }

    if (ret_code == SAVAPI_S_OK)
    {
        ret_code = SAVAPI_set_user_data(savapi_, this);
    }
    
    if (ret_code == SAVAPI_S_OK)
    {
        inited_ = true;
    }
    else
    {
        // Stop and uninitialize SAVAPI on error  
        StopSAVAPI();
    }

    return ret_code;
}

CSavapi::~CSavapi(void)
{
    StopSAVAPI();
}

void CSavapi::StopSAVAPI()
{
    if (savapi_ != NULL)
    {
        SAVAPI_unregister_callback(savapi_, SAVAPI_CALLBACK_REPORT_FILE_STATUS, StatusCallback);
        SAVAPI_unregister_callback(savapi_, SAVAPI_CALLBACK_REPORT_ERROR, ErrorCallback);
        SAVAPI_release_instance(&savapi_);
    }

    SAVAPI_APC_uninitialize();
    SAVAPI_uninitialize();

    inited_ = false;
    apc_inited_ = false;
}

bool CSavapi::IsInited()
{
    return inited_;
}

bool CSavapi::ApcIsInited()
{
    return apc_inited_;
}

SAVAPI_STATUS CSavapi::SetOption(SAVAPI_OPTION optionId, TCHAR *optionData)
{
    if (!savapi_)
    {
        return SAVAPI_E_NOT_INITIALIZED;
    }

    if (!optionData)
    {
        return SAVAPI_E_INVALID_PARAMETER;
    }

    return SAVAPI_set(savapi_, optionId, optionData);
}

SAVAPI_STATUS CSavapi::ScanFile(CString file_path)
{
    if (!savapi_)
    {
        return SAVAPI_E_NOT_INITIALIZED;
    }

    if (!file_path)
    {
        return SAVAPI_E_INVALID_PARAMETER;
    }
	scan_ErrorReport.Empty();
	m_bFound = false;
	m_bISRepairable = false;
	m_csThreatName = _T("");

    return SAVAPI_scan(savapi_, file_path.GetBuffer());
}

SAVAPI_STATUS CSavapi::AbortScan()
{
    if (!savapi_)
    {
        return SAVAPI_E_NOT_INITIALIZED;
    }

    return SAVAPI_send_signal(savapi_, SAVAPI_SIGNAL_SCAN_ABORT, NULL);
}

// -------BEGIN-------------------Callback functions-------------------------- 
int CC StatusCallback(SAVAPI_CALLBACK_DATA *data)
{
    // In short mode only the malware type and malware name are displayed

    SAVAPI_STATUS ret = SAVAPI_S_OK;
    SAVAPI_FILE_STATUS_DATA *fs_data = NULL;
    if(SAVAPI_CALLBACK_REPORT_FILE_STATUS == data->type)
    {
        // This is the expected callback -> process data  
        fs_data = data->callback_data.file_status_data;
        if(fs_data)
        {
            CSavapi *scanner = (CSavapi *)data->user_data;
            if((SAVAPI_S_OK == ret) && (1 == fs_data->scan_answer))
            {
				scanner->m_bFound = true;
				scanner->m_csThreatName = fs_data->malware_info.name;
				if (fs_data->malware_info.removable == 0x01)
				{
					scanner->m_bISRepairable = true;
				}
            }
        }
    }
    return ret;
}

int CC ErrorCallback(SAVAPI_CALLBACK_DATA *data)
{
    SAVAPI_ERROR_DATA *fs_data = NULL;
    CSavapi *scanner = (CSavapi*)data->user_data;

    if(!scanner)
    {
        return -1;
    }
    
    if(SAVAPI_CALLBACK_REPORT_ERROR == data->type)
    {
        // This is the expected callback -> process data  
        fs_data = data->callback_data.error_data;
    }
    if (NULL == fs_data)
    {
		scanner->scan_ErrorReport.Append(_T("Invalid error_data!!!\n"));
        return -1;
    }
    if(SAVAPI_ELEVEL_ERROR == fs_data->level)
    {
        switch(fs_data->code)
        {
        // Treat this errors  

        case SAVAPI_E_HIT_MAX_REC:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Maximum archive recursion reached! "));
            break;
        }
        case SAVAPI_E_HIT_MAX_SIZE:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Maximum archive size reached! "));
            break;
        }
        case SAVAPI_E_HIT_MAX_RATIO:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Maximum archive ratio reached! "));
            break;
        }
        case SAVAPI_E_HIT_MAX_COUNT:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Maximum archive number of files in archive reached! "));
            break;
        }
        case SAVAPI_E_ENCRYPTED:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Encrypted content! "));
            break;
        }
        case SAVAPI_E_UNSUPPORTED:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Archive type unsupported! "));
            break;
        }
        case SAVAPI_E_UNSUPPORTED_COMPRESSION:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Compression method unsupported! "));
            break;
        }
        case SAVAPI_E_PROC_ERROR:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Processing error! "));
            break;
        }
#if 0
        case SAVAPI_E_INCOMPLETE:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Processing incomplete! "));
            break;
        }    
        case SAVAPI_E_PARTIAL:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Partial archive! "));
            break;
        }
        case SAVAPI_E_ABORTED:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Archive scan aborted!"));
            break;
        }
#endif
        case SAVAPI_E_TIMEOUT:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Timeout reached while scanning! "));
            break;
        }
        case SAVAPI_E_MATCHED:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: File matched! "));
            break;
        }
        case SAVAPI_E_LICENSE_RESTRICTION:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: License restriction! "));
            break;
        }
        case SAVAPI_E_REPAIR_FAILED:
        {
            // Code to treat this error case here  
			scanner->scan_ErrorReport.Append(_T("Error: Repair failed! "));
            break;
        }
        case SAVAPI_E_APC_QUOTA:
        {
            // Code to treat this error case here
			scanner->scan_ErrorReport.Append(_T("Error: APC quota limit reached! "));
            break;
        }
        default:
            break;
        }
    }

    if(SAVAPI_ELEVEL_WARNING == fs_data->level)
    {
        if(SAVAPI_W_DAMAGED & fs_data->code)
        {
			scanner->scan_ErrorReport.Append(_T("Warning: The file might be damaged by a virus. "));
        }

        if(SAVAPI_W_OLE_DAMAGED & fs_data->code)
        {
			scanner->scan_ErrorReport.Append(_T("Warning: OLE-File might be damaged. "));
        }

        if(SAVAPI_W_SUSPICIOUS & fs_data->code)
        {
			scanner->scan_ErrorReport.Append(_T("Warning: File is suspicious. "));
        }

        if(SAVAPI_W_PROGRESS_ABORT & fs_data->code)
        {
			scanner->scan_ErrorReport.Append(_T("Warning: An abort was triggered by the progress callback. "));
        }
        
        if(SAVAPI_W_HEADER_MALFORMED & fs_data->code)
        {
			scanner->scan_ErrorReport.Append(_T("Warning: Archive has a malformed header. "));
        }

        if(SAVAPI_W_POTENTIAL_ARCH_BOMB & fs_data->code)
        {
			scanner->scan_ErrorReport.Append(_T("Warning: Archive bomb detected. "));
        }

        if(SAVAPI_W_RATIO_EXCEEDED & fs_data->code)
        {
			scanner->scan_ErrorReport.Append(_T("Warning: Archive ratio exceeded. "));
        }

        if(SAVAPI_W_MAX_EXTRACTED & fs_data->code)
        {
			scanner->scan_ErrorReport.Append(_T("Warning: Archive ratio exceeded. "));
        }
    }
    return SAVAPI_S_OK;
}
