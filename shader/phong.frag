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

struct light {
	vec4 Position;
	vec3 Color;
	float Ambient;
	vec3 SpecularColor;
	vec3 ConeDirection;
	float LinearFalloff;
	float QuadraticFalloff;
	float InnerCutoff;
	float OuterCutoff;	
};

#define NUM_LIGHTS 1
light Lights[NUM_LIGHTS];

vec3 DoLighting(light Light, vec3 Color, vec3 Normal, vec3 Pos, vec3 ToCamera) {
	vec3 ToLight;
	if(Light.Position.w == 0.0) {
		// Directional
		ToLight = normalize(Light.Position.xyz);
	} else {
		// Point
		ToLight = normalize(Light.Position.xyz - Vertex.Position);
		// TODO distance attenuation

		// Cone
		// TODO angular attenuation
	}

	vec3 AmbientColor = Light.Ambient * Light.Color * Color;

	float Diffuse = max(0.0, dot(Vertex.Normal, ToLight));
	vec3 DiffuseColor = Diffuse * Color * Light.Color;

	vec3 SpecularColor = vec3(0);
	if(Diffuse > 0.0) {
		float Specular = pow(max(0.0, dot(ToCamera, reflect(-ToLight, Normal))), 512);
		SpecularColor = Specular * Color * Light.SpecularColor;
	}

	return AmbientColor + DiffuseColor + SpecularColor;
}

void main() {
 	vec4 BaseColor = Color;
	if(bTexture) {
		BaseColor *= texture(Texture, Vertex.TexCoords);
	}

	Lights[0].Position = vec4(1, .5, 1, 0);
	Lights[0].Color = vec3(.5, .5, .5);
	Lights[0].SpecularColor = vec3(1, 1, 2);
	Lights[0].Ambient = 0.1;

	OutColor.rgb = vec3(0);
	OutColor.a = BaseColor.a;
	for(int i = 0; i < NUM_LIGHTS; ++i) {
		vec3 ToCamera = normalize(Camera.Position - Vertex.Position);
		OutColor.rgb += DoLighting(Lights[i], BaseColor.rgb, Vertex.Normal, Vertex.Position, ToCamera);
	}
}