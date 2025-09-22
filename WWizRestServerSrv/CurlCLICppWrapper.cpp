#include "stdafx.h"
#include <tchar.h>
#include <sys/stat.h>
#include "CurlCLICppWrapper.h"
#include <sstream>


typedef CURLcode(*fnCurl_Global_Init)(long flags);
typedef CURL* (*fnCurl_Easy_Init)();
typedef void(*fnCurl_Global_Cleanup)();
typedef CURLcode(*fnCurl_Easy_Setopt)(CURL *curl, CURLoption option, ...);
typedef CURLcode(*fnCurl_Easy_Perform)(CURL *curl);
typedef void(*fnCurl_Easy_Cleanup)(CURL *curl);
typedef CURLcode(*fnCurl_Easy_Getinfo)(CURL *curl, CURLINFO info, ...);
typedef const char * (*fnCurl_Easy_Strerror)(CURLcode);
typedef void (*fn_curl_slist_free_all)(struct curl_slist * list);
typedef struct curl_slist *(*fn_curl_slist_append)(struct curl_slist * list, const char * string);


fnCurl_Global_Init pfn_curl_global_init = NULL;
fnCurl_Easy_Init pfn_curl_easy_init = NULL;
fnCurl_Global_Cleanup pfn_curl_global_cleanup = NULL;
fnCurl_Easy_Setopt pfn_curl_easy_setopt = NULL;
fnCurl_Easy_Perform pfn_curl_easy_perform = NULL;
fnCurl_Easy_Cleanup pfn_curl_easy_cleanup = NULL;
fnCurl_Easy_Getinfo pfn_curl_easy_getinfo = NULL;
fnCurl_Easy_Strerror pfn_curl_easy_strerror = NULL;
fn_curl_slist_free_all pfn_curl_slist_free_all = NULL;
fn_curl_slist_append pfn_curl_slist_append = NULL;

using namespace CurlCppWrapper;

/// <summary>
/// Initializes the curl.
/// </summary>
/// <returns>true if libcurl library loads successfully else false</returns>
bool CCurlWrapper::InitCurl()
{
    //Load the CURL library
    _hMod = LoadLibraryEx(L"libcurl.dll", NULL, 0);
    if (_hMod)
    {
        //Loading the
        pfn_curl_global_init = (fnCurl_Global_Init)GetProcAddress(_hMod, "curl_global_init");
        pfn_curl_easy_init = (fnCurl_Easy_Init)GetProcAddress(_hMod, "curl_easy_init");
        pfn_curl_easy_setopt = (fnCurl_Easy_Setopt)GetProcAddress(_hMod, "curl_easy_setopt");
        pfn_curl_easy_perform = (fnCurl_Easy_Perform)GetProcAddress(_hMod, "curl_easy_perform");
        pfn_curl_easy_getinfo = (fnCurl_Easy_Getinfo)GetProcAddress(_hMod, "curl_easy_getinfo");
        pfn_curl_easy_cleanup = (fnCurl_Easy_Cleanup)GetProcAddress(_hMod, "curl_easy_cleanup");
        pfn_curl_global_cleanup = (fnCurl_Global_Cleanup)GetProcAddress(_hMod, "curl_global_cleanup");
		pfn_curl_slist_append = (fn_curl_slist_append)GetProcAddress(_hMod, "curl_slist_append");

        if (NULL != pfn_curl_global_init &&
            NULL != pfn_curl_easy_init &&
            NULL != pfn_curl_easy_setopt &&
            NULL != pfn_curl_easy_perform &&
            NULL != pfn_curl_easy_getinfo &&
            NULL != pfn_curl_easy_cleanup &&
            NULL != pfn_curl_global_cleanup)
        {
            pfn_curl_global_init(CURL_GLOBAL_DEFAULT);
            _isCurlInitalized = true;
        }
    }
    return _isCurlInitalized;
}

void CCurlWrapper::CleanupCurl()
{
	pfn_curl_global_cleanup();
	pfn_curl_global_init = NULL;
	pfn_curl_easy_init = NULL;
	pfn_curl_global_cleanup = NULL;
	pfn_curl_easy_setopt = NULL;
	pfn_curl_easy_perform = NULL;
	pfn_curl_easy_cleanup = NULL;
	pfn_curl_easy_getinfo = NULL;
	pfn_curl_easy_strerror = NULL;

	if (_hMod != NULL)
	{
		FreeLibrary(_hMod);
		_hMod = NULL;
	}

	_isCurlInitalized = false;
}

size_t static DownloadWriteFileCallback(void *buffer, size_t size, size_t nmemb, void *stream)
{
	HTTPFile *out = (HTTPFile *)stream;
	if (out != NULL)
	{
		if (out && !out->stream) {
			/* open file for writing */
			fopen_s(&out->stream, out->filename, "wb");
			if (!out->stream)
				return -1; /* failure, can't open file to write */
		}
	}
	if (out != NULL)
		return fwrite(buffer, size, nmemb, out->stream);
	else
		return 0;
}

size_t static UploadReadFileCallback(void *buffer, size_t size, size_t nmemb, void *stream)
{
    return fread(buffer, size, nmemb, (FILE*)stream);
}

/* NOTE: if you want this example to work on Windows with libcurl as a   DLL, you MUST also provide a read callback with CURLOPT_READFUNCTION.
Failing to do so will give you a crash since a DLL may not use the   
variable's memory when passed in to it from an app like this. */

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{  /* in real-world cases, this would probably get this data differently     
   as this fread() stuff is exactly what the library already would do     
   by default internally */ 
	size_t retcode = fread(ptr, size, nmemb,(FILE*) stream); 
fprintf(stderr, "*** We read %d bytes from file\n", retcode);  return retcode;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	std::string data((const char*)ptr, (size_t)size * nmemb);
	*((std::stringstream*)stream) << data << std::endl;
	return size * nmemb;
}


/// <summary>
/// Downloads a file from a server using HTTPS protocol
/// </summary>
/// <returns>true if downloading successful else false</returns>
bool CCurlWrapper::DownloadFileHTTPS(std::string downloadURL, std::string localPathAndFileName)
{
    if (!_isCurlInitalized)
        return false;
	//const char* pszDownloadURL = downloadURL.GetString();
    //const char* pszLocalPathAndFileName = context->marshal_as<const char*>(localPathAndFileName);
    //const char* pszUserName = context->marshal_as<const char*>(userName);
    //const char* pszPassword = context->marshal_as<const char*>(password);

	const char* pszDownloadURL = 0;
	const char* pszLocalPathAndFileName = 0;

	CT2A ascii(strURLToDownload);
	pszDownloadURL = ascii.m_psz;

	//	pszDownloadURL = downloadURL.c_str();

	//CT2A ascii2(localPathAndFileName);
	//pszLocalPathAndFileName = ascii2.m_psz;

	pszLocalPathAndFileName = localPathAndFileName.c_str();

    CURL*	curl;
    bool	curlDownloadResult = false;
	struct curl_slist *headerlist = NULL;

    curl = pfn_curl_easy_init();

    if (curl)
    {
        HTTPFile httpFile;
        httpFile.filename = pszLocalPathAndFileName;
        httpFile.stream = NULL;

		if (pfn_curl_slist_append)
		{
			if (m_csBearer.GetLength() != 0)
			{
				CT2A asBearer(m_csBearer);
				headerlist = pfn_curl_slist_append(headerlist, asBearer.m_psz);
				headerlist = pfn_curl_slist_append(headerlist, "Content-Type: application/octet-stream");
			}
		}

		pfn_curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        //Set the URL to upload the file to.
        /* upload to this place */
        pfn_curl_easy_setopt(curl, CURLOPT_URL, pszDownloadURL);

        pfn_curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadWriteFileCallback);

        /* Set a pointer to our struct to pass to the callback */
        pfn_curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpFile);

        pfn_curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        pfn_curl_easy_setopt(curl, CURLOPT_PORT, 443);

        // set SSH user name and password in libcurl in this
        // format "user:password"
        char pszCurlUserPwd[1024] = { 0 };
       // sprintf_s(pszCurlUserPwd, 1024, "%s:%s", pszUserName, pszPassword);
      //  pfn_curl_easy_setopt(curl, CURLOPT_USERPWD, pszCurlUserPwd);

        // set SSH authentication to user name and password
       // pfn_curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);

        CURLcode res = pfn_curl_easy_perform(curl);

        if (res == CURLE_OK) {
            //Check the respose code for the operation performed
            long responseCode;
            res = pfn_curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

            if (res == CURLE_OK)
            {
                if (responseCode == 200)
                    curlDownloadResult = true;
            }
            else
            {
                //If response code not available, then assume that the operation was OK.
                curlDownloadResult = true;
            }
        }

        if (NULL != httpFile.stream)
            fclose(httpFile.stream);

        pfn_curl_easy_cleanup(curl);
    }
    return curlDownloadResult;
}

/// <summary>
/// Uploads a file to a server using HTTPS protocol
/// </summary>
/// <returns>true if uploading successful else false</returns>
bool CCurlWrapper::UploadFileHTTPS(CString uploadURL,
                                            CString localPathAndFileName,
                                            CString userName,
                                            CString password)
{
    if (!_isCurlInitalized)
        return false;

    // Marshal System::String to PCWSTR, for using in the CURL library functions.

    //const char* pszUploadURL = context->marshal_as<const char*>(uploadURL);
    //const char* pszLocalPathAndFileName = context->marshal_as<const char*>(localPathAndFileName);
    //const char* pszUserName = context->marshal_as<const char*>(userName);
    //const char* pszPassword = context->marshal_as<const char*>(password);
	const char* pszUploadURL = 0;
	const char* pszLocalPathAndFileName = 0;
	const char* pszUserName = 0;
	const char* pszPassword = 0;

	CT2A ascii(uploadURL);
	pszUploadURL = ascii.m_psz;

	CT2A ascii2(localPathAndFileName);
	pszLocalPathAndFileName = ascii2.m_psz;

	CT2A ascii3(userName);
	pszUserName = ascii3.m_psz;

	CT2A ascii4(password);
	pszPassword = ascii4.m_psz;


    CURL*	curl;
    bool	curlDownloadResult = false;
    struct stat fileInfo;
    FILE* fd = NULL;

    fopen_s(&fd,pszLocalPathAndFileName, "rb"); /* open file to upload */
    if (!fd) {
        return false;
    }

    /* to get the file size */
    if (fstat(_fileno(fd), &fileInfo) != 0) {
        fclose(fd);
        return false;
    }

    curl = pfn_curl_easy_init();

    if (curl) {
        /* upload to this place */
        pfn_curl_easy_setopt(curl, CURLOPT_URL, pszUploadURL);

        /* tell it to "upload" to the URL */
        pfn_curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* set where to read from (on Windows you need to use READFUNCTION too) */
        pfn_curl_easy_setopt(curl, CURLOPT_READDATA, fd);

        // use custom read function
        pfn_curl_easy_setopt(curl, CURLOPT_READFUNCTION, UploadReadFileCallback);

        /* and give the size of the upload (optional) */
        pfn_curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
            (curl_off_t)fileInfo.st_size);

  //      pfn_curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        pfn_curl_easy_setopt(curl, CURLOPT_PORT, 80);

        // set SSH user name and password in libcurl in this
        // format "user:password"
        char pszCurlUserPwd[1024] = { 0 };
        sprintf_s(pszCurlUserPwd, 1024, "%s:%s", pszUserName, pszPassword);
     //   pfn_curl_easy_setopt(curl, CURLOPT_USERPWD, pszCurlUserPwd);

        // set SSH authentication to user name and password
     //   pfn_curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);

        CURLcode res = pfn_curl_easy_perform(curl);
        /* Check for errors */
        if (res == CURLE_OK) {
            //Need to check the response code to make sure the operation has
            //completed successfully.
            long responseCode;
            res = pfn_curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            if (res == CURLE_OK)
            {
                if (responseCode == 200 || responseCode == 201)
                {
                    //TODO:
                    //If reponse code is 201, this means the path of the file specified for upload
                    //was not present on the destination and was created. Need to log this case.
                    curlDownloadResult = true;
                }
            }
            else
            {
                //If response code not available, then assume that the operation was OK.
                curlDownloadResult = true;
            }
        }

        fclose(fd);
        /* always cleanup */
        pfn_curl_easy_cleanup(curl);
    }

   return curlDownloadResult;
}

bool CCurlWrapper::UploadFileToFTP(CString uploadURL,
					CString localPatAndFileName,
					CString UploadFileName,
					CString userName,
					CString password)
{
	CURL *curl;
	CURLcode res;
	FILE *hd_src;
	struct stat file_info;
	curl_off_t fsize;

	/* get the file size of the local file */

	CT2A strFileToUpload(localPatAndFileName);

	if (stat(strFileToUpload, &file_info)) {
		return 1;
	}

	fsize = (curl_off_t)file_info.st_size;

	/* get a FILE * of the same file */
	hd_src = fopen(strFileToUpload, "rb");

	/* In windows, this will init the winsock stuff */
	pfn_curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = pfn_curl_easy_init();

	if (curl) {

		/* we want to use our own read function */
		pfn_curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

		/* enable uploading */
		pfn_curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* specify target */
		CString strRemoteURL = L"ftp://172.168.1.253/"  ;
		strRemoteURL.Append(UploadFileName);

		CT2A strRemoteLocation(strRemoteURL);

		pfn_curl_easy_setopt(curl, CURLOPT_URL, strRemoteLocation);

		/* now specify which file to upload */
		pfn_curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

		/* Set the size of the file to upload (optional).  If you give a *_LARGE
		option you MUST make sure that the type of the passed-in argument is a
		curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
		make sure that to pass in a type 'long' argument. */
		pfn_curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,(curl_off_t)fsize);

		char pszCurlUserPwd[1024] = { 0 };
		sprintf_s(pszCurlUserPwd, 1024, "%s:%s", "WELSERV\\gayatri.a", "myDre@mz@6");

		pfn_curl_easy_setopt(curl, CURLOPT_USERPWD, pszCurlUserPwd);

		/* Now run off and do what you've been told! */
		res = pfn_curl_easy_perform(curl);

		/* always cleanup */
		pfn_curl_easy_cleanup(curl);
	}
	fclose(hd_src); /* close the local file */

	pfn_curl_global_cleanup();

	return 0;
}
/// <summary>
/// Downloads a file from a server using HTTPS protocol
/// </summary>
/// <returns>true if downloading successful else false</returns>
bool CCurlWrapper::DownloadFileHTTP(std::string downloadURL,
	std::string localPathAndFileName/*,
	std::string userName,
	std::string password*/)
{
	if (!_isCurlInitalized)
		return false;

	const char* pszDownloadURL = 0;
	const char* pszLocalPathAndFileName = 0;
	const char* pszUserName = 0;
	const char* pszPassword = 0;

	CT2A ascii(strURLToDownload);
	pszDownloadURL = ascii.m_psz;

	pszLocalPathAndFileName = localPathAndFileName.c_str();

	//CT2A ascii3(userName);
	//pszUserName = ascii3.m_psz;

	//CT2A ascii4(password);
	//pszPassword = ascii4.m_psz;

	CURL*	curl;
	bool	curlDownloadResult = false;

	curl = pfn_curl_easy_init();
	/*
	    CURL *curl;
    FILE *fp;
    CURLcode res;
    char *url = "http://localhost/aaa.txt";
    char outfilename[FILENAME_MAX] = "C:\\bbb.txt";
    curl = curl_easy_init();
    if (curl)
    {
        fp = fopen(outfilename,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }

	*/
	if (curl)
	{
		HTTPFile httpFile;
		httpFile.filename = pszLocalPathAndFileName;
		httpFile.stream = NULL;
		FILE* fp = NULL;
		fp = fopen("C:\\Program Files\\WardWiz\\FTPUploader\\bbb.txt", "wb");
		//Set the URL to upload the file to.
		/* download to this place */
		pfn_curl_easy_setopt(curl, CURLOPT_URL, pszDownloadURL);

		pfn_curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadWriteFileCallback);

		/* Set a pointer to our struct to pass to the callback */
		pfn_curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpFile);

		// format "user:password"
		//char pszCurlUserPwd[1024] = { 0 };
		//sprintf_s(pszCurlUserPwd, 1024, "%s:%s", "WELSERV\\gayatri.a", "myDre@mz@6");

		//pfn_curl_easy_setopt(curl, CURLOPT_USERPWD, pszCurlUserPwd);

		CURLcode res = pfn_curl_easy_perform(curl);

		if (res == CURLE_OK) {
			//Check the respose code for the operation performed
			long responseCode;
			res = pfn_curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

			if (res == CURLE_OK)
			{
				if (responseCode == 200)
					curlDownloadResult = true;
			}
			else
			{
				//If response code not available, then assume that the operation was OK.
				curlDownloadResult = true;
			}
		}

		if (NULL != httpFile.stream)
			fclose(httpFile.stream);

		pfn_curl_easy_cleanup(curl);
	}
	return curlDownloadResult;
}
