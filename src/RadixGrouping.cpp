#include "RadixGrouping.hpp"

#include "Matrix.hpp"
#include "ThreadPool.hpp"

#include <algorithm>

static inline
size_t hash(size_t x) {
	return x & ((1 << NUMBER_OF_RELEVANT_BITS) - 1);
}

struct histjob {
	histjob(ColumnPtr column, size_t index, size_t lower, size_t upper, Matrix& histogram, size_t* output) :
		column(column),
		index(index),
		lower(lower),
		upper(upper),
		histogram(histogram) {
	}
	
	void operator()() {
		for (size_t i = lower; i < upper; ++i) {
			size_t pos = hash((*column)[i]);
			++histogram.get(index, pos);
		}
		
		size_t table_size = (1 << NUMBER_OF_RELEVANT_BITS);
		size_t prefix_sum = 0;
		for (size_t i = 0; i < table_size; ++i) {
			prefix_sum += histogram.get(index, i);
			histogram.set(index, i, prefix_sum);
		}
	}
	
private:
	ColumnPtr column;
	size_t index;
	size_t lower;
	size_t upper;
	Matrix& histogram;
};

struct partjob {
	partjob(ColumnPtr column, size_t index, size_t lower, size_t upper, Matrix& histogram, size_t* output) :
		column(column),
		index(index),
		lower(lower),
		upper(upper),
		histogram(histogram),
		output(output) {
	}
	
	void operator()() {
		size_t table_size = (1 << NUMBER_OF_RELEVANT_BITS);
		
		size_t* dest = new size_t[table_size];
		memset(dest, 0, sizeof(size_t)*table_size);
		
		for (size_t i = 0; i < index; ++i) {
			for (size_t j = 0; j < table_size; ++j) {
				dest[j] += histogram.get(i, j);
			}
		}
		
		for (size_t i = 0; i < NUMBER_OF_TASKS; ++i) {
			for (size_t j = 1; j < table_size; ++j) {
				dest[j] += histogram.get(i, j-1);
			}
		}
		
		for (size_t i = lower; i < upper; ++i) {
			size_t pos = hash((*column)[i]);
			output[dest[pos]] = i;
			++dest[pos];
		}
		
		delete[] dest;
	}

private:
	ColumnPtr column;
	size_t index;
	size_t lower;
	size_t upper;
	Matrix& histogram;
	size_t* output;
};

typedef boost::shared_future<void> FutureResultType;

boost::mutex mutex;

template<class JobType>
void startJobs(ThreadPool& pool, ColumnPtr column, Matrix& histogram, size_t* output) {
	size_t column_size = column->size();
	std::vector<FutureResultType> jobs;
	jobs.resize(NUMBER_OF_TASKS);
	
	// Ceiling divison of column_size / NUMBER_OF_TASKS
	size_t chunk_size = 1 + ((column_size - 1) / NUMBER_OF_TASKS);
	for (size_t i = 0; i < NUMBER_OF_TASKS; ++i) {
		size_t lower = chunk_size * i;
		size_t upper = std::min(lower + chunk_size, column_size);
			
		jobs[i] = pool.addJob<void>(JobType(column, i, lower, upper, histogram, output));
	}
	
	boost::wait_for_all(jobs.begin(), jobs.end());
}

PositionListPtr
RadixGrouping::groupBy(const std::vector<ColumnPtr>& columns) const {
	if (columns.empty()) {
		return PositionListPtr();
	}
	
	// TODO: extend to multiple columns
	ColumnPtr column = columns[0];
	
	size_t hist_size = (1 << NUMBER_OF_RELEVANT_BITS);
	Matrix histogram(NUMBER_OF_TASKS, hist_size);
	ThreadPool pool(NUMBER_OF_THREADS);
	
	// Step 1: add histogram tasks
	startJobs<histjob>(pool, column, histogram, NULL);
	// Step 2: build partitions using histogram
	size_t* output = new size_t[column->size()];
	startJobs<partjob>(pool, column, histogram, output);
	
	std::cout << "output:" << std::endl;
	for (size_t i = 0; i < hist_size; ++i) {
		std::cout << output[i] << std::endl;
	}

	delete[] output;

	return NULL;
}
