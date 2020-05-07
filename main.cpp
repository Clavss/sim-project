#include <qapplication.h>
#include <QString>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "viewer.h"

using namespace std;

int main(int argc,char** argv) {
  QApplication application(argc,argv);

	QGLFormat fmt;
	fmt.setVersion(3, 3);
	fmt.setProfile(QGLFormat::CoreProfile);
	fmt.setSampleBuffers(true);
	/*
		S'il y a des problemes de versions de GLSL
		export MESA_GL_VERSION_OVERRIDE=3.3
		export MESA_GLSL_VERSION_OVERRIDE=330
	*/

  Viewer viewer(argv[0], fmt);

  viewer.setWindowTitle("Exercice 09 - Terrain rendering");
  viewer.show();
  
  return application.exec();
}
