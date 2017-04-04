#version 330 core

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 TexCoords;

out vec2 UV;

void main() {
    gl_Position = vec4(Position.x, Position.y, 0.0, 1.0);
    UV = TexCoords;
}