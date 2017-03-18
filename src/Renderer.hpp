//
//  Renderer.hpp
//  AlchemyEntityX
//
//  Created by Rob Dawson on 10/03/2017.
//
//

#ifndef Renderer_hpp
#define Renderer_hpp

#include <stdio.h>
#include <Box2D/Box2D.h>
#include "cinder/CinderMath.h"

using namespace ci;
using namespace entityx;
using namespace cinder;
using namespace cinder::app;
using namespace gl;


class RendererSystem : public entityx::System<RendererSystem>
{
public:
    
    const float scale = 36.0f;
    FboRef mFbo;
    
    
    explicit RendererSystem()
    {
        gl::Fbo::Format format;
        

        format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
        mFbo = gl::Fbo::create( 800 * getWindowContentScale(), 600 * getWindowContentScale(), format.depthTexture() );

    }
    
    
    void drawAimer( EntityManager &entities )
    {
        ComponentHandle<PotionShooter> shooter;
        for (Entity __unused e : entities.entities_with_components(shooter))
        {
            if( !shooter->firing ) continue;
            
            
            gl::color( 1 - shooter->shotPowerRatio(), shooter->shotPowerRatio(), 0 );
        
            vec2 start = shooter->start * scale;
            vec2 end = shooter->current * scale;
            drawSolidCircle( start, 5.0f );
            drawLine( start, end );
            drawSolidCircle( end, 5.0f );
            for (int i = 1; i < 49; i++) {
                drawLine( shooter->trajectory[i] * scale, shooter->trajectory[i+1] * scale );
            }
            
            
        }
        
    }
    
    void drawExplosions( EntityManager &entities )
    {
        ComponentHandle<Explosion> explosion;
        for (Entity __unused e : entities.entities_with_components(explosion))
        {
            gl::drawLine( explosion->position * 36.0f, (explosion->position * 36.0f) + (explosion->normal * 100.0f) );
        }
        
    }
    
    void drawFire( EntityManager &entities )
    {
        ComponentHandle<Fire> fire;
        ComponentHandle<Body> body;
        ComponentHandle<Expires> expires;
        for (Entity __unused e : entities.entities_with_components(body, fire, expires))
        {
            color( ColorA( 1, 0.5, 0.2, expires->lifeRatio() ) );
            gl::drawSolidCircle( body->position() * scale, scale );
        }
        
    }
    
    void drawFragments( entityx::EntityManager &entities )
    {
        ComponentHandle<Fragment> fragment;
        ComponentHandle<Body> body;
        
        mFbo->bindFramebuffer();
        gl::enableAlphaBlending();  //no additive blending when drawing to Fbo
        gl::color( ColorA(0,0,0,0.05f) );
        gl::drawSolidRect( Rectf(0,0,800,600)) ;
        
        gl::begin( GL_LINES );
        for (entityx::Entity __unused e : entities.entities_with_components(body,fragment))
        {
            if( body->lastPosition != vec2(0,0) )
            {
                float v = glm::length(body->velocity());
                
                gl::color( ColorA8u( min(v * 10.0f,255.0f), min(v * 5.0f,255.0f), min(v * 2.0f,255.0f), 255 ) );
                gl::vertex( body->lastPosition * scale );
                gl::vertex( body->position() * scale );
            }
            body->lastPosition = body->position();
        }
        gl::end();
        
        mFbo->unbindFramebuffer();
        
        gl::ScopedBlendAdditive add;
        gl::color( ColorA(1,1,1,1) );
        gl::draw( mFbo->getColorTexture(), Rectf(0,0,800,600) );

    }

    void update( entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt ) override
    {
        drawAimer(entities);
        drawExplosions(entities);
        drawFragments(entities);
        drawFire(entities);
        
    }
};



class DebugDrawSystem : public b2Draw
{
public:
    
    const float globalAlpha = 0.3f;
    
    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
        
        
        gl::color(color.r, color.g, color.b);
        
        ci::PolyLine2f pl;
        
        for(int i = 0; i < vertexCount; i++)
        {
            pl.push_back( vec2(vertices->x, vertices->y) * scale );
            vertices++;
        }
        
        gl::draw(pl);
    }
    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
        
        
        ci::Path2d pl;
        
        pl.moveTo( vertices->x * scale, vertices->y * scale);
        for(int i = 1; i < vertexCount; i++)
        {
            vertices++;
            pl.lineTo( vec2(vertices->x, vertices->y) * scale );
        }
        pl.close();
        
        gl::color(color.r, color.g, color.b, globalAlpha);
        gl::drawSolid(pl);
        gl::color(color.r, color.g, color.b, 1.0f);
        gl::draw(pl);
        
    }
    void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
    {
        gl::color(color.r, color.g, color.b);
        gl::drawStrokedCircle( vec2(center.x, center.y) * scale, radius * scale);
    }
    
    void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
    {
        gl::color(color.r, color.g, color.b, globalAlpha);
        gl::drawSolidCircle( vec2(center.x, center.y) * scale, radius * scale);
        gl::color(color.r, color.g, color.b, 1.0f);
        gl::drawStrokedCircle( vec2(center.x, center.y) * scale, radius * scale);
        
    }
    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
    {
        gl::color(color.r, color.g, color.b);
        gl::drawLine( vec2(p1.x, p1.y) * scale,  vec2(p2.x, p2.y) * scale);
    }
    void DrawTransform(const b2Transform& xf) {
        
        gl::drawSolidCircle( vec2(xf.p.x, xf.p.y) * scale, 5);
    }
    
    const float scale = 36.0f;
};


#endif /* Renderer_hpp */
