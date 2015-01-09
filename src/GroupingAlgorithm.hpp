#ifndef HAVE_GROUPING_ALGORITHM_HPP
#define HAVE_GROUPING_ALGORITHM_HPP

#include "CommonTypes.hpp"

class IGroupingAlgorithm {
public:
	virtual ~IGroupingAlgorithm() {}
	virtual PositionListPtr groupBy(const std::vector<ColumnPtr>& columns) = 0;
};

#endif