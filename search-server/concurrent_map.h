#pragma once
#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

using namespace std::string_literals;
using namespace std;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);
    
    struct Mut_and_map {
    mutex mut;
    map<Key,Value> mapa;
    };
    
    struct Access {
    lock_guard<mutex> guard; 
    Value& ref_to_value;    
    Access(Mut_and_map& ref,Key key) : guard(ref.mut), ref_to_value(ref.mapa[key]){};
    };

    explicit ConcurrentMap(size_t bucket_count) : maps(bucket_count) {}

    Access operator[](const Key& key) {
    uint64_t kk = static_cast<uint64_t>(key);
    auto index =  kk % maps.size();
    return Access(maps[index],kk);
    }

    std::map<Key, Value> BuildOrdinaryMap() {
    map<Key,Value> re_4;
    for (size_t i = 0; i < maps.size(); ++i) {
    lock_guard<mutex> guard(maps[i].mut);
    re_4.insert(maps[i].mapa.begin(),maps[i].mapa.end());
    }
    return re_4;
    }
    
    void erase(int key ) {
     uint64_t kk = static_cast<uint64_t>(key);
     auto index =  kk % maps.size();
     maps[index].mapa.erase(kk);
    }

private:
vector<Mut_and_map> maps;
};