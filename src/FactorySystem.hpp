//
//  FactorySystem.hpp
//  AlchemyEntityX
//
//  Created by Rob Dawson on 10/03/2017.
//
//

#ifndef FactorySystem_hpp
#define FactorySystem_hpp

#include <stdio.h>
#include <Box2D/Box2D.h>

#include "Components.hpp"

using namespace entityx;
using namespace std;



struct ExplosionEvent {
    ExplosionEvent( vec2 pos, vec2 vel, vec2 normal ) : position(pos), velocity(vel), normal(normal) { }
    vec2 position;
    vec2 velocity; //if the explosion has velocity
    vec2 normal;
};

struct CreateFireEvent {
    CreateFireEvent( vec2 pos ) : position(pos){ }
    vec2 position;
    vec2 normal;
};

struct AddPotionEvent {
    AddPotionEvent( vec2 pos, vec2 vel ) : position(pos), velocity(vel) { }
    vec2 position;
    vec2 velocity;
};

enum PhysicsType{
    PHYSICS_TYPE_POTION      = 1,
    PHYSICS_TYPE_FLOOR       = 2,
    PHYSICS_TYPE_GLASS       = 4,
    PHYSICS_TYPE_PLAYER      = 8,
    PHYSICS_TYPE_FIRE        = 16,
    PHYSICS_TYPE_FIRE_FRAGMENT = 32
};

enum BodyType{
    BODY_TYPE_KINEMTAIC,
    BODY_TYPE_DYNAMIC
};





class ExplosionSystem : public entityx::System<ExplosionSystem>, public entityx::Receiver<ExplosionSystem>
{
public:
    explicit ExplosionSystem( entityx::EntityManager & entities ) : _entities(entities)
    { }
        
    
    void configure(entityx::EventManager &events) override {
        events.subscribe<ExplosionEvent>(*this);
    }
    
    void receive(const ExplosionEvent &explosion) {

        auto exp = _entities.create();
        exp.assign<Explosion>( explosion.position, 2.3f, explosion.normal );
        exp.assign<soso::Expires>( 0.1f );
    }
    
    void update( entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt ) override
    {

        std::vector<vec2> explosionPositions;
        
        entityx::ComponentHandle<soso::Expires> expires;
        entityx::ComponentHandle<Explosion> explosion;
        entityx::ComponentHandle<Body> body;
        
        for (entityx::Entity __unused e : entities.entities_with_components(body))
        {
            if( !body->alive ) continue;
            if( body->body->GetType() != b2_dynamicBody ) continue;
            
            for (entityx::Entity __unused e2 : entities.entities_with_components(explosion, expires))
            {
                
                vec2 diff = body->position() - explosion->position;
                float dist = glm::length( diff );
                
                if( dist < 0.05f ) dist = 0.05f;
                
                float inverseSquareDist = 1.0f / (dist * dist);
                
                vec2 mvtDirection = glm::normalize( diff );
                
                float timeMod = expires->time;
            
                body->body->ApplyForceToCenter( mvtDirection * explosion->power * inverseSquareDist * timeMod );
                
            }
            
        }
    }
    
    void draw( entityx::EntityManager &entities )
    {
        entityx::ComponentHandle<Explosion> explosion;
        for (entityx::Entity __unused e : entities.entities_with_components(explosion))
        {
       
            gl::drawLine( explosion->position * 36.0f, (explosion->position * 36.0f) + (explosion->normal * 100.0f) );
            
        }
        
    }

    
    
    
    entityx::EntityManager & _entities;
};


class FactorySystem : public entityx::System<FactorySystem>, public entityx::Receiver<FactorySystem>
{
    
public:
    
    explicit FactorySystem( b2World * world, entityx::EntityManager & entities ) : _entities(entities), _world(world) {
    
        glassFragmentFixture = * new b2FixtureDef();
        b2PolygonShape * quad = new b2PolygonShape();
        quad->SetAsBox( randFloat(0.03,0.1), randFloat(0.03,0.1), b2Vec2(0,0), 0.0f );
        glassFragmentFixture.shape = quad;
        glassFragmentFixture.density = 1.0f;
        glassFragmentFixture.friction = 1.0f;
        glassFragmentFixture.restitution = 0.4f;
        glassFragmentFixture.filter.categoryBits = PHYSICS_TYPE_GLASS;
        glassFragmentFixture.filter.maskBits = PHYSICS_TYPE_GLASS | PHYSICS_TYPE_FLOOR;
    
        fragmentFixture = * new b2FixtureDef();
        b2CircleShape * c =  new b2CircleShape();
        c->m_radius = 0.05f;
        fragmentFixture.shape = c;
        fragmentFixture.density = 1.0f;
        fragmentFixture.friction = 1.0f;
        fragmentFixture.restitution = 0.4f;
        fragmentFixture.filter.categoryBits = PHYSICS_TYPE_FIRE_FRAGMENT;
        fragmentFixture.filter.maskBits = PHYSICS_TYPE_FLOOR;
        
        potionFixture = * new b2FixtureDef();
        b2CircleShape * circle = new b2CircleShape();
        circle->m_radius = 0.15f;
        potionFixture.shape = circle;
        potionFixture.density = 1.0f;
        potionFixture.friction = 1.0f;
        potionFixture.restitution = 0.4f;
        potionFixture.filter.categoryBits = PHYSICS_TYPE_POTION;
        potionFixture.filter.maskBits = PHYSICS_TYPE_POTION | PHYSICS_TYPE_FLOOR | PHYSICS_TYPE_PLAYER;
        
        groundFixture = * new b2FixtureDef();
        b2PolygonShape * quad2 = new b2PolygonShape();
        quad2->SetAsBox( 1, 1, b2Vec2(0,0), 0.0f );
        groundFixture.shape = quad2;
        groundFixture.density = 1.0f;
//        groundFixture.friction = 1.0f;
        groundFixture.restitution = 0.0f;
        groundFixture.filter.categoryBits = PHYSICS_TYPE_FLOOR;
        groundFixture.filter.maskBits = PHYSICS_TYPE_POTION | PHYSICS_TYPE_GLASS | PHYSICS_TYPE_FLOOR | PHYSICS_TYPE_PLAYER | PHYSICS_TYPE_FIRE_FRAGMENT;
        
                fireFixture = * new b2FixtureDef();
        quad = new b2PolygonShape();
        quad->SetAsBox( 1, 1, b2Vec2(0,0), 0.0f );
        fireFixture.shape = quad;
        fireFixture.density = 1.0f;
        fireFixture.friction = 1.0f;
        fireFixture.restitution = 0.0f;
        fireFixture.filter.categoryBits = PHYSICS_TYPE_FIRE;
    }


    void configure(entityx::EventManager &events) override {
        events.subscribe<ExplosionEvent>(*this);
        events.subscribe<AddPotionEvent>(*this);
    }
    
    void receive(const ExplosionEvent &explosion) {
        for( int i = 0; i < 5; i ++ )
        {
            createGlassFragment( explosion.position + randVec2() * 0.05f, explosion.velocity );
        }
        for( int i = 0; i < 5; i ++ )
        {
            createFragment( explosion.position + randVec2() * 0.05f, explosion.velocity );
        }
        createFire( explosion.position );
    }
    
    void receive(const AddPotionEvent &pot) {
        createPotion( pot.position, pot.velocity );
    }
    
    void update( entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt ) override
    {
        entityx::ComponentHandle<Body> body;
        for (entityx::Entity __unused e : entities.entities_with_components(body))
        {
            if(body->alive == false)
                generate( e );
            
            if(body->alive && body->kill )
            {
                e.destroy();
            }
        }
    
    }
    
    void createGround( const vec2 &pos, float width, float height)
    {
    
        auto ground = _entities.create();
        
        auto body = ground.assign<Body>();

        b2FixtureDef * tempGroundFixture = new b2FixtureDef( groundFixture );

        tempGroundFixture->shape =  new b2PolygonShape();
        ((b2PolygonShape*)tempGroundFixture->shape)->SetAsBox( width / 2, height / 2 , b2Vec2(0,0), 0.0f );

        body->bodyDefToAdd.position.Set( pos.x, pos.y );
        body->addFixtureDef( tempGroundFixture );
    }
    
    void createPotion( const vec2 &pos, const vec2 &velocity )
    {
        auto potion = _entities.create();
        
        auto body = potion.assign<Body>();
        potion.assign<Potion>();
        
        createDynamicBodyDef( pos, velocity, body->bodyDefToAdd );
        
        b2FixtureDef * tempPotionFixture = new b2FixtureDef( potionFixture );

        body->addFixtureDef( tempPotionFixture );
    }
    
    void createFragment( const vec2 &pos, const vec2 &velocity )
    {
        //Create entity
        auto frag = _entities.create();
        
        //Assign components to entity
        auto body = frag.assign<Body>( );
        frag.assign<Fragment>( FRAG_TYPE_FIRE );
        
        createDynamicBodyDef( pos, velocity, body->bodyDefToAdd );
        
        //Copy the source fixturedef, and then change whatever we need to
        b2FixtureDef * tempFragmentFixture = new b2FixtureDef( fragmentFixture );
        
        //Add the fixture to the body component's fixturedef list --- but DONT actually add it to the box2d Body yet. This needs to wait until an actual STEP
        body->addFixtureDef( tempFragmentFixture );
    }
    
    
    void createGlassFragment( const vec2 &pos, const vec2 &velocity )
    {
        //Create entity
        auto frag = _entities.create();
        
        //Assign components to entity
        auto body = frag.assign<Body>( );
        frag.assign<Fragment>( FRAG_TYPE_GLASS );
        frag.assign<soso::Expires>( randFloat(1.0f, 5.0f));
        
        createDynamicBodyDef( pos, velocity, body->bodyDefToAdd );
        
        //Copy the source fixturedef, and then change whatever we need to
        b2FixtureDef * tempFragmentFixture = new b2FixtureDef( glassFragmentFixture );
        
        //Have to create a new shape
        tempFragmentFixture->shape = new b2PolygonShape();
        ((b2PolygonShape*)tempFragmentFixture->shape)->SetAsBox( randFloat(0.03,0.12), randFloat(0.03,0.12), b2Vec2(0,0), 0.0f );
        
        //Add the fixture to the body component's fixturedef list --- but DONT actually add it to the box2d Body yet. This needs to wait until an actual STEP
        body->addFixtureDef( tempFragmentFixture );
    }
    
    void createFire( const vec2 &pos )
    {
        auto fire = _entities.create();
        auto body =  fire.assign<Body>();
        b2FixtureDef * tempFireFixture = new b2FixtureDef( fireFixture );
        body->bodyDefToAdd.position.Set( round(pos.x / 2.0f) * 2.0f , round(pos.y / 2.0f) * 2.0f );
        body->addFixtureDef( tempFireFixture );
    }
    
    
    ComponentHandle<Movement> createPlayer( const vec2 &pos )
    {
        auto player = _entities.create();
        auto body =  player.assign<Body>();
        auto mvt = player.assign<Movement>();

        b2FixtureDef * playerFixture = new b2FixtureDef();
        b2PolygonShape * quad = new b2PolygonShape();
        quad->SetAsBox( 0.5f, 1.0f, b2Vec2(0,0), 0.0f );
        playerFixture->shape = quad;
 //       playerFixture->density = 1.0f;
        playerFixture->friction = 0.0f;
//        playerFixture->restitution = 0.0f;
        playerFixture->filter.categoryBits = PHYSICS_TYPE_PLAYER;
        playerFixture->filter.maskBits = PHYSICS_TYPE_POTION | PHYSICS_TYPE_FLOOR;
        
        body->bodyDefToAdd.position.Set( pos.x, pos.y );
        body->bodyDefToAdd.type = b2_dynamicBody;
        body->addFixtureDef( playerFixture );
        
        return mvt;
    }
    
    
private:
    
    b2World * _world;
    entityx::EntityManager & _entities;
    
    b2FixtureDef glassFragmentFixture;
    b2FixtureDef fragmentFixture;
    b2FixtureDef potionFixture;
    b2FixtureDef groundFixture;
    b2FixtureDef fireFixture;

    void createDynamicBodyDef( const vec2 &pos, const vec2 &velocity, b2BodyDef & bodyDef )
    {
        bodyDef.position.Set( pos.x, pos.y );
        bodyDef.type = b2_dynamicBody;
        bodyDef.linearVelocity = b2Vec2(velocity);
        bodyDef.angularVelocity = randFloat(-2.0f, 2.0f );
        bodyDef.linearDamping = 0.0f;
    }

    void generate( entityx::Entity & entity )
    {
        auto bodyComponent = entity.component<Body>();
        bodyComponent->alive = true;

        //Save the ID of the entity in the body - so we can reference it back at collisions
        bodyComponent->bodyDefToAdd.id = entity.id().id();
        
        //Add the body to the world, using the saved bodyDef inside the entity
        auto body = bodyComponent->body = _world->CreateBody( &bodyComponent->bodyDefToAdd );
        
        //Add the fixtures to the body, using the saved fixtureDefs. delete them as we go
        for (auto it = bodyComponent->fixtureDefList.begin(); it != bodyComponent->fixtureDefList.end(); ++it){
            body->CreateFixture( *it );
            delete *it;
        }
        
        //If body is 'moveable' -- eg a PC or NPC
        if( entity.has_component<Movement>() )
        {
            bodyComponent->body->SetFixedRotation( true );
            bodyComponent->body->SetAngularVelocity( 0.0 );
            bodyComponent->body->SetLinearDamping(0.0f);
        }
    }
    
};



#endif /* FactorySystem_hpp */
