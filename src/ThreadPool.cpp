#include "ThreadPool.hpp"

#include <iostream>

void worker() {
	std::cout << "Fnord!" << std::endl;
}

ThreadPool::ThreadPool(size_t size) :
	_size(size),
	_threads(new boost::thread*[size]) {
	for (size_t i = 0; i < _size; ++i) {
		_threads[i] = new boost::thread(worker);
	}
}

ThreadPool::~ThreadPool() {
	for (size_t i = 0; i < _size; ++i) {
		_threads[i]->join();
		delete _threads[i];
	}
	delete[] _threads;
}

boost::shared_future<void>
ThreadPool::addTask(boost::function<void> function) {
	
}
