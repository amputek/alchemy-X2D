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
    ContactListener( EventManager & events, EntityManager & entities ) : _events(events), _entities(entities) {}
    
    EventManager & _events;
    EntityManager & _entities;

    

    
    
    void explodePotion( Entity & e, b2Contact * contact )
    {
        
        b2WorldManifold worldManifold;
        contact->GetWorldManifold( &worldManifold );
        
        _events.emit<ExplosionEvent>( b2v(worldManifold.points[0]), e.component<Body>()->velocity(), b2v(worldManifold.normal) );
        e.component<Body>()->kill = true;
    }
    
    void killFragment( Entity & e )
    {
        if( !e.has_component<Fragment>() || (e.component<Expires>()->timePassed() < 0.2f) || e.component<Fragment>()->type != FRAG_TYPE_FIRE ) return;
        e.component<Body>()->kill = true;
    }

    void fragment( Entity & eA, Entity & eB )
    {
        killFragment(eA);
        killFragment(eB);
    }
    
    template <typename C, typename C2>
    bool Test( const Entity & eA, const Entity & eB ) const {
        if( eA.has_component<C>() && eB.has_component<C2>() ) return true;
        if( eB.has_component<C>() && eA.has_component<C2>() ) return true;
        return false;
    }
    

    
    virtual void BeginContact(b2Contact * contact){

        auto eA = _entities.get( entityx::Entity::Id(contact->GetFixtureA()->GetBody()->GetId()) );
        auto eB = _entities.get( entityx::Entity::Id(contact->GetFixtureB()->GetBody()->GetId()) );
        
     //   uint64_t * a = (uint64_t*)contact->GetFixtureA()->GetUserData();
     //   uint64_t * b = (uint64_t*)contact->GetFixtureB()->GetUserData();
        
     //   auto eA2 = _entities->get( entityx::Entity::Id(*a) );
      //  auto eB2 = _entities->get( entityx::Entity::Id(*b) );
        
        
        
        
        
        
        if( eA.has_component<Potion>() )
            explodePotion( eA, contact );
        
        if( eB.has_component<Potion>() )
            explodePotion( eB, contact );
        
        
        fragment(eA, eB);
        

        
        
        
    }
    
    
    // Called when two fixtures cease to touch
    virtual void EndContact(b2Contact* contact){

    }

private:

    
};


#endif /* ContactListener_hpp */
