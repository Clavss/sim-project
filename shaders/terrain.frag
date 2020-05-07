#version 330

// input uniforms 
uniform vec3 light;
uniform vec3 motion;

// in variables 
in vec3  normalView;
in vec3  eyeView;
in float h;
//in vec2 uvcoord;
in vec4 shadcoord;

uniform sampler2D colormap;
uniform sampler2D normalmap;
uniform sampler2D shadowmap;

// out buffers 
layout(location = 0) out vec4 outColor;

float heightColor(float h) {
	float H = (h+0.1)*5;
	return pow(H, 2.);
}

vec3 getModifiedNormal() {
	vec3 tn = normalize(texture2D(normalmap, /*uvcoord*/ vec2(0.0, 0.0)).xyz * 2.0 - vec3(1.0));
	return normalize(tn);
}

void main() {
  const vec3 ambient  = vec3(0.2,0.3,0.4)*0.8;
  vec3 diffuse  = vec3(0.3*heightColor(h),0.6*heightColor(h), 0.9);
  const vec3 specular = vec3(0.2,0.2,0.2);
  const float et = 100.0;
  float v = 1.0;

  vec3 n = getModifiedNormal();
  vec3 e = normalize(eyeView);
  vec3 l = normalize(light);
	vec4 c = texture2D(colormap, /*uvcoord*/ vec2(0.0, 0.0));

  float diff = max(dot(l,n), 0.0);
  float spec = pow(max(dot(reflect(l,n),e),0.0),et);

  vec3 color = ambient + diff*diffuse + spec*specular;
  
  if (texture(shadowmap, shadcoord.xy).z < shadcoord.z) {
  	v = 0.5;
  }

  outColor = vec4(color,1.0) * c * v;
}
