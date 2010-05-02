#include "cinder/app/AppBasic.h"
#include "cinder/System.h"
#include "cinder/Rand.h"
using namespace ci;
using namespace ci::app;

#include <vector>
#include <map>
#include <list>
using namespace std;

struct TouchPoint {
	TouchPoint() {}
	TouchPoint( const Vec2f &initialPt, const Color &color ) : mColor( color ), mTimeOfDeath( -1.0 ) 
	{
		mPoints.push_back( initialPt ); 
	}
	
	void addPoint( const Vec2f &pt ) { mPoints.push_back( pt ); }
	
	void draw() const
	{
		if( mTimeOfDeath > 0 ) // are we dying? then fade out
			gl::color( ColorA( mColor, ( mTimeOfDeath - getElapsedSeconds() ) / 2.0 ) );
		else
			gl::color( mColor );

		glBegin( GL_LINE_STRIP );
		// iterate across our list of points, and pass each one to OpenGL
		for( vector<Vec2f>::const_iterator pointIter = mPoints.begin(); pointIter != mPoints.end(); ++pointIter ) {
			glVertex2f( pointIter->x, pointIter->y );
		}
		// tell OpenGL to actually draw the lines now
		glEnd();
	}
	
	void startDying() { mTimeOfDeath = getElapsedSeconds() + 2.0f; } // two seconds til dead
	
	bool isDead() const { return getElapsedSeconds() > mTimeOfDeath; }
	
	vector<Vec2f>	mPoints;
	Color			mColor;
	float			mTimeOfDeath;
};

// We'll create a new Cinder Application by deriving from the BasicApp class
class MultiTouchApp : public AppBasic {
 public:
	void	prepareSettings( Settings *settings );

	void	touchesBegan( TouchEvent event );
	void	touchesMoved( TouchEvent event );
	void	touchesEnded( TouchEvent event );

	void	setup();
	void	draw();
	
	map<uint32_t,TouchPoint>	mActivePoints;
	list<TouchPoint>			mDyingPoints;
};

void MultiTouchApp::setup()
{
	console() << "MT: " << System::hasMultiTouch() << " Max points: " << System::getMaxMultiTouchPoints() << std::endl;
}

void MultiTouchApp::prepareSettings( Settings *settings )
{
	settings->enableMultiTouch();
}

void MultiTouchApp::touchesBegan( TouchEvent event )
{
	console() << "Touches began:" << std::endl << event << std::endl;
	for( vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt ) {
		Color newColor( CM_HSV, Rand::randFloat(), 1, 1 );
		mActivePoints.insert( make_pair( touchIt->getId(), TouchPoint( touchIt->getPos(), newColor ) ) );
	}
}

void MultiTouchApp::touchesMoved( TouchEvent event )
{
	console() << "Touches moved:" << std::endl << event << std::endl;
	for( vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt )
		mActivePoints[touchIt->getId()].addPoint( touchIt->getPos() );
}

void MultiTouchApp::touchesEnded( TouchEvent event )
{
	console() << "Touches ended:" << std::endl << event << std::endl;
	for( vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt ) {
		mActivePoints[touchIt->getId()].startDying();
		mDyingPoints.push_back( mActivePoints[touchIt->getId()] );
		mActivePoints.erase( touchIt->getId() );
	}
}


void MultiTouchApp::draw()
{
	gl::enableAlphaBlending();
	gl::clear( Color( 0.1f, 0.1f, 0.1f ) );

	for( map<uint32_t,TouchPoint>::const_iterator activeIt = mActivePoints.begin(); activeIt != mActivePoints.end(); ++activeIt ) {
		activeIt->second.draw();
	}

	for( list<TouchPoint>::const_iterator dyingIt = mDyingPoints.begin(); dyingIt != mDyingPoints.end(); ) {
		dyingIt->draw();
		if( dyingIt->isDead() )
			dyingIt = mDyingPoints.erase( dyingIt );
		else
			++dyingIt;
	}
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( MultiTouchApp, RendererGl )
