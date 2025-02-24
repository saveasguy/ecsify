#include <ecsify/world.h>

#include <iostream>

struct Position {
  float x, y;
};

struct Velocity {
  float x, y;
};

using MyWorld = ecsify::World<>::With<Position, Velocity>;

int main() {
  MyWorld world;
  std::cout << "We created a world with " << numSupportedComponents(world)
            << " supported components" << std::endl;
}
