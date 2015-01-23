#include "RadixGrouping.hpp"
#include "HashBasedGrouping.hpp"
#include "IndependentHashGrouping.hpp"

#include <random>
#include <memory>
#include <chrono>
#include <array>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

static const size_t NUMBER_OF_PASSES = 20;

static std::mt19937 RANDOM_GENERATOR;

static size_t number_of_tuples(size_t size_in_mb) {
  return (size_in_mb * 1024 * 1024) / sizeof(size_t);
}

struct distribution {
  virtual ~distribution() {}
  virtual std::string name() const = 0;
  virtual size_t operator()(std::mt19937& gen) = 0;
};

struct uniform_dist : public distribution {
  uniform_dist(size_t lower, size_t upper) :
    _name(std::string("uniform")),
    _dist(std::uniform_int_distribution<>(lower, upper))
  {}

  virtual size_t operator()(std::mt19937& gen) {
    return _dist(gen);
  }

  virtual std::string name() const {
    return _name;
  }

private:
  const std::string _name;
  std::uniform_int_distribution<> _dist;
};

struct geo_dist : public distribution {
  geo_dist(size_t lower, size_t upper)
    : _name(std::string("geo")),
      _dist(std::geometric_distribution<size_t>(0.3)),
      _lower(lower),
      _upper(upper)
  {}

  virtual size_t operator()(std::mt19937& gen) {
    size_t x;
    do {
      x = _dist(gen);
    } while (x < _lower || x > _upper);
    return x;
  }

  virtual std::string name() const {
    return _name;
  }

private:
  const std::string _name;
  std::geometric_distribution<size_t> _dist;
  const size_t _lower;
  const size_t _upper;
};

struct algorithm {
  algorithm(const std::string& name, IGroupingAlgorithm* algo) :
    name(name),
    algo(algo)
  {
  }

  ~algorithm() {
    delete algo;
  }

  std::string name;
  IGroupingAlgorithm* algo;
};

static Column
make_column(size_t size_in_mb, distribution& dist) {
  size_t tuples = number_of_tuples(size_in_mb);

  Column column;
  column.resize(tuples);
  for (size_t i = 0; i < tuples; ++i) {
    column[i] = dist(RANDOM_GENERATOR);
  }

  return column;
}

class TestSample {
public:
  TestSample(Column& column, algorithm& algo)
    : _column(column), _algo(algo)
  {}

  PositionListPtr operator()() {
    std::vector<ColumnPtr> columns;
    columns.emplace_back(&_column);

    auto start = std::chrono::system_clock::now();
    auto result = _algo.algo->groupBy(columns);
    auto end = std::chrono::system_clock::now();

    _runtime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return result;
  }

  std::chrono::milliseconds runtime() const {
    return _runtime;
  }

private:
  Column& _column;
  algorithm& _algo;
  std::chrono::milliseconds _runtime;
};

std::unique_ptr<distribution>
make_dist(const std::string& name, size_t lower, size_t upper) {
  if (name == "uniform") {
    return std::unique_ptr<distribution>(new uniform_dist(lower, upper));
  } else if (name == "geometric") {
    return std::unique_ptr<distribution>(new geo_dist(lower, upper));
  } else {
    throw std::runtime_error("unknown distribution");
  }
}

int main() {
  std::array<size_t, 3> cardinalities = { 10, 1000, 1000000 };
  std::array<size_t, 8> test_sizes = { 1, 8, 16, 32, 64, 128, 256, 512 };

  std::array<algorithm, 2> algorithms =
  {
    algorithm("radix", new RadixGrouping(4, 16, 11)),
    algorithm("simple", new HashBasedGrouping())
  };

  std::array<std::string, 2> dists = { "uniform", "geometric" };

  std::cout << "<results>" << std::endl;

  for (size_t test_size : test_sizes) {
    for (size_t cardinality : cardinalities) {
      for (const std::string& name : dists) {
        std::unique_ptr<distribution> dist = make_dist(name, 1, cardinality);
        auto column = make_column(test_size, *dist);

        for (auto& algo : algorithms) {
          TestSample sample(column, algo);

          std::cout << "\t<result>" << std::endl;

          std::cout << "\t\t<algorithm>";
          std::cout << algo.name;
          std::cout << "</algorithm>" << std::endl;

          std::cout << "\t\t<distribution>";
          std::cout << dist->name();
          std::cout << "</distribution>" << std::endl;

          std::cout << "\t\t<number-of-tuples>";
          std::cout << number_of_tuples(test_size);
          std::cout << "</number-of-tuples>" << std::endl;

          std::cout << "\t\t<cardinality>";
          std::cout << cardinality;
          std::cout << "</cardinality>" << std::endl;

          std::cout << "\t\t<times>" << std::endl;
          for (size_t i = 0; i < NUMBER_OF_PASSES; ++i) {
            std::cout << "\t\t\t<time>";
            sample();
            std::cout << sample.runtime().count();
            std::cout << "</time>" << std::endl;
          }
          std::cout << "\t\t</times>" << std::endl;

          std::cout << "\t</result>" << std::endl;
        }
      }
    }
  }

  std::cout << "</results>" << std::endl;

  return 0;
}
