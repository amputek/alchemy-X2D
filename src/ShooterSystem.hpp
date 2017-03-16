//
//  PotionShooterSystem.hpp
//  AlchemyEntityX
//
//  Created by Rob Dawson on 11/03/2017.
//
//

#ifndef PotionShooterSystem_hpp
#define PotionShooterSystem_hpp

#include <stdio.h>
#include <Box2D/Box2D.h>



class MovementSystem : public entityx::System<MovementSystem>
{
public:
    entityx::EntityManager & _entities;
    entityx::EventManager & _events;
    
    explicit MovementSystem( entityx::EntityManager & entities, entityx::EventManager & events ) : _entities(entities), _events(events) {

    }
    
    
    void configure(entityx::EventManager &events) override {
        
    }
    
    void update( entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt ) override
    {
        entityx::ComponentHandle<Movement> movement;
        entityx::ComponentHandle<Body> body;
        for (entityx::Entity __unused e : entities.entities_with_components(movement, body))
        {
            if( !body->alive ) continue;
            
            if( movement->toStop )
            {
                body->body->SetLinearVelocity( b2Vec2(0,0)) ;
                movement->toStop = false;
            }
            
            if( movement->swapDirection )
            {
                body->body->SetLinearVelocity( b2Vec2(body->body->GetLinearVelocity().x * 0.75f, body->body->GetLinearVelocity().y ) );
                movement->swapDirection = false;
            }
            
            
            float x = movement->getHorizontalMovement();
            float y = 0.0f;
            
            if( abs( body->body->GetLinearVelocity().x ) < movement->maxHorizontalVelocity )
            {
                body->body->ApplyLinearImpulse( b2Vec2(x, y), body->body->GetPosition() );
            }
            else
            {
                
                float nx = x > 0.0f ? movement->maxHorizontalVelocity : -movement->maxHorizontalVelocity;
                body->body->SetLinearVelocity( b2Vec2(nx, body->body->GetLinearVelocity().y ) );

            }
            
      //      cout << body->body->GetLinearVelocity().x << "\n";
            
            if( movement->toJump )
            {
                cout << " jumpin' " << "n";
                float impulse = body->body->GetMass() * -8;
                body->body->ApplyLinearImpulse( b2Vec2(0,impulse), body->body->GetPosition() );
                movement->toJump = false;
            }
            
            movement->jumpTimeout--;
            

            
            
            
        }
        
    }
};




class ShooterSystem  : public entityx::System<ShooterSystem>
{
public:
    entityx::EntityManager & _entities;
    entityx::EventManager & _events;
    
    explicit ShooterSystem( entityx::EntityManager & entities, entityx::EventManager & events ) : _entities(entities), _events(events) {
    }
        
    
    void configure(entityx::EventManager &events) override {

    }
    
    void update( entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt ) override
    {
        entityx::ComponentHandle<PotionShooter> shooter;
        for (entityx::Entity __unused e : entities.entities_with_components(shooter))
        {
      
            
            
            if( shooter->release )
            {
                
                float velocityLimit = 12.0f;
                
                vec2 velocity = (shooter->current - shooter->start) * 3.5f;
                if( glm::length( velocity ) > velocityLimit ){
                    velocity = glm::normalize( velocity ) * velocityLimit;
                }
                
               // cout << "Shot power: " << glm::length(velocity) << " / " << velocityLimit << "\n";
                
            
                _events.emit<AddPotionEvent>( shooter->start, velocity );

                //addedObject = factory->createPotion( startPos, velocity );
              //  addObject = true;
                
                shooter->release = false;
            }

            
        }
        
    }
    
    
    void draw( entityx::EntityManager &entities )
    {
        entityx::ComponentHandle<PotionShooter> shooter;
        for (entityx::Entity __unused e : entities.entities_with_components(shooter))
        {
            if( shooter->firing )
            {
                
                vec2 start = shooter->start * 36.0f;
                vec2 end = shooter->current * 36.0f;
                gl::drawLine( start, end );
                gl::drawSolidCircle( start, 5.0f );
                
            
                gl::drawLine( start, end );
                gl::drawSolidCircle( end, 5.0f );
            }
        }
        
    }

};



#endif /* PotionShooterSystem_hpp */
