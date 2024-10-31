#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "../Mesh.hpp"
#include "Texture.h"

#include "Gpu/Draw.h"

namespace LLJGFX{
	struct TmpTextureSlotBinding{
		int32 slot_id = 0;
		Texture::Handle texture_handle = { nullptr };
	};

	struct TmpRT { //"rt" means "rendering target"
		VRef::Handle vr_handle;
		std::vector<TmpTextureSlotBinding> texture_bindings;
		ShaderProgram::Handle sh_handle = { nullptr };
		size_t instances = 1;
	};

	void Draw(VRef::Handle vr_handle,
			  const std::vector<TmpTextureSlotBinding>& texture_bindings = {},
			  ShaderProgram::Handle sh_handle = {nullptr }, size_t instances = 1){
		Internal::UnbindBuffers();


		const GLenum texture_slot_ids[] = {GL_TEXTURE0,GL_TEXTURE1,GL_TEXTURE2,GL_TEXTURE3,
										   GL_TEXTURE4,GL_TEXTURE5,GL_TEXTURE6,GL_TEXTURE7,
										   GL_TEXTURE8,GL_TEXTURE9,GL_TEXTURE10, GL_TEXTURE11,
										   GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15,
										   GL_TEXTURE16, GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19,
										   GL_TEXTURE20, GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23,
										   GL_TEXTURE24, GL_TEXTURE25, GL_TEXTURE26, GL_TEXTURE27,
										   GL_TEXTURE28, GL_TEXTURE29, GL_TEXTURE30, GL_TEXTURE31, };

		//Set textures
		for(const TmpTextureSlotBinding& i : texture_bindings){
#ifndef NDEBUG
			if(i.slot_id >= 32) throw std::runtime_error("Opengl does not support texture slot id, that is >= 32");
#endif
			glActiveTexture(texture_slot_ids[i.slot_id]);
			if(i.texture_handle.data != nullptr)
				glBindTexture(GL_TEXTURE_2D, i.texture_handle.data->texture_gpu_handle);
		}

		if(sh_handle.data != nullptr)
			glUseProgram(sh_handle.data->gpu_handle);

		glBindVertexArray(vr_handle.handle);

		const Internal::VRefDataHolder& dat = Internal::find_vr_data(vr_handle);
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)dat.ibuff_data.size(), GL_UNSIGNED_INT,
		                        nullptr, (GLsizei)instances);

		//Unset textures
		for(const TmpTextureSlotBinding& i : texture_bindings){
			glActiveTexture(texture_slot_ids[i.slot_id]);
			if(i.texture_handle.data != nullptr)
				glBindTexture(GL_TEXTURE_2D, 0);
		}

		glBindVertexArray(0);

		if(sh_handle.data != nullptr)
			Internal::RebindBoundShader();
	}
	inline void Draw(const TmpRT& target)
		{ Draw(target.vr_handle, target.texture_bindings, target.sh_handle, target.instances); }

	inline void DrawTo(Texture::Handle fb_handle,
					   VRef::Handle vr_handle, const std::vector<TmpTextureSlotBinding>& texture_bindings = {},
					   ShaderProgram::Handle sh_handle = { nullptr }, size_t instances = 1){
		Internal::Gpu::BindFramebuffer(Internal::GetTextureFramebufferHandle(fb_handle),
		                               Texture::GetSize(fb_handle));
		Draw(vr_handle, texture_bindings, sh_handle, instances);
		Internal::RebindBoundFramebuffer();
	}
	inline void DrawTo(Texture::Handle fb_handle, const TmpRT& target)
		{ DrawTo(fb_handle, target.vr_handle, target.texture_bindings, target.sh_handle, target.instances); }


	inline void Clear(rgb color) {
		const glm::vec4 c_color = color;
		glClearColor(c_color.r,
		             c_color.g,
		             c_color.b,
		             c_color.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	inline void Clear(uint8 r, uint8 g, uint8 b, uint8 a = 255) { Clear({r, g, b, a}); }
	inline void Clear(uint32 color) { Clear(rgb{color});}

	namespace Opt{
		inline void Enable(OptFtr feature){
			//enable for all windows
			if(!Internal::all_window_handles.empty()){
				for(Internal::WindowDataHolder* i : Internal::all_window_handles){
					glfwMakeContextCurrent(i->win);
					Internal::Gpu::Enable(feature);
				}
				glfwMakeContextCurrent(Internal::curr_context->win);
			}
			Internal::opt_modes[feature] = true;
		}
		inline void Disable(OptFtr feature){
			//disable for all windows
			if(!Internal::all_window_handles.empty()) {
				for (Internal::WindowDataHolder *i: Internal::all_window_handles) {
					glfwMakeContextCurrent(i->win);
					Internal::Gpu::Disable(feature);
				}
				glfwMakeContextCurrent(Internal::curr_context->win);
			}
			Internal::opt_modes[feature] = false;
		}
		inline bool IsActive(OptFtr feature) { return Internal::opt_modes[feature]; }
	}

	namespace Internal{
		void InitializeDraw(){
			Gpu::Enable = Gpu::Opengl33::Enable;
			Gpu::Disable = Gpu::Opengl33::Disable;
			//enable/disable opt for all windows
			for(int i = 0; i < Opt::STENCIL_TEST; i++){
				if(opt_modes[i]) Gpu::Enable((Opt::OptFtr)i);
				else Gpu::Disable((Opt::OptFtr)i);
			}
		}
	}
}