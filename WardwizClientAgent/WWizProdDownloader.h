#pragma once
#include "stdafx.h"
#include "WWizHttpManager.h"
#include "DownloadController.h"

class CWWizProdDownloader
{
public:
	CWWizProdDownloader();
	virtual ~CWWizProdDownloader();
	bool StartDownloadFile(LPCTSTR szUrlPath, LPTSTR lpszTargetPath);
	bool RunSetupInSilentMode(LPCTSTR szSetupPath);
private:
	CDownloadController*		m_pDownloadController;
};

