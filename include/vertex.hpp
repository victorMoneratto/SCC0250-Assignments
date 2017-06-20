#pragma once

#include <common.hpp>
#include <gl_33.hpp>

namespace mesh_vertex_layout {
	enum type : GLuint {
		Position = 0,
		Normal,
		TexCoords,
	};
}


struct mesh_vertex {
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;

	mesh_vertex(const vec3& Position = vec3{ 0.f }, const vec3& Normal = vec3{ 0.f }, const vec2& TexCoords = vec2{ 0.f })
		:	Position{Position},
			Normal{Normal},
			TexCoords{TexCoords} {}
};
