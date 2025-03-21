#include <gtest/gtest.h>

#include <cstdint>
#include <set>

#include "ecsify/component.h"
#include "ecsify/entity.h"
#include "ecsify/world_builder.h"

TEST(WorldTests, AddedEntitiesAreAlive) {
  auto world = ecsify::WorldBuilder{}.Build();
  ecsify::Entity entt1 = world->Add();
  ASSERT_TRUE(world->Alive(entt1));
  ecsify::Entity entt2 = world->Add();
  ASSERT_TRUE(world->Alive(entt1));
  ASSERT_TRUE(world->Alive(entt2));
}

TEST(WorldTests, RemovedEntitiesAreNotAlive) {
  auto world = ecsify::WorldBuilder{}.Build();
  ecsify::Entity entt1 = world->Add();
  ecsify::Entity entt2 = world->Add();
  world->Remove(entt1);
  ASSERT_FALSE(world->Alive(entt1));
  ASSERT_TRUE(world->Alive(entt2));
  world->Remove(entt2);
  ASSERT_FALSE(world->Alive(entt1));
  ASSERT_FALSE(world->Alive(entt2));
}

TEST(WorldTests, AddedRemovedEntitiesLiveness) {
  auto world = ecsify::WorldBuilder{}.Build();
  ecsify::Entity entt1 = world->Add();
  ecsify::Entity entt2 = world->Add();
  world->Remove(entt1);
  ASSERT_FALSE(world->Alive(entt1));
  ASSERT_TRUE(world->Alive(entt2));
  ecsify::Entity entt3 = world->Add();
  ASSERT_FALSE(world->Alive(entt1));
  ASSERT_TRUE(world->Alive(entt2));
  ASSERT_TRUE(world->Alive(entt3));
  world->Remove(entt3);
  ASSERT_FALSE(world->Alive(entt1));
  ASSERT_TRUE(world->Alive(entt2));
  ASSERT_FALSE(world->Alive(entt3));
  world->Remove(entt2);
  ASSERT_FALSE(world->Alive(entt1));
  ASSERT_FALSE(world->Alive(entt2));
  ASSERT_FALSE(world->Alive(entt3));
}

struct Dummy : ecsify::ComponentMixin<1> {};

TEST(WorldTests, EntitiesOwnAddedComponents) {
  auto world = ecsify::WorldBuilder{}.Component<Dummy>().Build();
  ecsify::Entity entt1 = world->Add();
  ASSERT_TRUE(world->Has<ecsify::Entity>(entt1));
  world->Add<Dummy>(entt1);
  ASSERT_TRUE(world->Has<ecsify::Entity>(entt1));
  ASSERT_TRUE(world->Has<Dummy>(entt1));
  ecsify::Entity entt2 = world->Add();
  world->Add<Dummy>(entt2);
  ASSERT_TRUE(world->Has<ecsify::Entity>(entt1));
  ASSERT_TRUE(world->Has<Dummy>(entt1));
  ASSERT_TRUE(world->Has<ecsify::Entity>(entt2));
  ASSERT_TRUE(world->Has<Dummy>(entt2));
}

TEST(WorldTests, DeadEntitiesDontOwnComponents) {
  auto world = ecsify::WorldBuilder{}.Component<Dummy>().Build();
  ecsify::Entity entt1 = world->Add();
  world->Add<Dummy>(entt1);
  ecsify::Entity entt2 = world->Add();
  world->Add<Dummy>(entt2);
  world->Remove(entt1);
  ASSERT_FALSE(world->Has<ecsify::Entity>(entt1));
  ASSERT_FALSE(world->Has<Dummy>(entt1));
  ASSERT_TRUE(world->Has<ecsify::Entity>(entt2));
  ASSERT_TRUE(world->Has<Dummy>(entt2));
  world->Remove(entt2);
  ASSERT_FALSE(world->Has<ecsify::Entity>(entt1));
  ASSERT_FALSE(world->Has<Dummy>(entt1));
  ASSERT_FALSE(world->Has<ecsify::Entity>(entt2));
  ASSERT_FALSE(world->Has<Dummy>(entt2));
}

struct Int : ecsify::ComponentMixin<1> {
  int val;
};

TEST(WorldTests, ValuesArePreservedOnOperations) {
  Int int1{.val = 1};
  Int int2{.val = 2};

  auto world = ecsify::WorldBuilder{}.Component<Int>().Build();
  ecsify::Entity entt1 = world->Add();
  world->Add<Int>(entt1);
  world->Get<Int>(entt1) = int1;
  ASSERT_EQ(world->Get<Int>(entt1).val, int1.val);
  ecsify::Entity entt2 = world->Add();
  world->Add<Int>(entt2);
  world->Get<Int>(entt2) = int2;
  ASSERT_EQ(world->Get<Int>(entt1).val, int1.val);
  ASSERT_EQ(world->Get<Int>(entt2).val, int2.val);
  world->Remove<Int>(entt1);
  ASSERT_EQ(world->Get<Int>(entt2).val, int2.val);
  world->Remove(entt1);
  ASSERT_EQ(world->Get<Int>(entt2).val, int2.val);
}

TEST(WorldTests, Queries) {
  Int int1{.val = 1};
  Int int2{.val = 2};

  auto world = ecsify::WorldBuilder{}.Component<Int>().Build();
  ecsify::Entity entt1 = world->Add();
  world->Add<Int>(entt1);
  world->Get<Int>(entt1) = int1;
  ecsify::Entity entt2 = world->Add();
  world->Add<Int>(entt2);
  world->Get<Int>(entt2) = int2;
  ecsify::Entity entt3 = world->Add();

  std::set<std::int64_t> queried_entity_ids = {entt1.id(), entt2.id()};
  std::set<int> queried_int_vals = {int1.val, int2.val};

  for (auto [entt, val] : world->Query<ecsify::Entity, Int>()) {
    ASSERT_TRUE(queried_entity_ids.contains(entt.id()));
    ASSERT_TRUE(queried_int_vals.contains(val.val));
    queried_entity_ids.erase(entt.id());
    queried_int_vals.erase(val.val);
  }
  ASSERT_TRUE(queried_entity_ids.empty());
  ASSERT_TRUE(queried_int_vals.empty());
}
