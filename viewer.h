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
	void initFBO();
  void deleteFBO();

  void createShaders();
  void deleteShaders();
  void reloadShaders();
  
  // drawing functions (one for each pass/shader)
  void drawNoise(GLuint id);
  void drawSceneFromLight(GLuint id);
  void drawShadowMap(GLuint id);
  void drawSceneFromCamera(GLuint id);
  void drawPostProcess(GLuint id);
  
  void drawQuad();
  
  // animation
  void animation();

  Grid   *_grid;   // the grid
  Camera *_cam;    // the camera

	QTimer				*_timer;			// timer to refresh the drawing
  glm::vec3 		_light;				// light direction
  glm::vec3 		_motion; 			// motion offset for the noise texture
  bool      		_mode;   			// camera motion or light motion
  bool					_showShadowMap;
  bool					_animation;		// boolean that controls the animation
  unsigned int	_ndResol;
	float					_len; 				// terrain is of size len*len
	int 					_currentTexture;

  // les shaders
  Shader *_noiseShader;
  Shader *_shadowMapShader;
  Shader *_debugShader;
  Shader *_terrainShader;
  Shader *_postProcessShader;
  
  // vbo/vao ids
  GLuint _vaoTerrain;
  GLuint _terrain[2];
  GLuint _vaoQuad;
  GLuint _quad;
  
  // imported texture ids
  GLuint _texWater[5];
  
  // fbo id and associated shader textures
  // noiseShader
  GLuint _fboNoise;
  
  GLuint _texNormal;
  GLuint _texHeight;
  
  // shadowShader
  GLuint _fboShadow;
  
  GLuint _texDepth;
  
  // terrainShader
  GLuint _fboTerrain;
  
  GLuint _texTerrainColor;
  GLuint _texTerrainNormal;
  GLuint _texTerrainDepth;
};

#endif // VIEWER_H
