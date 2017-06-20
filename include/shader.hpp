#pragma once

#include <common.hpp>
#include <array>
#include <gl_33.hpp>

namespace shader_stage {
	enum type : uint {
	    Vertex = 0,
		Geometry,
	    Fragment,
	    TOTAL
	};
}
static const uint InternalShaderTypes[] = {gl::VERTEX_SHADER, gl::GEOMETRY_SHADER, gl::FRAGMENT_SHADER};

struct render_program {
    // Invalid value for program and shader handles
    static const uint INVALID_ID = 0xFFFFFFFF;
	uint ID;

    std::array<uint, shader_stage::TOTAL> Shaders;
    std::array<std::string, shader_stage::TOTAL> ShaderPaths;

    render_program();

    ~render_program();

    bool32 LoadShaders();

    void KillShaders();

    void ReloadShaders();
};

inline render_program::render_program() : ID{INVALID_ID}, Shaders{}, ShaderPaths{} {
    for (uint i = 0; i < shader_stage::TOTAL; ++i) {
        Shaders[i] = INVALID_ID;
        ShaderPaths[i] = "";
    }
}

inline render_program::~render_program() {
	KillShaders();
    if (ID != INVALID_ID) gl::DeleteProgram(ID);
}

inline bool32 render_program::LoadShaders() {
    char Log[100];

    if (ID == INVALID_ID) { ID = gl::CreateProgram(); }

    for (uint iStage = 0; iStage < shader_stage::TOTAL; ++iStage) {
        const auto &Path = ShaderPaths[iStage];
        auto &Shader = Shaders[iStage];

        if (!Path.empty()) {
            Shader = gl::CreateShader(InternalShaderTypes[iStage]);

            auto Source = ReadFile(Path);
            auto SourceVar = Source.c_str();
            auto LengthVar = (int) Source.length();
            gl::ShaderSource(Shader, 1, &SourceVar, &LengthVar);

            gl::CompileShader(Shader);

            GLint Success;
            gl::GetShaderiv(Shader, gl::COMPILE_STATUS, &Success);

            if (Success) {
                gl::AttachShader(ID, Shader);
            } else {
                gl::GetShaderInfoLog(Shader, (GLsizei) SizeOf(Log)-1, nullptr, Log);
                fprintf(stderr, "Error compiling shader %s: %s\n", Path.c_str(), Log);
                continue;
            }
        }
    }

    gl::LinkProgram(ID);

    GLint Success;
    gl::GetProgramiv(ID, gl::LINK_STATUS, &Success);
    if (!Success) {
        gl::GetProgramInfoLog(ID, (GLsizei) SizeOf(Log) - 1, nullptr, Log);
        fprintf(stderr, "Error linking shader: %s\n", Log);
    }

    return (bool32) Success;
}

inline void render_program::KillShaders() {
    for(auto& Shader : Shaders) {
        if(Shader != INVALID_ID) {
            gl::DeleteShader(Shader);
            gl::DetachShader(ID, Shader);
            Shader = INVALID_ID;
        }
    }
}

inline void render_program::ReloadShaders() {
    KillShaders();
    LoadShaders();
}
