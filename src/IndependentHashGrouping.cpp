#include "IndependentHashGrouping.hpp"

#include "MergeableTable.hpp"
#include "ThreadPool.hpp"

#include <unordered_map>
#include <list>
#include <memory>

using HT = MergeableTable<size_t, std::list<size_t> >;
using HTPtr = std::shared_ptr<HT>;
using PartListPtr = std::unique_ptr<std::list<size_t> >;

static HTPtr
localHashing(Column& column, size_t lower, size_t upper, size_t table_size) {
  HTPtr tablePtr(new HT(table_size));
  auto& table = *tablePtr;

  for (size_t i = lower; i < upper; ++i) {
    table[column[i]].emplace_back(i);
  }

  return tablePtr;
}

static void
bucketHash(size_t i, std::vector<HTPtr>& tables, std::list<size_t>& tids) {
  std::unordered_map<size_t, std::list<size_t> > map;

  for (const HTPtr tablePtr : tables) {
    auto& table = *tablePtr;
    for (auto it = table.bucket_begin(i); it != table.bucket_end(i); ++it) {
      auto& lst = map[(*it)->first];
      lst.splice(lst.end(), (*it)->second);
    }
  }

  for (auto& kvp : map) {
    tids.splice(tids.end(), kvp.second);
  }
}

IndependentHashGrouping::IndependentHashGrouping(
  size_t number_of_threads,
  size_t number_of_tasks,
  size_t number_of_bits_for_hashing) :
  _number_of_threads(number_of_threads),
  _number_of_tasks(number_of_tasks),
  _number_of_bits_for_hashing(number_of_bits_for_hashing)
{
}

PositionListPtr
IndependentHashGrouping::groupBy(const std::vector<ColumnPtr>& columns) {
  ThreadPool pool(_number_of_threads);

  auto column = *(columns[0]);

  size_t chunk_size = column.size() / _number_of_tasks;
  size_t diff_cases = column.size() % _number_of_tasks;
  size_t regular_cases = _number_of_tasks - diff_cases;

  // Step 1: Build local hash tables
  std::vector<HTPtr> tables;

    using ResultType = std::shared_future<HTPtr>;
    std::vector<ResultType> results;
    size_t table_size = _table_size();

    for (size_t i = 0; i < regular_cases; ++i) {
      size_t lower = chunk_size * i;
      size_t upper = lower + chunk_size;

      auto job = [&column, lower, upper, table_size]() {
        return localHashing(column, lower, upper, table_size);
      };
      results.emplace_back(pool.addJob(job));
    }

    size_t lower = chunk_size * regular_cases;
    size_t upper;
    for (size_t i = 0; i < diff_cases; ++i) {
      upper = lower + chunk_size + 1;

      auto job = [&column, lower, upper, table_size]() {
        return localHashing(column, lower, upper, table_size);
      };
      results.emplace_back(pool.addJob(job));

      lower = upper;
    }


    tables.reserve(_number_of_tasks);
    for (size_t i = 0; i < _number_of_tasks; ++i) {
      tables.emplace_back(results[i].get());
    }

  // Step 2: Hash over each bucket
  std::vector<std::list<size_t> > partLists;
  partLists.resize(_table_size());

  std::vector<std::shared_future<void> > bucketGroupJobs;
  bucketGroupJobs.resize(_table_size());

  for (size_t i = 0; i < _table_size(); ++i) {
    auto job = [i, &tables, &partLists]() {
      bucketHash(i, tables, partLists[i]);
    };
    bucketGroupJobs[i] = pool.addJob(job);
  }
    
    for (auto& job : bucketGroupJobs) {
        job.wait();
    }

  std::default_delete<size_t[]> deleter;
  std::shared_ptr<size_t> posList(new size_t[column.size()], deleter);
  size_t i = 0;
  for (const auto& partList : partLists) {
    for (auto x : partList) {
      (posList.get())[i++] = x;
    }
  }

  // std::cout << "calculation done." << std::endl;

  return posList;
}
