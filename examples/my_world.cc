#include "ecsify/component.h"

struct Position : ecsify::ComponentMixin<1> {
  float x, y;
};

struct Velocity : ecsify::ComponentMixin<2> {
  float x, y;
};

int main() {
  return 0;
}
