#pragma once

#include <common.hpp>
#include <gl_33.hpp>

// a color, depth and stecil framebuffer
struct default_framebuffer {
	static constexpr uint INVALID_ID = (uint)-1;

	uint ID;
	uint ColorTexture;
	uint DepthStencilBuffer;

	glm::ivec2 Size;

	default_framebuffer() = delete;
	explicit default_framebuffer(glm::ivec2 Size);
	~default_framebuffer();
};

inline default_framebuffer::default_framebuffer(glm::ivec2 Size)
		: ID{INVALID_ID}
		, ColorTexture{INVALID_ID}
		, DepthStencilBuffer{INVALID_ID}
		, Size{Size} {

	gl::GenFramebuffers(1, &ID);
	gl::BindFramebuffer(gl::FRAMEBUFFER, ID);
//	defer{ gl::BindFramebuffer(gl::FRAMEBUFFER, 0); };

	gl::GenTextures(1, &ColorTexture);
	gl::BindTexture(gl::TEXTURE_2D, ColorTexture);
//	defer{ gl::BindTexture(gl::TEXTURE_2D, 0); };

	gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGB16F, Size.x, Size.y, 0, gl::RGB, gl::UNSIGNED_BYTE, nullptr);
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
	gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0, gl::TEXTURE_2D, ColorTexture, 0);

	gl::GenRenderbuffers(1, &DepthStencilBuffer);
	gl::BindRenderbuffer(gl::RENDERBUFFER, DepthStencilBuffer);
//	defer{ gl::BindRenderbuffer(gl::RENDERBUFFER, 0); };
	gl::RenderbufferStorage(gl::RENDERBUFFER, gl::DEPTH24_STENCIL8, Size.x, Size.y);

	if (gl::CheckFramebufferStatus(gl::FRAMEBUFFER) != gl::FRAMEBUFFER_COMPLETE) {
		Assert(!"Fullscreen framebuffer is not Complete");
	}
}

inline default_framebuffer::~default_framebuffer() {
	if (ID != INVALID_ID) { gl::DeleteFramebuffers(1, &ID); }
	if (ColorTexture != INVALID_ID) { gl::DeleteTextures(1, &ColorTexture); }
	if (DepthStencilBuffer != INVALID_ID) { gl::DeleteFramebuffers(1, &DepthStencilBuffer); }
}

