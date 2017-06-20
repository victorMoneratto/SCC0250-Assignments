#pragma once
#include <gl_33.hpp>
#include <common.hpp>

struct light {
	glm::vec4 Pos;
	glm::vec3 Color;
	float Ambient;
	glm::vec3 ConeDirection;
	float LinearFalloff;
	float QuadraticFalloff;
	float InnerCutoff;
	float OuterCutoff;
};

void BindUniforms(uint Program, const char* Variable, const light& Light) {
	char Buffer[64];
	auto MakeName = [&](auto Member) { sprintf(Buffer, "%s.%s", Variable, Member); return Buffer; };

	auto PosLoc = gl::GetUniformLocation(Program, MakeName("Pos"));
	auto ColorLoc = gl::GetUniformLocation(Program, MakeName("Color"));
	auto AmbientLoc = gl::GetUniformLocation(Program, MakeName("Ambient"));
	auto ConeDirectionLoc = gl::GetUniformLocation(Program, MakeName("ConeDirection"));
	auto LinearFalloffLoc = gl::GetUniformLocation(Program, MakeName("LinearFalloff"));
	auto QuadraticFalloffLoc = gl::GetUniformLocation(Program, MakeName("QuadraticFalloff"));
	auto InnerCutoffLoc = gl::GetUniformLocation(Program, MakeName("InnerCutoff"));
	auto OuterCutoffLoc = gl::GetUniformLocation(Program, MakeName("OuterCutoff"));
}