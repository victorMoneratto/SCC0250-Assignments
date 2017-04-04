#pragma once

#include <common.hpp>
#include <gl_33.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct texture {
	static constexpr uint INVALID_ID = (uint) -1;

	uint ID;
	int Width, Height, NumChannels;

	std::string Path;

	texture(std::string Path = "") 
		: ID{ (uint)-1 }, Path{ Path }
		, Width{ 0 }, Height{ 0 }
		, NumChannels{ 0 } {}

	inline bool Load() {
		if (Path.empty()) { return false; }

		// Load Image
		u8* Image = stbi_load(Path.c_str(), &Width, &Height, &NumChannels, 4);
		if (!Image) { return false; }

		defer{ stbi_image_free(Image); };

		// Generate Texture
		gl::GenTextures(1, &ID);
		gl::BindTexture(gl::TEXTURE_2D, ID);
		//		defer{ gl::BindTexture(gl::TEXTURE_2D, 0); }; // Unbinding is unnecessary if we guarantee no code uses leaking textures

		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);

		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);

		// As we wont do lighting in this asignment RGBA is acceptable, 
		// but on further assignments we'll have to linearize sRGBA textures
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, Width, Height, 0, gl::RGBA, gl::UNSIGNED_BYTE, Image);
		gl::GenerateMipmap(gl::TEXTURE_2D);		

		return true;
	}
};