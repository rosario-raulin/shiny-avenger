#include "SharedHashGrouping.hpp"

#include "Matrix.hpp"
#include "ThreadPool.hpp"

#include <algorithm> // min
#include <cmath>  // ceil
#include <memory>

#define NUMBER_OF_RELEVANT_BITS 4

struct mysort {
	Column& column;
	mysort(Column& column) : column(column) {}

	bool operator()(const std::size_t tid1, const std::size_t tid2) const {
		return column[tid1] < column[tid2];
	}
};

// Implementing Class HashTable
// create new array of buckets of size n
// each bucket is a list of TIDs
HashTable::HashTable() : buckets(new std::list<size_t>[(1 << NUMBER_OF_RELEVANT_BITS)]) {
}

HashTable::HashTable(const HashTable& other) : buckets(new std::list<size_t>[(1 << NUMBER_OF_RELEVANT_BITS)]) {
	std::size_t size = 1 << NUMBER_OF_RELEVANT_BITS;
	std::copy(other.buckets, other.buckets + size, buckets);
}

HashTable::~HashTable() {
	delete[] buckets;
}

void HashTable::insert(size_t key, size_t value) {
	size_t index = hash(key);
	buckets[index].push_back(value);
}

size_t HashTable::hash(size_t n) const {
	return n & ((1 << NUMBER_OF_RELEVANT_BITS) - 1);
}

void HashTable::merge(std::shared_ptr<HashTable> table, Column& column) {
	size_t n = 1 << NUMBER_OF_RELEVANT_BITS;
	
	for (std::size_t i = 0; i < n; ++i) {
		buckets[i].splice(buckets[i].end(), table->buckets[i]);
	}
}

void HashTable::sort(Column& column){
	size_t n = 1 << NUMBER_OF_RELEVANT_BITS;

	for(size_t i = 0; i < n; i++) {
		buckets[i].sort(mysort(column));
	}
}

std::shared_ptr<HashTable> buildHashTable(size_t lower, size_t upper, Column& column) {
	std::shared_ptr<HashTable> table(new HashTable());
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
	ColumnPtr columnPtr = columns[0];
	Column& column = *columnPtr;
	size_t column_size = column.size();
	
	ThreadPool pool(NUMBER_OF_THREADS);

	// Alpha is  the factor of spltting. Alpha=1 => One Task for Every Thread
	size_t number_of_tasks = NUMBER_OF_THREADS * ALPHA;

	size_t chunk_size = column_size / number_of_tasks;
	size_t diff_cases = column_size % number_of_tasks;
	size_t regular_cases = number_of_tasks - diff_cases;
	
	using ResultType = std::shared_future<std::shared_ptr<HashTable> >;
	std::vector<ResultType> results;
	
	for (auto i = 0; i < regular_cases; ++i) {
		std::size_t lower = chunk_size * i;
		std::size_t upper = lower + chunk_size;
		
		// do buildHashTable
		auto job = [&, lower, upper]() { return buildHashTable(lower, upper, column); };
		results.emplace_back(pool.addJob(job));
	}
	
	std::size_t lower = chunk_size * regular_cases;
	std::size_t upper;
	for (auto i = 0; i < diff_cases; ++i) {
		upper = lower + chunk_size + 1;

		// do buildHashTable
		auto job = [&, lower, upper]() { return buildHashTable(lower, upper, column); };

		results.emplace_back(pool.addJob(job));
		
		lower = upper;
	}

	// take the first table:
	// merge the table with all the others
	size_t all_cases = diff_cases + regular_cases;

	auto merged_table = results[0].get();
	for(size_t i = 1; i < all_cases; i++) {
		auto p = results[i].get();
		merged_table->merge(p, column);
	}
	
	std::vector<std::shared_future<void> > sortingResults;
	for (std::size_t i = 0; i < (1 << NUMBER_OF_RELEVANT_BITS); ++i) {
		auto& bucket = merged_table->buckets[i];
		auto sortJob = [&bucket, &column](){ bucket.sort(mysort(column)); };
		sortingResults.emplace_back(pool.addJob(sortJob));
	}
	
	//merged_table->sort(column);
	std::for_each(sortingResults.begin(), sortingResults.end(), [](std::shared_future<void>& res) { res.wait(); });

	auto indices = std::shared_ptr<std::size_t>(new size_t[column_size], std::default_delete<std::size_t[]>());
	size_t* indexPtr = indices.get();

	size_t n = 1 << NUMBER_OF_RELEVANT_BITS;

	size_t j = 0;
	for(size_t i = 0; i < n; i++) {
		for (auto index : merged_table->buckets[i]) {
			indexPtr[j++] = index;
		}
	}
	
	return indices;
}
