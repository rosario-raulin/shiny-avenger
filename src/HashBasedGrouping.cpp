#include "HashBasedGrouping.hpp"
#include "RadixGrouping.hpp"

#include <unordered_map>
#include <vector>
#include <memory>

#include <iostream>

PositionListPtr
HashBasedGrouping::groupBy(const std::vector<ColumnPtr>& columns) {
	if (columns.empty()) return PositionListPtr();
	
	// make a copy of the column
	auto& column = *(columns[0]);
	PositionListPtr columnPtr = std::shared_ptr<std::size_t>(new std::size_t[column.size()], std::default_delete<std::size_t[]>());
		
	std::unordered_map<std::size_t, std::vector<std::size_t> >  positionMap;
	std::size_t i = 0;
	for (const auto& value : column) {
		positionMap[value].emplace_back(i++);
	}
	
	std::size_t j = 0;
	for (const auto& pos : positionMap) {
		VALUES[pos.first] = pos.second.size();
		for (const auto& index : pos.second) {
			(columnPtr.get())[j++] = column[index];
		}
	}
	
	return columnPtr;
}