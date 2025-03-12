#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <ranges>
#include <vector>

#include "ecsify/internal/data_pool.h"

namespace {

constexpr std::size_t kCapacity =
    ecsify::internal::Bucket<std::uint8_t>::Capacity();

void BM_IterateFullBucket(benchmark::State &state) {
  ecsify::internal::Bucket<std::uint8_t> bucket;

  for (auto _ : std::views::iota(0UZ, kCapacity)) {
    bucket.Insert();
  }

  for (auto _ : state) {
    for (std::uint8_t &val : bucket) {
      benchmark::DoNotOptimize(val += 1);
    }
  }
}
BENCHMARK(BM_IterateFullBucket)->Repetitions(4);

void BM_IterateHalfBucket(benchmark::State &state) {
  ecsify::internal::Bucket<std::uint8_t> bucket;
  std::vector<std::size_t> removed_indicies;
  for (auto _ : std::views::iota(0UZ, kCapacity)) {
    std::size_t idx = bucket.Insert();
    if (idx % 2 == 0) {
      removed_indicies.push_back(idx);
    }
  }
  for (std::size_t idx : removed_indicies) {
    bucket.Erase(idx);
  }

  for (auto _ : state) {
    for (std::uint8_t &val : bucket) {
      benchmark::DoNotOptimize(val += 1);
    }
  }
}
BENCHMARK(BM_IterateHalfBucket)->Repetitions(4);

}  // namespace
