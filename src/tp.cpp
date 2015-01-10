#include "RadixGrouping.hpp"
#include "HashBasedGrouping.hpp"

#include <vector>
#include <iostream>
#include <memory>
#include <random>
#include <chrono>
#include <functional>

std::shared_ptr<Column> make_column(std::size_t size, std::size_t cardinality) {
	auto columnPtr = std::make_shared<Column>();
	Column& column = *columnPtr;
	column.resize(size);
	
	std::mt19937 generator;
	std::uniform_int_distribution<> dist(1, cardinality);
	
	for (auto i = 0; i < size; ++i) {
		column[i] = dist(generator);
	}
	
	return columnPtr;
}

void print_column(std::shared_ptr<Column> columnPtr) {
	Column& column = *columnPtr;
	for (auto i = 0; i < column.size(); ++i) {
		std::cout << i << "\t" << column[i] << std::endl;
	}
}

template<class TimeType = std::chrono::milliseconds, class Functor, class ...Args>
typename TimeType::rep measureTime(Functor function, Args&&... args) {
	auto start = std::chrono::system_clock::now();
	function(std::forward<Args>(args)...);
	auto duration = std::chrono::duration_cast<TimeType>(std::chrono::system_clock::now() - start);
	return duration.count();
}

int main() {
	auto algo = std::make_shared<RadixGrouping>();
	auto column = make_column(TESTSIZE, CARDINALITY);
	// print_column(column);
	std::vector<ColumnPtr> columns;
	columns.emplace_back(column.get());
	
	auto fn = [algo](const std::vector<ColumnPtr>& columns) { algo->groupBy(columns); };
	auto duration = measureTime(fn, columns);
	std::cout << "Grouping took " << duration / 1000.0 << " seconds." << std::endl;
	//auto ptr = algo.groupBy(columns);
	
	for (auto i = 0; i < CARDINALITY; ++i) {
		std::cout << i << " maps to " << VALUES[i] << " values." << std::endl;
	}
	
	// auto printer = [](const std::size_t& x) { std::cout << x << std::endl; };
	// std::for_each(ptr.get(), ptr.get() + column->size(), printer);
		
	return 0;
}
