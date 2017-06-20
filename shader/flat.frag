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

void main() {
 	OutColor.rgba = Color;
	if(bTexture) {
		OutColor = texture(Texture, Vertex.TexCoords);
	}
	OutColor.rgb = Vertex.Normal *.5 + .5;
	OutColor.a = 1.0;
}