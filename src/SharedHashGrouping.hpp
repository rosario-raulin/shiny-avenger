#ifndef HAVE_SHARED_HASH_GROUPING_HPP
#define HAVE_SHARED_HASH_GROUPING_HPP

#include "GroupingAlgorithm.hpp"

#include <list>


#define ALPHA 16
#define NUMBER_OF_RELEVANT_BITS 11

class SharedHashGrouping : public IGroupingAlgorithm {
public:
	PositionListPtr groupBy(const std::vector<ColumnPtr>& columns);
};

class HashTable {
public:
	HashTable();
	~HashTable();

	void insert(size_t key, size_t value);

	size_t hash(size_t n);

	std::vector<size_t> get(size_t key);

	void merge(HashTable& table, Column& column);

	void sort(Column& column);
	
	std::list<size_t> *buckets;

};

HashTable buildHashTable(size_t lower, size_t upper, size_t n, Column& column);

#endif
