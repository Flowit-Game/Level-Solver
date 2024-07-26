#pragma once

#include <vector>

template<typename K, typename V>
class SimpleApproximateMap {
        static constexpr size_t SIZE = 1e9;
        std::vector<std::tuple<size_t, K, V>> map;
        size_t epoch = 1;
    public:
        SimpleApproximateMap() {
            map.resize(SIZE);
        }

        void insert(K key, V value) {
            map[key % SIZE] = std::make_tuple(epoch, key, value);
        }

        V *get(K key) {
            auto &entry = map[key % SIZE];
            if (std::get<0>(entry) == epoch && std::get<1>(entry) == key) {
                return &std::get<2>(entry);
            }
            return nullptr;
        }

        void clear() {
            epoch++;
        }
};