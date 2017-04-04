#pragma once
#include <common.hpp>
#include <GLFW/glfw3.h>
#include <type_traits>

enum class mouse_button : uint {
	Left = GLFW_MOUSE_BUTTON_LEFT,
	Right = GLFW_MOUSE_BUTTON_RIGHT,
	Middle = GLFW_MOUSE_BUTTON_MIDDLE,

	Mouse1 = GLFW_MOUSE_BUTTON_1,
	Mouse2 = GLFW_MOUSE_BUTTON_2,
	Mouse3 = GLFW_MOUSE_BUTTON_3,
	Mouse4 = GLFW_MOUSE_BUTTON_4,
	Mouse5 = GLFW_MOUSE_BUTTON_5,
	Mouse6 = GLFW_MOUSE_BUTTON_6,
	Mouse7 = GLFW_MOUSE_BUTTON_7,
	Mouse8 = GLFW_MOUSE_BUTTON_8
};

struct mouse_state {
	glm::vec2 Pos;

	std::array<b8, 8> Pressed;
};

template <typename state, uint N>
struct captured_state {
	StaticAssert(N >= 2);

	union {
		std::array<state, N> States;

		struct {
			mouse_state Now;
			mouse_state Prev;
		};
	};

	inline captured_state() : States{} {}
};

static struct input {
	static constexpr uint NumCaptureFrames = 2;

	captured_state<mouse_state, NumCaptureFrames> Mouse;

	bool IsDown(mouse_button Button) const;
	bool IsUp(mouse_button Button) const;
	bool JustDown(mouse_button Button) const;
	bool JustUp(mouse_button Button) const;

	void EndFrame();

} Input;

inline void input::EndFrame() {
	for (uint iState = 0; iState < Mouse.States.size() - 1; ++iState) {
		Mouse.States[iState + 1] = Mouse.States[iState];
	}
}

inline bool input::IsDown(mouse_button Button) const {
	auto Index = (uint)Button;
	return Mouse.Now.Pressed[Index];
}

inline bool input::IsUp(mouse_button Button) const {
	auto Index = (uint)Button;
	return Mouse.Now.Pressed[Index];
}

inline bool input::JustDown(mouse_button Button) const {
	auto Index = (uint) Button;
	return Mouse.Now.Pressed[Index] && !Mouse.Prev.Pressed[Index];
}

inline bool input::JustUp(mouse_button Button) const {
	auto Index = (uint)Button;
	return Mouse.Now.Pressed[Index] && !Mouse.Prev.Pressed[Index];
}

void MouseButtonCallback(GLFWwindow* /*Window*/, int Button, int Action, int /*Mods*/) {
	Input.Mouse.Now.Pressed[Button] = (Action == GLFW_PRESS);
}

void CursorPosCallback(GLFWwindow* /*Window*/, double X, double Y) {
	Input.Mouse.Now.Pos.x = (float) X;
	Input.Mouse.Now.Pos.y = (float) Y;
}
