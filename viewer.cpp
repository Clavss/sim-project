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
    _ndResol(128) {

  setlocale(LC_ALL,"C");

  _grid = new Grid(_ndResol,-5.0f,5.0f);
  _cam  = new Camera(1.0f,glm::vec3(0.0f,0.0f,0.0f));

  _timer->setInterval(10);
  connect(_timer,SIGNAL(timeout()),this,SLOT(updateGL()));
}

Viewer::~Viewer() {
  delete _timer;
  delete _grid;
  delete _cam;

  // delete all GPU objects
  deleteShaders();
  deleteVAO();
	deleteFBO();
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

void Viewer::createVAO() {
  // cree les buffers associés au terrain 

  glGenBuffers(2,_terrain);
  glGenVertexArrays(1,&_vaoTerrain);

  // create the VBO associated with the grid (the terrain)
  glBindVertexArray(_vaoTerrain);
  glBindBuffer(GL_ARRAY_BUFFER,_terrain[0]); // vertices 
  glBufferData(GL_ARRAY_BUFFER,_grid->nbVertices()*3*sizeof(float),_grid->vertices(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_terrain[1]); // indices 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_grid->nbFaces()*3*sizeof(int),_grid->faces(),GL_STATIC_DRAW); 
}

void Viewer::deleteVAO() {
  glDeleteBuffers(2,_terrain);
  glDeleteVertexArrays(1,&_vaoTerrain);
}

void Viewer::createShaders() {
  _terrainShader = new Shader();
  
  _terrainShader->load("shaders/terrain.vert","shaders/terrain.frag");
}

void Viewer::deleteShaders() {
  delete _terrainShader;

  _terrainShader = NULL;
}

void Viewer::reloadShaders() {
  if(_terrainShader)
    _terrainShader->reload("shaders/terrain.vert","shaders/terrain.frag");
}


void Viewer::drawScene(GLuint id) {

  // send uniform variables 
  glUniformMatrix4fv(glGetUniformLocation(id,"mdvMat"),1,GL_FALSE,&(_cam->mdvMatrix()[0][0]));
  glUniformMatrix4fv(glGetUniformLocation(id,"projMat"),1,GL_FALSE,&(_cam->projMatrix()[0][0]));
  glUniformMatrix3fv(glGetUniformLocation(id,"normalMat"),1,GL_FALSE,&(_cam->normalMatrix()[0][0]));
  glUniform3fv(glGetUniformLocation(id,"light"),1,&(_light[0]));
  glUniform3fv(glGetUniformLocation(id,"motion"),1,&(_motion[0]));

  // draw faces 
  glBindVertexArray(_vaoTerrain);
  glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,(void *)0);
  glBindVertexArray(0);
}

void Viewer::paintGL() {
  
  // allow opengl depth test 
  //glEnable(GL_DEPTH_TEST);
  
  // screen viewport
  glViewport(0,0,width(),height());

  // clear buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // activate the buffer shader 
  glUseProgram(_terrainShader->id());

  // generate the map
  drawScene(_terrainShader->id());

  // disable depth test 
  glDisable(GL_DEPTH_TEST);

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
  const float step = 0.002;
  if(ke->key()==Qt::Key_Z) {
    glm::vec2 v = glm::vec2(glm::transpose(_cam->normalMatrix())*glm::vec3(0,0,-1))*step;
    if(v[0]!=0.0 && v[1]!=0.0) v = glm::normalize(v)*step;
    else v = glm::vec2(0,1)*step;
    _motion[0] += v[0];
    _motion[1] += v[1];
  }

  if(ke->key()==Qt::Key_S) {
    glm::vec2 v = glm::vec2(glm::transpose(_cam->normalMatrix())*glm::vec3(0,0,-1))*step;
    if(v[0]!=0.0 && v[1]!=0.0) v = glm::normalize(v)*step;
    else v = glm::vec2(0,1)*step;
    _motion[0] -= v[0];
    _motion[1] -= v[1];
  }

  if(ke->key()==Qt::Key_Q) {
    _motion[2] += step;
  }

  if(ke->key()==Qt::Key_D) {
    _motion[2] -= step;
  }

  



  // key a: play/stop animation
  if(ke->key()==Qt::Key_A) {
    if(_timer->isActive()) 
      _timer->stop();
    else 
      _timer->start();
  }

  // key i: init camera
  if(ke->key()==Qt::Key_I) {
    _cam->initialize(width(),height(),true);
  }
  
  // // key f: compute FPS
  // if(ke->key()==Qt::Key_F) {
  //   int elapsed;
  //   QTime timer;
  //   timer.start();
  //   unsigned int nb = 500;
  //   for(unsigned int i=0;i<nb;++i) {
  //     paintGL();
  //   }
  //   elapsed = timer.elapsed();
  //   double t = (double)nb/((double)elapsed);
  //   cout << "FPS : " << t*1000.0 << endl;
  // }

  // key r: reload shaders 
  if(ke->key()==Qt::Key_R) {
    reloadShaders();
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

  // init VAO/VBO
  createVAO();
  
  createFBO();

  // starts the timer 
  _timer->start();
}

