#pragma once

interface IExecContext
{
	virtual bool Initialize(HANDLE hQueueEvent) = 0;
    virtual bool Run(bool bLastOperation = false) = 0;
	virtual void DeleteContext() = 0;
	virtual void NotifyQueueEvent() = 0;
};