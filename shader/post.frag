#version 330 core

in vec2 UV;

out vec4 Color;

uniform sampler2D Texture;
uniform float Time;
uniform vec2 Resolution;

#define TAU 6.28318530718

// Based on Tileable Water Caustic by David Hoskins
float Wave(vec2 Pos, int Iterations, float Mult) {
	Pos = mod(Pos * TAU, TAU) - 2 * Mult;
	vec2 I = vec2(Pos);

	float C = 1.0;

	for (int n = 0; n < Iterations; n++)  {
		float T = Time * (1.0 - (3.5 / float(n+1)));
		I = Pos + vec2(cos(T - I.x) + sin(T + I.y), sin(T - I.y) + cos(T + I.x));
		vec2 Wave = vec2(Pos.x / (sin(I.x+T) * Mult), Pos.y / (cos(I.y+T) * Mult));
		C += 1.0/length(Wave);
	}

	C /= float(Iterations);
	C = 1.17-pow(C, 1.4);
	
	return clamp(pow(abs(C), 10.0), 0, 1);
}

void main() {
	vec2 FragCoords = UV * Resolution;

	float Distortion = Wave(FragCoords/Resolution.x, 5, 200);

	vec2 Offset = 20 * vec2(UV/Resolution) * (Distortion-.5) * 2;
	Color = texture(Texture, UV+Offset);
	Color.rgb += vec3(.1*Distortion);
	Color.a = 1;
}