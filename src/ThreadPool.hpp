#ifndef HAVE_THREAD_POOL_HPP
#define HAVE_THREAD_POOL_HPP

#include <cstddef>
#include <vector>

#include <boost/thread.hpp>

class ThreadPool {
public:
	ThreadPool(size_t size);
	~ThreadPool();
	
	// template<class Function>
	// boost::shared_future<void>
	// addTask(Function fn) {
	// }
	
private:
	size_t _size;
	boost::thread** _threads;
};

#endif
