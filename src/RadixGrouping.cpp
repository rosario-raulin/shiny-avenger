#include "RadixGrouping.hpp"
#include "ThreadPool.hpp"
#include "Matrix.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>

using Histogram = Matrix<std::size_t>;

static inline std::size_t
hash(std::size_t value) {
	return value & ((1 << NUMBER_OF_RELEVANT_BITS) - 1);
}

static void
indexJob(std::size_t id, Column& column, Histogram& histogram, std::size_t lower, std::size_t upper, std::shared_ptr<std::size_t> reordered, std::shared_ptr<std::size_t> indices, std::shared_ptr<std::size_t> destOutput) {
	auto table_size = (1 << NUMBER_OF_RELEVANT_BITS);
	
	std::unique_ptr<std::size_t[]> dest(new std::size_t[table_size]());
	
	for (auto i = 0; i < id; ++i) {
		for (auto j = 0; j < table_size; ++j) {
			dest[j] += histogram.get(i, j);
		}
	}
	
	for (auto i = id; i < NUMBER_OF_TASKS; ++i) {
		for (auto j = 1; j < table_size; ++j) {
			dest[j] += histogram.get(i, j-1);
		}
	}
	
	if (id == 0) {
		std::copy(dest.get(), dest.get() + table_size, destOutput.get());
	}
	
	for (auto i = lower; i < upper; ++i) {
		auto index = hash(column[i]);
		(indices.get())[dest[index]] = i;
		(reordered.get())[dest[index]] = column[i];
		++dest[index];
	}
}

static void
histJob(std::size_t id, Column& column, Histogram& histogram, std::size_t lower, std::size_t upper) {
	for (auto i = lower; i < upper; ++i) {
		auto index = hash(column[i]);
		++histogram.get(id, index);
	}
		
	auto table_size = (1 << NUMBER_OF_RELEVANT_BITS);
	auto prefix_sum = 0;
	for (auto i = 0; i < table_size; ++i) {
		prefix_sum += histogram.get(id, i);
		histogram.set(id, i, prefix_sum);
	}	
}

template<class Function, class... Args>
void startAndEndJobs(Function fn, ThreadPool& pool, Column& column, Histogram& histogram, Args&&... args) {
	std::size_t chunk_size = column.size() / NUMBER_OF_TASKS;
	std::size_t diff_cases = column.size() % NUMBER_OF_TASKS;
	std::size_t regular_cases = NUMBER_OF_TASKS - diff_cases;
	
	using ResultType = std::shared_future<void>;
	std::vector<ResultType> results;
	
	for (auto i = 0; i < regular_cases; ++i) {
		std::size_t lower = chunk_size * i;
		std::size_t upper = lower + chunk_size;
		
		auto job = [&, i, lower, upper]() { fn(i, column, histogram, lower, upper, args...); };
		results.emplace_back(pool.addJob(job));
	}
	
	std::size_t lower = chunk_size * regular_cases;
	std::size_t upper;
	for (auto i = 0; i < diff_cases; ++i) {
		upper = lower + chunk_size + 1;
		auto job = [&, i, regular_cases, lower, upper]() {
			fn(i + regular_cases, column, histogram, lower, upper, std::forward<Args>(args)...);
		};
		results.emplace_back(pool.addJob(job));
		
		lower = upper;
	}
	
	std::for_each(results.begin(), results.end(), [](ResultType& r) { r.wait(); });
}

void localGrouping(std::shared_ptr<std::size_t> reordered, std::shared_ptr<std::size_t> indices, std::size_t lower, std::size_t upper) {
	std::unordered_map<std::size_t, std::vector<std::size_t> > groups;
	std::size_t* values = reordered.get();
	std::size_t* indexMap = indices.get();
	
	for (auto i = lower; i < upper; ++i) {
		groups[values[i]].emplace_back(indexMap[i]);
	}
	
	auto i = lower;
	for (const auto& pair : groups) {
		for (const auto& index : pair.second) {
			indexMap[i++] = index;
		}
	}
}

PositionListPtr
RadixGrouping::groupBy(const std::vector<ColumnPtr>& columns) {
	auto table_size = (1 << NUMBER_OF_RELEVANT_BITS);

	Column& column = *(columns[0]);
	Histogram histogram(NUMBER_OF_TASKS, table_size);
	ThreadPool pool(NUMBER_OF_THREADS);

	// Step 1: build local histograms
	startAndEndJobs(histJob, pool, column, histogram);
	
	// little workaround: C++11 doesn't have shared_ptr arrays, so we need a custom deallocator
	std::default_delete<std::size_t[]> array_deleter;
	std::shared_ptr<std::size_t> reordered(new std::size_t[column.size()], array_deleter);
	std::shared_ptr<std::size_t> indices(new std::size_t[column.size()], array_deleter); 
	
	// Step 2: calculate start/end indicies
	std::shared_ptr<std::size_t> destOutput(new std::size_t[table_size], array_deleter);
	startAndEndJobs(indexJob, pool, column, histogram, reordered, indices, destOutput);
	
	// Step 3: for each partition, create local grouping jobs
	using ResultType = std::shared_future<void>;
	std::vector<ResultType> results;
	std::size_t* offsets = destOutput.get();
	
	for (auto i = 0; i < table_size-1; ++i) {
		auto lower = offsets[i];
		auto upper = offsets[i+1];
		
		auto job = [reordered, indices, lower, upper]() { localGrouping(reordered, indices, lower, upper); };
		results.emplace_back(pool.addJob(job));
	}
	
	auto last_part_start = offsets[table_size-1];
	auto last_part_end = column.size();
	auto job = [reordered, indices, last_part_start, last_part_end]() { localGrouping(reordered, indices, last_part_start, last_part_end); };
	results.emplace_back(pool.addJob(job));
	
	std::for_each(results.begin(), results.end(), [](ResultType& r) { r.wait(); });
	
	return indices;
}
