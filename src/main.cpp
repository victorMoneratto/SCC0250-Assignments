// GL
#include <gl_33.hpp>
#include <GLFW/glfw3.h>

// Ours
#include <common.hpp>
#include <file.hpp>
#include <shader.hpp>
#include <texture.hpp>
#include <transform.hpp>
#include <collision.hpp>
#include <input.hpp>
#include <quad.hpp>
#include <framebuffer.hpp>

//GLM
#include <glm/gtc/random.hpp>
#include <glm/gtx/euler_angles.inl>

// rand
#include <cstdlib>
#include <ctime>

void GLFWErrorCallback(int Error, const char* Desc);

void APIENTRY GLErrorLog(GLenum Source, GLenum Type, GLuint ID, GLenum Severity,
	GLsizei Length, const GLchar *Message, const void * UserParam);

struct fish {
	// Position, Scaling and Rotation
	transform Transform;

	// noramlized movement direction
	glm::vec3 Direction;

	// Linear movement speed
	f32 Speed;
	static constexpr f32 MinSpeed = 500.f;
	static constexpr f32 MaxSpeed = 1000.f;
	
	// Angular speed
	static constexpr f32 AngularSpeed = 5.f*Pi;

	// result of "collision" between bounding circle and screen
	circle_inside_rect InsideScreen;

	// Texture data
	texture Texture;
	// texture unit to bound texture
	static constexpr uint TextureUnit = 0;

	explicit fish(transform Transform, std::string TexturePath)
		: Transform{Transform}
		, Direction{glm::vec3{0.f}}
		, Speed{0}
		, Texture{ TexturePath } {
	}
};
int main() {
	// Seed entropy
	std::srand((uint)time(nullptr));

	// Initialize glfw systems
	glfwInit();
	defer{ glfwTerminate(); };
	glfwSetErrorCallback(GLFWErrorCallback);

	// OpenGL version and parameters
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	// Resizable
	glfwWindowHint(GLFW_RESIZABLE, false);

	// Create a window of this dimension
	glm::vec2 ScreenDimension = { 1280, 720 };
	auto Window = glfwCreateWindow((int)ScreenDimension.x, (int)ScreenDimension.y, "Porogarama", nullptr, nullptr);
	Assert(Window);

	// Set input callbacks
	glfwSetMouseButtonCallback(Window, MouseButtonCallback);
	glfwSetCursorPosCallback(Window, CursorPosCallback);

	// Load OpenGL
	glfwMakeContextCurrent(Window);
	if (!gl::sys::LoadFunctions()) {
		Assert(!"Could not load opengl functions");
	}

	// Show info on window title
	{
		char Title[100];
		auto Renderer = gl::GetString(gl::RENDERER);
		sprintf(Title, "Porogaramu (OpenGL %d.%d) [%s]", gl::sys::GetMajorVersion(), gl::sys::GetMinorVersion(), Renderer);
		glfwSetWindowTitle(Window, Title);
	}

#if 1
	if (gl::exts::var_KHR_debug) {
		int ContextFlags;
		gl::GetIntegerv(gl::CONTEXT_FLAGS, &ContextFlags);
		if (ContextFlags & gl::CONTEXT_FLAG_DEBUG_BIT) {
			gl::DebugMessageCallback(GLErrorLog, nullptr);
		}
		else { LogError("Not in a OpenGL debugging context\n"); }

		gl::Enable(gl::DEBUG_OUTPUT_SYNCHRONOUS);
	}
	else { LogError("KHR_DEBUG was not found\n"); }
#endif

	// We want to be always drawing to the entire framebuffer
	{
		glm::ivec2 ScreenDimensionInt;
		glfwGetFramebufferSize(Window, &ScreenDimensionInt.x, &ScreenDimensionInt.y);
		gl::Viewport(0, 0, ScreenDimensionInt.x, ScreenDimensionInt.y);
		
		ScreenDimension = ScreenDimensionInt;
	}

	// Dont take vsync into account
	glfwSwapInterval(0);

	// Enable backface culling
//	gl::Enable(gl::CULL_FACE);
	gl::FrontFace(gl::CCW);
	gl::CullFace(gl::BACK);

	// Enable alpha blending
	gl::Enable(gl::BLEND);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);

	std::array<fish, 3> Fishes = {
		fish{ transform{glm::vec3{ ScreenDimension.x *.25f, ScreenDimension.y *.25f, 0.f }}, "content/fish0.png" },
		fish{ transform{glm::vec3{ ScreenDimension.x *.50f, ScreenDimension.y *.75f, 0.f }}, "content/fish1.png" },
		fish{ transform{glm::vec3{ ScreenDimension.x *.75f, ScreenDimension.y *.25f, 0.f }}, "content/fish2.png" }
	};

	for(auto& Quad : Fishes) {
		Quad.Speed = glm::linearRand(fish::MinSpeed, fish::MaxSpeed);
		if(Quad.Texture.Load()) {
				Quad.Transform.Scale = glm::vec3{ Quad.Texture.Width, Quad.Texture.Height, 1.f } * .5f;
		} else {
			LogError("Failed loading texture: %s", Quad.Texture.Path.c_str());
		}
	}

	// Create VAO for a simple centered quad
	quad_vertices FishQuad{};

	// Create renderprogs and load shaders
	render_program FishRenderprog{};
	FishRenderprog.ShaderPaths[shader_stage::Vertex] = "shader/quad.vert";
	FishRenderprog.ShaderPaths[shader_stage::Fragment] = "shader/quad.frag";
	if(!FishRenderprog.LoadShaders()) {}

	// We never move the camera in this assignment
	// transformation to view space equals identity and can be ignored
	//auto View = glm::mat4{};
	
	// Build orthographic projection
	auto Projection = glm::ortho(0.f, ScreenDimension.x, ScreenDimension.y, 0.f);

	// Intermediate framebuffer
	default_framebuffer IntermediateFramebuffer{glm::ivec2{ScreenDimension}};

	// Intermediate quad
// 
	quad_vertices ScreenQuad{ transform{ glm::vec3{0}, glm::vec3{2.f} } };

	// Intermediate renderprog
	render_program ScreenRenderprog{};
	ScreenRenderprog.ShaderPaths[shader_stage::Vertex] = "shader/post.vert";
	ScreenRenderprog.ShaderPaths[shader_stage::Fragment] = "shader/post.frag";
	if (!ScreenRenderprog.LoadShaders()) {}

	// timing from start of simulation
	float StartTime = (float) glfwGetTime();
	float LastTime = (float) StartTime;

	//////////////////////////////////
	// INTERACTION LOOP
	//////////////////////////////////
    while(!glfwWindowShouldClose(Window)) {
    	// Handle OS events
		glfwPollEvents();

		/////////////////////////////////
		// UPDATE LOGIC
		/////////////////////////////////
	
    	// Per-frame timing
		float TimeSinceStart = (float)glfwGetTime() - StartTime;
		float DeltaTime = (float)glfwGetTime() - LastTime;
		LastTime = (float)glfwGetTime();

		for (uint iFish = 0; iFish < Fishes.size(); ++iFish) {
			auto& Fish = Fishes[iFish];

			// Left click makes the fishes go after the cursor
			if(Input.JustDown(mouse_button::Left)) {
				auto Offset = glm::vec3{ Input.Mouse.Now.Pos, 0.f } - Fish.Transform.Position;
				auto DirectionLength = glm::length(Offset);
				if (DirectionLength >= .1f) {
					// Set direction (with entropy)
					Fish.Direction = glm::normalize(Offset);
				}

			// Right click makes the fishes take a random direction
			} else if(Input.JustDown(mouse_button::Right)) {
				Fish.Direction = glm::vec3{ glm::circularRand(1.f), 0.f };
			}
	
			// Simple euler integration for position
			Fish.Transform.Position += Fish.Direction * Fish.Speed * DeltaTime;

			// Take angle from direction
			float Angle = glm::atan(Fish.Direction.y, Fish.Direction.x);
			auto ConstantInterpAngle = [](float Current, float Target, float AngularSpeed, float DeltaTime) -> float{
				float Result;

				auto DeltaAngle0 = Target - Current;			// Common case
				auto DeltaAngle1 = Target - Current - (2.f*Pi); // Going from +Pi to -Pi
				auto DeltaAngle2 = Target - Current + (2.f*Pi); // Going from -Pi to +Pi
				
				// Take the least of the three is absolute
				auto DeltaAngle = glm::abs(DeltaAngle0) < glm::abs(DeltaAngle1)? DeltaAngle0 : DeltaAngle1;
				DeltaAngle = glm::abs(DeltaAngle) < glm::abs(DeltaAngle2) ? DeltaAngle : DeltaAngle2;

				auto MaxDeltaAngle = AngularSpeed * DeltaTime;
				
				Result = Current + glm::clamp(DeltaAngle, -MaxDeltaAngle, MaxDeltaAngle);
				if(glm::abs(Result) > Pi) {
					Result -= glm::sign(Result) * 2.f * Pi;
				}

				return Result;
			};
			Fish.Transform.Rotation.z = ConstantInterpAngle(Fish.Transform.Rotation.z, Angle, fish::AngularSpeed, DeltaTime);

			// Fish bounding-circle x Rect collision
			auto CircleCenter = Fish.Transform.Position;
			auto CircleRadius = (float)glm::sqrt(2) / 2.f * glm::max(Fish.Transform.Scale.x, Fish.Transform.Scale.y);
			Fish.InsideScreen = CircleInsideRect(CircleCenter, CircleRadius, ScreenDimension * .5f, ScreenDimension * .5f);

			// If fish is completely out of the screen in x, move it back
			if (Fish.InsideScreen.Inside.x == inside::Not) {
				Fish.Transform.Position.x += -Fish.InsideScreen.LeavingDirection.x * ScreenDimension.x;
				Fish.InsideScreen.Inside.x = inside::Completely;
			}

			// If fish is completely out of the screen in y, move it back
			if (Fish.InsideScreen.Inside.y == inside::Not) {
				Fish.Transform.Position.y += -Fish.InsideScreen.LeavingDirection.y * ScreenDimension.y;
				Fish.InsideScreen.Inside.y = inside::Completely;
			}
	
		}

		/////////////////////////////////
		// DRAWING
		/////////////////////////////////
    	
		// Bind intermediate framebuffer
		gl::BindFramebuffer(gl::FRAMEBUFFER, IntermediateFramebuffer.ID);

		// Clear buffers
		const auto ClearColor = glm::vec3{ .2, .3, .65 };
		gl::ClearColor(ClearColor.r, ClearColor.g, ClearColor.b, 1.f);
		gl::Clear(gl::COLOR_BUFFER_BIT);

		// Bind shader
        gl::UseProgram(FishRenderprog.ID);

		// Get MVP uniform location for later
		auto MVPLoc = gl::GetUniformLocation(FishRenderprog.ID, "MVP");
		Assert(MVPLoc >= 0);

    	// Set shader Texture uniform to texture unit = fish::TextureUnit
		auto TextureLoc = gl::GetUniformLocation(FishRenderprog.ID, "Texture");
		Assert(TextureLoc >= 0);
		gl::Uniform1i(TextureLoc, fish::TextureUnit);

		// Bind VAO
		gl::BindVertexArray(FishQuad.VAO);

		// Function to draw the fish given that VAO is bound, 
    	// projection is set and uniform locations are valid
		auto DrawQuad = [=](auto Model) {
			auto MVP = Projection * Model;
			gl::UniformMatrix4fv(MVPLoc, 1, false, glm::value_ptr(MVP));

			gl::DrawElements(gl::TRIANGLES, 6, gl::UNSIGNED_SHORT, nullptr);
		};

		for(auto& Quad : Fishes) {
			// Bind proper texture to texture unit 
			gl::ActiveTexture(gl::TEXTURE0 + fish::TextureUnit);
			if (Quad.Texture.ID != texture::INVALID_ID) {
				gl::BindTexture(gl::TEXTURE_2D, Quad.Texture.ID);
			}
			else {
				gl::BindTexture(gl::TEXTURE_2D, 0);
			}

			auto Model = Quad.Transform.Calculate();
			DrawQuad(Model);

			// For the warping effect we just translate the fish to were it should be and draw again (at most 3 times)
			glm::tvec2<bool> DrawWarped = { Quad.InsideScreen.Inside.x != inside::Completely,
											Quad.InsideScreen.Inside.y != inside::Completely };

			if(DrawWarped.x) {
				auto Offset = -ScreenDimension * glm::vec2{ Quad.InsideScreen.LeavingDirection.x, 0 };
				auto WarpedModel = glm::translate(glm::mat4{}, glm::vec3{ Offset, 0.f }) * Model;
				DrawQuad(WarpedModel);
			}

			if (DrawWarped.y) {
				auto Offset = -ScreenDimension * glm::vec2{ 0, Quad.InsideScreen.LeavingDirection.y };
				auto WarpedModel = glm::translate(glm::mat4{}, glm::vec3{ Offset, 0.f }) * Model;
				DrawQuad(WarpedModel);
			}

			if(DrawWarped.x && DrawWarped.y) {
				auto Offset = -ScreenDimension * glm::vec2{ Quad.InsideScreen.LeavingDirection.x, Quad.InsideScreen.LeavingDirection.y };
				auto WarpedModel = glm::translate(glm::mat4{}, glm::vec3{ Offset, 0.f }) * Model;
				DrawQuad(WarpedModel);
			}
		}
		
    	// Draw to screen
		{
			// Go back to screen framebuffer
			gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
			gl::ClearColor(0.f, 0.f, 1.f, 1.f);
			gl::Clear(gl::COLOR_BUFFER_BIT);
	
			const uint PostTextureUnit = 0;
			gl::ActiveTexture(gl::TEXTURE0 + PostTextureUnit);
			gl::BindTexture(gl::TEXTURE_2D, IntermediateFramebuffer.ColorTexture);

			if(glfwGetKey(Window, GLFW_KEY_INSERT) == GLFW_PRESS) {
				ScreenRenderprog.ReloadShaders();
			}

			gl::UseProgram(ScreenRenderprog.ID);

			auto PostTextureLoc = gl::GetUniformLocation(ScreenRenderprog.ID, "Texture");
			gl::Uniform1i(PostTextureLoc, PostTextureUnit);

			auto TimeLoc = gl::GetUniformLocation(ScreenRenderprog.ID, "Time");
			gl::Uniform1f(TimeLoc, TimeSinceStart);

			auto ResolutionLoc = gl::GetUniformLocation(ScreenRenderprog.ID, "Resolution");
			gl::Uniform2f(ResolutionLoc, ScreenDimension.x, ScreenDimension.y);

			gl::BindVertexArray(ScreenQuad.VAO);
			gl::DrawElements(gl::TRIANGLES, 6, gl::UNSIGNED_SHORT, nullptr);
		}

        glfwSwapBuffers(Window);
		Input.EndFrame();
    }

    return 0;
}

void GLFWErrorCallback(int Error, const char* Desc) {
	LogError("[GLFW] Error %d: %s\n", Error, Desc);

	Assert(!"GLFW Error");
}

void APIENTRY GLErrorLog(GLenum Source, GLenum Type, GLuint /*ID*/, GLenum Severity,
	GLsizei /*Length*/, const GLchar *Message, const void * /*UserParam*/) {

	const char* SourceText = "";
	switch (Source) {
	case gl::DEBUG_SOURCE_API: SourceText = "API"; break;
	case gl::DEBUG_SOURCE_WINDOW_SYSTEM: SourceText = "WINDOW"; break;
	case gl::DEBUG_SOURCE_SHADER_COMPILER: SourceText = "SHADER"; break;
	case gl::DEBUG_SOURCE_THIRD_PARTY: SourceText = "THIRD PARTY"; break;
	case gl::DEBUG_SOURCE_APPLICATION: SourceText = "APPLICATION"; break;
	case gl::DEBUG_SOURCE_OTHER: SourceText = "OTHER"; break;
	default: break;
	}

	const char* TypeText = "";
	switch (Type) {
	case gl::DEBUG_TYPE_ERROR: TypeText = "ERROR"; break;
	case gl::DEBUG_TYPE_DEPRECATED_BEHAVIOR: TypeText = "DEPRECATED BEHAVIOR"; break;
	case gl::DEBUG_TYPE_UNDEFINED_BEHAVIOR: TypeText = "UNDEFINED BEHAVIOR"; break;
	case gl::DEBUG_TYPE_PORTABILITY: TypeText = "PORTABILITY"; break;
	case gl::DEBUG_TYPE_PERFORMANCE: TypeText = "PERFORMANCE"; break;
	case gl::DEBUG_TYPE_MARKER: TypeText = "MARKER"; break;
	case gl::DEBUG_TYPE_PUSH_GROUP: TypeText = "PUSH_GROUP"; break;
	case gl::DEBUG_TYPE_POP_GROUP: TypeText = "POP_GROUP"; break;
	case gl::DEBUG_TYPE_OTHER: TypeText = "OTHER"; break;
	default: break;
	}

	const char* SeverityText = "";
	switch (Severity) {
	case gl::DEBUG_SEVERITY_HIGH: SeverityText = "HIGH"; break;
	case gl::DEBUG_SEVERITY_MEDIUM: SeverityText = "MEDIUM"; break;
	case gl::DEBUG_SEVERITY_LOW: SeverityText = "LOW"; break;
	case gl::DEBUG_SEVERITY_NOTIFICATION: SeverityText = "NOTIFICATION"; break;
	default: break;
	}

	LogError("GL: [%s][%s][%s] %s\n", SourceText, TypeText, SeverityText, Message);
}