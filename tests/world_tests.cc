#include <gtest/gtest.h>

#include <iostream>

#include "ecsify/world.h"

using DummyWorld = ecsify::World<>;
using BoolWorld = ecsify::World<>::With<bool>;
using BoolIntWorld = ecsify::World<>::With<bool, int>;

TEST(WorldTests, TestNumSupportedComponents) {
  ASSERT_EQ(DummyWorld::kNumComponentTypes, 0);
  ASSERT_EQ(BoolWorld::kNumComponentTypes, 1);
  ASSERT_EQ(BoolIntWorld::kNumComponentTypes, 2);
}
