#include "RadixGrouping.hpp"

#include "Matrix.hpp"
#include "ThreadPool.hpp"

void call() {
	std::cout << "fnord" << std::endl;
}

PositionListPtr
RadixGrouping::groupBy(const std::vector<ColumnPtr> columns) const {
	if (columns.empty()) {
		return PositionListPtr();
	}
		
	// TODO: extend to multiple columns
	ColumnPtr column = columns[0];
	size_t column_size = column->size();
	
	ThreadPool pool(NUMBER_OF_THREADS);
	
	size_t hist_size = (1 << NUMBER_OF_RELEVANT_BITS);
	Matrix histogram(NUMBER_OF_TASKS, hist_size);
	
	// Step 1: add histogram tasks
	for (size_t i = 0; i < NUMBER_OF_TASKS; ++i) {
		size_t lower = 0;
		size_t upper = 0;
		
		boost::shared_future<void> result(pool.addTask(&call));
		result.wait();
	}
	
	return NULL;
}
