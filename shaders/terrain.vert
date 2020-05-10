#version 330

// input attributes 
layout(location = 0) in vec3 position;

// input uniforms
uniform mat4 mvpMat;
uniform mat4 mdvMat;      // modelview matrix 
uniform mat4 projMat;     // projection matrix
uniform mat3 normalMat;   // normal matrix

uniform sampler2D normalmap; // pour la height

// out variables
out vec3 normalView;
out vec3 eyeView;
out vec2 texcoord;
out float depth;
out float height;
out vec4 shadcoord;

void main() {
	texcoord = position.xy * 0.5 + 0.5;
	
	// on récupère la height dans la texture normalmap, canal alpha
	height =  texture2D(normalmap, texcoord).w;
	vec3 pos =  position - vec3(0.0, 0.0, height);

  gl_Position = projMat*mdvMat*vec4(pos,1);
  normalView  = normalize(normalMat * texture2D(normalmap, texcoord).xyz);
  eyeView     = normalize((mdvMat * vec4(position, 1.0)).xyz);
  depth				= (mdvMat * vec4(pos, 1.0)).z;
  shadcoord		= mvpMat * vec4(pos, 1.0) * 0.5 + 0.5;
}
