#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/Rand.h"

#include "FullDome.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace sb;

class BasicSampleApp : public App {
  public:
	void setup() override;
	void resize() override;
	void update() override;
	void draw() override;
    
    void keyDown( KeyEvent event ) override;
    
    void renderDome();
    
    CameraPersp         mCam;
    CameraUi            mCamUi;
    
    gl::BatchRef		mBatch;
    gl::VboRef			mInstanceDataVbo;
    gl::GlslProgRef		mGlsl;
    
    FullDomeRef         mFullDome;
    
    bool                bShowFisheye = true;
};

const int NUM_TEAPOTS = 100;

void BasicSampleApp::setup()
{
    mCam.lookAt( vec3(0, 0, 0), vec3(0, 0, 1), vec3(0,1,0) );
    mCamUi.setCamera( &mCam );
    mCamUi.connect( getWindow() );
    
    // Setup a batched teapot render. Check out the InstancedTeapots sample for more info on this.
    gl::VboMeshRef mesh = gl::VboMesh::create( geom::Teapot().subdivisions( 4 ) );
    
    // create an array of initial per-instance positions laid out in a 2D grid
    std::vector<vec3> positions;
    for( size_t i = 0; i < NUM_TEAPOTS; ++i ) {
        positions.push_back( vec3(  Rand::randVec3() * vec3(20.0) ) );
    }
    
    // create the VBO which will contain per-instance (rather than per-vertex) data
    mInstanceDataVbo = gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_DYNAMIC_DRAW );
    
    // we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
    geom::BufferLayout instanceDataLayout;
    instanceDataLayout.append( geom::Attrib::CUSTOM_0, 3, 0, 0, 1 /* per instance */ );
    
    // now add it to the VboMesh we already made of the Teapot
    mesh->appendVbo( instanceDataLayout, mInstanceDataVbo );
    
    mGlsl = gl::GlslProg::create( loadAsset( "shaders/shader.vert" ), loadAsset( "shaders/shader.frag" ) );
    
    // and finally, build our batch, mapping our CUSTOM_0 attribute to the "vInstancePosition" GLSL vertex attribute
    mBatch = gl::Batch::create( mesh, mGlsl, { { geom::Attrib::CUSTOM_0, "vInstancePosition" } } );
    
    gl::enableDepthWrite();
    gl::enableDepthRead();
    
    mFullDome = FullDome::create( &mCam );
}

void BasicSampleApp::keyDown( KeyEvent event )
{
    bShowFisheye = !bShowFisheye;
}

void BasicSampleApp::resize()
{
    mCam.setAspectRatio( getWindowAspectRatio() );
}

void BasicSampleApp::update()
{
}

void BasicSampleApp::renderDome()
{
    mFullDome->bindCamera(FullDome::DomeCam::LEFT);
    mBatch->drawInstanced( NUM_TEAPOTS );
    mFullDome->unbindCamera(FullDome::DomeCam::LEFT);
    
    mFullDome->bindCamera(FullDome::DomeCam::RIGHT);
    mBatch->drawInstanced( NUM_TEAPOTS );
    mFullDome->unbindCamera(FullDome::DomeCam::RIGHT);
    
    mFullDome->bindCamera(FullDome::DomeCam::UP);
    mBatch->drawInstanced( NUM_TEAPOTS );
    mFullDome->unbindCamera(FullDome::DomeCam::UP);
    
    mFullDome->bindCamera(FullDome::DomeCam::DOWN);
    mBatch->drawInstanced( NUM_TEAPOTS );
    mFullDome->unbindCamera(FullDome::DomeCam::DOWN);
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 1, 1, 1 ) );
    gl::color(1,1,1);
    
    if(!bShowFisheye){
        gl::setMatrices( mCam );
//        gl::rotate(getElapsedSeconds() * 0.1, vec3(0, 1, 0));
        mBatch->drawInstanced( NUM_TEAPOTS );
    }else{
        renderDome();
        mFullDome->draw();
    }
}

CINDER_APP( BasicSampleApp, RendererGl, [&](App::Settings *settings){
    settings->setWindowSize(900, 900);
})
