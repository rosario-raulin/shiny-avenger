#include "GroupingAlgorithm.hpp"
#include "RadixGrouping.hpp"

#include <vector>

int main() {
	GroupingAlgorithm* g = new RadixGrouping();
	
	Column fnord;
	for (size_t i = 0; i < 999; ++i) {
		fnord.push_back(i);
	}
	
	std::vector<ColumnPtr> v;
	v.push_back(&fnord);
	g->groupBy(v);
	
	return 0;
}
