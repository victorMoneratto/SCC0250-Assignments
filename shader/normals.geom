#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vertex {
	vec3 Normal;
    vec2 TexCoords;
} VertexIn[];

out vertex {
	vec3 Normal;
    vec2 TexCoords;
} VertexOut;

// void EmitNormal(int id) {
// 	gl_Position = gl_in[id].gl_Position;
//     EmitVertex();

//     gl_Position = gl_in[id].gl_Position + vec4(Vertex[id].Normal, 0.0);
//     EmitVertex();
// }

const float LineSize = .1;
vec4 CalcNormal(int vert1) {
    int vert2 = (vert1+1)%3;
    vec3 N = cross(gl_in[vert1].gl_Position.xyz, gl_in[vert2].gl_Position.xyz);
    N = normalize(N);
    return vec4(N, 0.0);
}

void EmitNormal(int id) {
    gl_Position = gl_in[id].gl_Position;
    VertexOut.Normal = VertexIn[id].Normal;
    VertexOut.TexCoords = VertexIn[id].TexCoords;
    EmitVertex();

    gl_Position = gl_in[id].gl_Position + vec4(VertexIn[id].Normal, 0.0);// vec4(VertexIn[0].Normal, 0.0);
    VertexOut.Normal = VertexIn[id].Normal;
    VertexOut.TexCoords = VertexIn[id].TexCoords;
    EmitVertex();
}

void Lines(int i) {
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();

    gl_Position = gl_in[(i+1)%3].gl_Position;
    EmitVertex();
}

void main() {
    Lines(0);
    Lines(1);
    Lines(2);

    EndPrimitive();
}