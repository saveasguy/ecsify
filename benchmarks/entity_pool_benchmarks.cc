#include <benchmark/benchmark.h>

#include <ranges>
#include <vector>

#include "ecsify/entity.h"
#include "ecsify/internal/entity_pool.h"

namespace {

constexpr benchmark::IterationCount kMaxIterations = 1000000;

void BM_FillColdEntityPool(benchmark::State &state) {
  ecsify::internal::EntityPool pool(0);
  for (auto _ : state) {
    pool.Add();
  }
}
BENCHMARK(BM_FillColdEntityPool)->Iterations(kMaxIterations)->Repetitions(4);

void BM_FillWarmEntityPool(benchmark::State &state) {
  ecsify::internal::EntityPool pool(0);
  std::vector<ecsify::Entity> entities;
  entities.reserve(state.max_iterations);
  for (auto _ : std::views::iota(0, state.max_iterations)) {
    entities.push_back(pool.Add());
  }
  for (ecsify::Entity entity : entities) {
    pool.Remove(entity);
  }
  for (auto _ : state) {
    pool.Add();
  }
}
BENCHMARK(BM_FillWarmEntityPool)->Iterations(kMaxIterations)->Repetitions(4);

void BM_EntityPoolRemove(benchmark::State &state) {
  ecsify::internal::EntityPool pool(0);
  std::vector<ecsify::Entity> entities;
  entities.reserve(state.max_iterations);
  for (auto _ : std::views::iota(0, state.max_iterations)) {
    entities.push_back(pool.Add());
  }
  for (auto _ : state) {
    pool.Remove(entities.back());
    entities.pop_back();
  }
}
BENCHMARK(BM_EntityPoolRemove)->Iterations(kMaxIterations)->Repetitions(4);

void BM_EntityPoolAddRemove(benchmark::State &state) {
  ecsify::internal::EntityPool pool(0);
  for (auto _ : state) {
    ecsify::Entity entity = pool.Add();
    pool.Remove(entity);
  }
}
BENCHMARK(BM_EntityPoolAddRemove)->Iterations(kMaxIterations)->Repetitions(4);

}  // namespace
