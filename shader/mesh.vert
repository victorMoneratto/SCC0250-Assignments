#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoords;

uniform mat4 MVP;
uniform mat4 NormalMat;

out vertex {
	vec3 Normal;
    vec2 TexCoords;
} Vertex;

void main() {
    gl_Position = MVP * vec4(Position, 1.0);
	Vertex.Normal = vec3(NormalMat * vec4(Normal, 0.0));
    Vertex.TexCoords = TexCoords;
}