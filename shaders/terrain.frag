#version 330

// input uniforms 
uniform vec3 light;
uniform vec3 motion;

// in variables 
in vec3  normalView;
in vec3  eyeView;
in float h;
in vec2 uvcoord;
in vec4 shadcoord;

uniform sampler2D colormap;
uniform sampler2D normalmap;
uniform sampler2DShadow shadowmap;

// out buffers 
layout(location = 0) out vec4 outColor;

// Couleur différente en fonction de la hauteur
float heightColor(float h) {
	float H = (h+0.1)*5;
	return pow(H, 2.);
}


vec3 getModifiedNormal() {
	vec3 n = normalize(normalView);
	mat3 tbn = mat3(n.zxy, n.xzy, n);
	vec3 tn = normalize(texture2D(normalmap, uvcoord).xyz * 2.0 - vec3(1.0));
	
	return normalize(tbn*tn);
}

void main() {
  const vec3 ambient  = vec3(0.2,0.3,0.4)*0.8;
  vec3 diffuse  = vec3(0.3*heightColor(h),0.6*heightColor(h), 0.9);
  const vec3 specular = vec3(0.2,0.2,0.2);
  const float et = 100.0;
  float v = 1.0;
  float b = 0.005;

  vec3 n = getModifiedNormal();
  vec3 e = normalize(eyeView);
  vec3 l = normalize(light);
	vec4 c = texture2D(colormap, uvcoord);

  float diff = max(dot(l,n), 0.0);
  float spec = pow(max(dot(reflect(l,n),e),0.0),et);

  vec3 color = ambient + diff*diffuse + spec*specular;
  
  // PCE
  //v -= 0.8 * (1.0 - texture(shadowmap, vec3(shadcoord.xy, (shadcoord.z - b) / shadcoord.w)));
  
	outColor = vec4(color * v, 1.0) * c;
}
