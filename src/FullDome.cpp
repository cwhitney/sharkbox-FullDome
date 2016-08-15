#include "FullDome.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace sb;

FullDome::FullDome(ci::CameraPersp *cam, const int &fboSize) {

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

	mBU = gl::Batch::create(loadObj("fulldome/top.obj"), gl::getStockShader(gl::ShaderDef().texture()));
	mBD = gl::Batch::create(loadObj("fulldome/bottom.obj"), gl::getStockShader(gl::ShaderDef().texture()));
	mBL = gl::Batch::create(loadObj("fulldome/left.obj"), gl::getStockShader(gl::ShaderDef().texture()));
	mBR = gl::Batch::create(loadObj("fulldome/right.obj"), gl::getStockShader(gl::ShaderDef().texture()));

//	int fboSize = 2160;//2k
	//int fboSize = 3840;//4k

	mFboL = gl::Fbo::create(fboSize, fboSize);
	mFboR = gl::Fbo::create(fboSize, fboSize);
	mFboU = gl::Fbo::create(fboSize, fboSize);
	mFboD = gl::Fbo::create(fboSize, fboSize);
    
    int CANVAS_WIDTH = 2048;
    int CANVAS_HEIGHT = 2048;

	mCompositeFbo = gl::Fbo::create(CANVAS_WIDTH, CANVAS_HEIGHT);

	setupQuats();
}

gl::VboMeshRef FullDome::loadObj(std::string loc)
{
	ObjLoader loader(loadAsset(loc));
	TriMesh tm = ci::TriMesh(loader);

	return gl::VboMesh::create(tm);
}

void FullDome::resize() {
	mCam.setAspectRatio(getWindowAspectRatio());
}

void FullDome::setupQuats()
{
	glm::fquat fwd;
	fwd = glm::rotate(fwd, (float)toRadians(-180.0f + lookDir.x), vec3(0, 1, 0));
	fwd = glm::rotate(fwd, (float)toRadians(-90.0f + lookDir.y), vec3(0, 0, 1));
	fwd = glm::rotate(fwd, (float)toRadians(90.0 + lookDir.z), vec3(0, 1, 0));

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
	gl::clear(ColorA(0, 0, 0, 1));
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

		gl::rotate(3.2425926535*-0.5f, vec3(0, 0, 1));
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
		gl::rotate(3.2425926535*-0.5f, vec3(0,0,1));
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