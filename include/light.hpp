#pragma once
#include <gl_33.hpp>
#include <common.hpp>

struct light {
	vec4 Position;
	vec3 Color;
	float Ambient;
	vec3 SpecularColor;vec3 ConeDirection;
	float LinearFalloff;
	float QuadraticFalloff;
	float InnerCone;
	float OuterCone;
};

void BindLight(uint Program, const char* Variable, const light& Light) {
	char Buffer[64];
	auto MakeName = [&](auto Member) { sprintf(Buffer, "%s.%s", Variable, Member); return Buffer; };

	auto PositionLoc = gl::GetUniformLocation(Program, MakeName("Position"));
	auto ColorLoc = gl::GetUniformLocation(Program, MakeName("Color"));
	auto AmbientLoc = gl::GetUniformLocation(Program, MakeName("Ambient"));
	auto SpecularColorLoc = gl::GetUniformLocation(Program, MakeName("SpecularColor"));
	auto ConeDirectionLoc = gl::GetUniformLocation(Program, MakeName("ConeDirection"));
	auto LinearFalloffLoc = gl::GetUniformLocation(Program, MakeName("LinearFalloff"));
	auto QuadraticFalloffLoc = gl::GetUniformLocation(Program, MakeName("QuadraticFalloff"));
	auto InnerConeLoc = gl::GetUniformLocation(Program, MakeName("InnerCone"));
	auto OuterConeLoc = gl::GetUniformLocation(Program, MakeName("OuterCone"));

	gl::Uniform4f(PositionLoc, Light.Position.x, Light.Position.y, Light.Position.z, Light.Position.w);
	gl::Uniform3f(ColorLoc, Light.Color.r, Light.Color.g, Light.Color.b);
	gl::Uniform1f(AmbientLoc, Light.Ambient);
	gl::Uniform3f(SpecularColorLoc, Light.SpecularColor.r, Light.SpecularColor.g, Light.SpecularColor.b);
	gl::Uniform3f(ConeDirectionLoc, Light.ConeDirection.x, Light.ConeDirection.y, Light.ConeDirection.z);
	gl::Uniform1f(LinearFalloffLoc, Light.LinearFalloff);
	gl::Uniform1f(QuadraticFalloffLoc, Light.QuadraticFalloff);
	gl::Uniform1f(InnerConeLoc, Light.InnerCone);
	gl::Uniform1f(OuterConeLoc, Light.OuterCone);
}