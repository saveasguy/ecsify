#include <gtest/gtest.h>

#include <iostream>

#include "ecsify/world.h"

using DummyWorld = ecsify::MakeWorld::WithComponents<>;
using BoolWorld = ecsify::MakeWorld::WithComponents<bool>;
using BoolIntWorld = ecsify::MakeWorld::WithComponents<bool, int>;

TEST(WorldTests, TestNumComponentTypes) {
  ASSERT_EQ(DummyWorld::kNumSupportedComponents, 0);
  ASSERT_EQ(BoolWorld::kNumSupportedComponents, 1);
  ASSERT_EQ(BoolIntWorld::kNumSupportedComponents, 2);
}
