#version 330 core

in vertex {
	vec3 Position;
	vec3 Normal;
    vec2 TexCoords;
} Vertex;

out vec4 OutColor;

uniform sampler2D Texture;
uniform vec4 Color;

 // Former shininess, this is a better name as shininess may be confused in a PBR context
float SpecularPower = 128; 

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

#define NUM_LIGHTS 3
light Lights[NUM_LIGHTS];

#define saturate(x) (clamp(x, 0.0, 1.0))
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
		float Theta = dot(ToLight, normalize(-Light.ConeDirection));
		Attenuation *= saturate((Theta - Light.OuterCutoff)/(Light.InnerCutoff-Light.OuterCutoff));
	}

	vec3 AmbientColor = Light.Ambient * Light.Color * Color;

	float Diffuse = max(0.0, dot(Vertex.Normal, ToLight));
	vec3 DiffuseColor = Diffuse * Color * Light.Color;

	vec3 SpecularColor = vec3(0);
	if(Diffuse > 0.0) {
		float Specular = pow(max(0.0, dot(ToCamera, reflect(-ToLight, Normal))), SpecularPower);
		SpecularColor = Specular * Color * Light.SpecularColor;
	}

	return AmbientColor + Attenuation * (DiffuseColor + SpecularColor);
}

void main() {
	vec4 BaseColor = texture(Texture, Vertex.TexCoords);
 	BaseColor *= Color;

	Lights[0].Position = vec4(.125, 1, 0, 0);
	Lights[0].Color = vec3(.25, .25, .5);
	Lights[0].SpecularColor = vec3(1, 1, 2);
	Lights[0].Ambient = 0.05;

	Lights[1].Position = vec4(-3, 1, 0, 1);
	Lights[1].Color = vec3(.5, .25, .25);
	Lights[1].SpecularColor = vec3(2, 1, 1);
	Lights[1].Ambient = 0.05;
	Lights[1].LinearFalloff = .025;
	Lights[1].QuadraticFalloff = .01;
	Lights[1].InnerCutoff = cos(PI);
	Lights[1].OuterCutoff = cos(PI);

	Lights[2].Position = vec4(0, 0, -1, 1);
	Lights[2].ConeDirection = vec3(0, 0, -1);
	Lights[2].Color = vec3(.25, .25, .5);
	Lights[2].SpecularColor = vec3(1, 1, 2);
	Lights[2].Ambient = 0.05;
	Lights[2].LinearFalloff = .05;
	Lights[2].QuadraticFalloff = .01;
	Lights[2].InnerCutoff = cos(PI/2);
	Lights[2].OuterCutoff = cos(PI/2);

	OutColor.rgb = vec3(0);
	OutColor.a = BaseColor.a;
	for(int i = 0; i < NUM_LIGHTS; ++i) {
		vec3 ToCamera = normalize(Camera.Position - Vertex.Position);
		OutColor.rgb += DoLighting(Lights[i], BaseColor.rgb, Vertex.Normal, Vertex.Position, ToCamera);
	}
}