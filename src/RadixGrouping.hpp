#ifndef HAVE_RADIX_GROUPING_HPP
#define HAVE_RADIX_GROUPING_HPP

#include "GroupingAlgorithm.hpp"

#define NUMBER_OF_TASKS 16
#define NUMBER_OF_RELEVANT_BITS 11

extern std::array<std::size_t, CARDINALITY> VALUES;

class RadixGrouping : public IGroupingAlgorithm {
public:
	PositionListPtr groupBy(const std::vector<ColumnPtr>& columns);
};

#endif
