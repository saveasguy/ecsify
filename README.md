### ECSify

ECSify is yet another library implementing Entity-Component-System. It is header-only and uses C++23 features!

## Usage
To create the world we should build it first:
```C++
#include "ecsify/component.h"
#include "ecsify/world_builder.h"

struct Position : ecsify::ComponentMixin<1> {
  float x, y;
};

struct Velocity : ecsify::ComponentMixin<2> {
  float x, y;
};

auto BuildTheWorld() {
  return ecsify::WorldBuilder{}
             .Component<Position>()
             .Component<Velocity>()
             .Build();
}
```

And now it's easy to create entities and attach components!
```C++
#include "ecsify/entity.h"
#include "ecsify/world.h"

ecsify::Entity CreateTurtle(ecsify::World &world) {
  ecsify::Entity turtle = world.Add();
  world.Add<Position>(turtle);
  world.Get<Position>(turtle) = Position{.x = 0, .y = 0};
  world.Add<Velocity>(turtle);
  world.Get<Velocity>(turtle) = Velocity{.x = 1, .y = 1};
  return turtle;
}
```
