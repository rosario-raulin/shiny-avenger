#include "RadixGrouping.hpp"
#include "HashBasedGrouping.hpp"

#include <vector>
#include <iostream>
#include <memory>
#include <random>
#include <chrono>
#include <functional>
#include <string>

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

std::shared_ptr<IGroupingAlgorithm>
make_grouping_algorithm(const std::string& algorithm) {
	if (algorithm == "radix") {
		return std::make_shared<RadixGrouping>();
	} else if (algorithm == "simple") {
		return std::make_shared<HashBasedGrouping>();
	} else {
		return nullptr;
	}
}

int main(int argc, char** argv) {
	if (argc > 1) {
		auto algo = make_grouping_algorithm(argv[1]);
		if (algo == nullptr) {
			std::cerr << "error: unknown grouping algorithm!" << std::endl;
			return 1;
		}
		
		auto column = make_column(TESTSIZE, CARDINALITY);

		std::vector<ColumnPtr> columns;
		columns.emplace_back(column.get());
		
		auto fn = [algo](const std::vector<ColumnPtr>& columns) { algo->groupBy(columns); };
		auto duration = measureTime(fn, columns);
		std::cout << "Grouping took " << duration / 1000.0 << " seconds." << std::endl;
		
		for (auto i = 0; i < CARDINALITY; ++i) {
			std::cout << i << " maps to " << VALUES[i] << " values." << std::endl;
		}
		
		return 0;
	} else {
		return 1;
	}
}
