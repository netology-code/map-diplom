#pragma once
#include<memory>
#include<utility>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <iostream>
#include <functional>
#include"safequeue.h"
#include"joiner.h"

class thread_pool
{
private:
	std::atomic_bool done;
	threadsafe_queue<std::function<void()> > work_queue;
	std::vector<std::thread> threads;
	void worker_thread()
	{
		while (!done)
		{
			std::function<void()> task;
			if (!work_queue.empty())
			{
				work_queue.try_pop();
				try
				{
					task();
				}
				catch (...)
				{
					cout << this_thread::get_id() << endl;
				}
			
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}
public:
	void stop()
	{
		done = true;
	}
	thread_pool() :	done(false)
	{
		unsigned const thread_count = std::thread::hardware_concurrency();
		try
		{
			for (unsigned i = 0; i < thread_count; ++i)
			{
				threads.push_back(std::thread(&thread_pool::worker_thread, this));
			}
		}
		catch (...)
		{
			done = true;
			throw;
		}
	}
	~thread_pool()
	{
		done = true;
		for (unsigned long i = 0; i < threads.size(); ++i)
		{
			if (threads[i].joinable())
				threads[i].join();
		}
	}
	template<typename FunctionType>
	void submit(FunctionType f)
	{
		work_queue.push(std::function<void()>(f));
	}
};