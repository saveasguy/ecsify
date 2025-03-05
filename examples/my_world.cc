#include <ecsify/world.h>

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
