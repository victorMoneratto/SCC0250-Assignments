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
#include <framebuffer.hpp>
#include <mesh.hpp>

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

#if 1
	// Enable backface culling
	gl::Enable(gl::CULL_FACE);
	gl::FrontFace(gl::CCW);
	gl::CullFace(gl::BACK);
#endif

#if 0
	// Enable alpha blending
	gl::Enable(gl::BLEND);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
#endif

	//////////////////////////////////////
	// RENDER PROGRAMS (aka shaders)
	/////////////////////////////////////

	// Mesh
	render_program MeshRenderProg{};
	MeshRenderProg.ShaderPaths[shader_stage::Vertex] = "shader/mesh.vert";
	MeshRenderProg.ShaderPaths[shader_stage::Fragment] = "shader/mesh.frag";
	if(!MeshRenderProg.LoadShaders()) {}

	// Mesh normals
	render_program NormalsRenderProg{};
	NormalsRenderProg.ShaderPaths[shader_stage::Vertex] = "shader/mesh.vert";
	NormalsRenderProg.ShaderPaths[shader_stage::Geometry] = "shader/normals.geom";
	NormalsRenderProg.ShaderPaths[shader_stage::Fragment] = "shader/mesh.frag";
	if (!NormalsRenderProg.LoadShaders()) {}

	// Skybox
	render_program SkyRenderProg{};
	SkyRenderProg.ShaderPaths[shader_stage::Vertex] = "shader/sky.vert";
	SkyRenderProg.ShaderPaths[shader_stage::Fragment] = "shader/sky.frag";
	if (!SkyRenderProg.LoadShaders()) {}

	//Post Process
	render_program ScreenRenderProg{};
	ScreenRenderProg.ShaderPaths[shader_stage::Vertex] = "shader/postprocess.vert";
	ScreenRenderProg.ShaderPaths[shader_stage::Fragment] = "shader/postprocess.frag";
	if (!ScreenRenderProg.LoadShaders()) {}

	// We never move the camera in this assignment
	// transformation to view space equals identity and can be ignored
	auto View = glm::translate(mat4{}, vec3{ 0, 0, -1 });
	
	const auto NearZ = 0.01f, FarZ = 100.0f;
#if 0
	// Build orthographic projection
	vec2 OrthoHalfSize = 3.f * vec2{ ScreenDimension.x / ScreenDimension.y, 1.0f };
	auto Projection = glm::ortho(-OrthoHalfSize.x, OrthoHalfSize.x, -OrthoHalfSize.y, OrthoHalfSize.y, NearZ, FarZ);
#else
	const auto Fovy = glm::radians(90.f);
	auto Projection = glm::perspectiveFov(Fovy, ScreenDimension.x, ScreenDimension.y, NearZ, FarZ);
#endif

	// Intermediate framebuffer
	default_framebuffer IntermediateFramebuffer{glm::ivec2{ScreenDimension}};

	// Intermediate quad
	mesh ScreenQuad{GenerateQuadTriangles(transform{ vec3{ 0 }, vec3{ 2.f } }), gl::TRIANGLES};
	defer { ScreenQuad.Destroy(); };

	mesh Arrow{ GenerateArrowTriangles(.05f, .1f, .6f, .4f, 16), gl::TRIANGLES };
	defer{ Arrow.Destroy(); };

	mesh Cube{ GenerateCubeTriangles(/*transform{vec3{0.f}, vec3{2.f}}*/), gl::TRIANGLES };
	defer{ Cube.Destroy(); };

	mesh Cone{ GenerateConeTriangles(.5f, 1.f, 4), gl::TRIANGLES };
	defer{ Cone.Destroy(); };

	texture CubeTexture{ "content/box.jpg" };
	Assert(CubeTexture.Load());

	texture TriangleTexture{ "content/triangle.tga" };
	Assert(TriangleTexture.Load());

	auto Skybox = MakeCubemap("content/skyboxes/day_", "tga", true);
	Assert(Skybox.ID > 0);

	// timing from start of simulation
	float StartTime = (float) glfwGetTime();
	float LastTime = (float) StartTime;

	camera Camera{ScreenDimension};
	Camera.Transform.Position = vec3(0.f, 1.5f, 3.5f);

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
		float TimeSinceStart = (float)glfwGetTime() - StartTime;
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
			//CameraEulerAngles.x = glm::clamp(CameraEulerAngles.x, glm::radians(-89.f), glm::radians(89.0f));
			CameraEulerAngles.y -= MouseDelta.x * AngularSpeed * DeltaTime;
			CameraEulerAngles.z = 0.0f;
			Camera.Transform.Rotation = glm::normalize(glm::angleAxis(CameraEulerAngles.y, vec3{0.f, 1.f, 0.f}) * glm::angleAxis(CameraEulerAngles.x, vec3{1.f, 0.f, 0.f}));
		}

		// Things moving
		vec3 ConePosition;
		vec3 TransformScale;
		quat CubeRotation;
		{
			static float ConePositionAlpha = 0.f;
			static float TransformScaleAlpha = 1.f;
			static float CubeRotationAlpha = 0.f;

			if (Input.IsDown(GLFW_KEY_LEFT)) { ConePositionAlpha += Pi * DeltaTime; }
			if (Input.IsDown(GLFW_KEY_UP))	 { TransformScaleAlpha += Pi * DeltaTime; }
			if (Input.IsDown(GLFW_KEY_DOWN)) { TransformScaleAlpha = glm::max(.1f, TransformScaleAlpha - Pi * DeltaTime); }
			if (Input.IsDown(GLFW_KEY_RIGHT)){ CubeRotationAlpha += Pi * DeltaTime; }

			ConePosition = 2.f * vec3{0.f, 1.f, 0.f} * glm::abs(glm::sin(ConePositionAlpha));
			TransformScale = vec3{ TransformScaleAlpha };
			CubeRotation = glm::rotate(mat4{}, CubeRotationAlpha, vec3{ 0.f, 1.f, 0.f });
		}

#if DEBUGGING
		gl::UseProgram(0);
		// Reload shaders
		if (Input.IsDown(GLFW_KEY_F7)) {
			MeshRenderProg.ReloadShaders();
			NormalsRenderProg.ReloadShaders();
			SkyRenderProg.ReloadShaders();
			ScreenRenderProg.ReloadShaders();
		}
#endif

		/////////////////////////////////
		// DRAWING
		/////////////////////////////////

#define POSTPROCESS false
#if POSTPROCESS
		// Bind intermediate framebuffer
		gl::BindFramebuffer(gl::FRAMEBUFFER, IntermediateFramebuffer.ID);
#else
		gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
#endif

		gl::Enable(gl::DEPTH_TEST);
		gl::DepthMask(true);
		gl::DepthFunc(gl::LESS);

		// Clear buffers
		const auto ClearColor = vec3{ .2, .3, .65 };
		gl::ClearColor(ClearColor.r, ClearColor.g, ClearColor.b, 1.f);
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		// Uniforms
		gl::UseProgram(MeshRenderProg.ID);
		auto MVPLoc = gl::GetUniformLocation(MeshRenderProg.ID, "MVP");
		auto NormalMatLoc = gl::GetUniformLocation(MeshRenderProg.ID, "NormalMat");
		auto ColorLoc = gl::GetUniformLocation(MeshRenderProg.ID, "Color");
		auto TextureLoc = gl::GetUniformLocation(MeshRenderProg.ID, "Texture");
		auto bSolidColorLoc = gl::GetUniformLocation(MeshRenderProg.ID, "bTexture");

		{
			// Draw Cone
			auto Transform = transform{ vec3{-1.f, 0.f, 0.f} + ConePosition, vec3{1.f}, glm::rotate(mat4{}, Pi/2, vec3{0.f, 0.f, 1.f}) };
			auto MVP = Camera.ViewProjection() * Transform.ToMatrix();
			auto NormalMat = glm::transpose(glm::inverse(MVP));
			auto Color = vec4{1.f, 1.f, 1.f, 1.f};
			auto TextureSampler = 0;
			auto bSolidColor = true;

			gl::UniformMatrix4fv(MVPLoc, 1, false, glm::value_ptr(MVP));
			gl::UniformMatrix4fv(NormalMatLoc, 1, false, glm::value_ptr(NormalMat));
			gl::Uniform4f(ColorLoc, Color.r, Color.g, Color.b, Color.a);
			gl::Uniform1i(TextureLoc, TextureSampler);
			gl::Uniform1i(bSolidColorLoc, bSolidColor);
			
			gl::ActiveTexture(gl::TEXTURE0 + TextureSampler);
			gl::BindTexture(gl::TEXTURE_2D, TriangleTexture.ID);
			
			gl::BindVertexArray(Cone.VAO);
			Cone.Draw();

			gl::BindTexture(gl::TEXTURE_2D, 0);
		}

		{
			// Draw Cube
			auto Transform = transform{ vec3{ 1.f, .5f, 0.f }, vec3{1}, CubeRotation };
			auto MVP = Camera.ViewProjection() * Transform.ToMatrix();
			auto NormalMat = glm::transpose(glm::inverse(MVP));
			auto Color = vec4{ 1.f, 1.f, 1.f, 1.f };
			auto TextureSampler = 0;
			auto bSolidColor = true;

			gl::UniformMatrix4fv(MVPLoc, 1, false, glm::value_ptr(MVP));
			gl::UniformMatrix4fv(NormalMatLoc, 1, false, glm::value_ptr(NormalMat));
			gl::Uniform4f(ColorLoc, Color.r, Color.g, Color.b, Color.a);
			gl::Uniform1i(TextureLoc, TextureSampler);
			gl::Uniform1i(bSolidColorLoc, bSolidColor);

			gl::ActiveTexture(gl::TEXTURE0 + TextureSampler);
			gl::BindTexture(gl::TEXTURE_2D, CubeTexture.ID);

			gl::BindVertexArray(Cube.VAO);
			Cube.Draw();

			gl::BindTexture(gl::TEXTURE_2D, 0);
		}

		
		{
			// Draw transform widget
			std::array<transform, 3> Model = {transform{}, transform{}, transform{}};
			Model[0].Position = vec3{ 0.f, 2.f, 0.f };
			Model[1].Position = vec3{ 0.f, 2.f, 0.f };
			Model[2].Position = vec3{ 0.f, 2.f, 0.f };

			Model[0].Rotation = quat{};
			Model[1].Rotation = glm::rotate(mat4{}, glm::radians(90.f), vec3{ 0, 0, 1 });
			Model[2].Rotation = glm::rotate(mat4{}, glm::radians(-90.f), vec3{ 0, 1, 0 });

			Model[0].Scale = TransformScale;
			Model[1].Scale = TransformScale;
			Model[2].Scale = TransformScale;

			std::array<vec3, 3> Colors = { vec3{1.f, 0.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec3{0.f, 0.f, 1.f} };

			gl::BindVertexArray(Arrow.VAO);
			for (int i = 0; i < 3; ++i) {
				auto MVP = Camera.ViewProjection() * Model[i].ToMatrix();
				auto NormalMat = glm::transpose(glm::inverse(MVP));
				auto Color = vec4{ Colors[i], 1.f };
				auto bSolidColor = false;

				gl::UniformMatrix4fv(MVPLoc, 1, false, glm::value_ptr(MVP));
				gl::UniformMatrix4fv(NormalMatLoc, 1, false, glm::value_ptr(NormalMat));
				gl::Uniform4f(ColorLoc, Color.r, Color.g, Color.b, Color.a);
				gl::Uniform1i(bSolidColorLoc, bSolidColor);

				Arrow.Draw();
			}
#if 0
			gl::UseProgram(NormalsRenderProg.ID);
			gl::Uniform3f(ColorLoc, 1.f, 1.f, 0.f);
			for (int i = 0; i < 3; ++i) {
				auto MVP = Camera.ViewProjection() * Rotations[i];
				gl::UniformMatrix4fv(MVPLoc, 1, false, glm::value_ptr(MVP));

				auto NormalMat = glm::transpose(glm::inverse(MVP));
				gl::UniformMatrix4fv(NormalMatLoc, 1, false, glm::value_ptr(NormalMat));

				Arrow.Draw();
			}
#endif
		}

#if 1
		{
			//Render skybox
			gl::CullFace(gl::FRONT);
			defer{ gl::CullFace(gl::BACK); };

			gl::DepthFunc(gl::LEQUAL);	// We have to use LEQUAL because the depth buffer starts filled with 1.0, the max value
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
#endif

#if 0
    	// Draw to screen
		{
			gl::Disable(gl::DEPTH_TEST);
			// Go back to screen framebuffer
			gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
			gl::ClearColor(0.f, 0.f, 1.f, 1.f);
			gl::Clear(gl::COLOR_BUFFER_BIT);

            // Enable texture unit
			const uint PostTextureUnit = 0;
			gl::ActiveTexture(gl::TEXTURE0 + PostTextureUnit);
			gl::BindTexture(gl::TEXTURE_2D, IntermediateFramebuffer.ColorTexture);

			gl::UseProgram(ScreenRenderProg.ID);

            // Pass uniforms
			auto PostTextureLoc = gl::GetUniformLocation(ScreenRenderProg.ID, "Texture");
			gl::Uniform1i(PostTextureLoc, PostTextureUnit);

			auto TimeLoc = gl::GetUniformLocation(ScreenRenderProg.ID, "Time");
			gl::Uniform1f(TimeLoc, TimeSinceStart);

			auto ResolutionLoc = gl::GetUniformLocation(ScreenRenderProg.ID, "Resolution");
			gl::Uniform2f(ResolutionLoc, ScreenDimension.x, ScreenDimension.y);

            // Draw
			gl::BindVertexArray(ScreenQuad.VAO);
			ScreenQuad.Draw();
		}
#endif

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