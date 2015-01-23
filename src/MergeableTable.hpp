#ifndef HAVE_MERGEABLE_TABLE_HPP
#define HAVE_MERGEABLE_TABLE_HPP

#include <functional>
#include <algorithm>
#include <utility>
#include <list>
#include <stdexcept>

template<
  class Key,
  class Value,
  class Hash = std::hash<Key>,
  class Eq = std::equal_to<Key>
>
class MergeableTable {
public:
  using KVPair = std::pair<const Key, Value>;
  using KVPairPtr = std::unique_ptr<KVPair>;
  using KVPList = std::list<KVPairPtr>;

  MergeableTable(size_t bucket_number,
    Hash hash = std::hash<Key>(),
    Eq eq = std::equal_to<Key>()) :
    _size(0),
    _kvp(new KVPList[bucket_number]),
    _bucket_number(bucket_number),
    _hash(hash),
    _eq(eq)
  {
  }

  MergeableTable(const MergeableTable& other) :
    _size(other._size),
    _kvp(new KVPList[other._bucket_number]),
    _bucket_number(other._bucket_number),
    _hash(other._hash),
    _eq(other._eq)
  {
    for (size_t i = 0; i < _bucket_number; ++i) {
      auto& lst = other._kvp[i];
      std::for_each(lst.cbegin(), lst.cend(), [this, i](const KVPairPtr& p) {
        this->_kvp[i].emplace_back(new KVPair(*p));
      });
    }
  }

  MergeableTable(MergeableTable&& other) :
    _size(other._size),
    _kvp(std::move(other._kvp)),
    _bucket_number(other._bucket_number),
    _hash(std::move(other._hash)),
    _eq(std::move(other._eq))
  {
  }

  Value& operator[](const Key& key) {
    auto hash = _hash(key) % _bucket_number;
    auto& lst = _kvp[hash];
    auto pos = std::find_if(lst.begin(), lst.end(), [this, &key](KVPairPtr& p) {
      return this->_eq(key, p->first);
    });

    if (pos == lst.end()) {
      ++_size;
      KVPairPtr kvp(new KVPair(key, Value()));
      Value& val = kvp->second;
      lst.emplace_back(std::move(kvp));
      return val;
    } else {
      return (*pos)->second;
    }
  }

  const Value& operator[](const Key& key) const {
    auto hash = _hash(key) % _bucket_number;
    auto& lst = _kvp[hash];
    auto pos = std::find_if(lst.begin(), lst.end(), [this, &key](KVPairPtr& p) {
      return this->_eq(key, p->first);
    });

    if (pos == lst.end()) {
      throw std::runtime_error("key not found");
    } else {
      return (*pos)->second;
    }
  }

  size_t size() const {
    return _size;
  }

  void merge(MergeableTable& other) {
    auto begin = other._kvp.get();
    auto end = begin + other._bucket_number;

    size_t i = 0;
    std::for_each(begin, end, [this, &i](KVPList& lst) {
      this->_kvp[i].splice(this->_kvp[i].end(), lst);
      ++i;
    });

    _size += other._size;
  }

  using bucket_iterator = typename KVPList::const_iterator;

  bucket_iterator bucket_begin(size_t i) const {
    return _kvp[i].cbegin();
  }

  bucket_iterator bucket_end(size_t i) const {
    return _kvp[i].cend();
  }

private:
  size_t _size;
  std::unique_ptr<KVPList[]> _kvp;

  const size_t _bucket_number;
  const Hash _hash;
  const Eq _eq;
};

#endif
