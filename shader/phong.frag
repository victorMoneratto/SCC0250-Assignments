#version 330 core

in vertex {
	vec3 Position;
	vec3 Normal;
    vec2 TexCoords;
} Vertex;

out vec4 OutColor;

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
	vec4 BaseColor = texture(Material.Texture, Vertex.TexCoords);
 	BaseColor *= Material.Color;

	OutColor.rgb = vec3(0);
	OutColor.a = BaseColor.a;
	for(int i = 0; i < NUM_LIGHTS; ++i)
	{
		vec3 ToCamera = normalize(Camera.Position - Vertex.Position);
		OutColor.rgb += DoLighting(Lights[i], BaseColor.rgb, Vertex.Normal, Vertex.Position, ToCamera);
	}
}