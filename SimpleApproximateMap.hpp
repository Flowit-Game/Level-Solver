#pragma once

#include <vector>

template<typename K, typename V>
class SimpleApproximateMap {
        static constexpr size_t SIZE = 5e8;
        std::vector<std::tuple<size_t, K, V>> map;
        size_t epoch = 1000;
    public:
        SimpleApproximateMap() {
            map.resize(SIZE);
        }

        void insert(K key, V value) {
            map[key % SIZE] = std::make_tuple(epoch, key, value);
        }

        struct Result {
            V *value;
            bool isSameEpoch;
        };

        Result get(K key) {
            auto &entry = map[key % SIZE];
            if (std::get<1>(entry) == key && std::get<0>(entry) > epoch - 1000 - 1) {
                return { &std::get<2>(entry), std::get<0>(entry) == epoch };
            }
            return { nullptr };
        }

        void clear() {
            epoch += 1000;
        }

        void nextEpoch() {
            epoch += 1;
        }
};