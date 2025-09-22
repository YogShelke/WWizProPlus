#ifndef _CURLWRAPPERH_
#define _CURLWRAPPERH_ 
#pragma once
#include "curl.h"


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
		CString m_csBearer;
		bool InitCurl();
		
		bool DownloadFileHTTP(std::string downloadURL,
			std::string localPathAndFileName/*,
			std::string userName,
			std::string password*/);

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