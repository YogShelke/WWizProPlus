// dllmain.h : Declaration of module class.

class CWWizInteropeDLLModule : public ATL::CAtlDllModuleT< CWWizInteropeDLLModule >
{
public :
	DECLARE_LIBID(LIBID_WWizInteropeDLLLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WWIZINTEROPEDLL, "{85795CE1-E777-4E26-A937-71815D398753}")
};

extern class CWWizInteropeDLLModule _AtlModule;
