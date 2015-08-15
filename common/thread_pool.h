#ifndef EQEMU_COMMON_THREAD_POOL_H
#define EQEMU_COMMON_THREAD_POOL_H

#include <functional>

class ThreadPoolWork
{
public:
	ThreadPoolWork() { }
	virtual ~ThreadPoolWork() { }

	virtual void Run() { }
	virtual void Finished() { }
};

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();
	void ClearWork();
	void AddWork(ThreadPoolWork *w);
	void Process();

private:
	bool GetWork(ThreadPoolWork **work);
	void AddWorkFinished(ThreadPoolWork *w);
	void WorkerThread();

	struct impl;
	impl *imp;
};

#endif
