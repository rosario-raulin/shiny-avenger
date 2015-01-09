#include "RadixGrouping.hpp"

#include <vector>
#include <iostream>
#include <memory>
#include <random>

#define TESTSIZE 100
#define CARDINALITY 10

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

int main() {
	RadixGrouping algo;
	auto column = make_column(TESTSIZE, CARDINALITY);
	print_column(column);
	std::vector<ColumnPtr> columns;
	columns.emplace_back(column.get());
	auto ptr = algo.groupBy(columns);
	
	auto printer = [](const std::size_t& x) { std::cout << x << std::endl; };
	std::for_each(ptr.get(), ptr.get() + column->size(), printer);
		
	return 0;
}
