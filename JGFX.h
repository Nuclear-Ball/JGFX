#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "LowLevel/Window.h"
#include "Shader.h"
#include "Mesh.hpp"
#include "Texture.h"
#include "Window.h"

#include "LowLevel/Draw.h"


bool InitFuncPrimary(){
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");
	return true;
}

bool InitFunc() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Failed to initialize GLAD");

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	LLJGFX::Internal::InitializeTextures();
	LLJGFX::Internal::InitializeShaders();
	LLJGFX::Internal::InitializeDraw();
	return true;
}

namespace JGFX {
	namespace Internal{
		struct VRefHandleWrapper{
			LLJGFX::VRef::Handle handle_ = { LLJGFX::INVALID_HANDLE };
			VRefHandleWrapper() = default;
			VRefHandleWrapper(LLJGFX::VRef::Handle handle) { handle_ = handle; }
			VRefHandleWrapper(const VertexReferencer& vref) { handle_ = vref.handle(); }
			VRefHandleWrapper(const VertexCollection& vcl) { handle_ = vcl.handle(); }
		};

		struct TextureSlotBinding{
			int32 slot_id = 0;
			TxtFnArg txt = {};
		};
	}

	inline void Draw(Internal::VRefHandleWrapper vref,
	                 const std::vector<Internal::TextureSlotBinding>& txt = {},
	                 Internal::ShHandleArgumentWrapper prg = NULL_PRG, size_t instances = 1){

		std::vector<LLJGFX::TmpTextureSlotBinding> tmp(txt.size());
		memcpy(tmp.data(), txt.data(), txt.size() * sizeof(Internal::TextureSlotBinding));
		LLJGFX::Draw(vref.handle_, tmp, prg, instances);
	}
	inline void Draw(Internal::VRefHandleWrapper vref,
	                 Internal::TxtFnArg txt = NULL_TXT,
	                 Internal::ShHandleArgumentWrapper prg = NULL_PRG, size_t instances = 1){
		LLJGFX::Draw(vref.handle_, {{0, txt}}, prg, instances);
	}

	using LLJGFX::Clear;

	namespace Opt{
		using namespace LLJGFX::Opt;
	}
	//inline Texture& Texture::operator<<(const LLJGFX::TemporaryRT& target){
	//	LLJGFX::DrawTo(handle_, target);
	//	return *this;
	//}
}