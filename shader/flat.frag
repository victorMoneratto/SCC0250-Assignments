#version 330 core


in vec2 UV;
flat in vec3 Lighting;

out vec4 OutColor;

uniform sampler2D Texture;
uniform vec4 Color;
uniform bool bTexture;

void main() {
 	OutColor = vec4(Lighting,1) * texture(Texture, UV);
}