#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TinderTestApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void TinderTestApp::setup()
{
}

void TinderTestApp::mouseDown( MouseEvent event )
{
}

void TinderTestApp::update()
{
}

void TinderTestApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( TinderTestApp, RendererGl )
