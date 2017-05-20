#pragma once

#include <common.hpp>
#include <transform.hpp>

struct camera {
	transform Transform;
	float32 VerticalFov;
	float32 NearPlane, FarPlane;
	vec2 ViewportDimensions;

	camera() = delete;
	camera(vec2 ViewportDimensions, float32 VerticalFov = glm::radians(60.0f), float32 NearPlane = .1f, float32 FarPlane = 100.f);

	mat4 View() const;
	mat4 Projection() const;
	mat4 ViewProjection() const;
};

camera::camera(vec2 ViewportDimensions, float32 VerticalFov, float32 NearPlane, float32 FarPlane)
	: Transform(transform{}),
	VerticalFov(VerticalFov),
	NearPlane(NearPlane),
	FarPlane(FarPlane),
	ViewportDimensions(ViewportDimensions) {}

mat4
camera::ViewProjection() const {
	return Projection() * View();
}


mat4 camera::View() const {
	auto Rot = glm::toMat4(conjugate(Transform.Rotation));
	return glm::translate(Rot, -Transform.Position);
}

mat4 camera::Projection() const {
	const auto AspectRatio = ViewportDimensions.x / ViewportDimensions.y;
	return glm::perspective(VerticalFov, AspectRatio, NearPlane, FarPlane);
}
