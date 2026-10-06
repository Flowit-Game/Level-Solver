#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

class SimpleApproximateMap {
        using key_t = uint64_t;
        static constexpr size_t SIZE = 1.6e9;
        static constexpr size_t EPOCH_BITS = 24;
        static constexpr uint64_t EPOCH_RANGE = uint64_t(1) << EPOCH_BITS;

        struct Entry {
            uint64_t value : 8;
            uint64_t epoch : EPOCH_BITS;
            uint64_t fingerprint : 32;
        };
        static_assert(sizeof(Entry) == sizeof(uint64_t));

        std::unique_ptr<std::atomic<Entry>[]> map;
        uint64_t epoch = 1;
        uint64_t levelStartEpoch = 1;

        static uint32_t fingerprint(key_t key) {
            return key >> 32;
        }

    public:
        SimpleApproximateMap() : map(new std::atomic<Entry>[SIZE]()) {
        }

        void prefetch(key_t key) {
            __builtin_prefetch(&map[key % SIZE]);
        }

        void insert(key_t key, uint8_t value) {
            map[key % SIZE].store({ value, epoch % EPOCH_RANGE, fingerprint(key) }, std::memory_order_relaxed);
        }

        struct Result {
            bool found;
            uint8_t value;
            bool isSameEpoch;
        };

        Result get(key_t key) {
            Entry entry = map[key % SIZE].load(std::memory_order_relaxed);
            uint64_t age = (epoch - entry.epoch) % EPOCH_RANGE;
            if (entry.fingerprint == fingerprint(key) && age <= epoch - levelStartEpoch) {
                return { true, static_cast<uint8_t>(entry.value), age == 0 };
            }
            return { false, 0, false };
        }

        void clear() {
            nextEpoch();
            levelStartEpoch = epoch;
        }

        void nextEpoch() {
            epoch++;
            if (epoch % EPOCH_RANGE == 0) {
                // Stored epochs would become ambiguous after wrapping around
                for (size_t i = 0; i < SIZE; i++) {
                    map[i].store({}, std::memory_order_relaxed);
                }
                epoch++;
                levelStartEpoch = epoch;
            }
        }
};
