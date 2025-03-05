#include <gtest/gtest.h>

#include "ecsify/entity.h"
#include "ecsify/internal/entity_pool.h"

TEST(EntityPoolTests, Add) {
  ecsify::internal::EntityPool pool(0);
  ecsify::Entity entity1 = pool.Add();
  ASSERT_TRUE(pool.Alive(entity1));
  ecsify::Entity entity2 = pool.Add();
  ASSERT_TRUE(pool.Alive(entity1));
  ASSERT_TRUE(pool.Alive(entity2));
}

TEST(EntityPoolTests, Remove) {
  ecsify::internal::EntityPool pool(0);
  ecsify::Entity entity1 = pool.Add();
  ecsify::Entity entity2 = pool.Add();
  pool.Remove(entity1);
  ASSERT_FALSE(pool.Alive(entity1));
  ASSERT_TRUE(pool.Alive(entity2));
  pool.Remove(entity2);
  ASSERT_FALSE(pool.Alive(entity2));
}

TEST(EntityPoolTests, Link) {
  ecsify::internal::EntityPool pool(2);
  ecsify::Entity entity = pool.Add();
  pool.Link(entity, 0);
  ASSERT_TRUE(pool.Has(entity, 0));
  pool.Link(entity, 0);
  ASSERT_TRUE(pool.Has(entity, 0));
  pool.Link(entity, 1);
  ASSERT_TRUE(pool.Has(entity, 0));
  ASSERT_TRUE(pool.Has(entity, 1));
  pool.Remove(entity);
  entity = pool.Add();
  ASSERT_FALSE(pool.Has(entity, 0));
  ASSERT_FALSE(pool.Has(entity, 1));
}

TEST(EntityPoolTests, Unlink) {
  ecsify::internal::EntityPool pool(2);
  ecsify::Entity entity = pool.Add();
  pool.Link(entity, 0);
  pool.Link(entity, 1);
  pool.Unlink(entity, 0);
  ASSERT_FALSE(pool.Has(entity, 0));
  ASSERT_TRUE(pool.Has(entity, 1));
  pool.Unlink(entity, 1);
  ASSERT_FALSE(pool.Has(entity, 1));
}
