#include "RadixGrouping.hpp"

#include "Matrix.hpp"
#include "ThreadPool.hpp"

#include <algorithm>

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
	
	// Ceiling divison of column_size / NUMBER_OF_TASKS
	size_t chunk_size = 1 + ((column_size - 1) / NUMBER_OF_TASKS);
	for (size_t i = 0; i < NUMBER_OF_TASKS; ++i) {
		size_t lower = chunk_size * i;
		size_t upper = std::min(lower + chunk_size, column_size);
		
		// pool.addTask();
	}
	
	return NULL;
}
