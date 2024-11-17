#pragma once
#include <fstream>
#include <unordered_set>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#include <JSerialUtils.h>

#include "Gpu/Texture.h"

namespace LLJGFX {
	namespace Internal {
		struct TxtDataHolder {
			HandleType texture_gpu_handle = INVALID_HANDLE;
			pos2du16 size = {0, 0};
			JGFX::TxtFiltMode filtering = JGFX::TxtFiltMode::NEAREST;
			JGFX::TxtWrapMode wrapping = JGFX::TxtWrapMode::REPEAT;
			std::unordered_map<WindowDataHolder*, HandleType> context_framebuffers;
			//HandleType bound_framebuffer = INVALID_HANDLE;
			HandleType bound_renderbuffer = INVALID_HANDLE;
		};
		const TxtDataHolder default_txt = {};

		std::unordered_set<TxtDataHolder*> all_txt_handles;

		void InitializeTextures(){
			Gpu::MakeTexture = Gpu::Opengl33::MakeTexture;
			Gpu::DeleteTexture = Gpu::Opengl33::DeleteTexture;
			Gpu::SetTextureData = Gpu::Opengl33::SetTextureData;
			Gpu::GetTextureData = Gpu::Opengl33::GetTextureData;
			Gpu::SetTextureFilteringMode = Gpu::Opengl33::SetTextureFilteringMode;
			Gpu::SetTextureWrapMode = Gpu::Opengl33::SetTextureWrapMode;

			Gpu::MakeFramebuffer = Gpu::Opengl33::MakeFramebuffer;
			Gpu::DeleteFramebuffer = Gpu::Opengl33::DeleteFramebuffer;
			Gpu::MakeRenderbuffer = Gpu::Opengl33::MakeRenderbuffer;
			Gpu::DeleteRenderbuffer = Gpu::Opengl33::DeleteRenderbuffer;
			Gpu::ResizeRenderbuffer = Gpu::Opengl33::ResizeRenderbuffer;
			Gpu::BindRenderbufferToFramebuffer = Gpu::Opengl33::BindRenderbufferToFramebuffer;
			Gpu::BindTextureToFramebuffer = Gpu::Opengl33::BindTextureToFramebuffer;

			for(TxtDataHolder* i : all_txt_handles){
				const Gpu::PreInit::PI_TextureData& pi_data = Gpu::PreInit::txt_data[i->texture_gpu_handle];

				i->texture_gpu_handle = Gpu::MakeTexture();
				Gpu::SetTextureData(i->texture_gpu_handle, (const uint8*)pi_data.data.data(), i->size);
				Gpu::SetTextureWrapMode(i->texture_gpu_handle, i->wrapping);
				Gpu::SetTextureFilteringMode(i->texture_gpu_handle, i->filtering);
			}
			Gpu::PreInit::txt_data.clear();
			Gpu::PreInit::txt_last_handle = 1;
		}
	}

	namespace Texture {
		struct Handle {
			Internal::TxtDataHolder *data = nullptr;
		};

		bool IsValid(Handle txt_handle) { return Internal::all_txt_handles.contains(txt_handle.data); }
	}

	namespace Internal{
		void CheckTextureValidity(Texture::Handle txt_handle){
			if(!IsValid(txt_handle)){
				throw std::runtime_error(txt_handle.data == nullptr ?
				                         "Non-existent texture was requested using uninitialized handle" :
				                         "Deleted texture was requested");
			}
		}
		inline const TxtDataHolder& get_ro_txt_data(Texture::Handle txt_handle) {//"ro" means "read only"
			if(txt_handle.data == nullptr) return default_txt;
#ifndef NDEBUG
			Internal::CheckTextureValidity(txt_handle);
#endif
			return *txt_handle.data;
		}

		Texture::Handle bound_framebuffer_texture = { nullptr };
	}

	namespace Texture {
		inline void SetData(Handle txt_handle, const uint8 *data, pos2du16 size) {
#ifndef NDEBUG
			Internal::CheckTextureValidity(txt_handle);
#endif
			Internal::TxtDataHolder& dat = *txt_handle.data;
			Internal::Gpu::SetTextureData(dat.texture_gpu_handle, data, size);

			dat.size = size;
			if(dat.bound_renderbuffer != INVALID_HANDLE)
				Internal::Gpu::ResizeRenderbuffer( dat.bound_renderbuffer, size);

			if(txt_handle.data == Internal::bound_framebuffer_texture.data)
				Internal::Gpu::SetViewportSize(size);
		}
		inline void SetData(Handle txt_handle, const std::string &data, pos2du16 size)
			{ SetData(txt_handle, (const uint8 *) data.data(), size); }
		inline void LoadCompressed(Handle txt_handle, const std::string& compressed_string){
			pos2d32 size_adapter = {0, 0 };
			uint8* converted = stbi_load_from_memory((const uint8*)compressed_string.data(),
			                                         (int)compressed_string.size(), &size_adapter.x, &size_adapter.y,
			                                         nullptr, JGFX::PIXEL_BINARY_SIZE);
			SetData(txt_handle, converted, size_adapter);
			stbi_image_free(converted);
		}
		inline void LoadFromFile(Handle txt_handle, const std::string& path){
			pos2d32 size_adapter = { 0, 0 };
			uint8* converted = stbi_load(path.c_str(),
										 &size_adapter.x, &size_adapter.y,
			                             nullptr, JGFX::PIXEL_BINARY_SIZE);

			
			SetData(txt_handle, converted, size_adapter);
			stbi_image_free(converted);
		}
		inline void Resize(Handle txt_handle, pos2du16 new_size){ //for usage as a framebuffer
			Texture::SetData(txt_handle,
			                 std::string(new_size.x * new_size.y * JGFX::PIXEL_BINARY_SIZE, (char)0),
			                 new_size);
		}

		inline void SetFilteringMode(Handle txt_handle, JGFX::TxtFiltMode mode) {
#ifndef NDEBUG
			Internal::CheckTextureValidity(txt_handle);
#endif
			Internal::Gpu::SetTextureFilteringMode(txt_handle.data->texture_gpu_handle, mode);
			txt_handle.data->filtering = mode;
		}
		inline void SetWrapMode(Handle txt_handle, JGFX::TxtWrapMode mode) {
#ifndef NDEBUG
			Internal::CheckTextureValidity(txt_handle);
#endif
			Internal::Gpu::SetTextureWrapMode(txt_handle.data->texture_gpu_handle, mode);
			txt_handle.data->wrapping = mode;
		}
		inline void SetModes(Handle txt_handle, JGFX::TxtFiltMode filtering = JGFX::TxtFiltMode::NEAREST,
							                    JGFX::TxtWrapMode wrapping = JGFX::TxtWrapMode::REPEAT){
			SetFilteringMode(txt_handle, filtering);
			SetWrapMode(txt_handle, wrapping);
		}

		inline JGFX::TxtFiltMode GetFilteringMode(Handle txt_handle) { return Internal::get_ro_txt_data(txt_handle).filtering; }
		inline JGFX::TxtWrapMode GetWrapMode(Handle txt_handle) { return Internal::get_ro_txt_data(txt_handle).wrapping; }
		inline std::string GetData(Handle txt_handle) {
			const Internal::TxtDataHolder& dat = Internal::get_ro_txt_data(txt_handle);
			if(dat.texture_gpu_handle == INVALID_HANDLE) return "";
			const pos2du16& size = dat.size;
			std::string res(size.x * size.y * JGFX::PIXEL_BINARY_SIZE, 0);
			Internal::Gpu::GetTextureData(dat.texture_gpu_handle, (uint8*)res.data());
			return res;
		}
		inline pos2du16 GetSize(Handle txt_handle) { return Internal::get_ro_txt_data(txt_handle).size; }

		inline void Copy(Handle to, Handle from) {
			LLJGFX::Texture::SetData(to, LLJGFX::Texture::GetData(from), LLJGFX::Texture::GetSize(from));
			LLJGFX::Texture::SetModes(to, LLJGFX::Texture::GetFilteringMode(from), LLJGFX::Texture::GetWrapMode(from));
		}

		inline Handle Make() {
			Handle handle = { new Internal::TxtDataHolder{} };
			Internal::all_txt_handles.insert(handle.data);
			Internal::TxtDataHolder& dat = *handle.data;

			dat.texture_gpu_handle = Internal::Gpu::MakeTexture();
			SetFilteringMode(handle, GetFilteringMode(handle));
			SetWrapMode(handle, GetWrapMode(handle));
			return handle;
		}
		inline Handle Make(const uint8 *data, pos2du16 size, JGFX::TxtFiltMode filtering = JGFX::TxtFiltMode::NEAREST,
						                                     JGFX::TxtWrapMode wrapping = JGFX::TxtWrapMode::REPEAT) {
			Handle handle = Make();
			SetModes(handle, filtering, wrapping);
			SetData(handle, data, size);
			return handle;
		}
		inline Handle Make(const std::string &data, pos2du16 size, JGFX::TxtFiltMode filtering = JGFX::TxtFiltMode::NEAREST,
		                                                           JGFX::TxtWrapMode wrapping = JGFX::TxtWrapMode::REPEAT)
			{ return Make((const uint8*) data.data(), size, filtering, wrapping); }
		inline Handle Make(const std::string &path, JGFX::TxtFiltMode filtering = JGFX::TxtFiltMode::NEAREST,
						                            JGFX::TxtWrapMode wrapping = JGFX::TxtWrapMode::REPEAT) {
			Handle handle = Make();
			SetModes(handle, filtering, wrapping);
			LLJGFX::Texture::LoadFromFile(handle, path);
			return handle;
		}

		inline void Delete(Handle txt_handle) {
			if(txt_handle.data == nullptr) return;

#ifndef NDEBUG
			Internal::CheckTextureValidity(txt_handle);
#endif
			Internal::TxtDataHolder& dat = *txt_handle.data;

			//actions with a framebuffer
			if(!dat.context_framebuffers.empty()){
				for(const std::pair<Internal::WindowDataHolder*const, HandleType>& i : dat.context_framebuffers){
					glfwMakeContextCurrent(i.first->win);
					Internal::Gpu::DeleteFramebuffer(i.second);
					i.first->local_framebuffers.erase(txt_handle.data);
					//Also delete reference from the window
				}
				Internal::Gpu::DeleteRenderbuffer(dat.bound_renderbuffer);

				glfwMakeContextCurrent(Internal::curr_context->win);
			}
			Internal::Gpu::DeleteTexture(dat.texture_gpu_handle);

			if(Internal::bound_framebuffer_texture.data == txt_handle.data) Internal::bound_framebuffer_texture = { nullptr };

			Internal::all_txt_handles.erase(txt_handle.data);
			delete txt_handle.data;
		}
	}
	namespace Internal{
		inline HandleType GetTextureFramebufferHandle(Texture::Handle txt_handle){
#ifndef NDEBUG
			Internal::CheckTextureValidity(txt_handle);
#endif
			Internal::TxtDataHolder& dat = *txt_handle.data;
			if(!dat.context_framebuffers.contains(Internal::curr_context)){
				const HandleType& c_fb = (dat.context_framebuffers[Internal::curr_context] = Internal::Gpu::MakeFramebuffer());
				dat.bound_renderbuffer = Internal::Gpu::MakeRenderbuffer();
				Internal::Gpu::ResizeRenderbuffer(dat.bound_renderbuffer, dat.size);
				Internal::Gpu::BindRenderbufferToFramebuffer(c_fb, dat.bound_renderbuffer);
				Internal::Gpu::BindTextureToFramebuffer(c_fb, dat.texture_gpu_handle);
			}
			return dat.context_framebuffers[Internal::curr_context];
		}

		inline void RebindBoundFramebuffer(){
			Gpu::BindFramebuffer(GetTextureFramebufferHandle(bound_framebuffer_texture),
			                               Texture::GetSize(bound_framebuffer_texture));
		}
	}

	inline void BindFramebuffer(Texture::Handle txt_handle){
		Internal::bound_framebuffer_texture = txt_handle;
		Internal::Gpu::BindFramebuffer(Internal::GetTextureFramebufferHandle(txt_handle),
		                               Texture::GetSize(txt_handle));
	}
}