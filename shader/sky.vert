#version 330 core

uniform mat4 ViewProjection;

layout(location = 0) in vec3 Position;
out vec3 TexCoords;

void main()
{
    gl_Position = ViewProjection * vec4(Position, 1.0);
	gl_Position = gl_Position.xyww; // Setting z to w makes the perspective division z/w = 1, the max value in NDC
    TexCoords = Position;
}