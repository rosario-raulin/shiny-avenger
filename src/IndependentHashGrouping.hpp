#ifndef HAVE_INDEPENDENT_HASH_GROUPING_HPP
#define HAVE_INDEPENDENT_HASH_GROUPING_HPP

#include "GroupingAlgorithm.hpp"

class IndependentHashGrouping : public IGroupingAlgorithm {
public:
  virtual PositionListPtr groupBy(const std::vector<ColumnPtr>& columns);
};

#endif
