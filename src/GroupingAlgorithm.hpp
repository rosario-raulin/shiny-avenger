#ifndef HAVE_GROUPING_ALGORITHM_HPP
#define HAVE_GROUPING_ALGORITHM_HPP

#include "CommonTypes.hpp"
#include <vector>

/*
 * This class defines an abstract interface to grouping algorithms.
 */
class GroupingAlgorithm {
public:
	virtual PositionListPtr groupBy(const std::vector<ColumnPtr>& columns) const = 0;
};

#endif
