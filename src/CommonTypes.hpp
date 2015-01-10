#ifndef HAVE_COMMON_TYPES_HPP
#define HAVE_COMMON_TYPES_HPP

#include <vector>
#include <memory>

using PositionList = std::size_t;
using PositionListPtr = std::shared_ptr<PositionList>;
using Column = std::vector<std::size_t>;
using ColumnPtr = Column*;

#define TESTSIZE 100
#define CARDINALITY 10000

#endif