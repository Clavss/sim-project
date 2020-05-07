#include "viewer.h"

#include <math.h>
#include <iostream>
#include <QTime>

using namespace std;

Viewer::Viewer(char *,const QGLFormat &format)
  : QGLWidget(format),
  	_timer(new QTimer(this)),
    _light(glm::vec3(0,0,1)),
    _motion(glm::vec3(0,0,0)),
    _mode(false),
    _showShadowMap(false),
    _animation(true),
    _ndResol(512) {

  setlocale(LC_ALL,"C");

  _grid = new Grid(_ndResol,-5.0f,5.0f);
  _cam  = new Camera(3.0f,glm::vec3(0.0f,0.0f,0.0f));

  _timer->setInterval(1);
  connect(_timer,SIGNAL(timeout()),this,SLOT(updateGL()));
}

Viewer::~Viewer() {
  delete _timer;
  delete _grid;
  delete _cam;

  // delete all GPU objects
  deleteShaders();
  deleteTextures();
  deleteVAO();
	deleteFBO();
}

void Viewer::deleteTextures() {
	// delete loaded textures
	glDeleteTextures(1, &_texColor);
	glDeleteTextures(1, &_texNormal);
}

void Viewer::deleteFBO() {
  // delete all FBO Ids
  glDeleteFramebuffers(1, &_fbo);
  glDeleteTextures(1, &_texDepth);
}

void Viewer::createFBO() {
  // generate fbo and associated textures
  glGenFramebuffers(1, &_fbo);
  glGenTextures(1, &_texDepth);

  // create the texture for rendering depth values
  glBindTexture(GL_TEXTURE_2D,_texDepth);
  glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,_ndResol,_ndResol,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  // attach textures to framebuffer object 
  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);
  glBindTexture(GL_TEXTURE_2D,_texDepth);
  glFramebufferTexture2D(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,_texDepth,0);

  // test if everything is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "Warning: FBO not complete!" << endl;

  // disable FBO
  glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void Viewer::loadTexture(GLuint id, const char *filename) {
	// load image
	QImage image = QGLWidget::convertToGLFormat(QImage(filename));
	
	// activate texture
	glBindTexture(GL_TEXTURE_2D, id);
	
	// set textute parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	
	// store texture in the GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *)image.bits());
	
	// generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Viewer::createTextures() {
	// generate texture ids
	glGenTextures(1, &_texColor);
	glGenTextures(1, &_texNormal);
	
	// load all needed textures
	loadTexture(_texColor, "textures/color.jpg");
	loadTexture(_texNormal, "textures/normal.jpg");
}

void Viewer::createVAO() {
	// data associated with the screen quad
	const GLfloat quadData[] = {
		-1.0f,-1.0f,0.0f, 1.0f,-1.0f,0.0f, -1.0f,1.0f,0.0f, -1.0f,1.0f,0.0f, 1.0f,-1.0f,0.0f, 1.0f,1.0f,0.0f
	};

  // cree les buffers associés au terrain 
  glGenBuffers(2,_terrain);
  glGenVertexArrays(1,&_vaoTerrain);
  glGenVertexArrays(1, &_vaoQuad);

  // create the VBO associated with the grid (the terrain)
  glBindVertexArray(_vaoTerrain);
  glBindBuffer(GL_ARRAY_BUFFER,_terrain[0]); // vertices 
  glBufferData(GL_ARRAY_BUFFER,_grid->nbVertices()*3*sizeof(float),_grid->vertices(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_terrain[1]); // indices 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_grid->nbFaces()*3*sizeof(int),_grid->faces(),GL_STATIC_DRAW); 
  
  glGenBuffers(1, &_quad);
  glBindVertexArray(_vaoQuad);
  glBindBuffer(GL_ARRAY_BUFFER, _quad);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);
  
  glBindVertexArray(0);
}

void Viewer::deleteVAO() {
  glDeleteBuffers(2,_terrain);
  glDeleteBuffers(1, &_quad);
  glDeleteVertexArrays(1,&_vaoTerrain);
  glDeleteVertexArrays(1, &_vaoQuad);
}

void Viewer::createShaders() {
  _terrainShader = new Shader();
  _shadowMapShader = new Shader();
  _debugShader = new Shader();
  
  _terrainShader->load("shaders/terrain.vert","shaders/terrain.frag");
  _shadowMapShader->load("shaders/shadow-map.vert","shaders/shadow-map.frag");
  _debugShader->load("shaders/show-shadow-map.vert","shaders/show-shadow-map.frag");
}

void Viewer::deleteShaders() {
  delete _terrainShader;
  delete _shadowMapShader;
  delete _debugShader;

  _terrainShader = NULL;
  _shadowMapShader = NULL;
  _debugShader = NULL;
}

void Viewer::reloadShaders() {
  if (_terrainShader) {
    _terrainShader->reload("shaders/terrain.vert","shaders/terrain.frag");
		_shadowMapShader->load("shaders/shadow-map.vert","shaders/shadow-map.frag");
  	_debugShader->load("shaders/show-shadow-map.vert","shaders/show-shadow-map.frag");
	}
}

void Viewer::animation() {
	const float animationStep = 0.001;
	
  _motion[0] -= animationStep;
  _motion[1] -= animationStep;
}

void Viewer::drawSceneFromCamera(GLuint id) {
	// mdv matrix from the light point of view
	const float size = _cam->getRadius()*10;
	glm::vec3 l = glm::transpose(_cam->normalMatrix())*_light;
	glm::mat4 p = glm::ortho<float>(-size, size, -size, size, -size, 2*size);
	glm::mat4 v = glm::lookAt(l, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 m = glm::mat4(1.0);
	glm::mat4 mv = v*m;

	// modify the terrain
	if (_animation) {
		animation();
	}
	
  // send uniform variables 
  glUniformMatrix4fv(glGetUniformLocation(id,"mdvMat"),1,GL_FALSE,&(_cam->mdvMatrix()[0][0]));
  glUniformMatrix4fv(glGetUniformLocation(id,"projMat"),1,GL_FALSE,&(_cam->projMatrix()[0][0]));
  glUniformMatrix3fv(glGetUniformLocation(id,"normalMat"),1,GL_FALSE,&(_cam->normalMatrix()[0][0]));
  glUniform3fv(glGetUniformLocation(id,"light"),1,&(_light[0]));
  glUniform3fv(glGetUniformLocation(id,"motion"),1,&(_motion[0]));

	// send textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texColor);
	glUniform1i(glGetUniformLocation(id, "colormap"), 0);
	
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, _texNormal);
	glUniform1i(glGetUniformLocation(id, "normalmap"), 1);
	
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, _texDepth);
	glUniform1i(glGetUniformLocation(id, "shadowmap"), 2);

	const glm::mat4 mvpDepth = p*mv;
	glUniformMatrix4fv(glGetUniformLocation(id, "mvpDepthMat"), 1, GL_FALSE, &mvpDepth[0][0]);

  // draw the terrain
  glBindVertexArray(_vaoTerrain);
  glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,(void *)0);
  
  // disable VAO
  glBindVertexArray(0);
}

void Viewer::drawSceneFromLight(GLuint id) {
	// mdv matrix from the light point of view
	const float size = _cam->getRadius()*10;
	glm::vec3 l = glm::transpose(_cam->normalMatrix())*_light;
	glm::mat4 p = glm::ortho<float>(-size, size, -size, size, -size, 2*size);
	glm::mat4 v = glm::lookAt(l, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 m = glm::mat4(1.0);
	glm::mat4 mv = v*m;

	const glm::mat4 mvp = p*mv;
	glUniformMatrix4fv(glGetUniformLocation(id, "mvpMat"), 1, GL_FALSE, &mvp[0][0]);

  // draw the terrain
  glBindVertexArray(_vaoTerrain);
  glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,(void *)0);
  
  // disable VAO
  glBindVertexArray(0);
}

void Viewer::drawShadowMap(GLuint id) {
	// send depth texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texDepth);
	glUniform1i(glGetUniformLocation(id, "shadowmap"), 0);
	
	// draw the quad
	glBindVertexArray(_vaoQuad);
	glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,(void *)0);
	
	// disable VAO
  glBindVertexArray(0);
}

void Viewer::paintGL() {
  // allow opengl depth test 
  //glEnable(GL_DEPTH_TEST);
  
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  glDrawBuffer(GL_NONE);
  glViewport(0, 0, _ndResol, _ndResol);
  glClear(GL_DEPTH_BUFFER_BIT);
  glUseProgram(_shadowMapShader->id());
  drawSceneFromLight(_shadowMapShader->id());
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  // screen viewport
  glViewport(0,0,width(),height());

  // activate the buffer shader 
  glUseProgram(_terrainShader->id());

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // generate the map
  drawSceneFromCamera(_terrainShader->id());

	if (_showShadowMap) {
		glUseProgram(_debugShader->id());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawShadowMap(_debugShader->id());
	}

  // disable depth test 
  //glDisable(GL_DEPTH_TEST);

  // disable shader 
  glUseProgram(0);
}

void Viewer::resizeGL(int width,int height) {
  _cam->initialize(width,height,false);
  glViewport(0,0,width,height);
  updateGL();
}

void Viewer::mousePressEvent(QMouseEvent *me) {
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

  if(me->button()==Qt::LeftButton) {
    _cam->initRotation(p);
    _mode = false;
  } else if(me->button()==Qt::MidButton) {
    _cam->initMoveZ(p);
    _mode = false;
  } else if(me->button()==Qt::RightButton) {
    _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
    _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
    _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
    _light = glm::normalize(_light);
    _mode = true;
  } 

  updateGL();
}

void Viewer::mouseMoveEvent(QMouseEvent *me) {
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));
 
  if(_mode) {
    // light mode
    _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
    _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
    _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
    _light = glm::normalize(_light);
  } else {
    // camera mode
    _cam->move(p);
  }

  updateGL();
}

void Viewer::keyPressEvent(QKeyEvent *ke) {
  const float movementSpeed = 2.0;

	if(ke->key()==Qt::Key_Z) {
    _cam->addZ(movementSpeed);
  }

  if(ke->key()==Qt::Key_S) {
    _cam->addZ(-movementSpeed);
  }

  if(ke->key()==Qt::Key_Q) {
    _cam->addX(movementSpeed);
  }

  if(ke->key()==Qt::Key_D) {
    _cam->addX(-movementSpeed);
  }

  // key a: play/stop animation
  if(ke->key()==Qt::Key_A) {
    _animation = !_animation;
  }

  // key i: init camera
  if(ke->key()==Qt::Key_I) {
    _cam->initialize(width(),height(),true);
  }
  
  // key f: compute FPS
  if(ke->key()==Qt::Key_F) {
     int elapsed;
     QTime timer;
     timer.start();
     unsigned int nb = 500;
     for(unsigned int i=0;i<nb;++i) {
       paintGL();
     }
     elapsed = timer.elapsed();
     double t = (double)nb/((double)elapsed);
     cout << "FPS : " << t << endl;
   }

  // key r: reload shaders 
  if (ke->key()==Qt::Key_R) {
    reloadShaders();
  }
  
  // key m: show the shadow map 
  if (ke->key()==Qt::Key_M) {
    _showShadowMap = !_showShadowMap;
  }

  updateGL();
}

void Viewer::initializeGL() {
  // make this window the current one
  makeCurrent();

  // init and chack glew
  if(glewInit()!=GLEW_OK) {
    cerr << "Warning: glewInit failed!" << endl;
  }

  if(!GLEW_ARB_vertex_program   ||
     !GLEW_ARB_fragment_program ||
     !GLEW_ARB_texture_float    ||
     !GLEW_ARB_draw_buffers     ||
     !GLEW_ARB_framebuffer_object) {
    cerr << "Warning: Shaders not supported!" << endl;
  }

  // init OpenGL settings
  glClearColor(0.0,0.0,0.0,1.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glViewport(0,0,width(),height());

  // initialize camera
  _cam->initialize(width(),height(),true);

  // init shaders 
  createShaders();

  // init VAO/Textures/VBO
  createVAO();
  createTextures();
  createFBO();
  
  // starts the timer
  _timer->start();
}

