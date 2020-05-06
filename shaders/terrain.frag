#version 330

// input uniforms 
uniform vec3 light;
uniform vec3 motion;

// in variables 
in vec3  normalView;
in vec3  eyeView;
in float h;

// out buffers 
layout(location = 0) out vec4 outColor;

float heightColor(float h) {
	float H = (h+0.1)*5;
	return pow(H, 2.);
}

void main() {
  const vec3 ambient  = vec3(0.2,0.3,0.4)*0.8;
  vec3 diffuse  = vec3(0.3*heightColor(h),0.6*heightColor(h), 0.9);
  const vec3 specular = vec3(0.2,0.2,0.2);
  const float et = 100.0;

  vec3 n = normalize(normalView);
  vec3 e = normalize(eyeView);
  vec3 l = normalize(light);

  float diff = dot(l,n);
  float spec = pow(max(dot(reflect(l,n),e),0.0),et);

  vec3 color = ambient + diff*diffuse + spec*specular;

  outColor = vec4(color,1.0);
}
