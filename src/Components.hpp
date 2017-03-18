//
//  Components.hpp
//  AlchemyEntityX
//
//  Created by Rob Dawson on 10/03/2017.
//
//

#ifndef Components_hpp
#define Components_hpp

#include <stdio.h>

using namespace ci;
using namespace std;


enum FragmentType
{
    FRAG_TYPE_GLASS,
    FRAG_TYPE_FIRE
};


struct Body{
public:
    Body() = default;
    ~Body()
    {
        body->GetWorld()->DestroyBody( body );
    }
    
    //A pointer to the actual b2Body. This starts as empty
    b2Body * body;

    //Flags for body state
    bool alive = false;
    bool kill = false;
    bool isColliding = false;
    
    //A BodyDef, for when the body actually gets created
    b2BodyDef bodyDefToAdd;
    
    //Pointers to the fixtures that the body will use, when it's created
    vector<b2FixtureDef*> fixtureDefList;
    
    vec2 position() const
    {
        return vec2(body->GetPosition().x, body->GetPosition().y);
    }
    
    vec2 lastPosition;
    
    vec2 velocity() const
    {
        return vec2(body->GetLinearVelocity().x, body->GetLinearVelocity().y);
    }
    
    void addFixtureDef( b2FixtureDef * fd )
    {
        fixtureDefList.push_back( fd );
    }
    
};

struct PotionShooter {
public:
    PotionShooter() = default;
    
    void startFiring( const vec2 & pos )
    {
        current = pos;
        start = pos;
        firing = true;
    }
    
    void updateFiring( const vec2 & pos )
    {
        current = pos;
    }
    
    void stopFiring()
    {
        release = true;
    }
    
    float shotPowerRatio() const
    {
        return currentShotPower / maxPower;
    }
    
    
    float const power = 3.5f;
    float const maxPower = 12.0f;
    
    
    vec2 trajectory[50];
    vec2 current;
    vec2 start;
    
    float currentShotPower = 0.0f;
    
    bool release = false;
    bool firing = false;
};


struct Potion{
    Potion() = default;
};

struct Fire{
    Fire() = default;
};

struct Floor{
    Floor() = default;
};

struct Fragment{
    Fragment() = default;
    Fragment( FragmentType type ) : type(type){}
    FragmentType type;
};

struct Explosion{
    Explosion() = default;
    Explosion( vec2 pos, float p, vec2 n ) : position(pos), power(p), normal(n){}
    
    vec2 position;
    float power;
    vec2 normal;
};


enum Facing
{
    LEFT,
    RIGHT
};


struct Movement {
public:
    Movement(){
        
    };

    void start( Facing newDir )
    {
        if( newDir == Facing::LEFT && !moveLeft )
        {
            if( moveRight ) swapDirection = true;
            moveLeft = true;
            horizontal = -1;
        }
        
        if( newDir == Facing::RIGHT && !moveRight )
        {
            if( moveLeft ) swapDirection = true;
            moveRight = true;
            horizontal = 1;
        }
    }
    
    void stop( Facing stopDir )
    {
        if( stopDir == Facing::LEFT ) moveLeft = false;
        if( stopDir == Facing::RIGHT ) moveRight = false;
        
        if( moveLeft )
        {
            swapDirection = true;
            horizontal = -1;
        }
        
        if( moveRight )
        {
            swapDirection = true;
            horizontal = 1;
        }
        
        if( !moveLeft && !moveRight )
        {
            horizontal = 0;
            toStop = true;
        }
    }
    
    void startJumping()
    {
        if( jumpTimeout <= 0 )
        {
            jumpTimeout = 30;
            toJump = true;
        }
    }
    
    float getHorizontalMovement() const
    {
        return horizontal * runSpeed;
    }
    
    
    typedef char facing_t;

    facing_t horizontal = 0;
    
    const float maxHorizontalVelocity = 3.0f;
    
    
    bool toStop = false;
    bool toJump = false;
    bool swapDirection = false;
    
    int jumpTimeout = 0;
    
    int numFootContacts = 0;
    
private:
    
    float runSpeed = 1.0f;
    
    bool moveLeft = false;
    bool moveRight = false;

};


#endif /* Components_hpp */
