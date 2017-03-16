#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include <unordered_set>
#include <Box2D/Box2D.h>

#include "entityx/Entity.h"
#include "soso/BehaviorSystem.h"
#include "soso/ExpiresSystem.h"
#include "soso/Expires.h"

#include "Components.hpp"
#include "FactorySystem.hpp"
#include "CollisionSystem.hpp"
#include "ShooterSystem.hpp"
#include "ContactListener.hpp"
#include "Renderer.hpp"


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace soso;

DebugDrawSystem debugDrawSystem;

class AlchemyEntityXApp : public App {

public:
    AlchemyEntityXApp();
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    void keyUp( KeyEvent event ) override;
	void update() override;
	void draw() override;
    void createTestEntities( b2World * world );
    
private:
    entityx::EventManager  _events;
    entityx::EntityManager _entities;
    entityx::SystemManager _systems;
    ci::Timer              _frame_timer;
    
    b2World * mWorld;
    ContactListener * mContactListener;

    ComponentHandle<PotionShooter> shooter;
    
    
    ComponentHandle<Movement> playerMvt;
};

void AlchemyEntityXApp::setup()
{
    
    b2Vec2 gravity( 0.0f, 10.0f );
    mWorld = new b2World( gravity );

    _systems.add<BehaviorSystem>(_entities);
    _systems.add<ExpiresSystem>();
    _systems.add<FactorySystem>(mWorld, _entities);
    _systems.add<RendererSystem>();
    _systems.add<ShooterSystem>(_entities, _events);
    _systems.add<ExplosionSystem>(_entities);
    _systems.add<MovementSystem>(_entities, _events);
    _systems.configure();
    
    
    mContactListener = new ContactListener( &_events, &_entities );
    mWorld->SetContactListener(mContactListener);
    
    
    mWorld->SetDebugDraw( &debugDrawSystem );
    debugDrawSystem.SetFlags( b2Draw::e_shapeBit );

    _systems.system<FactorySystem>()->createGround(vec2( getWindowWidth() / 2 / debugDrawSystem.scale, getWindowHeight() / debugDrawSystem.scale), getWindowWidth() / debugDrawSystem.scale, 1.0f );
    
    _systems.system<FactorySystem>()->createGround(vec2( 0, getWindowHeight() / 2 / debugDrawSystem.scale), 1.0f, getWindowHeight() / debugDrawSystem.scale );
    
    playerMvt = _systems.system<FactorySystem>()->createPlayer( vec2(5,5) );

    auto e =_entities.create();
    shooter = e.assign<PotionShooter>();

}

AlchemyEntityXApp::AlchemyEntityXApp() : _entities(_events), _systems(_entities, _events){ }

void AlchemyEntityXApp::mouseDown( MouseEvent event )
{
    shooter->startFiring( vec2(event.getPos()) / debugDrawSystem.scale );
}

void AlchemyEntityXApp::mouseDrag( MouseEvent event )
{
    shooter->updateFiring( vec2(event.getPos()) / debugDrawSystem.scale );
}

void AlchemyEntityXApp::mouseUp( MouseEvent event )
{
    shooter->stopFiring();
}

void AlchemyEntityXApp::keyDown( KeyEvent event )
{
    if( event.getChar() == 'a' ) playerMvt->start( Facing::LEFT );
    if( event.getChar() == 'd' ) playerMvt->start( Facing::RIGHT );
    if( event.getChar() == ' ' ) playerMvt->startJumping();
}

void AlchemyEntityXApp::keyUp( KeyEvent event )
{
    if( event.getChar() == 'a' ) playerMvt->stop( Facing::LEFT );
    if( event.getChar() == 'd' ) playerMvt->stop( Facing::RIGHT );
}


void AlchemyEntityXApp::update()
{
    auto dt = _frame_timer.getSeconds();
    if (dt < std::numeric_limits<double>::epsilon() || dt > 0.1)
    {
        dt = 1.0 / 60.0;
    }
    _frame_timer.start();
    

    for( int i = 0; i < 3; i++)
        mWorld->Step( dt, 8, 3 );
    
    _systems.update<BehaviorSystem>(dt);
    _systems.update<ExpiresSystem>(dt);
    _systems.update<MovementSystem>(dt);
    _systems.update<ShooterSystem>(dt);
    _systems.update<ExplosionSystem>(dt);
    _systems.update<FactorySystem>(dt);

    
     
}

void AlchemyEntityXApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    gl::enableAlphaBlending();
    
//    cout << "Entities: " << "\n";
//    for (auto e : _entities.entities_with_components<Body>())
//    {
//        auto index = to_string(e.id().index());
//        cout << index << " " << e.valid() << endl;
//    }
    
    _systems.system<ShooterSystem>()->draw( _entities );
    _systems.system<ExplosionSystem>()->draw( _entities );
    
    mWorld->DrawDebugData();
    
    _systems.update<RendererSystem>(1.0/60.0f);
    gl::drawString( "Framerate: " + to_string( roundf(getAverageFps()) ), vec2( 10.0f, 10.0f ) );
}

CINDER_APP( AlchemyEntityXApp, RendererGl( RendererGl::Options().msaa( 4 ) ), [&]( App::Settings *settings ) {
    settings->setWindowSize(800, 600);
    settings->setFrameRate(60.0f);
    settings->setHighDensityDisplayEnabled();

})
