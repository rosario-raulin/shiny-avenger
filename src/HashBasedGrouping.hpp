#ifndef HAVE_HASH_BASED_GROUPING_HPP
#define HAVE_HASH_BASED_GROUPING_HPP

#include "GroupingAlgorithm.hpp"

class HashBasedGrouping : public IGroupingAlgorithm {
public:
	PositionListPtr groupBy(const std::vector<ColumnPtr>& columns);
};

#endif