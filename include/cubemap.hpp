#pragma once

#include <common.hpp>
#include <gl_33.hpp>
#include <texture.hpp>

struct cubemap {
	uint ID;

	//@Important This order is precise to match Opengl:: 
	enum class face : uint8 {
		RIGHT = 0, LEFT,
		UP, DOWN,
		BACK, FRONT
	};
};

cubemap MakeCubemap(const char *Path, const char *Extension, bool32 EnablesRGB = true) {
	cubemap Map;
	gl::GenTextures(1, &Map.ID);
	gl::BindTexture(gl::TEXTURE_CUBE_MAP, Map.ID);
	defer{ gl::BindTexture(gl::TEXTURE_CUBE_MAP, 0); };

	gl::TexParameteri(gl::TEXTURE_CUBE_MAP, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
	gl::TexParameteri(gl::TEXTURE_CUBE_MAP, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
	gl::TexParameteri(gl::TEXTURE_CUBE_MAP, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
	gl::TexParameteri(gl::TEXTURE_CUBE_MAP, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);
	gl::TexParameteri(gl::TEXTURE_CUBE_MAP, gl::TEXTURE_WRAP_R, gl::CLAMP_TO_EDGE);

	// @Important This order is precise to match Opengl::
	const char* FaceNames[] = {
		"right", "left",
		"up", "down",
		"back", "front"
	};

	for (size iFace = 0; iFace < ArraySize(FaceNames); ++iFace) {
		char Filename[FilenameMax];
		sprintf(Filename, "%s%s.%s", Path, FaceNames[iFace], Extension);

		auto Image = LoadImageFromFile(Filename);
		defer{ FreeImage(Image); };
		Assert(Image.Data);

		int InternalFormat = EnablesRGB ? gl::SRGB : gl::RGB;

		gl::TexImage2D((GLenum)(gl::TEXTURE_CUBE_MAP_POSITIVE_X + iFace),
			0, InternalFormat, Image.Width, Image.Height, 0, gl::RGB, gl::UNSIGNED_BYTE, Image.Data);
	}

	return Map;
}
