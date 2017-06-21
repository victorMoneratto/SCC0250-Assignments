#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoords;

uniform mat4 Model;
uniform mat4 MVP;
uniform mat4 NormalMat;

out vec2 UV;
flat out vec3 Lighting;

struct vertex {
	vec3 Position;
	vec3 Normal;
} Vertex;

struct material {
	sampler2D Texture;
	vec4 Color;
	float SpecularPower;
};

uniform material Material;

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
	float InnerCone;
	float OuterCone;
};

#define NUM_LIGHTS 3
uniform light Lights[NUM_LIGHTS];

#define saturate(x) (clamp(x, 0.0, 1.0))
#define lengthSqr(x) (dot(x,x))
#define PI 3.1415926535897932384626433832795

vec3 DoLighting(light Light, vec3 Color, vec3 Normal, vec3 Pos, vec3 ToCamera) {
	vec3 ToLight;
	float Attenuation;
	if(Light.Position.w == 0.0) {
		// Directional
		ToLight = normalize(Light.Position.xyz);
		Attenuation = 1.0;
	} else {
		// Point
		ToLight = Light.Position.xyz - Vertex.Position;
		float DistanceToLight = length(ToLight);
		ToLight = normalize(ToLight);

		Attenuation = 1.0/(1.0 + Light.LinearFalloff * DistanceToLight
			+ Light.QuadraticFalloff * DistanceToLight * DistanceToLight);

		// Cone
		if(lengthSqr(Light.ConeDirection) > 0) {
			float Theta = dot(ToLight, normalize(-Light.ConeDirection));
			Attenuation *= saturate((Theta - Light.OuterCone)/(Light.InnerCone-Light.OuterCone));
		}
	}

	vec3 AmbientColor = Light.Ambient * Light.Color * Color;

	float Diffuse = max(0.0, dot(Vertex.Normal, ToLight));
	vec3 DiffuseColor = Diffuse * Color * Light.Color;

	float Specular = pow(max(0.0, dot(ToCamera, reflect(-ToLight, Normal))), Material.SpecularPower);
	vec3 SpecularColor = Specular * Color * Light.SpecularColor;

	return AmbientColor + Attenuation * (DiffuseColor + SpecularColor);
}

void main() {
    gl_Position = MVP * vec4(Position, 1.0);
    Vertex.Position = vec3(Model * vec4(Position, 1.0));
	Vertex.Normal = vec3(NormalMat * vec4(Normal, 0.0));
    UV = TexCoords;

	Lighting = vec3(0);
	for(int i = 0; i < NUM_LIGHTS; ++i)
	{
		vec3 ToCamera = normalize(Camera.Position - Vertex.Position);
		Lighting.rgb += DoLighting(Lights[i], Material.Color.rgb, Vertex.Normal, Vertex.Position, ToCamera);
	}
}