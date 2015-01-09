#ifndef HAVE_THREAD_POOL_HPP
#define HAVE_THREAD_POOL_HPP

#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <future>

#define NUMBER_OF_THREADS 4

using Job = std::function<void()>;

class ThreadPool {
public:
	ThreadPool(std::size_t size);
	~ThreadPool();
	
	template<class Function>
	auto
	addJob(Function&& fn) -> std::shared_future<typename std::result_of<Function()>::type >
	{
		using ResultType = typename std::result_of<Function()>::type;
		auto task = std::make_shared<std::packaged_task<ResultType()> >(std::forward<Function>(fn));
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_jobs.emplace([task](){ (*task)(); });
		}
		_cv.notify_one();
		return task->get_future().share();
	}
	
private:
	ThreadPool(const ThreadPool& other) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	
	std::vector<std::thread> _threads;
	std::mutex _mutex;
	std::condition_variable _cv;
	std::queue<Job> _jobs;
	bool _stop;
};

#endif
