#include <gtest/gtest.h>

#include "ecsify/internal/data_pool.h"

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
