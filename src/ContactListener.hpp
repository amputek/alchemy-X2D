#ifndef ContactListener_hpp
#define ContactListener_hpp

#include <stdio.h>
#include <Box2D/Box2D.h>
#include "FactorySystem.hpp"
#include "CollisionSystem.hpp"
#include "Components.hpp"

using namespace ci;
using namespace entityx;





class ContactListener : public b2ContactListener
{
public:
    ContactListener( EventManager * events, EntityManager * entities ) : _events(events), _entities(entities) {}
    
    EventManager * _events;
    EntityManager * _entities;

    
    void explodePotion( Entity & e, b2Contact * contact )
    {
        
        b2WorldManifold worldManifold;
        contact->GetWorldManifold( &worldManifold );
        
        _events->emit<ExplosionEvent>( b2v(worldManifold.points[0]), e.component<Body>()->velocity(), b2v(worldManifold.normal) );
        e.component<Body>()->kill = true;
    }
    
//    void FragmentFloor( Entity & e )
//    {
//        
//    }

    virtual void BeginContact(b2Contact * contact){
    
        auto eA = _entities->get( entityx::Entity::Id(contact->GetFixtureA()->GetBody()->GetId()) );
        auto eB = _entities->get( entityx::Entity::Id(contact->GetFixtureB()->GetBody()->GetId()) );
        

        if( eA.has_component<Potion>() )
            explodePotion( eA, contact );
        
        if( eB.has_component<Potion>() )
            explodePotion( eB, contact );
        
        
        
        
        
    }
    
    
    // Called when two fixtures cease to touch
    virtual void EndContact(b2Contact* contact){

    }

private:

    
};


#endif /* ContactListener_hpp */