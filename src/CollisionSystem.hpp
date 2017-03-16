//
//  CollisionSystem.hpp
//  AlchemyEntityX
//
//  Created by Rob Dawson on 15/03/2017.
//
//

#ifndef CollisionSystem_hpp
#define CollisionSystem_hpp

#include <stdio.h>
#include <Box2D/Box2D.h>
#include "entityx/Entity.h"
#include "Components.hpp"

using namespace entityx;
using namespace std;
using namespace soso;


enum COLLISION_TYPE
{
    COLLISION_BEGIN,
    COLLISION_END
    
};

inline vec2 b2v( b2Vec2 b )
{
    return vec2(b.x,b.y);
}






#endif /* CollisionSystem_hpp */
