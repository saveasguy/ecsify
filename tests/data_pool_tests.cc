#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <set>
#include <vector>

#include "ecsify/internal/data_pool.h"

TEST(BucketTests, ContainsInserted) {
  ecsify::internal::Bucket<int> bucket{0};
  std::size_t idx0 = bucket.Insert(1);
  ASSERT_TRUE(bucket.Contains(idx0));
  std::size_t idx1 = bucket.Insert(2);
  ASSERT_TRUE(bucket.Contains(idx1));
  // Test lvalue insertion.
  int val2 = 3;
  std::size_t idx2 = bucket.Insert(val2);
  ASSERT_TRUE(bucket.Contains(idx2));
  int val3 = 4;
  std::size_t idx3 = bucket.Insert(val3);
  ASSERT_TRUE(bucket.Contains(idx3));
}

TEST(BucketTests, InsertionIsCorrect) {
  ecsify::internal::Bucket<int> bucket{0};
  std::size_t idx0 = bucket.Insert(1);
  ASSERT_EQ(bucket[idx0], 1);
  std::size_t idx1 = bucket.Insert(2);
  ASSERT_EQ(bucket[idx0], 1);
  ASSERT_EQ(bucket[idx1], 2);
  // Test const version of operator[]. Results should be identical.
  const auto &bucket_cref = bucket;
  ASSERT_EQ(bucket_cref[idx0], bucket[idx0]);
  ASSERT_EQ(bucket_cref[idx1], bucket[idx1]);
}

TEST(BucketTests, FullWhenCapacityReached) {
  ecsify::internal::Bucket<std::size_t> bucket{0};
  for (std::size_t i = 0; i < ecsify::internal::Bucket<std::size_t>::Capacity();
       ++i) {
    ASSERT_FALSE(bucket.Full());
    bucket.Insert(i);
  }
  ASSERT_TRUE(bucket.Full());
}

TEST(BucketTests, DoesntContainNotInserted) {
  ecsify::internal::Bucket<int> bucket{0};
  ASSERT_FALSE(bucket.Contains(0));
  ASSERT_FALSE(bucket.Contains(1));
  bucket.Insert(1);
  ASSERT_FALSE(bucket.Contains(1));
}

TEST(BucketTests, ErasureIsCorrect) {
  ecsify::internal::Bucket<int> bucket{0};
  std::size_t idx0 = bucket.Insert(1);
  std::size_t idx1 = bucket.Insert(2);
  bucket.Erase(idx0);
  ASSERT_FALSE(bucket.Contains(idx0));
  ASSERT_TRUE(bucket.Contains(idx1));
  ASSERT_EQ(bucket[idx1], 2);
  bucket.Erase(idx1);
  ASSERT_FALSE(bucket.Contains(idx1));
}

TEST(BucketTests, InsertionIsCorrectAfterErasure) {
  ecsify::internal::Bucket<int> bucket{0};
  std::size_t idx0 = bucket.Insert(1);
  std::size_t idx1 = bucket.Insert(2);
  bucket.Erase(idx0);
  std::size_t idx2 = bucket.Insert(3);
  ASSERT_TRUE(bucket.Contains(idx2));
  ASSERT_EQ(bucket[idx2], 3);
  ASSERT_TRUE(bucket.Contains(idx1));
}

TEST(BucketTests, NumIterationsIsCorrect) {
  constexpr std::size_t kCapacity = ecsify::internal::Bucket<int>::Capacity();
  for (std::size_t size : std::views::iota(0UZ, kCapacity + 1)) {
    ecsify::internal::Bucket<int> bucket{0};
    for (std::size_t _ : std::views::iota(0UZ, size)) {
      bucket.Insert(0);
    }
    const auto &bucket_cref = bucket;
    ASSERT_EQ(std::distance(bucket.begin(), bucket.end()), size);
    ASSERT_EQ(std::distance(bucket_cref.begin(), bucket_cref.end()), size);
    ASSERT_EQ(std::ranges::distance(bucket), size);
    ASSERT_EQ(std::ranges::distance(bucket_cref), size);
  }
}

TEST(BucketTests, AllValuesAreIterated) {
  constexpr std::size_t kCapacity =
      ecsify::internal::Bucket<std::size_t>::Capacity();
  std::set<std::size_t> vals;
  ecsify::internal::Bucket<std::size_t> bucket{0};
  std::vector<std::size_t> indices;
  // First, check if all the values are iterated.
  for (std::size_t i = 0; i < kCapacity; ++i) {
    std::size_t val = i * 2;
    vals.insert(val);
    indices.push_back(bucket.Insert(val));
  }
  for (std::size_t val : bucket) {
    ASSERT_TRUE(vals.contains(val));
  }
  for (std::size_t val : vals) {
    ASSERT_NE(std::ranges::find(bucket, val), bucket.end());
  }
  // Then remove half of the elements and check again.
  auto is_even = [](std::size_t num) { return num % 2 == 0; };
  for (std::size_t removed_idx : indices | std::views::filter(is_even)) {
    vals.erase(bucket[removed_idx]);
    bucket.Erase(removed_idx);
  }
  for (std::size_t val : bucket) {
    ASSERT_TRUE(vals.contains(val));
  }
  for (std::size_t val : vals) {
    ASSERT_NE(std::ranges::find(bucket, val), bucket.end());
  }
}

TEST(DataPoolTests, ContainsInserted) {
  ecsify::internal::DataPool<int> pool;
  std::size_t idx0 = pool.Insert(0);
  ASSERT_TRUE(pool.Contains(idx0));
  ASSERT_NE(pool.At(idx0), nullptr);
  std::size_t idx1 = pool.Insert(1);
  ASSERT_TRUE(pool.Contains(idx1));
  ASSERT_NE(pool.At(idx1), nullptr);
  // Test lvalue insertion.
  int val2 = 2;
  std::size_t idx2 = pool.Insert(val2);
  ASSERT_TRUE(pool.Contains(idx2));
  ASSERT_NE(pool.At(idx2), nullptr);
  int val3 = 3;
  std::size_t idx3 = pool.Insert(val3);
  ASSERT_TRUE(pool.Contains(idx3));
  ASSERT_NE(pool.At(idx3), nullptr);
}

TEST(DataPoolTests, InsertionIsCorrect) {
  ecsify::internal::DataPool<int> pool;
  std::size_t idx0 = pool.Insert(0);
  ASSERT_EQ(pool[idx0], 0);
  ASSERT_EQ(*pool.At(idx0), 0);
  std::size_t idx1 = pool.Insert(1);
  ASSERT_EQ(pool[idx0], 0);
  ASSERT_EQ(*pool.At(idx0), 0);
  ASSERT_EQ(pool[idx1], 1);
  ASSERT_EQ(*pool.At(idx1), 1);
  // Test const version of operator[]. Results should be identical.
  const auto &pool_cref = pool;
  ASSERT_EQ(pool_cref[idx0], pool[idx0]);
  ASSERT_EQ(*pool.At(idx0), pool[idx0]);
  ASSERT_EQ(pool_cref[idx1], pool[idx1]);
  ASSERT_EQ(*pool.At(idx1), pool[idx1]);
}

TEST(DataPoolTests, DoesntContainNotInserted) {
  ecsify::internal::DataPool<int> pool;
  ASSERT_FALSE(pool.Contains(0));
  ASSERT_FALSE(pool.Contains(1));
  pool.Insert(0);
  ASSERT_FALSE(pool.Contains(1));
}

TEST(DataPoolTests, ErasureIsCorrect) {
  ecsify::internal::DataPool<int> pool;
  std::size_t idx0 = pool.Insert(0);
  std::size_t idx1 = pool.Insert(1);
  pool.Erase(idx0);
  ASSERT_FALSE(pool.Contains(idx0));
  ASSERT_TRUE(pool.Contains(idx1));
  ASSERT_EQ(pool[idx1], 1);
  ASSERT_EQ(*pool.At(idx1), 1);
  pool.Erase(idx1);
  ASSERT_FALSE(pool.Contains(idx1));
}

TEST(DataPoolTests, InsertionIsCorrectAfterErasure) {
  ecsify::internal::DataPool<int> pool;
  std::size_t idx0 = pool.Insert(0);
  std::size_t idx1 = pool.Insert(1);
  pool.Erase(idx0);
  std::size_t idx2 = pool.Insert(2);
  ASSERT_TRUE(pool.Contains(idx2));
  ASSERT_EQ(pool[idx2], 2);
  ASSERT_EQ(*pool.At(idx2), 2);
  ASSERT_TRUE(pool.Contains(idx1));
}

TEST(DataPoolTests, NumIterationsIsCorrect) {
  constexpr std::size_t kCapacity = 512;
  for (std::size_t size : std::views::iota(0UZ, kCapacity + 1)) {
    ecsify::internal::DataPool<int> pool{};
    for (std::size_t _ : std::views::iota(0UZ, size)) {
      pool.Insert(0);
    }
    const auto &pool_cref = pool;
    ASSERT_EQ(std::distance(pool.begin(), pool.end()), size);
    ASSERT_EQ(std::distance(pool_cref.begin(), pool_cref.end()), size);
    ASSERT_EQ(std::ranges::distance(pool), size);
    ASSERT_EQ(std::ranges::distance(pool_cref), size);
  }
}

TEST(DataPoolTests, AllValuesAreIterated) {
  constexpr std::size_t kCapacity = 512;
  std::set<std::size_t> vals;
  ecsify::internal::DataPool<std::size_t> pool{};
  std::vector<std::size_t> indices;
  // First, check if all the values are iterated.
  for (std::size_t i = 0; i < kCapacity; ++i) {
    std::size_t val = i * 2;
    vals.insert(val);
    indices.push_back(pool.Insert(val));
  }
  for (std::size_t val : pool) {
    ASSERT_TRUE(vals.contains(val));
  }
  for (std::size_t val : vals) {
    ASSERT_NE(std::ranges::find(pool, val), pool.end());
  }
  // Then remove half of the elements and check again.
  auto is_even = [](std::size_t num) { return num % 2 == 0; };
  for (std::size_t removed_idx : indices | std::views::filter(is_even)) {
    vals.erase(pool[removed_idx]);
    pool.Erase(removed_idx);
  }
  for (std::size_t val : pool) {
    ASSERT_TRUE(vals.contains(val));
  }
  for (std::size_t val : vals) {
    ASSERT_NE(std::ranges::find(pool, val), pool.end());
  }
}

TEST(DataPoolTests, Stability) {
  constexpr std::size_t kCapacity = 512;
  ecsify::internal::DataPool<std::size_t> pool1{};
  ecsify::internal::DataPool<std::size_t> pool2{};
  std::vector<std::size_t> indices;
  for (std::size_t i = 0; i < kCapacity; ++i) {
    std::size_t val = i * 2;
    indices.push_back(pool1.Insert(val));
    ASSERT_EQ(pool2.Insert(val), indices.back());
  }
  auto is_even = [](std::size_t num) { return num % 2 == 0; };
  for (std::size_t removed_idx : indices | std::views::filter(is_even)) {
    pool1.Erase(removed_idx);
    pool2.Erase(removed_idx);
  }
  ASSERT_TRUE(std::ranges::equal(pool1, pool2));
}
