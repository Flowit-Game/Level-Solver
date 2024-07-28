#pragma once

#include <vector>

template<typename V>
class SimpleApproximateMap {
        using key_t = uint64_t;
        using epoch_t = uint32_t;
        static constexpr size_t SIZE = 1e9;
        std::vector<std::tuple<key_t, epoch_t, V>> map;
        size_t epoch = 1000;
    public:
        SimpleApproximateMap() {
            map.resize(SIZE);
        }

        void insert(key_t key, V value) {
            map[key % SIZE] = std::make_tuple(key, epoch, value);
        }

        struct Result {
            V *value;
            bool isSameEpoch;
        };

        Result get(key_t key) {
            auto &entry = map[key % SIZE];
            if (std::get<0>(entry) == key && std::get<1>(entry) > epoch - 1000 - 1) {
                return { &std::get<2>(entry), std::get<1>(entry) == epoch };
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