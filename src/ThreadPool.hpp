#ifndef HAVE_THREAD_POOL_HPP
#define HAVE_THREAD_POOL_HPP

#include <cstddef>
#include <queue>

#include <boost/thread.hpp>

typedef boost::packaged_task<void> TaskType;

class ThreadPool {
public:
	ThreadPool(size_t size);
	~ThreadPool();
	
	template<class Function>
	boost::shared_future<void>
	addTask(Function fn) {
		TaskType* task = new TaskType(fn);
		boost::shared_future<void> future(task->get_future());
		
		{
			boost::unique_lock<boost::mutex> lock(_mutex);
			_tasks.push(task);
		}
		
		_cv.notify_one();
		return future;
	}
	
private:
	size_t _size;
	boost::thread** _threads;
	std::queue<TaskType*> _tasks;
	bool _stop;
	boost::condition_variable _cv;
	boost::mutex _mutex;
	
	
	bool check_cond() const;
	void worker();
};

#endif
