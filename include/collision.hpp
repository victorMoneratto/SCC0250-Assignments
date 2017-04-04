#pragma once

#include <common.hpp>


namespace inside {
	typedef u8 underlying_type;

	enum type : underlying_type {
		Not = 0, // <- there surely is a more expressive name for this completely outside case
		Partially,
		Completely,
	};
}

struct circle_inside_rect {
	glm::tvec2<inside::underlying_type> Inside;
	glm::ivec2 LeavingDirection;
};

inline auto CircleInsideRect(glm::vec2 CircleCenter, float CircleRadius, glm::vec2 RectCenter, glm::vec2 RectHalfDimensions) {
	circle_inside_rect Result = {
		{ inside::Completely, inside::Completely },
		{ 0, 0 }
	};

	// Perform a circle x rect for a single axis
	auto DoAxis = [](auto CircleLesser, auto CircleGreater,
		auto RectLesser, auto RectGreater,
		auto& ResultInside, auto& ResultDirection) {

		// Greater Side
		if (CircleLesser > RectGreater) {
			ResultInside = inside::Not;
			ResultDirection = +1;
		}
		else if (CircleGreater > RectGreater) {
			ResultInside = inside::Partially;
			ResultDirection = +1;
		}

		// Lesser Side
		else if (CircleGreater < RectLesser) {
			ResultInside = inside::Not;
			ResultDirection = -1;
		}
		else if (CircleLesser < RectLesser) {
			ResultInside = inside::Partially;
			ResultDirection = -1;
		}
	};

	auto CircleLeft = CircleCenter.x - CircleRadius;
	auto CircleRight = CircleCenter.x + CircleRadius;
	auto RectLeft = RectCenter.x - RectHalfDimensions.x;
	auto RectRight = RectCenter.x + RectHalfDimensions.x;
	DoAxis(CircleLeft, CircleRight, RectLeft, RectRight, Result.Inside.x, Result.LeavingDirection.x);

	auto CircleTop = CircleCenter.y - CircleRadius;
	auto CircleBottom = CircleCenter.y + CircleRadius;
	auto RectTop = RectCenter.y - RectHalfDimensions.y;
	auto RectBottom = RectCenter.y + RectHalfDimensions.y;
	DoAxis(CircleTop, CircleBottom, RectTop, RectBottom, Result.Inside.y, Result.LeavingDirection.y);

	return Result;
}