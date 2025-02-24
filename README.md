### ECSify

ECSify is yet another library implementing Entity-Component-System.

## Usage
To declare a world, first we have to define a set of components to use in our world:
```C++
using MyWorld = ecsify::World<>::With<Position, Velocity>;
/* Or
using MyWorld = ecsify::World<>::With<Position>::With<Velocity>;
*/
```