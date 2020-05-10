#version 330

// input attributes 
layout(location = 0) in vec3 position; 

// input uniforms
uniform mat4 mvpMat;
uniform sampler2D heightmap;

void main() {
	vec2 texcoord = position.xy * 0.5 + 0.5;
	
	// on récupère la height dans la texture (n'importe quel canal)
	float height = texture2D(heightmap, texcoord).x;
  gl_Position =  mvpMat*vec4(position - vec3(0.0, 0.0, height),1);
}
