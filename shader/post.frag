#version 330 core

in vec2 UV;

out vec4 Color;

uniform sampler2D Texture;

void main() {
 	Color = vec4(1, 0, 1, 1);
}