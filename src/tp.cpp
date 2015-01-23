#include "RadixGrouping.hpp"
#include "HashBasedGrouping.hpp"
#include "IndependentHashGrouping.hpp"
#include "SharedHashGrouping.hpp"

#include <random>
#include <memory>
#include <chrono>
#include <array>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#define RANDOM_GENERATOR_TYPE std::mt19937_64

static const size_t NUMBER_OF_PASSES = 20;
static RANDOM_GENERATOR_TYPE RANDOM_GENERATOR;

static size_t number_of_tuples(size_t size_in_mb) {
  return (size_in_mb * 1024 * 1024) / sizeof(size_t);
}

template<class Random>
struct distribution {
  virtual std::string name() const = 0;
  virtual size_t operator()(Random& rand) = 0;
};

template<class Random>
struct uniform_dist : public distribution<Random> {
  uniform_dist(size_t lower, size_t upper) :
    _name(std::string("uniform")),
    _dist(std::uniform_int_distribution<>(lower, upper))
  {}

  virtual size_t operator()(Random& gen) {
    return _dist(gen);
  }

  virtual std::string name() const {
    return _name;
  }

private:
  const std::string _name;
  std::uniform_int_distribution<> _dist;
};

template<class Random>
struct geo_dist : public distribution<Random> {
  geo_dist(size_t lower, size_t upper)
    : _name(std::string("geo")),
      _dist(std::geometric_distribution<size_t>(0.3)),
      _lower(lower),
      _upper(upper)
  {}

  virtual size_t operator()(Random& gen) {
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

template<class Random>
static Column
make_column(size_t size_in_mb, distribution<Random>& dist) {
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

template<class Random>
std::unique_ptr<distribution<Random> >
make_dist(const std::string& name, size_t lower, size_t upper) {
  if (name == "uniform") {
    return std::unique_ptr<distribution<Random> >(new uniform_dist<Random>(lower, upper));
  } else if (name == "geometric") {
    return std::unique_ptr<distribution<Random> >(new geo_dist<Random>(lower, upper));
  } else {
    throw std::runtime_error("unknown distribution");
  }
}

int main() {
  std::array<size_t, 3> cardinalities = { 10, 1000, 1000000 };
  std::array<size_t, 8> test_sizes = { 1, 8, 16, 32, 64, 128, 256, 512 };

  std::array<algorithm, 8> algorithms =
  {
    algorithm("radix 1", new RadixGrouping(4, 16, 11)),
    algorithm("radix 2", new RadixGrouping(8, 8, 11)),
    algorithm("radix 3", new RadixGrouping(8, 16, 11)),

    algorithm("radix 4", new RadixGrouping(8, 32, 11)),
    algorithm("radix 5", new RadixGrouping(8, 32, 7)),
    algorithm("radix 6", new RadixGrouping(8, 32, 14)),

    algorithm("simple", new HashBasedGrouping()),
    algorithm("independent", new SharedHashGrouping())
  };

  std::array<std::string, 2> dists = { "uniform", "geometric" };

  std::cout << "<results>" << std::endl;

  for (size_t test_size : test_sizes) {
    for (size_t cardinality : cardinalities) {
      for (const std::string& name : dists) {
        auto dist = make_dist<RANDOM_GENERATOR_TYPE>(name, 1, cardinality);
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
