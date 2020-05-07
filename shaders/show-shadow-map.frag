#version 330

out vec4 outBuffer;

uniform sampler2D shadowmap;

void main() {
  outBuffer = texelFetch(shadowmap,ivec2(gl_FragCoord.xy),0);
}
