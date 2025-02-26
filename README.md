### ECSify

ECSify is yet another library implementing Entity-Component-System.

## Usage
To declare a world, first we have to define a set of components to use in our world:
```C++
// CxE stands for Components x Entities
using CxE = ecsify::Components<Position, Velocity>::xEntities;
/* 
Or
using CxE = ecsify::Components<Position>::x<Velocity>::xEntities;
Or
using CxE = ecsify::Components<>::x<Position>::x<Velocity>::xEntities;
*/
```