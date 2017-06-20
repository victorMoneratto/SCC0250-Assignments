#pragma once

#include <common.hpp>
#include <gl_33.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct image {
	uint8* Data;
	int32 Width, Height;
	int32 NumChannels;
};

image LoadImageFromFile(const char* Filename) {
	image Image;
	Image.Data = stbi_load(Filename, &Image.Width, &Image.Height, &Image.NumChannels, 0);
	Assert(Image.Data);

	return Image;
}

void FreeImage(image Image) {
	if (Image.Data) stbi_image_free(Image.Data);
}


struct texture {
	static constexpr uint INVALID_ID = (uint) -1;

	uint ID;
	int Width, Height, NumChannels;
	bool32 SRGB;

	std::string Path;

	texture(std::string Path = "", bool32 SRGB = true) 
		: ID{ (uint)-1 }
		, Width{ 0 }, Height{ 0 }
		, NumChannels{ 0 }
		, Path{ Path }
		, SRGB{ SRGB } {}

	~texture() {
		gl::DeleteTextures(1, &ID); ID = -1;
	}

	bool Load() {
		if (Path.empty()) { return false; }

		// Load Image
		auto Image = LoadImageFromFile(Path.c_str());
		defer{ FreeImage(Image); };

		Width = Image.Width;
		Height = Image.Height;
		NumChannels = Image.NumChannels;

		// Generate Texture
		gl::GenTextures(1, &ID);
		gl::BindTexture(gl::TEXTURE_2D, ID);
		//		defer{ gl::BindTexture(gl::TEXTURE_2D, 0); }; // Unbinding is unnecessary if we guarantee no code uses leaking textures

		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);

		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);

		GLint InternalFormat = 0;
		GLenum Format = 0;
		if(SRGB) {
			switch(NumChannels) {
			case 3: 
				InternalFormat = gl::SRGB8;
				Format = gl::RGB;
				break;
			case 4:
				InternalFormat = gl::SRGB8_ALPHA8;
				Format = gl::RGBA;
				break;
			default: Assert(!"Image with number of channels not yet supported");
			}
		} else {
			switch (NumChannels) {
			case 3:
				InternalFormat = gl::RGB;
				Format = gl::RGB;
				break;
			case 4:
				InternalFormat = gl::RGBA;
				Format = gl::RGBA;
				break;
			default: Assert(!"Image with number of channels not yet supported");
			}
		}

		// As we wont do lighting in this asignment RGBA is acceptable, 
		// but on further assignments we'll have to linearize sRGBA textures
		gl::TexImage2D(gl::TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, gl::UNSIGNED_BYTE, Image.Data);
		gl::GenerateMipmap(gl::TEXTURE_2D);		

		return true;
	}
};

uint MakeBlankTexture() {
	uint ID;
	gl::GenTextures(1, &ID);
	gl::BindTexture(gl::TEXTURE_2D, ID);
	uint8 White[4] = { 255, 255, 255, 255 };
	gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, 1, 1, 0, gl::RGBA, gl::UNSIGNED_BYTE, White);

	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR_MIPMAP_LINEAR);
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);

	return ID;
}