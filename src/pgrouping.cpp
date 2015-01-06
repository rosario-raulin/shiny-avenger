#include "GroupingAlgorithm.hpp"
#include "RadixGrouping.hpp"

#include <vector>

int main() {
	GroupingAlgorithm* g = new RadixGrouping();
	
	Column fnord;
	std::vector<ColumnPtr> v;
	v.push_back(&fnord);
	g->groupBy(v);
	
	return 0;
}
