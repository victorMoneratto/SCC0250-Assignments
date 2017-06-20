#version 330 core

in vertex {
	vec3 Position;
	vec3 Normal;
    vec2 TexCoords;
} Vertex;

out vec4 OutColor;

uniform sampler2D Texture;
uniform vec4 Color;
uniform bool bTexture;

struct camera {
	vec3 Position;
};

uniform camera Camera;

#define NUM_LIGHTS 3
struct light {
	vec4 Pos;
	vec3 Color;
	float Ambient;
	vec3 Direction;
	float LinearFalloff;
	float QuadraticFalloff;
	float InnerCutoff;
	float OuterCutoff;	
};

uniform light Lights[NUM_LIGHTS];

vec3 DoLighting(light Light, vec3 Color, vec3 Normal, vec3 Pos, vec3 ToCamera) {
	return Pos/2.0;

	vec3 AmbientColor = Light.Ambient * Light.Color * Color;
	return AmbientColor;
}

void main() {
 	OutColor.rgba = Color;
	if(bTexture) {
		OutColor *= texture(Texture, Vertex.TexCoords);
	}
	for(int i = 0; i < NUM_LIGHTS; ++i) {
		vec3 ToCamera = normalize(Camera.Position - Vertex.Position);
		OutColor.rgb = DoLighting(Lights[i], OutColor.rgb, Vertex.Normal, Vertex.Position, ToCamera);
	}
}