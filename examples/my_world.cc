#include <ecsify/world.h>

#include <iostream>

struct Position {
  float x, y;
};

struct Velocity {
  float x, y;
};

using MyWorld = ecsify::MakeWorld::WithComponents<Position, Velocity>;

int main() {
  MyWorld world{};
  return 0;
}
