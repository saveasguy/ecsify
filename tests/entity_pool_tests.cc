#include <gtest/gtest.h>

#include "ecsify/entity.h"
#include "ecsify/internal/entity_pool.h"

TEST(EntityPoolTests, Add) {
  ecsify::internal::EntityPool<0> pool;
  ecsify::Entity entity1 = pool.Add();
  ASSERT_TRUE(pool.Alive(entity1));
  ecsify::Entity entity2 = pool.Add();
  ASSERT_TRUE(pool.Alive(entity1));
  ASSERT_TRUE(pool.Alive(entity2));
}

TEST(EntityPoolTests, Remove) {
  ecsify::internal::EntityPool<0> pool;
  ecsify::Entity entity1 = pool.Add();
  ecsify::Entity entity2 = pool.Add();
  pool.Remove(entity1);
  ASSERT_FALSE(pool.Alive(entity1));
  ASSERT_TRUE(pool.Alive(entity2));
  pool.Remove(entity2);
  ASSERT_FALSE(pool.Alive(entity2));
}
