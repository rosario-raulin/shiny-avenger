#ifndef HAVE_SHARED_HASH_GROUPING_HPP
#define HAVE_SHARED_HASH_GROUPING_HPP

#include "GroupingAlgorithm.hpp"

#include <list>

#define ALPHA 4

class SharedHashGrouping : public IGroupingAlgorithm {
public:
	PositionListPtr groupBy(const std::vector<ColumnPtr>& columns);
};

class HashTable {
public:
	HashTable();
	HashTable(const HashTable& other);
	~HashTable();

	void insert(size_t key, size_t value);
	size_t hash(size_t n) const;
	void merge(std::shared_ptr<HashTable> table, Column& column);
	void sort(Column& column);
	
	std::list<size_t> *buckets;
};

#endif
