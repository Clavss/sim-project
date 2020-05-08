#ifndef VIEWER_H
#define VIEWER_H

// GLEW lib: needs to be included first!!
#include <GL/glew.h> 

// OpenGL library 
#include <GL/gl.h>

// OpenGL Utility library
#include <GL/glu.h>

// OpenGL Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <QGLFormat>
#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <stack>

#include "camera.h"
#include "shader.h"
#include "grid.h"

class Viewer : public QGLWidget {
 public:
  Viewer(char *filename,const QGLFormat &format=QGLFormat::defaultFormat());
  ~Viewer();
  
 protected :
  virtual void paintGL();
  virtual void initializeGL();
  virtual void resizeGL(int width,int height);
  virtual void keyPressEvent(QKeyEvent *ke);
  virtual void mousePressEvent(QMouseEvent *me);
  virtual void mouseMoveEvent(QMouseEvent *me);

 private:
  // OpenGL objects creation
  void createVAO();
  void deleteVAO();
  
  void createTextures();
  void deleteTextures();
  void loadTexture(GLuint id, const char *filename);

	void createFBO();
  void deleteFBO();

  void createShaders();
  void deleteShaders();
  void reloadShaders();
  
  // drawing functions 
  void drawSceneFromCamera(GLuint id);
  void drawSceneFromLight(GLuint id);
  void drawShadowMap(GLuint id);
  
  // animation
  void animation();

  Grid   *_grid;   // the grid
  Camera *_cam;    // the camera

	QTimer *_timer;
  glm::vec3 _light;  // light direction
  glm::vec3 _motion; // motion offset for the noise texture 
  bool      _mode;   // camera motion or light motion
  bool			_showShadowMap;
  bool			_animation;    // boolean that controls the animation

  // les shaders
  Shader *_shadowMapShader;
  Shader *_terrainShader;
  Shader *_debugShader;
  
  // vbo/vao ids 
  GLuint _vaoTerrain;
  GLuint _terrain[2];
  GLuint _vaoQuad;
  GLuint _quad;
  
  // texture ids
  GLuint _texColor;
  GLuint _texNormal;
  
  // fbo id and associated depth texture 
  GLuint _fbo;
  GLuint _texDepth;

  unsigned int _ndResol;
};

#endif // VIEWER_H
