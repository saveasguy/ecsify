#include <iostream>

#include "ecsify/component.h"
#include "ecsify/entity.h"
#include "ecsify/world.h"
#include "ecsify/world_builder.h"

struct Position : ecsify::ComponentMixin<1> {
  float x, y;
};

struct Velocity : ecsify::ComponentMixin<2> {
  float x, y;
};

ecsify::Entity CreateTurtle(ecsify::World &world, float posx, float posy, float velx,
                            float vely) {
  ecsify::Entity turtle = world.Add();
  world.Add<Position>(turtle);
  world.Get<Position>(turtle) = Position{.x = posx, .y = posy};
  world.Add<Velocity>(turtle);
  world.Get<Velocity>(turtle) = Velocity{.x = velx, .y = vely};
  return turtle;
}

int main() {
  auto world = ecsify::WorldBuilder{}
                   .Component<Position>()
                   .Component<Velocity>()
                   .Build();

  CreateTurtle(*world, 0, 0, 1, 1);
  CreateTurtle(*world, 2, 3, 1, 0);

  constexpr int kIter = 10;
  for (int i = 0; i < kIter; ++i) {
    for (auto [entt, pos, vel] : world->Query<ecsify::Entity, Position, Velocity>()) {
      pos.x += vel.x;
      pos.y += vel.y;
      std::cout << "Turtle #" << entt.id() << " moved to (" << pos.x << ", " << pos.y << ")\n";
    }
  }
  return 0;
}
