#ifndef HAVE_THREAD_POOL_HPP
#define HAVE_THREAD_POOL_HPP

#include <vector>
#include <queue>

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

template<class ResultType>
void wrapper(boost::shared_ptr<boost::packaged_task<ResultType> > task) {
	(*task)();
}

class ThreadPool {
public:
	ThreadPool(size_t size);
	~ThreadPool();
	
	template<class ResultType, class FunctionType>
	boost::shared_future<ResultType>
	addJob(FunctionType fn) {
		boost::shared_ptr<boost::packaged_task<ResultType> > task
			= boost::make_shared<boost::packaged_task<ResultType> >(fn);
		
		{
			boost::lock_guard<boost::mutex> lock(_mutex);
			_jobs.push(boost::bind(&wrapper<ResultType>, task));
		}
	
		_cv.notify_one();
		
		return boost::shared_future<ResultType>(task->get_future());
	}
	
private:
	size_t _size;
	std::vector<boost::thread*> _threads;
	bool _stop;
	boost::mutex _mutex;
	boost::condition_variable _cv;
	std::queue<boost::function<void(void)> > _jobs;
	
	void worker();
	bool check_wait_cond() const;
};

#endif
