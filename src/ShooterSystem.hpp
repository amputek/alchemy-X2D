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

using namespace entityx;

class MovementSystem : public entityx::System<MovementSystem>
{
public:
    
    EntityManager & _entities;
    EventManager & _events;
    
    explicit MovementSystem( EntityManager & entities, EventManager & events ) : _entities(entities), _events(events) {

    }
    
    
    void configure(entityx::EventManager &events) override {
        
    }
    
    void update( EntityManager &entities, EventManager &events, TimeDelta dt ) override
    {
        ComponentHandle<Movement> movement;
        ComponentHandle<Body> body;
        for (Entity __unused e : entities.entities_with_components(movement, body))
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

class TrajectoryRayCastClosestCallback : public b2RayCastCallback
{
public:
    TrajectoryRayCastClosestCallback() : m_hit(false) {}
    
    float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
    {
        m_hit = true;
        m_point = point;
        m_normal = normal;
        return fraction;
    }
    
    bool m_hit;
    b2Vec2 m_point;
    b2Vec2 m_normal;
};


class ShooterSystem  : public entityx::System<ShooterSystem>
{
public:
    entityx::EntityManager & _entities;
    entityx::EventManager & _events;
    b2World & world;
    
    explicit ShooterSystem( entityx::EntityManager & entities, entityx::EventManager & events, b2World & world ) : _entities(entities), _events(events), world(world) {
    
    }
        
    
    void configure(entityx::EventManager &events) override {

    }
    
    vec2 getTrajectoryPoint( const vec2 & startingPosition, const vec2 & startingVelocity, float stepsAlong, TimeDelta dt )
    {
        stepsAlong *= 2.0f;
        
        //velocity and gravity are given per second but we want time step values here
        float t = 1.0 / 60.0f; // seconds per time step (at 60fps)
        vec2 stepVelocity = t * startingVelocity; // m/s
        vec2 stepGravity = t * t * b2v(world.GetGravity()); // m/s/s
        
        return startingPosition + stepsAlong * stepVelocity + 0.5f * (stepsAlong*stepsAlong+stepsAlong) * stepGravity;
    }
    
    void update( EntityManager &entities, EventManager &events, TimeDelta dt ) override
    {
        ComponentHandle<PotionShooter> shooter;
        for (Entity __unused e : entities.entities_with_components(shooter))
        {
            
      
            if( !shooter->firing ) continue;
      
            vec2 shotVelocity = (shooter->current - shooter->start) * shooter->power;

            if( glm::length( shotVelocity ) > shooter->maxPower )
            {
                shotVelocity = glm::normalize(shotVelocity) * shooter->maxPower;
            }
            
            shooter->currentShotPower = glm::length( shotVelocity );
            
            vec2 current = shooter->start;
            bool endFound = false;
            vec2 end;
            
            for (int i = 0; i < 50; i++) {
                
                vec2 trajectoryPosition;
                
                if( !endFound )
                {
                    trajectoryPosition = getTrajectoryPoint( shooter->start, shotVelocity, i, dt );
               
                    if( i > 0 ) { //avoid degenerate raycast where start and end point are the same
          
                        if( glm::length( b2v(current) - b2v(trajectoryPosition) ) )
                        {
                            TrajectoryRayCastClosestCallback raycastCallback;
                            world.RayCast(&raycastCallback, b2Vec2(current.x, current.y), b2Vec2(trajectoryPosition.x, trajectoryPosition.y) );

                            if ( raycastCallback.m_hit ) {
                                endFound = true;
                                end = b2v(raycastCallback.m_point);
                                trajectoryPosition = end;
                            }
                        }

                    }
                }
                else
                {
                    trajectoryPosition = end;
                }
                    
                current = trajectoryPosition;
                shooter->trajectory[i] = trajectoryPosition;
            }
        
        
            if( shooter->release )
            {
            
               // cout << "Shot power: " << glm::length(velocity) << " / " << velocityLimit << "\n";
                
                _events.emit<AddPotionEvent>( shooter->start, shotVelocity );

                shooter->release = false;
                shooter->firing = false;
            }

          
            
        }
        
    }


};



#endif /* PotionShooterSystem_hpp */
