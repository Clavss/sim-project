#version 330

#define DROSOPHILA 0

// input attributes 
layout(location = 0) in vec3 position;

// out variables 
out vec2 texcoord;

void main() {
  gl_Position = vec4(position, 1);

	#if DROSOPHILA
		texcoord = floor(position.xy);
	#else
		texcoord = position.xy * 0.5 + 0.5;
	#endif // DROSOPHILA
}
