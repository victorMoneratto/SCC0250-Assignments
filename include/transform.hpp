#pragma once

#include <common.hpp>

struct transform {
	glm::vec3 Position;
	glm::vec3 Scale;
	glm::vec3 Rotation; // Pitch, Yaw, Roll

	explicit transform(glm::vec3 Position = glm::vec3{ 0.f }
			, glm::vec3 Scale = glm::vec3{ 1.f }
			, glm::vec3 Rotation = glm::vec3{0.f});

	glm::mat4 Calculate() const;
};

inline transform::transform(glm::vec3 Position, glm::vec3 Scale, glm::vec3 Rotation) 
		: Position{ Position }
		, Scale{ Scale }
		, Rotation{ Rotation } {}

inline glm::mat4 transform::Calculate() const {
	return  glm::scale(
				glm::rotate(
					glm::rotate(
						glm::rotate(
							glm::translate(glm::mat4{}, this->Position), 
						Rotation.z, {0, 0, 1}),
					Rotation.y, {0.f, 1.f, 0.f}),
				Rotation.x, {1.f, 0.f, 0.f}),
			this->Scale);
}
