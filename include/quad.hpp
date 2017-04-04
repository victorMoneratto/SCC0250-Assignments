#pragma once

#include <common.hpp>
#include <transform.hpp>

struct quad_vertices {
	GLuint VAO, VBO, IBO;

	enum class vertex_attrib_location : GLuint {
		Position = 0,
		TexCoords = 1,
	};

	inline ~quad_vertices() {
		gl::DeleteVertexArrays(1, &VAO);
		gl::DeleteBuffers(1, &VBO);
		gl::DeleteBuffers(1, &IBO);
	};

	inline quad_vertices(transform TransformPos = transform{}, transform TransformTexCoords = transform{}) {

		struct quad_vertex {
			glm::vec2 Position;
			glm::vec2 TexCoords;
		} QuadVertices[] =
		{
			// Position			// TexCoords
			{ { -.5f, +.5f },	{ 0.f, 1.f } },
			{ { -.5f, -.5f },	{ 0.f, 0.f } },
			{ { +.5f, -.5f },	{ 1.f, 0.f } },
			{ { +.5f, +.5f },	{ 1.f, 1.f } },
		};

		GLushort QuadIndices[] = {
			0, 1, 2,
			0, 2, 3
		};

		// Transform vertices
		for(auto& Vertex : QuadVertices) {
			Vertex.Position = glm::vec2{ TransformPos.Calculate() * glm::vec4{ Vertex.Position, 0.f, 1.f } };
			Vertex.TexCoords = glm::vec2{ TransformTexCoords.Calculate() * glm::vec4{ Vertex.TexCoords, 0.f, 1.f } };
		}

		// Create VAO
		gl::GenVertexArrays(1, &VAO);
		gl::BindVertexArray(VAO);
		//		defer{ gl::BindVertexArray(0); } // This is wasteful if code is leak-free

		// Create and load vertices
		gl::GenBuffers(1, &VBO);
		gl::BindBuffer(gl::ARRAY_BUFFER, VBO);
		gl::BufferData(gl::ARRAY_BUFFER, SizeOf(quad_vertex) * 6, QuadVertices, gl::STATIC_DRAW);

		// Enable and set Position attrib pointer
		auto PositionLoc = (GLuint)vertex_attrib_location::Position;
		gl::EnableVertexAttribArray(PositionLoc);
		gl::VertexAttribPointer(PositionLoc, 2, gl::FLOAT, false, SizeOf(quad_vertex), (void*)OffsetOf(quad_vertex, Position));

		// Enable and set Position attrib pointer
		auto TexCoordLoc = (GLuint)vertex_attrib_location::TexCoords;
		gl::EnableVertexAttribArray(TexCoordLoc);
		gl::VertexAttribPointer(TexCoordLoc, 2, gl::FLOAT, false, SizeOf(quad_vertex), (void*)OffsetOf(quad_vertex, TexCoords));

		// Create and load indices
		gl::GenBuffers(1, &IBO);
		gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, IBO);
		gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, SizeOf(GLushort) * 6, QuadIndices, gl::STATIC_DRAW);
	}
};