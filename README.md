# sharkbox-FullDome

A fulldome fisheye renderer for Cinder 0.9.0.

Big thanks to Paul Bourke whose work was critical to figuring this out, specifically [this post](http://paulbourke.net/dome/unityfisheye/).

How it works
---
Internally FullDome will create four cameras.  Two that look left and right of center 45 degrees on the X-axis, and then 2 that look straight up and straight down with a 45 degree Y-axis twist.

Each draw cycle each, one is bound and then rendered to an FBO.  Each FBO is then bound and mapped to a piece of geometry that makes up a piece of the final circular fisheye render.  This means that for each one frame, the scene is drawn four times.

Usage
--
Create a normal PerspCamera to drive the fulldome camera. 

```
CameraPersp mCam;
mFullDome = FullDome::create( &mCam );
```

You can change the size of the four individual camera fbos in the constructor.  The default is 2048 x 2048. Below we're setting it to 1024 x 1024.

```mFullDome = FullDome::create( &mCam, 1024 );```

During your draw render your scene once for each camera. 

```
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
```

The image can then be drawn directly, or rendered to an FBO.

```
mFullDome->draw();
```

or

```
mFullDome->renderToFbo();
gl::TextureRef texRef = mFullDome->getFboTexture();
```

As a caveat, if your lighting calculations are in camera space, they will render funny since the final image is made up of four different cameras.  Check the included shaders for an example.

----
The MIT License (MIT)

Copyright (c) 2016 Charlie Whitney

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

