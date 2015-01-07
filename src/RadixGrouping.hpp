#ifndef HAVE_RADIX_GROUPING_HPP
#define HAVE_RADIX_GROUPING_HPP

#include "GroupingAlgorithm.hpp"

#define NUMBER_OF_TASKS 8
#define NUMBER_OF_RELEVANT_BITS 4

class RadixGrouping : public GroupingAlgorithm {
public:
	virtual PositionListPtr groupBy(const std::vector<ColumnPtr>& columns) const;
};

#endif
