#version 330 core

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 TexCoords;

uniform mat4 MVP;

out vec2 UV;

void main() {
    gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);
    UV = TexCoords;
}