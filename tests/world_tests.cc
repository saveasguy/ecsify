#include <gtest/gtest.h>

#include <iostream>

#include "ecsify/world.h"

using xE = ecsify::Components<>::xEntities;
using BoolxE = ecsify::Components<bool>::xEntities;
using BoolIntxE = ecsify::Components<bool>::x<int>::xEntities;

TEST(WorldTests, TestNumComponentTypes) {
  ASSERT_EQ(xE::kNumComponentTypes, 0);
  ASSERT_EQ(BoolxE::kNumComponentTypes, 1);
  ASSERT_EQ(BoolIntxE::kNumComponentTypes, 2);
  ecsify::World<xE> dummy_world;
  ecsify::World<BoolxE> bool_world;
  ecsify::World<BoolIntxE> bool_int_world;
  ASSERT_EQ(ecsify::numComponentTypes(dummy_world), 0);
  ASSERT_EQ(BoolxE::kNumComponentTypes, 1);
  ASSERT_EQ(BoolIntxE::kNumComponentTypes, 2);
}
