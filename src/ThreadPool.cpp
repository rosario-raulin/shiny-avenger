#include "ThreadPool.hpp"

#include <iostream>

void
ThreadPool::worker() {
	while (true) {
		TaskType* task;
		
		{
			boost::unique_lock<boost::mutex> lock(_mutex);
			_cv.wait(lock, boost::bind(&ThreadPool::check_cond, this));
			if (_stop && _tasks.empty()) {
				return;
			}
			
			task = _tasks.front();
			_tasks.pop();
		}
		
		(*task)();
		delete task;
	}
}

ThreadPool::ThreadPool(size_t size) :
	_size(size),
	_threads(new boost::thread*[size]),
	_stop(false) {
	for (size_t i = 0; i < _size; ++i) {
		_threads[i] = new boost::thread(boost::bind(&ThreadPool::worker, this));
	}
}

ThreadPool::~ThreadPool() {
	{
		boost::unique_lock<boost::mutex> lock(_mutex);
		_stop = true;
	}
	_cv.notify_all();
	for (size_t i = 0; i < _size; ++i) {
		_threads[i]->join();
		delete _threads[i];
	}
	delete[] _threads;
}

bool
ThreadPool::check_cond() const {
	return _stop || !_tasks.empty();
}
