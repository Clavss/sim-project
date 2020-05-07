#version 330

// input attributes 
layout(location = 0) in vec3 position; 

// input uniforms
uniform mat4 mvpMat;

void main() {
  gl_Position =  mvpMat*vec4(position,1);
}
