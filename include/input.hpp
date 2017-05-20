#pragma once
#include <common.hpp>
#include <GLFW/glfw3.h>
#include <map>

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
	vec2 Pos;

	std::array<bool8, 8> Pressed;
};

template <typename state>
struct captured_state {
		state Now;
		state Prev;
};

struct input {
	input(){}
	~input(){}

	bool Initialize();
	bool Shutdown();
	

	captured_state<mouse_state> Mouse;

	bool8 IsDown(mouse_button Button) const;
	bool8 IsUp(mouse_button Button) const;
	bool8 JustDown(mouse_button Button) const;
	bool8 JustUp(mouse_button Button) const;
	vec2 MouseDelta() const;

	bool8 IsDown(int Key) const;
	bool8 IsUp(int Key) const;

	void StartFrame();
	void EndFrame();
};

static input Input;

inline bool input::Initialize() {
	StartFrame();
	Mouse.Prev.Pos = Mouse.Now.Pos;
	return true;
}

inline bool input::Shutdown() { return true; }

inline void input::StartFrame() {
	glm::dvec2 Pos;
	glfwGetCursorPos(Window, &Pos.x, &Pos.y);

	Mouse.Now.Pos = Pos;
}

inline void input::EndFrame() {
	Mouse.Prev = Mouse.Now;
}

inline bool8 input::IsDown(mouse_button Button) const {
	auto Index = (uint)Button;
	return Mouse.Now.Pressed[Index];
}

inline bool8 input::IsUp(mouse_button Button) const {
	auto Index = (uint)Button;
	return Mouse.Now.Pressed[Index];
}

inline bool8 input::JustDown(mouse_button Button) const {
	auto Index = (uint) Button;
	return Mouse.Now.Pressed[Index] && !Mouse.Prev.Pressed[Index];
}

inline bool8 input::JustUp(mouse_button Button) const {
	auto Index = (uint)Button;
	return Mouse.Now.Pressed[Index] && !Mouse.Prev.Pressed[Index];
}

inline vec2 input::MouseDelta() const {
	return Mouse.Now.Pos - Mouse.Prev.Pos;
}

inline bool8 input::IsDown(int Key) const {
	return glfwGetKey(Window, Key) == GLFW_PRESS;
}

inline bool8 input::IsUp(int Key) const {
	return glfwGetKey(Window, Key) == GLFW_RELEASE;
}


void MouseButtonCallback(GLFWwindow* /*Window*/, int Button, int Action, int /*Mods*/) {
	Input.Mouse.Now.Pressed[Button] = (Action == GLFW_PRESS);
}

void CursorPosCallback(GLFWwindow* /*Window*/, double X, double Y) {
	Input.Mouse.Now.Pos.x = (float) X;
	Input.Mouse.Now.Pos.y = (float) Y;
}