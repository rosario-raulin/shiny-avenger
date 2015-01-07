#include "GroupingAlgorithm.hpp"
#include "RadixGrouping.hpp"

#include <vector>

#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

#define TESTSIZE 128

static Column generateColumn() {
	Column column;
	column.resize(TESTSIZE);
	
	boost::random::mt19937 rng;
	boost::random::uniform_int_distribution<> gen(0,10);
	
	for (size_t i = 0; i < TESTSIZE; ++i) {
		column[i] = gen(rng);
	}
	
	return column;
}

int main() {
	GroupingAlgorithm* g = new RadixGrouping();
	
	Column column = generateColumn();
	
	// for (size_t i = 0; i < column.size(); ++i) {
	// 	std::cout << column[i] << std::endl;
	// }
	
	std::vector<ColumnPtr> columns;
	columns.push_back(&column);
	
	g->groupBy(columns);
	
	return 0;
}
