#version 330 core

uniform samplerCube Skybox;

in vec3 TexCoords;
out vec4 Color;

void main() {
    Color = texture(Skybox, TexCoords);
}