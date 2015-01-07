#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t size) : _stop(false){
	_threads.resize(size);
	for (size_t i = 0; i < size; ++i) {
		_threads[i] = new boost::thread(boost::bind(&ThreadPool::worker, this));
	}
}

ThreadPool::~ThreadPool() {
	{
		boost::unique_lock<boost::mutex> lock(_mutex);
		_stop = true;
	}
	_cv.notify_all();
	
	for (size_t i = 0; i < _threads.size(); ++i) {
		_threads[i]->join();
		delete _threads[i];
	}
}

void
ThreadPool::worker() {
	while (true) {
		boost::function<void(void)> task;
		
		{
			boost::unique_lock<boost::mutex> lock(_mutex);
			_cv.wait(lock, boost::bind(&ThreadPool::check_wait_cond, this));
			if (_stop && _jobs.empty()) {
				return;
			}
			
			task = _jobs.front();
			_jobs.pop();
		}
		
		task();
	}
}

bool
ThreadPool::check_wait_cond() const {
	return _stop || !_jobs.empty();
}
