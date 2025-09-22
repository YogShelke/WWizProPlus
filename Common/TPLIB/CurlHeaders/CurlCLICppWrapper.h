#ifndef _CURLWRAPPERH_
#define _CURLWRAPPERH_ 
#pragma once
#include "CurlHeaders\curl.h"


namespace CurlCppWrapper {

	struct HTTPFile {
		const char *filename;
		FILE *stream;
	};

	class CCurlWrapper
	{
	private:
		bool	_isCurlInitalized;
		HMODULE _hMod;
		void CleanupCurl();
	public:
		CCurlWrapper() {
			_isCurlInitalized = false;
			_hMod = NULL;
		};

		~CCurlWrapper() 
		{  
			if (_isCurlInitalized)
				CleanupCurl();
		};
		CString strURLToDownload;
		bool InitCurl();
		
		bool DownloadFileHTTP( CString downloadURL, 
								CString localPathAndFileName, 
								CString userName, 
								CString password);

		bool UploadFileHTTPS(	CString uploadURL, 
								CString localPatAndFileName,
								CString userName,
								CString password);

		bool UploadFile(CString uploadURL,
						CString localPatAndFileName,
						CString userName,
						CString password);

		bool UploadFileToFTP(CString uploadURL,
						CString localPatAndFileName,
						CString UploadFileName,
						CString userName,
						CString password);

		bool DownloadFileHTTPS(std::string downloadURL,
					std::string localPathAndFileName);

	};
}
#endif