// GL
#include <gl_33.hpp>
#include <GLFW/glfw3.h>

// Ours
#include <common.hpp>
#include <file.hpp>
#include <shader.hpp>
#include <texture.hpp>
#include <cubemap.hpp>
#include <transform.hpp>
#include <camera.hpp>
#include <input.hpp>
#include <mesh.hpp>
#include <light.hpp>
#include <glm/gtx/euler_angles.hpp>

void GLFWErrorCallback(int Error, const char* Desc);

void APIENTRY GLErrorLog(GLenum Source, GLenum Type, GLuint ID, GLenum Severity,
	GLsizei Length, const GLchar *Message, const void * UserParam);


GLFWwindow* Window;

int main() {

	// Initialize glfw systems
	glfwInit();
	defer{ glfwTerminate(); };
	glfwSetErrorCallback(GLFWErrorCallback);

	// OpenGL version and parameters
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_SRGB_CAPABLE, true);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Resizable
	glfwWindowHint(GLFW_RESIZABLE, false);

	// Create a window of this dimension
	vec2 ScreenDimension = { 1280, 720 };
	Window = glfwCreateWindow((int)ScreenDimension.x, (int)ScreenDimension.y, "Porogarama", nullptr, nullptr);
	Assert(Window);

	// Set input callbacks
	Input.Initialize();
	defer{ Input.Shutdown(); };
	glfwSetMouseButtonCallback(Window, MouseButtonCallback);
	glfwSetCursorPosCallback(Window, CursorPosCallback);
	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Load OpenGL
	glfwMakeContextCurrent(Window);
	if (!gl::sys::LoadFunctions()) {
		Assert(!"Could not load opengl functions");
	}

	// Show info on window title
	{
		char Title[100];
		auto Renderer = (char*) gl::GetString(gl::RENDERER);
		sprintf(Title, "Porogaramu (OpenGL %d.%d) [%s]", gl::sys::GetMajorVersion(), gl::sys::GetMinorVersion(), Renderer);
		glfwSetWindowTitle(Window, Title);
	}

#if DEBUGGING
	if (gl::exts::var_KHR_debug) {
		int ContextFlags;
		gl::GetIntegerv(gl::CONTEXT_FLAGS, &ContextFlags);
		if (ContextFlags & gl::CONTEXT_FLAG_DEBUG_BIT) {
			gl::DebugMessageCallback(GLErrorLog, nullptr);
		} else { LogError("No debugging callback due to OpenGL context not set to debug\n"); }

		gl::Enable(gl::DEBUG_OUTPUT_SYNCHRONOUS);
	} else { LogError("KHR_DEBUG was not found\n"); }
#endif

	// We want to be always drawing to the entire framebuffer
	{
		glm::ivec2 ScreenDimensionInt;
		glfwGetFramebufferSize(Window, &ScreenDimensionInt.x, &ScreenDimensionInt.y);
		gl::Viewport(0, 0, ScreenDimensionInt.x, ScreenDimensionInt.y);
		
		ScreenDimension = ScreenDimensionInt;
	}

	// Enable vsync
    glfwSwapInterval(1);

	// Backface culling
	gl::Enable(gl::CULL_FACE);
	gl::FrontFace(gl::CCW);
	gl::CullFace(gl::BACK);

	// Z-buffering
	gl::Enable(gl::DEPTH_TEST);

	// Gamma correction
	gl::Enable(gl::FRAMEBUFFER_SRGB);

	// alpha blending
	gl::Enable(gl::BLEND);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);

	//////////////////////////////////////
	// RENDER PROGRAMS (aka shaders)
	/////////////////////////////////////

	// Blinn-Phong shading render program
	render_program PhongRenderProg{};
	PhongRenderProg.ShaderPaths[shader_stage::Vertex] = "shader/phong.vert";
	PhongRenderProg.ShaderPaths[shader_stage::Fragment] = "shader/phong.frag";
	if (!PhongRenderProg.LoadShaders()) {}

	// Flat shading render program
	render_program FlatRenderProg{};
	FlatRenderProg.ShaderPaths[shader_stage::Vertex] = "shader/flat.vert";
	FlatRenderProg.ShaderPaths[shader_stage::Fragment] = "shader/flat.frag";
	if(!FlatRenderProg.LoadShaders()) {}

	// Gourard shading render program
	render_program GouraudRenderProg{};
	GouraudRenderProg.ShaderPaths[shader_stage::Vertex] = "shader/gouraud.vert";
	GouraudRenderProg.ShaderPaths[shader_stage::Fragment] = "shader/gouraud.frag";
	if (!GouraudRenderProg.LoadShaders()) {}

	// Skybox
	render_program SkyRenderProg{};
	SkyRenderProg.ShaderPaths[shader_stage::Vertex] = "shader/sky.vert";
	SkyRenderProg.ShaderPaths[shader_stage::Fragment] = "shader/sky.frag";
	if (!SkyRenderProg.LoadShaders()) {}

	mesh Arrow{ GenerateArrowTriangles(.05f, .1f, .6f, .4f, 32), gl::TRIANGLES };
	defer{ Arrow.Destroy(); };

	mesh Cube{ GenerateCubeTriangles(), gl::TRIANGLES };
	defer{ Cube.Destroy(); };

	mesh Cone{ GenerateConeTriangles(.5f, 1.f, 4), gl::TRIANGLES };
	defer{ Cone.Destroy(); };

	uint BlankTextureID = MakeBlankTexture();

	texture CubeTexture{ "content/box.jpg" };
	Assert(CubeTexture.Load());

	texture TriangleTexture{ "content/triangle.tga" };
	Assert(TriangleTexture.Load());

	auto Skybox = MakeCubemap("content/skyboxes/day_", "tga", true);
	Assert(Skybox.ID > 0);

	std::array<light, 3> Lights;
	// Directional Light
	Lights[0].Position = vec4(.125f, 1.f, 0.f, 0.f);
	Lights[0].Color = vec3(.25f, .25f, 1.f);
	Lights[0].Ambient = 0.05f;
	Lights[0].SpecularColor = vec3(.5f, .5f, 5.f);

	// Positional Light
	Lights[1].Position = vec4(0.f, .5f, 0.f, 1.f);
	Lights[1].Color = vec3(1.f, .25f, .25f);
	Lights[1].Ambient = 0.05f;
	Lights[1].SpecularColor = vec3(5.f, .5f, .5f);
	Lights[1].LinearFalloff = .025f;
	Lights[1].QuadraticFalloff = .01f;
	Lights[1].ConeDirection = vec3(0.f, 0.f, 0.f);

	// Spotlight
	Lights[2].Position = vec4(1.f, .5f, .75f, 1.f);
	Lights[2].Color = vec3(.25f, 1.f, .25f);
	Lights[2].Ambient = 0.05f;
	Lights[2].SpecularColor = vec3(.5f, 5.f, .5f);
	Lights[2].LinearFalloff = .05f;
	Lights[2].QuadraticFalloff = .01f;
	Lights[2].ConeDirection = vec3(0, 0, -1.f);
	Lights[2].InnerCone = cos(Pi / 16);
	Lights[2].OuterCone = cos(Pi / 12);

	// timing from start of simulation
	float StartTime = (float) glfwGetTime();
	float LastTime = (float) StartTime;

	camera Camera{ScreenDimension};
	Camera.Transform.Position = vec3(0.f, 1.5f, 3.5f);

	enum class lighting_model {
		Phong,
		Gouraud,
		Flat,
	};

	auto Lighting = lighting_model::Phong;

	//////////////////////////////////
	// INTERACTION LOOP
	//////////////////////////////////
    while(!glfwWindowShouldClose(Window)) {
    	// Handle OS events
		glfwPollEvents();

		if (Input.IsDown(GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(Window, true);
			continue;
		}

		/////////////////////////////////
		// UPDATE LOGIC
		/////////////////////////////////

		Input.StartFrame();
	
    	// Per-frame timing
		// float TimeSinceStart = (float)glfwGetTime() - StartTime;
		float DeltaTime = (float)glfwGetTime() - LastTime;
		LastTime = (float)glfwGetTime();

		{
			// Camera movement
			vec3 LocalMoveDir = vec3{ 0.f };
			if (Input.IsDown(GLFW_KEY_A)) { LocalMoveDir.x -= 1.f; }
			if (Input.IsDown(GLFW_KEY_D)) { LocalMoveDir.x += 1.f; }
			if (Input.IsDown(GLFW_KEY_W)) { LocalMoveDir.z -= 1.f; }
			if (Input.IsDown(GLFW_KEY_S)) { LocalMoveDir.z += 1.f; }
			if (Input.IsDown(GLFW_KEY_SPACE)) { LocalMoveDir.y += 1.f; }
			if (Input.IsDown(GLFW_KEY_LEFT_SHIFT)) { LocalMoveDir.y -= 1.f; }
			if(glm::dot(LocalMoveDir, LocalMoveDir) > 0.f) {
				LocalMoveDir = glm::normalize(LocalMoveDir);
			}

			auto MoveDir = glm::rotate(Camera.Transform.Rotation, LocalMoveDir);
			
			const auto Speed = 2.5f;
			Camera.Transform.Position += MoveDir * Speed * DeltaTime;
			
			const auto& MouseDelta = Input.MouseDelta();
			const auto AngularSpeed = .5f;
			static vec3 CameraEulerAngles{0.f};
			CameraEulerAngles.x -= MouseDelta.y * AngularSpeed * DeltaTime;
			CameraEulerAngles.x = glm::clamp(CameraEulerAngles.x, glm::radians(-89.f), glm::radians(89.0f));
			CameraEulerAngles.y -= MouseDelta.x * AngularSpeed * DeltaTime;
			CameraEulerAngles.z = 0.0f;
			Camera.Transform.Rotation = glm::normalize(glm::angleAxis(CameraEulerAngles.y, vec3{ 0.f, 1.f, 0.f }) * glm::angleAxis(CameraEulerAngles.x, vec3{ 1.f, 0.f, 0.f }));
		}

		auto& Spotlight = Lights[2];
		static bool IsFlashlightOn = true;
		if (Input.JustUp(mouse_button::Left) || Input.JustUp(mouse_button::Right)) {
			IsFlashlightOn = !IsFlashlightOn;
		}

		if (IsFlashlightOn) {
			Lights[2].InnerCone = cos(Pi / 16);
			Lights[2].OuterCone = cos(Pi / 12);
			Spotlight.Position = vec4{ Camera.Transform.Position, 1.f };
			Spotlight.ConeDirection = glm::rotate(Camera.Transform.Rotation, vec3{ 0.f, 0.f, -1.f });
		} else {
			Lights[2].InnerCone = 1;
			Lights[2].OuterCone = 1;
		}

		if (Input.IsDown(GLFW_KEY_1)) { Lighting = lighting_model::Phong; }
		else if (Input.IsDown(GLFW_KEY_2)) { Lighting = lighting_model::Gouraud; }
		else if (Input.IsDown(GLFW_KEY_3)) { Lighting = lighting_model::Flat; }

#if DEBUGGING
		gl::UseProgram(0);
		// Reload shaders
		if (Input.IsDown(GLFW_KEY_F7)) {
			PhongRenderProg.ReloadShaders();
			FlatRenderProg.ReloadShaders();
			GouraudRenderProg.ReloadShaders();
			SkyRenderProg.ReloadShaders();
		}
#endif

		/////////////////////////////////
		// DRAWING
		/////////////////////////////////

		// Clear buffers
		const auto ClearColor = vec3{ .2f, .3f, .65f };
		gl::ClearColor(ClearColor.r, ClearColor.g, ClearColor.b, 1.f);
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		render_program* RenderProg = nullptr;
	    switch (Lighting) {
		case lighting_model::Phong: RenderProg = &PhongRenderProg; break;
		case lighting_model::Gouraud: RenderProg = &GouraudRenderProg; break;
		case lighting_model::Flat: RenderProg = &FlatRenderProg; break;
		default: Assert(!"Invalid Lighting model");
	    }

		gl::UseProgram(RenderProg->ID);

		// Lighting uniforms
		for (int i = 0; i < Lights.size(); ++i) {
			char Buffer[24];
			sprintf(Buffer, "Lights[%d]", i);
			BindLight(RenderProg->ID, Buffer, Lights[i]);
		}
		
		// Transform uniforms
		auto ModelLoc = gl::GetUniformLocation(RenderProg->ID, "Model");
		auto MVPLoc = gl::GetUniformLocation(RenderProg->ID, "MVP");
		auto NormalMatLoc = gl::GetUniformLocation(RenderProg->ID, "NormalMat");
		
		// Material uniforms
		auto TextureLoc = gl::GetUniformLocation(RenderProg->ID, "Material.Texture");
    	auto ColorLoc = gl::GetUniformLocation(RenderProg->ID, "Material.Color");
		auto SpecularPowerLoc = gl::GetUniformLocation(RenderProg->ID, "Material.SpecularPower");
		
		
    	auto CameraPositionLoc = gl::GetUniformLocation(RenderProg->ID, "Camera.Position");
		gl::Uniform3f(CameraPositionLoc, Camera.Transform.Position.x, Camera.Transform.Position.y, Camera.Transform.Position.z);
		

		auto SetupRender = [&] (glm::mat4 Model, glm::mat4 MVP, glm::mat4 NormalMat, glm::vec4 Color, float SpecularPower, int TextureSampler, int Texture) {
			gl::UniformMatrix4fv(ModelLoc, 1, false, glm::value_ptr(Model));
			gl::UniformMatrix4fv(MVPLoc, 1, false, glm::value_ptr(MVP));
			gl::UniformMatrix4fv(NormalMatLoc, 1, false, glm::value_ptr(NormalMat));
			gl::Uniform4f(ColorLoc, Color.r, Color.g, Color.b, Color.a);
			gl::Uniform1f(SpecularPowerLoc, SpecularPower);
			gl::Uniform1i(TextureLoc, TextureSampler);
			gl::ActiveTexture(gl::TEXTURE0 + TextureSampler);
			gl::BindTexture(gl::TEXTURE_2D, Texture);
		};

		{
			// Draw Cone
			transform Transform;
			Transform.Position = vec3{ -1.f, 0.f, 0.f };
			Transform.Rotation = glm::rotate(mat4{}, Pi / 2, vec3{ 0.f, 0.f, 1.f });
			auto Model = Transform.ToMatrix();
			auto MVP = Camera.ViewProjection() * Model;
			auto NormalMat = glm::transpose(glm::inverse(Transform.ToMatrix()));
			auto Color = vec4{1.f, 1.f, 1.f, 1.f};
			auto SpecularPower = 32.f;
			auto TextureSampler = 0;
			auto Texture = TriangleTexture.ID;

			SetupRender(Model, MVP, NormalMat, Color, SpecularPower, TextureSampler, Texture);
			
			gl::BindVertexArray(Cone.VAO);
			Cone.Draw();

			gl::BindTexture(gl::TEXTURE_2D, 0);
		}

		{
			// Draw Cube
			transform Transform;
			Transform.Position = vec3{ 1.f, .5f, 0.f };
			auto Model = Transform.ToMatrix();
			auto MVP = Camera.ViewProjection() * Model;
			auto NormalMat = glm::transpose(glm::inverse(Transform.ToMatrix()));
			auto Color = vec4{ 1.f, 1.f, 1.f, 1.f };
			auto SpecularPower = 256.f;
			auto TextureSampler = 0;
			auto Texture = CubeTexture.ID;

			SetupRender(Model, MVP, NormalMat, Color, SpecularPower, TextureSampler, Texture);

			gl::BindVertexArray(Cube.VAO);
			Cube.Draw();

			gl::BindTexture(gl::TEXTURE_2D, 0);
		}

		
		{
			// Draw transform widget
			vec3 Position = vec3{ 0.f, 2.f, 0.f };
			std::array<transform, 3> Models = {transform{}, transform{}, transform{}};
			Models[0].Position = Position;
			Models[1].Position = Position;
			Models[2].Position = Position;

			Models[0].Rotation = quat{};
			Models[1].Rotation = glm::rotate(mat4{}, glm::radians(90.f), vec3{ 0, 0, 1 });
			Models[2].Rotation = glm::rotate(mat4{}, glm::radians(-90.f), vec3{ 0, 1, 0 });

			vec3 TransformScale = vec3{ 1.f };
			Models[0].Scale = TransformScale;
			Models[1].Scale = TransformScale;
			Models[2].Scale = TransformScale;

			std::array<vec3, 3> Colors = { vec3{1.f, 0.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec3{0.f, 0.f, 1.f} };

			gl::BindVertexArray(Arrow.VAO);
			for (int i = 0; i < 3; ++i) {
				auto Model = Models[i].ToMatrix();
				auto MVP = Camera.ViewProjection() * Model;
				auto NormalMat = glm::transpose(glm::inverse(Model));
				auto Color = vec4{ Colors[i], 1.f };
				auto SpecularPower = 32.f;
				auto TextureSampler = 0;
				auto Texture = BlankTextureID;

				SetupRender(Model, MVP, NormalMat, Color, SpecularPower, TextureSampler, Texture);

				Arrow.Draw();
			}
		}

		{
			//Render skybox
			gl::CullFace(gl::FRONT);
			defer{ gl::CullFace(gl::BACK); };

			gl::DepthFunc(gl::LEQUAL);
			defer{ gl::DepthFunc(gl::LESS); };

			gl::UseProgram(SkyRenderProg.ID);

			auto ViewProjection = Camera.Projection() * mat4(mat3(Camera.View())); // Dirty trick to remove translation information
			auto ViewProjectionUniform = gl::GetUniformLocation(SkyRenderProg.ID, "ViewProjection");
			Assert(ViewProjectionUniform >= 0);
			gl::UniformMatrix4fv(ViewProjectionUniform, 1, false, value_ptr(ViewProjection));

			auto TextureUniform = gl::GetUniformLocation(SkyRenderProg.ID, "Skybox");
			Assert(TextureUniform >= 0);
			gl::Uniform1i(TextureUniform, 0);

			gl::ActiveTexture(gl::TEXTURE0);
			gl::BindTexture(gl::TEXTURE_CUBE_MAP, Skybox.ID);
			defer{ gl::BindTexture(gl::TEXTURE_CUBE_MAP, 0); };

			gl::BindVertexArray(Cube.VAO);
			defer{ gl::BindVertexArray(0); };

			Cube.Draw();
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

