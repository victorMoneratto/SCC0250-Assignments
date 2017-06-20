#pragma once

#include <common.hpp>

struct transform {
	vec3 Position;
	vec3 Scale;
	quat Rotation; // Pitch, Yaw, Roll

	explicit transform(vec3 Position = vec3{ 0.f }, vec3 Scale = vec3{ 1.f }, quat Rotation = quat{})
		: Position{ Position }
		, Scale{ Scale }
		, Rotation{ Rotation } {}

	mat4 ToMatrix() const {
		auto Rot = glm::toMat4(Rotation);
		return  scale(translate(mat4{}, this->Position) * Rot, this->Scale);
	}
};
