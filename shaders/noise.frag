#version 330

in vec2 texcoord;

uniform vec3 motion;

// out buffers
layout(location = 0) out vec4 outBufferNormal;
layout(location = 1) out vec4 outBufferHeight;

// fonctions utiles pour créer des terrains en général
vec2 hash(vec2 p) {
  p = vec2( dot(p,vec2(127.1,311.7)),
	    dot(p,vec2(269.5,183.3)) );  
  return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float gnoise(in vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
	
  vec2 u = f*f*(3.0-2.0*f);
  
  return mix(mix(dot(hash(i+vec2(0.0,0.0)),f-vec2(0.0,0.0)), 
		 dot(hash(i+vec2(1.0,0.0)),f-vec2(1.0,0.0)),u.x),
	     mix(dot(hash(i+vec2(0.0,1.0)),f-vec2(0.0,1.0)), 
		 dot(hash(i+vec2(1.0,1.0)),f-vec2(1.0,1.0)),u.x),u.y);
}

float pnoise(in vec2 p,in float amplitude,in float frequency,in float persistence, in int nboctaves) {
  float a = amplitude;
  float f = frequency;
  float n = 0.0;
  
  for(int i=0;i<nboctaves;++i) {
    n = n+a*gnoise(p*f);
    f = f*2.;
    a = a*persistence;
  }
  
  return n;
}

float computeHeight(in vec2 p) {
  // sinus animé
  return 0.1 * sin((pnoise(p, 0.5, 1.5, 0.5, 2)+motion.x)*12); // [-0.1; 0.1]
}

vec3 computeNormal(in vec2 p) {
  const float EPS = 0.01;
  const float SCALE = 2000.;
  
  vec2 g = vec2(computeHeight(p+vec2(EPS,0.))-computeHeight(p-vec2(EPS,0.)),
		computeHeight(p+vec2(0.,EPS))-computeHeight(p-vec2(0.,EPS)))/2.*EPS;
  
  vec3 n1 = vec3(1.,0.,g.x*SCALE);
  vec3 n2 = vec3(0.,1.,-g.y*SCALE);
  vec3 n = normalize(cross(n1,n2));

  return n;
}

void main() {
	float h = computeHeight(texcoord.xy);
	vec3 n = computeNormal(texcoord.xy);
	
	outBufferNormal = vec4(n, h);
	outBufferHeight = vec4(h);
}
