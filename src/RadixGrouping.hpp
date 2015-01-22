#ifndef HAVE_RADIX_GROUPING_HPP
#define HAVE_RADIX_GROUPING_HPP

#include "GroupingAlgorithm.hpp"

class RadixGrouping : public IGroupingAlgorithm {
public:
	RadixGrouping(size_t _number_of_threads,
		size_t number_of_tasks,
		size_t number_of_bits);
	PositionListPtr groupBy(const std::vector<ColumnPtr>& columns);

	size_t number_of_bits() const {
		return _number_of_bits;
	}

	size_t number_of_tasks() const {
		return _number_of_tasks;
	}

	size_t hash(size_t value) const {
		return value & ((1 << _number_of_bits) - 1);
	}

private:
	const size_t _number_of_threads;
	const size_t _number_of_tasks;
	const size_t _number_of_bits;
};

#endif
