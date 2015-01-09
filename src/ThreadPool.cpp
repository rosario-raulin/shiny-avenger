#include "ThreadPool.hpp"

#include <algorithm>

ThreadPool::ThreadPool(std::size_t size) : _stop(false) {
	auto worker = [this]() {
		while (true) {
			Job job;
			{
				std::unique_lock<std::mutex> lock(this->_mutex);
				_cv.wait(lock, [this]() { return this->_stop || !_jobs.empty(); });
				if (this->_stop && this->_jobs.empty()) {
					return;
				}
				job = std::move(this->_jobs.front());
				this->_jobs.pop();
			}
			job();
		}
	};
	
	for (std::size_t i = 0; i < size; ++i) {
		_threads.emplace_back(std::thread(worker));
	}
}

ThreadPool::~ThreadPool() {
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_stop = true;
	}
	_cv.notify_all();
	std::for_each(_threads.begin(), _threads.end(), [](std::thread& ptr) { ptr.join(); });
}
