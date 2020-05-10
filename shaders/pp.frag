#version 330

#define FOG_EFFECT 0
#define PIXEL_EFFECT 0

// in variables
in vec2 texcoord;

// input uniforms
uniform sampler2D colormap;
uniform sampler2D normalmap;

out vec4 outBuffer;

void main() {
	vec2 newcoord;

	#if PIXEL_EFFECT
		float nbpix = 32.0;
		newcoord = floor(texcoord.xy * nbpix) / nbpix;
	#else
		newcoord = texcoord;
	#endif // PIXEL_EFFECT

	vec4 color = texture2D(colormap, newcoord);

	#if FOG_EFFECT
		vec4 fogcolor = vec4(vec3(0.8), 1.0);
		float depth = clamp(texture2D(normalmap, newcoord).w, 0.0, 1.0);
		outBuffer = (1 - depth) * color + depth * fogcolor;
	#else
		outBuffer = color;
	#endif // FOG_EFFECT
}
