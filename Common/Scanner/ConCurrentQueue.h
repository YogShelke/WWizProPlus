/**********************************************************************************************************
	Program Name          : CConCurrentQueue
	Description           : This class is implementation of queue which is thread synchronized
	Author Name			  : Ramkrushna Shelke
	Date Of Creation      : 06 Sep 2016
	Version No            : 2.1.0.101
	Special Logic Used    : This class is used to maintain Queue which is thread synchronized by Mutex object.
	Modification Log      :
***********************************************************************************************************/
#pragma once

#ifndef CONCURRENT_QUEUE_
#define CONCURRENT_QUEUE_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class CConCurrentQueue
{
public:

	T pop()
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		while(queue_.empty())
		{
			cond_.wait(mlock);
			//This wait has been added as because in onacess we are adding accessed files into queue 
			//and reply immediate to file system to proceed futher but for some milisecods the handle
			//is opened by scanner and third party application not able to save the file.
			//so this was synchronization issue, this is workaround for this issue.
			Sleep(12);
		}
		auto val = queue_.front();
		queue_.pop();
		return val;
	}

	void pop(T& item)
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		while (queue_.empty())
		{
			cond_.wait(mlock);
			//This wait has been added as because in onacess we are adding accessed files into queue 
			//and reply immediate to file system to proceed futher but for some milisecods the handle
			//is opened by scanner and third party application not able to save the file.
			//so this was synchronization issue, this is workaround for this issue.
			Sleep(12);
		}
		item = queue_.front();
		queue_.pop();
	}

	void push(const T& item)
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		queue_.push(item);
		
		mlock.unlock();
		cond_.notify_one();
	}

	void NotifyALL()
	{
		//mutex_.unlock();
		bexit = true;
		cond_.notify_all();
	}
	
	

	CConCurrentQueue() = default;
	CConCurrentQueue(const CConCurrentQueue&) = delete;            // disable copying
	CConCurrentQueue& operator=(const CConCurrentQueue&) = delete; // disable assignment

//private:
public:
	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;
	bool  bexit = false;
};

#endif

