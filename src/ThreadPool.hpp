#ifndef HAVE_THREAD_POOL_HPP
#define HAVE_THREAD_POOL_HPP

#include <cstddef>
#include <vector>

#include <boost/thread/thread.hpp>

class ThreadPool {
public:
	ThreadPool(size_t size);
	~ThreadPool();
	
	boost::shared_future<void>
	ThreadPool::addTask(boost::function<void> function);
	
private:
	size_t _size;
	boost::thread** _threads;
};

#endif
