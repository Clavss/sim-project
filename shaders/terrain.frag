#version 330

// in variables
in vec3 normalView;
in vec3 eyeView;
in vec2 texcoord;
in float depth;
in float height;
in vec4 shadcoord;

uniform vec3 light;
uniform sampler2D texWater;
uniform sampler2D normalmap;
uniform sampler2DShadow shadowmap;

// out buffers
layout(location = 0) out vec4 outColorBuffer;
layout(location = 1) out vec4 outNormalBuffer;

// Phong shading
vec4 shading(in vec2 coord, float height, vec3 n, sampler2D texture) {
	// remet entre 0 et 1 pour la couleur
	float modifierColorHeight = max(0.4, (1 - (height + 0.1) * 5));
	
	const vec3 ambient = vec3(0.4,0.6,0.8);
  const vec3 diffuse = vec3(0.6, 0.6, 0.9);
  const vec3 specular = vec3(0.3);
  
  const float et = 100.0;

  vec3 e = normalize(eyeView);
  vec3 l = normalize(light);
  vec4 c = texture2D(texture, coord);

  float diff = max(dot(l,n), 0.0);
  float spec = pow(max(dot(reflect(l,n),e),0.0),et);

  return vec4(ambient + diff*diffuse + spec*specular, 1.0) * c * modifierColorHeight;
}

void main() {
	float v = 1.0;
  float b = 0.05;

	vec3 n = normalize(normalView);
	
	// PCF
  v -= 0.2 * (1.0 - texture(shadowmap, vec3(shadcoord.xy, (shadcoord.z - b) / shadcoord.w)));
	
	outColorBuffer = shading(texcoord, height, n, texWater) * v;
	outNormalBuffer = vec4(n, depth);
}
