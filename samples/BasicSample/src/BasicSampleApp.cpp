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
    
    void render();
    void renderDome();
    
    CameraPersp         mCam;
    CameraUi            mCamUi;
    
    gl::BatchRef		mBatch;
    gl::VboRef			mInstanceDataVbo;
    gl::GlslProgRef		mGlsl;
    gl::TextureRef      mTeapotTex;
    
    FullDomeRef         mFullDome;
    
    bool                bShowFisheye = true;
};

const int NUM_TEAPOTS = 100;

void BasicSampleApp::setup()
{
    mCam.lookAt( vec3(0, 0, 0), vec3(0, 0, 1), vec3(0,1,0) );
    mCamUi.setCamera( &mCam );
    mCamUi.connect( getWindow() );
    
    // The following is all based off of the InstancedTeapots sample. Check it out for more info
    mTeapotTex = gl::Texture::create( loadImage( loadAsset("texture.jpg") ) );
    mGlsl = gl::GlslProg::create( loadAsset( "shaders/shader.vert" ), loadAsset( "shaders/shader.frag" ) );
    gl::VboMeshRef mesh = gl::VboMesh::create( geom::Teapot().subdivisions( 4 ) );
    std::vector<vec3> positions;
    for( size_t i = 0; i < NUM_TEAPOTS; ++i ) {
        positions.push_back( vec3(  Rand::randVec3() * vec3(20.0) ) );
    }
    mInstanceDataVbo = gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_DYNAMIC_DRAW );
    geom::BufferLayout instanceDataLayout;
    instanceDataLayout.append( geom::Attrib::CUSTOM_0, 3, 0, 0, 1 /* per instance */ );
    mesh->appendVbo( instanceDataLayout, mInstanceDataVbo );
    mBatch = gl::Batch::create( mesh, mGlsl, { { geom::Attrib::CUSTOM_0, "vInstancePosition" } } );
    
    gl::enableDepthWrite();
    gl::enableDepthRead();
    
    
    // Create the dome renderer!
    // The args we're passing in mean that each individual camera will have an fbo size of 1024x1024, and the final composite fbo will be 2048x2048
    mFullDome = FullDome::create( &mCam, 1024, 2048 );
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
    gl::ScopedTextureBind scTex(mTeapotTex, 0);
    
    mFullDome->bindCamera(FullDome::DomeCam::LEFT);
    render();
    mFullDome->unbindCamera(FullDome::DomeCam::LEFT);
    
    mFullDome->bindCamera(FullDome::DomeCam::RIGHT);
    render();
    mFullDome->unbindCamera(FullDome::DomeCam::RIGHT);
    
    mFullDome->bindCamera(FullDome::DomeCam::UP);
    render();
    mFullDome->unbindCamera(FullDome::DomeCam::UP);
    
    mFullDome->bindCamera(FullDome::DomeCam::DOWN);
    render();
    mFullDome->unbindCamera(FullDome::DomeCam::DOWN);
}

void BasicSampleApp::render()
{
    gl::clear(ColorA(0, 0, 0, 1));
    mBatch->drawInstanced( NUM_TEAPOTS );
    gl::drawSphere(vec3(0, 0, 15), 1);
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 0.5, 0.5, 0.5 ) );
    gl::color(1,1,1);
    
    if(!bShowFisheye){
        gl::setMatrices( mCam );
        mBatch->drawInstanced( NUM_TEAPOTS );
        gl::drawSphere(vec3(0, 0, 15), 1);
    }else{
        renderDome();
        mFullDome->draw();
        
        // You can also render to a texture like so
        // mFullDome->renderToFbo();
        // gl::TextureRef texRef = mFullDome->getFboTexture();
    }
}

CINDER_APP( BasicSampleApp, RendererGl, [&](App::Settings *settings){
    settings->setWindowSize(900, 900);
})
