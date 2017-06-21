#version 330 core

in vertex {
	vec3 Position;
	vec3 Normal;
    vec2 TexCoords;
    vec3 Lighting;
} Vertex;

out vec4 OutColor;

uniform sampler2D Texture;
uniform vec4 Color;
uniform bool bTexture;

void main() {
 	OutColor = vec4(Vertex.Lighting,1) * texture(Texture, Vertex.TexCoords);
}