#ifndef HAVE_INDEPENDENT_HASH_GROUPING_HPP
#define HAVE_INDEPENDENT_HASH_GROUPING_HPP

#include "GroupingAlgorithm.hpp"

class IndependentHashGrouping : public IGroupingAlgorithm {
public:
  IndependentHashGrouping(
    size_t number_of_threads,
    size_t number_of_tasks,
    size_t number_of_bits_for_hashing);

  virtual PositionListPtr groupBy(const std::vector<ColumnPtr>& columns);

private:
  const size_t _number_of_threads;
  const size_t _number_of_tasks;
  const size_t _number_of_bits_for_hashing;

  size_t _table_size() const {
    return (1 << _number_of_bits_for_hashing);
  }
};

#endif
