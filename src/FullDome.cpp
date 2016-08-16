#include "FullDome.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace sb;

FullDome::FullDome(ci::CameraPersp *cam, const int &fboSize, const int &renderFboSize) {

    // Save the users camera for orientation
    mUserCam = cam;
    
    // This is the camera we'll use to render the scene 4x in different orientations
	mCam.setAspectRatio(1.0);
	mCam.setFov(90);
	mCam.setFovHorizontal(90);
	mCam.lookAt(vec3(0), vec3(0,0,1),vec3(0,1,0));

    // This is the camera we'll use to render the skinned meshes for our final fisheye image
	mScreenCam.setAspectRatio(1.0);
	mScreenCam.setEyePoint(vec3(0, 0, 1.35));
	mScreenCam.setWorldUp(vec3(0, 1, 0));
    
    gl::GlslProgRef texShader = gl::getStockShader(gl::ShaderDef().texture());
    texShader->uniform("uTex0", 0);
    
	mBU = gl::Batch::create(*loadObj(FULLDOME_TOP_OBJ), texShader);
	mBD = gl::Batch::create(*loadObj(FULLDOME_BOTTOM_OBJ), texShader);
	mBL = gl::Batch::create(*loadObj(FULLDOME_LEFT_OBJ), texShader);
	mBR = gl::Batch::create(*loadObj(FULLDOME_RIGHT_OBJ), texShader);

//	int fboSize = 2160;//2k
	//int fboSize = 3840;//4k

	mFboL = gl::Fbo::create(fboSize, fboSize);
	mFboR = gl::Fbo::create(fboSize, fboSize);
	mFboU = gl::Fbo::create(fboSize, fboSize);
	mFboD = gl::Fbo::create(fboSize, fboSize);
    
	mCompositeFbo = gl::Fbo::create(renderFboSize, renderFboSize);

	setupQuats();
}

TriMeshRef FullDome::loadObj(fs::path loc)
{
    console() << "Loading :: " << loc << endl;
    
	ObjLoader loader(loadResource(loc));
	TriMeshRef tm = TriMesh::create(loader);
    
    if( ! loader.getAvailableAttribs().count( geom::NORMAL ) ){
        tm->recalculateNormals();
    }

    return tm;//gl::VboMesh::create(*tm);
}

void FullDome::resize() {
	mCam.setAspectRatio(getWindowAspectRatio());
}

void FullDome::setupQuats()
{
	glm::fquat fwd = mUserCam->getOrientation();
    /*
	fwd = glm::rotate(fwd, (float)toRadians(-180.0f), vec3(0, 1, 0));
	fwd = glm::rotate(fwd, (float)toRadians(-90.0f), vec3(0, 0, 1));
	fwd = glm::rotate(fwd, (float)toRadians(90.0), vec3(0, 1, 0));
     */
    
	//  L
	mQuatL = fwd;
	mQuatL = glm::rotate(mQuatL, (float)toRadians(-45.0f), vec3(1, 0, 0));

	// R
	mQuatR = fwd;
	mQuatR = glm::rotate(mQuatR, (float)toRadians(45.0f), vec3(1, 0, 0));

	// U
	mQuatU = fwd;
	mQuatU = glm::rotate(mQuatU, (float)toRadians(90.0), vec3(0, 1, 0));
	mQuatU = glm::rotate(mQuatU, (float)toRadians(-45.0), vec3(0, 0, 1));

	// D
	mQuatD = fwd;
	mQuatD = glm::rotate(mQuatD, (float)toRadians(-90.0), vec3(0, 1, 0));
	mQuatD = glm::rotate(mQuatD, (float)toRadians(45.0), vec3(0, 0, 1));
}

void FullDome::bindCamera(DomeCam dir) {

	setupQuats();

	mCurViewport = gl::getViewport();
	gl::pushMatrices();

	switch (dir) {
	case UP:
		mFboU->bindFramebuffer();
		mCam.setOrientation(mQuatU);
		gl::viewport(mFboU->getSize());
		break;
	case DOWN:
		mFboD->bindFramebuffer();
		mCam.setOrientation(mQuatD);
		gl::viewport(mFboD->getSize());
		break;
	case LEFT:
		mFboL->bindFramebuffer();
		mCam.setOrientation(mQuatL);
		gl::viewport(mFboL->getSize());
		break;
	case RIGHT:
		mFboR->bindFramebuffer();
		mCam.setOrientation(mQuatR);
		gl::viewport(mFboR->getSize());
		break;
	default:
		break;
	}
	gl::setMatrices(mCam);
}

void FullDome::unbindCamera(DomeCam dir) {
	switch (dir) {
	case UP:
		mFboU->unbindFramebuffer();
		break;
	case DOWN:
		mFboD->unbindFramebuffer();
		break;
	case LEFT:
		mFboL->unbindFramebuffer();
		break;
	case RIGHT:
		mFboR->unbindFramebuffer();
		break;
	}
	gl::popMatrices();
	gl::viewport( mCurViewport.first, mCurViewport.second  );
}

void FullDome::renderToFbo() {
	gl::ScopedFramebuffer scComp(mCompositeFbo);
	gl::clear();

	gl::pushMatrices(); {
		gl::ScopedViewport scCompVp( mCompositeFbo->getSize() );

		mScreenCam.setEyePoint(vec3(0, 0, 1));
		mScreenCam.setOrtho(-1, 1, -1, 1, 0.0001f, 5000.f);
		gl::setMatrices(mScreenCam);
		{
			gl::ScopedTextureBind scTx1(mFboU->getColorTexture());
			mBU->draw();
		}
		{
			gl::ScopedTextureBind scTx2(mFboD->getColorTexture());
			mBD->draw();
		}
		{
			gl::ScopedTextureBind scTx3(mFboL->getColorTexture());
			mBL->draw();
		}
		{
			gl::ScopedTextureBind scTx4(mFboR->getColorTexture());
			mBR->draw();
		}

	}gl::popMatrices();
}

void FullDome::draw() {
	gl::color(Color::white());

	gl::pushMatrices(); {
		mScreenCam.setEyePoint(vec3(0, 0, 1));
		mScreenCam.setOrtho(-1, 1, -1, 1, 0.0001f, 5000.f);
		gl::setMatrices(mScreenCam);
		{
			gl::ScopedTextureBind scTx1(mFboU->getColorTexture());
			mBU->draw();
		}
		{
			gl::ScopedTextureBind scTx2(mFboD->getColorTexture());
			mBD->draw();
		}
		{
			gl::ScopedTextureBind scTx3(mFboL->getColorTexture());
			mBL->draw();
		}
		{
			gl::ScopedTextureBind scTx4(mFboR->getColorTexture());
			mBR->draw();
		}
        
	}gl::popMatrices();
}