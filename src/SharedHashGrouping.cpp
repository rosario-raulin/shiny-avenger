#include "SharedHashGrouping.hpp"

#include "Matrix.hpp"
#include "ThreadPool.hpp"

#include <algorithm> // min

#include <math.h>  // ceil

#include <memory>

struct mysort {
	Column& column;
	mysort(Column& column) : column(column) {}

	bool operator()(const std::size_t tid1, const std::size_t tid2) const {
		return column[tid1] < column[tid2];
	}
};

// Implementing Class HashTable
HashTable::HashTable(){
	 size_t n = 1 << NUMBER_OF_RELEVANT_BITS;

	// create new array of buckets of size n
	// each bucket is a list of TIDs

	buckets = new std::list<size_t>[n];
}

HashTable::~HashTable() {
	delete[] buckets;
}

void HashTable::insert(size_t key, size_t value) {
	size_t index = hash(key);
	buckets[index].push_back(value);
}

size_t HashTable::hash(size_t n) {
	return n & ((1 << NUMBER_OF_RELEVANT_BITS) - 1);
}

void HashTable::merge(HashTable& table, Column& column) {
	size_t n = 1 << NUMBER_OF_RELEVANT_BITS;
	for (size_t i = 0; i < n; i++) {
		std::list<size_t>& first = buckets[i];
		std::list<size_t>& second = table.buckets[i];

		first.merge(second, mysort(column));
	}
}

void HashTable::sort(Column& column){
	size_t n = 1 << NUMBER_OF_RELEVANT_BITS;

	for(size_t i = 0; i < n; i++) {
		buckets[i].sort(mysort(column));
	}
}



HashTable* buildHashTable(size_t lower, size_t upper, Column& column) {
	HashTable* table = new HashTable();
	for(size_t i = lower; i < upper; i++ ) {

		// key = grouping value
		size_t key = column[i];

		// i = TID
		table->insert(key, i);
	}
	table->sort(column);
	return table;
}

PositionListPtr
SharedHashGrouping::groupBy(const std::vector<ColumnPtr>& columns) {
	if (columns.empty()) {
		return PositionListPtr();
	}
		
	// TODO: extend to multiple columns
	ColumnPtr column = columns[0];
	size_t column_size = column->size();
	
	ThreadPool pool(NUMBER_OF_THREADS);

	// Alpha is  the factor of spltting. Alpha=1 => One Task for Every Thread
	size_t number_of_tasks = NUMBER_OF_THREADS * ALPHA;

	size_t chunk_size = column_size / number_of_tasks;
	size_t diff_cases = column_size % number_of_tasks;
	size_t regular_cases = number_of_tasks - diff_cases;
	
	using ResultType = std::shared_future<HashTable*>;
	std::vector<ResultType> results;
	
	for (auto i = 0; i < regular_cases; ++i) {
		std::size_t lower = chunk_size * i;
		std::size_t upper = lower + chunk_size;
		
		// do buildHashTable
		auto job = [&, lower, upper]() { return buildHashTable(lower, upper, *column); };
		results.emplace_back(pool.addJob(job));
	}
	
	std::size_t lower = chunk_size * regular_cases;
	std::size_t upper;
	for (auto i = 0; i < diff_cases; ++i) {
		upper = lower + chunk_size + 1;

		// do buildHashTable
		auto job = [&, lower, upper]() { return buildHashTable(lower, upper, *column); };

		results.emplace_back(pool.addJob(job));
		
		lower = upper;
	}
	
	std::for_each(results.begin(), results.end(), [](ResultType& r) { r.wait(); });


	// take the first table:
	// merge the table with all the others
	size_t all_cases = diff_cases + regular_cases;

	HashTable& merged_table = *results[0].get();
	for(size_t i = 1; i < all_cases; i++) {

		HashTable* p = results[i].get();

		merged_table.merge(*p, *column);

		delete p;
	}

	auto indices = std::shared_ptr<std::size_t>(new size_t[column_size], std::default_delete<std::size_t[]>());
	size_t* indexPtr = indices.get();

	size_t n = 1 << NUMBER_OF_RELEVANT_BITS;


	size_t j = 0;
	for(size_t i = 0; i < n; i++) {
		for (auto index : merged_table.buckets[i]) {
			indexPtr[j++] = index;
		}
	}
	
	return indices;
}
