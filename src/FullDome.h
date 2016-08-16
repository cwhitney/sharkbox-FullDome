#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/ObjLoader.h"
#include "cinder/Camera.h"

#include "cinder/CinderResources.h"
#include "Resources.h"

namespace sb {

	using FullDomeRef = std::shared_ptr<class FullDome>;

	class FullDome {
	public:
		static FullDomeRef create( ci::CameraPersp *cam, const int &fboSize = 2048, const int &renderFboSize = 2048) { return std::make_shared<FullDome>(cam, fboSize); }

		enum DomeCam {
			UP, DOWN, LEFT, RIGHT
		};

		FullDome( ci::CameraPersp *cam, const int &fboSize = 2048, const int &renderFboSize = 2048);
		void resize();
		void renderToFbo();
		void draw();

		ci::gl::TextureRef	getFboTexture() { return mCompositeFbo->getColorTexture(); }

		void bindCamera(DomeCam dir);
		void unbindCamera(DomeCam dir);

	private:
		void                    setupQuats();
		ci::TriMeshRef          loadObj(ci::fs::path loc);

		glm::fquat							mQuatL, mQuatR, mQuatU, mQuatD;

		ci::CameraPersp						*mUserCam;
        ci::CameraPersp						mCam;
		ci::CameraOrtho						mScreenCam;
		ci::gl::VboMeshRef					mLeftVbo, mRightVbo, mUpVbo, mDownVbo;
		ci::gl::BatchRef					mBU, mBD, mBL, mBR;

		ci::gl::FboRef						mFboL, mFboR, mFboU, mFboD;
		ci::gl::FboRef						mCompositeFbo;
		std::pair<ci::ivec2, ci::ivec2>		mCurViewport;
	};

}