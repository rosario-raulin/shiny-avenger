#include "RadixGrouping.hpp"
#include "HashBasedGrouping.hpp"
#include "SharedHashGrouping.hpp"

#include <vector>
#include <iostream>
#include <memory>
#include <random>
#include <chrono>
#include <functional>
#include <string>

static std::shared_ptr<Column>
make_column(std::size_t size, std::size_t cardinality) {
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

static void
print_column(std::shared_ptr<Column> columnPtr) {
	Column& column = *columnPtr;
	for (auto i = 0; i < column.size(); ++i) {
		std::cout << i << "\t" << column[i] << std::endl;
	}
}

static std::unique_ptr<IGroupingAlgorithm>
make_grouping_algorithm(const std::string& algorithm) {
	if (algorithm == "radix") {
		return std::unique_ptr<RadixGrouping>(new RadixGrouping());
	} else if (algorithm == "simple") {
		return std::unique_ptr<HashBasedGrouping>(new HashBasedGrouping());
	} else if (algorithm == "shared") {
		return std::unique_ptr<SharedHashGrouping>(new SharedHashGrouping());
	} else {
		return nullptr;
	}
}

static inline void
printUsage() {
	std::cerr << "usage: tp algorithm-name test-size cardinality" << std::endl;
}

static void
testGroupingAlgorithm(std::unique_ptr<IGroupingAlgorithm> algo, std::size_t testsize, std::size_t cardinality) {
	auto column = make_column(testsize, cardinality);
	
	std::vector<ColumnPtr> columns;
	columns.emplace_back(column.get());
	
	auto start = std::chrono::system_clock::now();
	auto res = algo->groupBy(columns);
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
	
	std::cout << "Grouping took " << (duration.count() / 1000.0) << " seconds." << std::endl;
	
	auto groups = 1; // there is at least one group
	auto ptr = res.get();
	for (std::size_t i = 0; i < column->size()-1; ++i) {
		auto v1 = (*column)[ptr[i]];
		auto v2 = (*column)[ptr[i+1]];
		
		if (v1 != v2) {
			//std::cout << v1 << " and " << v2 << " differ." << std::endl;
			++groups;
		}
	}
	
	std::cout << "Found " << groups << " groups." << std::endl;
}

int main(int argc, char** argv) {
	if (argc > 3) {
		auto algo = make_grouping_algorithm(argv[1]);
		if (algo == nullptr) {
			std::cerr << "error: unknown grouping algorithm!" << std::endl;
			return 1;
		}
		
		try {
			auto testsize = std::stoi(argv[2]);
			auto cardinality = std::stoi(argv[3]);
			testGroupingAlgorithm(std::move(algo), testsize, cardinality);
			return 0;
		} catch (const std::invalid_argument& e) {
			std::cerr << "error: invalid arguments, see usage" << std::endl;
			printUsage();
			return 1;
		}
	} else {
		printUsage();
		return 1;
	}
}
