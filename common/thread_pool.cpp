#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

#include "thread_pool.h"

struct ThreadPool::impl
{
	bool m_running;
	std::vector<std::thread> m_threads;
	std::deque<ThreadPoolWork*> m_work;
	std::deque<ThreadPoolWork*> m_work_finished;
	std::mutex m_work_lock;
	std::mutex m_work_finished_lock;
	std::condition_variable m_cv;
	std::mutex m_lock;
};

ThreadPool::ThreadPool(int t_count) {
	imp = new impl;
	imp->m_running = true;

	for(int i = 0; i < t_count; ++i) {
		imp->m_threads.push_back(std::thread(&ThreadPool::WorkerThread, this));
	}
}

ThreadPool::~ThreadPool() {
	ClearWork();

	imp->m_lock.lock();
	imp->m_running = false;
	imp->m_lock.unlock();
	imp->m_cv.notify_all();

	for(auto &t : imp->m_threads) {
		t.join();
	}

	delete imp;
}

void ThreadPool::ClearWork() {
	imp->m_work_lock.lock();
	imp->m_work.clear();
	imp->m_work_lock.unlock();

}

void ThreadPool::AddWork(ThreadPoolWork *w) {
	imp->m_work_lock.lock();
	imp->m_work.push_back(w);
	imp->m_work_lock.unlock();
	imp->m_cv.notify_one();
}

void ThreadPool::Process() {
	std::lock_guard<std::mutex> lock(imp->m_work_finished_lock);

	while(!imp->m_work_finished.empty()) {
		auto front = imp->m_work_finished.front();

		front->Finished();
		delete front;
		imp->m_work_finished.pop_front();
	}
}

bool ThreadPool::GetWork(ThreadPoolWork **work) {
	std::lock_guard<std::mutex> lock(imp->m_work_lock);

	if(imp->m_work.empty()) {
		*work = nullptr;
		return false;
	}

	*work = imp->m_work.front();
	imp->m_work.pop_front();
	return true;
}

void ThreadPool::AddWorkFinished(ThreadPoolWork *w) {
	std::lock_guard<std::mutex> lock(imp->m_work_finished_lock);
	imp->m_work_finished.push_back(w);
}

void ThreadPool::WorkerThread() {
	ThreadPoolWork *work = nullptr;
	for(;;) {
		std::unique_lock<std::mutex> lck(imp->m_lock);
		while(imp->m_running && !GetWork(&work)) {
			imp->m_cv.wait(lck);
		}

		if(!imp->m_running) {
			return;
		}

		lck.unlock();
		imp->m_cv.notify_one();

		work->Run();
		AddWorkFinished(work);
	}
}
