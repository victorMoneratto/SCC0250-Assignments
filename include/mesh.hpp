#pragma once

#include <common.hpp>
#include <vertex.hpp>
#include <gl_33.hpp>
#include <vector>
#include <transform.hpp>

struct mesh {
	GLuint VAO, VBO, IBO;
	GLenum GeometryMode;
	uint NumVerts;
	uint NumIndices;

	mesh(GLuint VAO, GLuint VBO, GLuint IBO, GLenum GeometryMode, uint NumVerts, uint NumIndices)
		: VAO{VAO},
		  VBO{VBO},
		  IBO{IBO},
		  GeometryMode{GeometryMode},
		  NumVerts{NumVerts},
		  NumIndices{NumIndices} {}

	mesh(std::vector<mesh_vertex> Vertices, GLenum GeometryMode, std::vector<uint>* Indices = nullptr) : GeometryMode{ GeometryMode } {
		gl::GenVertexArrays(1, &VAO);
		gl::BindVertexArray(VAO);
		defer{ gl::BindVertexArray(0); };

		// Vertex Buffer
		NumVerts = (uint) Vertices.size();
		gl::GenBuffers(1, &VBO);
		gl::BindBuffer(gl::ARRAY_BUFFER, VBO);
		gl::BufferData(gl::ARRAY_BUFFER, Vertices.size() * sizeof(mesh_vertex), Vertices.data(), gl::STATIC_DRAW);

		// Index Buffer
		NumIndices = 0;
		if (Indices != nullptr && Indices->size() > 0) {
			NumIndices = (uint) Indices->size();
			gl::GenBuffers(1, &IBO);
			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, IBO);
			gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, Indices->size() * sizeof(GLuint), Indices->data(), gl::STATIC_DRAW);
		} else {
			IBO = 0;
		}

		gl::EnableVertexAttribArray(mesh_vertex_layout::Position);
		gl::VertexAttribPointer(mesh_vertex_layout::Position, 3, gl::FLOAT, false, SizeOf(Vertices[0]), (void*)OffsetOf(mesh_vertex, Position));

		gl::EnableVertexAttribArray(mesh_vertex_layout::Normal);
		gl::VertexAttribPointer(mesh_vertex_layout::Normal, 3, gl::FLOAT, false, SizeOf(Vertices[0]), (void*)OffsetOf(mesh_vertex, Normal));

		gl::EnableVertexAttribArray(mesh_vertex_layout::TexCoords);
		gl::VertexAttribPointer(mesh_vertex_layout::TexCoords, 2, gl::FLOAT, false, SizeOf(Vertices[0]), (void*)OffsetOf(mesh_vertex, TexCoords));
	}

	/** This function expects the VAO to be bound already */
	void Draw(GLenum OverrideMode = 0) {
		GLenum Mode = OverrideMode != 0 ? OverrideMode : GeometryMode;
		if(IBO != 0) {
			gl::DrawElements(Mode, NumVerts, gl::UNSIGNED_INT, nullptr);
		} else {
			gl::DrawArrays(Mode, 0, NumVerts);
		}
	}

	void Destroy() {
		if (VAO > 0) { gl::DeleteVertexArrays(1, &VAO); VAO = 0; }
		if (VBO > 0) { gl::DeleteBuffers(1, &VBO); VBO = 0; }
		if (IBO > 0) { gl::DeleteBuffers(1, &IBO); IBO = 0; }
		NumVerts = 0; 
		NumIndices = 0;
		GeometryMode = 0;
	}
};

inline
std::vector<mesh_vertex> GenerateArrowTriangles(float InnerRadius, float OuterRadius, float CilynderLen, float ConeLen, int NumSteps) {
	const uint VertsPerStep = 18;
	std::vector<mesh_vertex> Result{ NumSteps * VertsPerStep };

	auto ArrowVert = [](mesh_vertex& Vert, float PosX, float Radius, auto Angle, const vec3* NormalOverride = nullptr) {
		const auto Direction = vec3{ 0, cos(Angle), sin(Angle) };
		Vert.Position = Radius * vec3{ 0.f, Direction.y, Direction.z };
		Vert.Position.x = PosX;
		if (NormalOverride) {
			Vert.Normal = *NormalOverride;
		} else {
			Vert.Normal = vec3{ 0.f, Direction.y, Direction.z };
		}
			
		Vert.TexCoords = { 0, 0 }; // @Improvement generate some uvs?
	};

	const float AngleStep = 2.f * Pi / NumSteps;
	for (auto iStep = 0; iStep < NumSteps; ++iStep) {
		const float Angle1 = iStep * AngleStep;
		const float Angle2 = (iStep + 1) * AngleStep;

		const auto iVert = iStep * VertsPerStep;
		auto i = 0;

		const auto BackNormal = vec3{ -1.f, 0.f, 0.f };

		// Cylinder back
		ArrowVert(Result[iVert+(i++)], 0.f, InnerRadius, Angle2, &BackNormal);
		ArrowVert(Result[iVert+(i++)], 0.f, InnerRadius, Angle1, &BackNormal);
		ArrowVert(Result[iVert+(i++)], 0.f, 0.f, .5f*(Angle1+Angle2), &BackNormal);

		// Cylinder
		ArrowVert(Result[iVert+(i++)], 0.f, InnerRadius, Angle1);
		ArrowVert(Result[iVert+(i++)], 0.f, InnerRadius, Angle2);
		ArrowVert(Result[iVert+(i++)], CilynderLen, InnerRadius, Angle1);
		ArrowVert(Result[iVert+(i++)], 0.f, InnerRadius, Angle2);
		ArrowVert(Result[iVert+(i++)], CilynderLen, InnerRadius, Angle2);
		ArrowVert(Result[iVert+(i++)], CilynderLen, InnerRadius, Angle1);

		// Cone back
		ArrowVert(Result[iVert+(i++)], CilynderLen, InnerRadius, Angle1, &BackNormal);
		ArrowVert(Result[iVert+(i++)], CilynderLen, InnerRadius, Angle2, &BackNormal);
		ArrowVert(Result[iVert+(i++)], CilynderLen, OuterRadius, Angle1, &BackNormal);
		ArrowVert(Result[iVert+(i++)], CilynderLen, InnerRadius, Angle2, &BackNormal);
		ArrowVert(Result[iVert+(i++)], CilynderLen, OuterRadius, Angle2, &BackNormal);
		ArrowVert(Result[iVert+(i++)], CilynderLen, OuterRadius, Angle1, &BackNormal);

		// Cone 
		const auto ConeNormal = glm::normalize(vec3{ OuterRadius/ConeLen, glm::cos(.5f*(Angle1 + Angle2)), glm::sin(.5f*(Angle1 + Angle2)) });
		ArrowVert(Result[iVert + (i++)], CilynderLen, OuterRadius, Angle1, &ConeNormal);
		ArrowVert(Result[iVert + (i++)], CilynderLen, OuterRadius, Angle2, &ConeNormal);
		ArrowVert(Result[iVert + (i++)], CilynderLen+ConeLen, 0, .5f*(Angle1+Angle2), &ConeNormal);
	}

	return Result;
}


inline
std::vector<mesh_vertex> GenerateConeTriangles(float Radius, float Height, int NumSteps) {
	const uint VertsPerStep = 6;
	std::vector<mesh_vertex> Result{ NumSteps * VertsPerStep };

	auto ConeVert = [](mesh_vertex& Vert, float PosX, float Radius, auto Angle, const vec3* NormalOverride, const vec2 TexCoords) {
		const auto Direction = vec3{ 0, cos(Angle), sin(Angle) };
		Vert.Position = Radius * vec3{ 0.f, Direction.y, Direction.z };
		Vert.Position.x = PosX;
		if (NormalOverride) {
			Vert.Normal = *NormalOverride;
		} else {
			Vert.Normal = vec3{ 0.f, Direction.y, Direction.z };
		}

		Vert.TexCoords = TexCoords;
	};

	const float AngleStep = 2.f * Pi / NumSteps;
	for (auto iStep = 0; iStep < NumSteps; ++iStep) {
		const float Angle1 = iStep * AngleStep;
		const float Angle2 = (iStep + 1) * AngleStep;

		const auto iVert = iStep * VertsPerStep;
		auto i = 0;

		const auto BackNormal = vec3{ -1.f, 0.f, 0.f };

		// Cone back
		//ConeVert(Result[iVert + (i++)], 0.f, 0.f, Angle1, &BackNormal);
		//ConeVert(Result[iVert + (i++)], 0.f, 0.f, Angle2, &BackNormal);
		//ConeVert(Result[iVert + (i++)], 0.f, Radius, Angle1, &BackNormal);
		ConeVert(Result[iVert + (i++)], 0.f, 0.f, .5f*(Angle1 + Angle2), &BackNormal, vec2{.5f, 1.f});
		ConeVert(Result[iVert + (i++)], 0.f, Radius, Angle2, &BackNormal, vec2{1.f, 0.f});
		ConeVert(Result[iVert + (i++)], 0.f, Radius, Angle1, &BackNormal, vec2{0.f, 0.f});

		// Cone 
		const auto ConeNormal = glm::normalize(vec3{ Radius / Height, glm::cos(.5f*(Angle1 + Angle2)), glm::sin(.5f*(Angle1 + Angle2)) });
		ConeVert(Result[iVert + (i++)], 0.f, Radius, Angle1, &ConeNormal, vec2{0.f, 0.f});
		ConeVert(Result[iVert + (i++)], 0.f, Radius, Angle2, &ConeNormal, vec2{1.f, 0.f});
		ConeVert(Result[iVert + (i++)], Height, 0, .5f*(Angle1 + Angle2), &ConeNormal, vec2{.5f, 1.f});
	}

	return Result;
}


inline std::vector<mesh_vertex> GenerateQuadTriangles(transform PositionTransform = transform{}, transform UVTransform = transform{}) {
	std::vector<mesh_vertex> Result =
	{
		{{-.5f, -.5f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 0.f}},
		{{+.5f, -.5f, 0.f}, {0.f, 0.f, -1.f}, {1.f, 0.f}},
		{{-.5f, +.5f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 1.f}},

		{{+.5f, -.5f, 0.f}, {0.f, 0.f, -1.f}, {1.f, 0.f}},
		{{+.5f, +.5f, 0.f}, {0.f, 0.f, -1.f}, {1.f, 1.f}},
		{{-.5f, +.5f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 1.f}},
	};

	// Transform vertices
	auto PosTrans = PositionTransform.ToMatrix();
	auto UVTrans = UVTransform.ToMatrix();
	for (auto& Vertex : Result) {
		Vertex.Position = PosTrans * vec4{ Vertex.Position, 1.f };
		Vertex.TexCoords = vec2{ UVTrans * vec4 { Vertex.TexCoords, 0.f, 1.f } };
	}

	return Result;
}

inline std::vector<mesh_vertex> GenerateCubeTriangles(transform PositionTransform = transform{}, transform UVTransform = transform{}) {
	std::vector<mesh_vertex> Result =
	{ {
		{ { -0.5f, -0.5f, -0.5f },{ +0.0f, +0.0f, -1.0f },{ +0.0f, +0.0f } },
		{ { +0.5f, +0.5f, -0.5f },{ +0.0f, +0.0f, -1.0f },{ +1.0f, +1.0f } },
		{ { +0.5f, -0.5f, -0.5f },{ +0.0f, +0.0f, -1.0f },{ +1.0f, +0.0f } },
		{ { +0.5f, +0.5f, -0.5f },{ +0.0f, +0.0f, -1.0f },{ +1.0f, +1.0f } },
		{ { -0.5f, -0.5f, -0.5f },{ +0.0f, +0.0f, -1.0f },{ +0.0f, +0.0f } },
		{ { -0.5f, +0.5f, -0.5f },{ +0.0f, +0.0f, -1.0f },{ +0.0f, +1.0f } },

		{ { +0.5f, +0.5f, +0.5f },{ +0.0f, +0.0f, +1.0f },{ +1.0f, +1.0f } },
		{ { -0.5f, -0.5f, +0.5f },{ +0.0f, +0.0f, +1.0f },{ +0.0f, +0.0f } },
		{ { +0.5f, -0.5f, +0.5f },{ +0.0f, +0.0f, +1.0f },{ +1.0f, +0.0f } },
		{ { -0.5f, +0.5f, +0.5f },{ +0.0f, +0.0f, +1.0f },{ +0.0f, +1.0f } },
		{ { -0.5f, -0.5f, +0.5f },{ +0.0f, +0.0f, +1.0f },{ +0.0f, +0.0f } },
		{ { +0.5f, +0.5f, +0.5f },{ +0.0f, +0.0f, +1.0f },{ +1.0f, +1.0f } },

		{ { -0.5f, +0.5f, +0.5f },{ -1.0f, +0.0f, +0.0f },{ +1.0f, +0.0f } },
		{ { -0.5f, +0.5f, -0.5f },{ -1.0f, +0.0f, +0.0f },{ +1.0f, +1.0f } },
		{ { -0.5f, -0.5f, -0.5f },{ -1.0f, +0.0f, +0.0f },{ +0.0f, +1.0f } },
		{ { -0.5f, -0.5f, -0.5f },{ -1.0f, +0.0f, +0.0f },{ +0.0f, +1.0f } },
		{ { -0.5f, -0.5f, +0.5f },{ -1.0f, +0.0f, +0.0f },{ +0.0f, +0.0f } },
		{ { -0.5f, +0.5f, +0.5f },{ -1.0f, +0.0f, +0.0f },{ +1.0f, +0.0f } },

		{ { +0.5f, +0.5f, +0.5f },{ +1.0f, +0.0f, +0.0f },{ +1.0f, +0.0f } },
		{ { +0.5f, -0.5f, -0.5f },{ +1.0f, +0.0f, +0.0f },{ +0.0f, +1.0f } },
		{ { +0.5f, +0.5f, -0.5f },{ +1.0f, +0.0f, +0.0f },{ +1.0f, +1.0f } },
		{ { +0.5f, -0.5f, -0.5f },{ +1.0f, +0.0f, +0.0f },{ +0.0f, +1.0f } },
		{ { +0.5f, +0.5f, +0.5f },{ +1.0f, +0.0f, +0.0f },{ +1.0f, +0.0f } },
		{ { +0.5f, -0.5f, +0.5f },{ +1.0f, +0.0f, +0.0f },{ +0.0f, +0.0f } },

		{ { -0.5f, -0.5f, -0.5f },{ +0.0f, -1.0f, +0.0f },{ +0.0f, +1.0f } },
		{ { +0.5f, -0.5f, -0.5f },{ +0.0f, -1.0f, +0.0f },{ +1.0f, +1.0f } },
		{ { +0.5f, -0.5f, +0.5f },{ +0.0f, -1.0f, +0.0f },{ +1.0f, +0.0f } },
		{ { +0.5f, -0.5f, +0.5f },{ +0.0f, -1.0f, +0.0f },{ +1.0f, +0.0f } },
		{ { -0.5f, -0.5f, +0.5f },{ +0.0f, -1.0f, +0.0f },{ +0.0f, +0.0f } },
		{ { -0.5f, -0.5f, -0.5f },{ +0.0f, -1.0f, +0.0f },{ +0.0f, +1.0f } },

		{ { -0.5f, +0.5f, -0.5f },{ +0.0f, +1.0f, +0.0f },{ +0.0f, +1.0f } },
		{ { +0.5f, +0.5f, +0.5f },{ +0.0f, +1.0f, +0.0f },{ +1.0f, +0.0f } },
		{ { +0.5f, +0.5f, -0.5f },{ +0.0f, +1.0f, +0.0f },{ +1.0f, +1.0f } },
		{ { +0.5f, +0.5f, +0.5f },{ +0.0f, +1.0f, +0.0f },{ +1.0f, +0.0f } },
		{ { -0.5f, +0.5f, -0.5f },{ +0.0f, +1.0f, +0.0f },{ +0.0f, +1.0f } },
		{ { -0.5f, +0.5f, +0.5f },{ +0.0f, +1.0f, +0.0f },{ +0.0f, +0.0f } },
	} };

	// Transform vertices
	auto PosTrans = PositionTransform.ToMatrix();
	auto UVTrans = UVTransform.ToMatrix();
	for (auto& Vertex : Result) {
		Vertex.Position = PosTrans * vec4{ Vertex.Position, 1.f };
		Vertex.TexCoords = vec2{ UVTrans * vec4{ Vertex.TexCoords, 0.f, 1.f } };
	}

	return Result;
}