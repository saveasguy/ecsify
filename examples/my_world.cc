#include <ecsify/world.h>

#include <iostream>

struct Position {
  float x, y;
};

struct Velocity {
  float x, y;
};

using MyCxE = ecsify::Components<Position, Velocity>::xEntities;

int main() {
  ecsify::World<MyCxE> world;
  std::cout << "We created a world with " << ecsify::numComponentTypes(world)
            << " supported components" << std::endl;
}
