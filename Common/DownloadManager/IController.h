#pragma once

interface IController
{
    virtual bool InitController(LPVOID lParam) = 0;
    virtual bool StartController(LPVOID lParam) = 0;
    virtual bool StopController() = 0;
	virtual void DeleteController() = 0;
};

interface IGUIInterface
{
	virtual void SetDownloadedBytes(DWORD dwTotalFileLength, DWORD dwDownloadedBytes,double dTransferRate) = 0;
	virtual void SetPercentDownload(int nPercent) = 0;
};