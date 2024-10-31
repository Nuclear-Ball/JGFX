#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <JSerialUtils.h>

#include "pos2d.h"
#include "Shared.h"

namespace JGFX{
	constexpr unsigned char PIXEL_BINARY_SIZE = 4;

	enum class TxtFiltMode{
		NEAREST = GL_NEAREST,
		LINEAR = GL_LINEAR
	};
	enum class TxtWrapMode{
		REPEAT = GL_REPEAT,
		MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
		CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
	};
}

namespace LLJGFX {
	namespace Texture{
		struct Handle {
			HandleType handle;

			bool operator==(Handle oth) const { return handle == oth.handle; }
		};
	}
	namespace FBuff{
		struct Handle {
			HandleType handle;

			bool operator==(Handle oth) const { return handle == oth.handle; }
		};
	}
}

template<> struct std::hash<LLJGFX::Texture::Handle>{
	size_t operator()(LLJGFX::Texture::Handle handle) const { return handle.handle; }
};
template<> struct std::hash<LLJGFX::FBuff::Handle>{
	size_t operator()(LLJGFX::FBuff::Handle handle) const { return handle.handle; }
};

namespace LLJGFX {
	namespace Internal {
		struct TxtDataHolder {
			pos2du16 size = {0, 0};
			JGFX::TxtFiltMode filtering = JGFX::TxtFiltMode::NEAREST;
			JGFX::TxtWrapMode wrapping = JGFX::TxtWrapMode::REPEAT;
			FBuff::Handle bound_to = {INVALID_HANDLE };
		};
		const TxtDataHolder default_txt = {};

		std::unordered_map<Texture::Handle, TxtDataHolder> texture_data;
		inline TxtDataHolder &find_txt_data(Texture::Handle txt_handle) {
#ifndef NDEBUG
			if (!texture_data.contains(txt_handle))
				throw std::runtime_error("The requested texture does not exists");
#endif
			return texture_data.find(txt_handle)->second;
		}
		inline const TxtDataHolder& find_ro_txt_data(Texture::Handle txt_handle) {//"ro" means "read only"
			if(txt_handle.handle == INVALID_HANDLE)
				return default_txt;
#ifndef NDEBUG
			if (!texture_data.contains(txt_handle))
				throw std::runtime_error("The requested texture does not exists");
#endif
			return texture_data.find(txt_handle)->second;
		}




		struct FbDataHolder{
			Texture::Handle bound_texture = { INVALID_HANDLE };
			HandleType bound_renderbuffer = INVALID_HANDLE;
		};
		const FbDataHolder default_fb = {};

		std::unordered_map<FBuff::Handle, FbDataHolder> framebuffer_data;

		inline FbDataHolder &find_fb_data(FBuff::Handle fb_handle) {
#ifndef NDEBUG
			if (!framebuffer_data.contains(fb_handle))
				throw std::runtime_error("The requested texture does not exists");
#endif
			return framebuffer_data.find(fb_handle)->second;
		}
		inline const FbDataHolder &find_ro_fb_data(FBuff::Handle fb_handle) {
			if(fb_handle.handle == INVALID_HANDLE)
				return default_fb;
#ifndef NDEBUG
			if (!framebuffer_data.contains(fb_handle))
				throw std::runtime_error("The requested texture does not exists");
#endif
			return framebuffer_data.find(fb_handle)->second;
		}

		namespace Gpu{
			HandleType MakeTexture(){
				HandleType handle;
				glGenTextures(1, &handle);
				return handle;
			}
			void DeleteTexture(HandleType txt_handle) { glDeleteTextures(1, &txt_handle);}
			void SetTextureData(HandleType txt_handle, const uint8* data, pos2du16 size){
				glBindTexture(GL_TEXTURE_2D, txt_handle);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0,
				             GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			void GetTextureData(HandleType txt_handle, uint8* dest){
				glBindTexture(GL_TEXTURE_2D, txt_handle);
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, dest);
			}
			void SetTextureFilteringMode(HandleType txt_handle, JGFX::TxtFiltMode mode){
				glBindTexture(GL_TEXTURE_2D, txt_handle);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)mode);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)mode);
			}
			void SetTextureWrapMode(HandleType txt_handle, JGFX::TxtWrapMode mode){
				glBindTexture(GL_TEXTURE_2D, txt_handle);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int) mode);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int) mode);
			}


			HandleType MakeFramebuffer(){
				HandleType handle;
				glGenFramebuffers(1, &handle);
				return handle;
			}
			void DeleteFramebuffer(HandleType fb_handle){ glDeleteFramebuffers(1, &fb_handle); }
			HandleType MakeRenderbuffer(){
				HandleType handle;
				glGenRenderbuffers(1, &handle);
				return handle;
			}
			void DeleteRenderbuffer(HandleType rb_handle){ glDeleteRenderbuffers(1, &rb_handle); }
			void ResizeRenderbuffer(HandleType rb_handle, pos2du16 new_size){
				glBindRenderbuffer(GL_RENDERBUFFER, rb_handle);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, new_size.x, new_size.y);
			}
			void BindRenderbufferToFramebuffer(HandleType fb_handle, HandleType rb_handle){
				glBindFramebuffer(GL_FRAMEBUFFER, fb_handle);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
				                          GL_RENDERBUFFER, rb_handle);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
			void BindTextureToFramebuffer(HandleType fb_handle, HandleType txt_attachment){
				glBindFramebuffer(GL_FRAMEBUFFER, fb_handle);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txt_attachment, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}
	}

	namespace Texture {
		inline void SetData(Handle txt_handle, const uint8 *data, pos2du16 size) {
			Internal::Gpu::SetTextureData(txt_handle.handle, data, size);
			Internal::TxtDataHolder& dat =Internal::find_txt_data(txt_handle);
			dat.size = size;
			if(dat.bound_to.handle != INVALID_HANDLE)
				{ Internal::Gpu::ResizeRenderbuffer(
				Internal::find_fb_data(dat.bound_to).bound_renderbuffer, size); }
		}
		inline void SetData(Handle txt_handle, const std::string &data, pos2du16 size)
			{ SetData(txt_handle, (const uint8 *) data.data(), size); }
		inline void LoadCompressed(Handle txt_handle, const std::string& compressed_string){
			pos2d32 size_adapter = {0, 0 };
			uint8* converted = stbi_load_from_memory((const uint8*)compressed_string.data(),
			                                         compressed_string.size(), &size_adapter.x, &size_adapter.y,
			                                         nullptr, JGFX::PIXEL_BINARY_SIZE);
			SetData(txt_handle, converted, size_adapter);
			stbi_image_free(converted);
		}
		inline void LoadFromFile(Handle txt_handle, const std::string& path)
			{ LoadCompressed(txt_handle, JSerial::Utilities::ReadFile(path)); }
		inline std::string GetData(Handle txt_handle) {
			if(txt_handle.handle == INVALID_HANDLE) return "";
			const pos2du16 size = Internal::find_ro_txt_data(txt_handle).size;
			std::string res(size.x * size.y * JGFX::PIXEL_BINARY_SIZE, 0);
			Internal::Gpu::GetTextureData(txt_handle.handle, (uint8*)res.data());
			return res;
		}

		inline pos2du16 GetSize(Handle txt_handle)
			{ return Internal::find_ro_txt_data(txt_handle).size; }

		inline FBuff::Handle BoundTo(Handle txt_handle)
			{ return Internal::find_ro_txt_data(txt_handle).bound_to; }

		inline void SetFilteringMode(Handle txt_handle, JGFX::TxtFiltMode mode) {
			Internal::find_txt_data(txt_handle).filtering = mode;
			Internal::Gpu::SetTextureFilteringMode(txt_handle.handle, mode);
		}
		inline JGFX::TxtFiltMode GetFilteringMode(Handle txt_handle)
			{ return Internal::find_ro_txt_data(txt_handle).filtering; }

		inline void SetWrapMode(Handle txt_handle, JGFX::TxtWrapMode mode) {
			Internal::find_txt_data(txt_handle).wrapping = mode;
			Internal::Gpu::SetTextureWrapMode(txt_handle.handle, mode);
		}
		inline JGFX::TxtWrapMode GetWrapMode(Handle txt_handle)
			{ return Internal::find_ro_txt_data(txt_handle).wrapping; }

		inline void SetModes(Handle txt_handle, JGFX::TxtFiltMode filtering = JGFX::TxtFiltMode::NEAREST,
							                    JGFX::TxtWrapMode wrapping = JGFX::TxtWrapMode::REPEAT){
			SetFilteringMode(txt_handle, filtering);
			SetWrapMode(txt_handle, wrapping);
		}

		inline Handle Make() {
			Handle handle = {Internal::Gpu::MakeTexture() };
			Internal::texture_data[handle];//Create an object in the hash table
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
			if (txt_handle.handle != INVALID_HANDLE) {
				Internal::TxtDataHolder& txt_data = Internal::find_txt_data(txt_handle);

				//actions with a framebuffer
				if(txt_data.bound_to.handle != INVALID_HANDLE){
					Internal::FbDataHolder& fb_data = Internal::find_fb_data(txt_data.bound_to);
					Internal::Gpu::BindTextureToFramebuffer(txt_data.bound_to.handle, INVALID_HANDLE);
					fb_data.bound_texture = { INVALID_HANDLE };
				}
				Internal::Gpu::DeleteTexture(txt_handle.handle);
				Internal::texture_data.erase(txt_handle);
			}
		}
	}
	namespace FBuff {
		inline void Resize(Handle fb_handle, pos2du16 new_size){
			Internal::FbDataHolder& fb_data = Internal::find_fb_data(fb_handle);
			Texture::SetData(fb_data.bound_texture,
							 std::string(new_size.x * new_size.y * JGFX::PIXEL_BINARY_SIZE, (char)0), new_size);
		}
		inline pos2du16 GetSize(Handle fb_handle) {
			const Internal::FbDataHolder& fb_data = Internal::find_ro_fb_data(fb_handle);
			return Internal::find_txt_data(fb_data.bound_texture).size;
		}

		inline void AttachOutputTexture(Handle fb_handle, Texture::Handle txt_attachment){
			//Previously attached texture will be overridden
			Internal::FbDataHolder& fb_data = Internal::find_fb_data(fb_handle);

			//Attach on gpu
			Internal::Gpu::BindTextureToFramebuffer(fb_handle.handle, txt_attachment.handle);
			const pos2du16 txt_size = Texture::GetSize(txt_attachment);
			Internal::Gpu::ResizeRenderbuffer(fb_data.bound_renderbuffer, txt_size);


			//Add references to both objects
			if(fb_data.bound_texture.handle != INVALID_HANDLE) //remove the old one
				Internal::find_txt_data(fb_data.bound_texture).bound_to = { INVALID_HANDLE };
			if(txt_attachment.handle != INVALID_HANDLE)
				Internal::find_txt_data(txt_attachment).bound_to = fb_handle;

			fb_data.bound_texture = txt_attachment;
		}
		inline Texture::Handle CreateAndAttachOutputTexture(Handle fb_handle, pos2du16 size){
			const Texture::Handle txt_handle = Texture::Make();
			AttachOutputTexture(fb_handle, txt_handle);
			Resize(fb_handle, size);
			return txt_handle;
		}
		inline void RemoveTextureAttachment(Handle fb_handle)
			{ AttachOutputTexture(fb_handle, {INVALID_HANDLE}); }
		inline Texture::Handle GetOutputTexture(Handle fb_handle)
			{ return Internal::find_ro_fb_data(fb_handle).bound_texture; }

		inline Handle Make(){
			Handle handle = { Internal::Gpu::MakeFramebuffer() };
			Internal::FbDataHolder& fb_data = Internal::framebuffer_data[handle];

			fb_data.bound_renderbuffer = Internal::Gpu::MakeRenderbuffer();
			Internal::Gpu::ResizeRenderbuffer(fb_data.bound_renderbuffer, {0, 0});
			Internal::Gpu::BindRenderbufferToFramebuffer(handle.handle, fb_data.bound_renderbuffer);

			return handle;
		}
		inline Handle Make(Texture::Handle txt_attachment){
			Handle handle = Make();
			AttachOutputTexture(handle, txt_attachment);
			return handle;
		}
		inline Handle Make(pos2du16 size){
			Handle handle = Make();
			CreateAndAttachOutputTexture(handle, size);
			return handle;
		}
		inline void Delete(FBuff::Handle fb_handle){
			if(fb_handle.handle != INVALID_HANDLE){
				Internal::FbDataHolder& fb_data = Internal::find_fb_data(fb_handle);

				//Deallocate the attached texture
				RemoveTextureAttachment(fb_handle);

				Internal::Gpu::DeleteRenderbuffer(fb_data.bound_renderbuffer);
				Internal::Gpu::DeleteFramebuffer(fb_handle.handle);
				Internal::framebuffer_data.erase(fb_handle);
			}
		}
	}
}
namespace JGFX {
	class Texture {
	protected:
		LLJGFX::Texture::Handle handle_ = {LLJGFX::INVALID_HANDLE};
		virtual void BeforeModification() {
			if(handle_.handle == LLJGFX::INVALID_HANDLE)
				handle_ = LLJGFX::Texture::Make();
		}
	public:
		inline Texture &SetData(const uint8 *raw, pos2du16 src_size) {
			BeforeModification();
			LLJGFX::Texture::SetData(handle_, raw, src_size);
			return *this;
		}
		inline Texture &SetData(const std::string &raw, pos2du16 src_size) {
			BeforeModification();
			LLJGFX::Texture::SetData(handle_, raw, src_size);
			return *this;
		}
		inline Texture &LoadCompressed(const std::string &compr_src) {
			this->BeforeModification();
			LLJGFX::Texture::LoadCompressed(handle_, compr_src);
			return *this;
		}
		inline Texture &LoadFile(const std::string &path) {
			BeforeModification();
			LLJGFX::Texture::LoadFromFile(handle_, path);
			return *this;
		}

		inline Texture &SetFilteringMode(TxtFiltMode mode) {
			BeforeModification();
			LLJGFX::Texture::SetFilteringMode(handle_, mode);
			return *this;
		}
		inline Texture &SetWrapMode(TxtWrapMode mode) {
			BeforeModification();
			LLJGFX::Texture::SetWrapMode(handle_, mode);
			return *this;
		}
		inline Texture &SetModes(TxtFiltMode filtering, TxtWrapMode wrap) {
			BeforeModification();
			LLJGFX::Texture::SetModes(handle_, filtering, wrap);
			return *this;
		}

		inline std::string Dump() const { return LLJGFX::Texture::GetData(handle_); }
		inline pos2du16 size() const { return LLJGFX::Texture::GetSize(handle_); }
		inline LLJGFX::Texture::Handle handle() const { return handle_; }
		inline TxtFiltMode filtering() const { return LLJGFX::Texture::GetFilteringMode(handle_); }
		inline TxtWrapMode wrapping() const { return LLJGFX::Texture::GetWrapMode(handle_); }

		inline Texture() = default;
		inline Texture(const uint8* raw, pos2du16 size, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		               TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			SetData(raw, size);
			SetModes(filtering, wrap);
		}
		inline Texture(const std::string &raw, pos2du16 size, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		               TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			SetData(raw, size);
			SetModes(filtering, wrap);
		}
		inline Texture(const std::string &path, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		                                        TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			LoadFile(path);
			SetModes(filtering, wrap);
		}
		inline Texture(const std::string &compressed, bool, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		                                                    TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			LoadCompressed(compressed);
			SetModes(filtering, wrap);
		}

		inline Texture(LLJGFX::Texture::Handle handle) { handle_ = handle; }
		inline Texture &operator=(LLJGFX::Texture::Handle handle) {
			this->~Texture();
			handle_ = handle;
			return *this;
		}

		inline Texture &operator=(const Texture &cpy) {
			SetData(cpy.Dump(), cpy.size());
			SetModes(cpy.filtering(), cpy.wrapping());
			return *this;
		}
		inline Texture(const Texture &cpy) { operator=(cpy); }

		inline Texture &operator=(Texture &&cpy) noexcept {
			this->~Texture();
			std::swap(handle_, cpy.handle_);
			return *this;
		}
		inline Texture(Texture &&cpy) noexcept { operator=(cpy); }

		inline ~Texture() {
			LLJGFX::Texture::Delete(handle_);
			handle_ = {LLJGFX::INVALID_HANDLE};
		}
	};


	class Framebuffer : public Texture {
	protected:
		LLJGFX::FBuff::Handle fb_handle_ = { LLJGFX::INVALID_HANDLE };

		inline void BeforeModification() {
			Texture::BeforeModification();
			if(fb_handle_.handle == LLJGFX::INVALID_HANDLE)
				fb_handle_ = LLJGFX::FBuff::Make(handle_);
		}
	public:
		inline LLJGFX::FBuff::Handle fb_handle() const { return fb_handle_; }
		inline Framebuffer& Resize(pos2du16 new_size){
			BeforeModification();
			LLJGFX::FBuff::Resize(fb_handle_, new_size);
			return *this;
		}

		inline Framebuffer() = default;
		inline Framebuffer(const uint8* raw, pos2du16 size, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		                   TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			SetData(raw, size);
			SetModes(filtering, wrap);
		}
		inline Framebuffer(const std::string &raw, pos2du16 size, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		                   TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			SetData(raw, size);
			SetModes(filtering, wrap);
		}
		inline Framebuffer(const std::string &path, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		               TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			LoadFile(path);
			SetModes(filtering, wrap);
		}
		inline Framebuffer(const std::string &compressed, bool, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		               TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			LoadCompressed(compressed);
			SetModes(filtering, wrap);
		}

		inline Framebuffer(LLJGFX::Texture::Handle handle) { handle_ = handle; }
		inline Framebuffer &operator=(LLJGFX::Texture::Handle handle) {
			LLJGFX::Texture::Delete(handle_);
			handle_ = handle;
			LLJGFX::FBuff::AttachOutputTexture(fb_handle_, handle_);
			return *this;
		}
		inline Framebuffer(pos2du16 size, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		                   TxtWrapMode wrap = TxtWrapMode::REPEAT){
			Resize(size);
			SetModes(filtering, wrap);
		}

		Framebuffer& operator=(const Framebuffer& cpy)
			{ operator=(dynamic_cast<const Texture&>(cpy)); return *this; }//Copy txt and fb
		Framebuffer& operator=(const Texture& cpy) { Texture::operator=(cpy); return *this; }
		inline Framebuffer(const Framebuffer& cpy) { operator=(cpy); }
		inline Framebuffer(const Texture& cpy) { operator=(cpy); }

		Framebuffer& operator=(Framebuffer&& cpy) noexcept {
			this->~Framebuffer();
			std::swap(fb_handle_, cpy.fb_handle_);
			operator=(std::move(dynamic_cast<Texture&>(cpy)));
			return *this;
		}//Move both txt and fb
		Framebuffer& operator=(Texture&& cpy) noexcept {
			Texture::operator=(cpy);
			LLJGFX::FBuff::AttachOutputTexture(fb_handle_, handle_);
		}
		Framebuffer(Framebuffer&& cpy) noexcept { operator=(std::move(cpy)); }
		Framebuffer(Texture&& cpy) noexcept { operator=(std::move(cpy)); }

		~Framebuffer() {
			Texture::~Texture();
			LLJGFX::FBuff::Delete(fb_handle_);
			fb_handle_ = {LLJGFX::INVALID_HANDLE};
		}

		//void Bind() const {
		//	glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);
		//	glViewport(0, 0, size_.x, size_.y);
		//}

		//inline glm::dmat4 MakeProjectionMatrix() const
		//{ return glm::ortho<double>(0, size_.x, 0, size_.y); }

		//inline unsigned int get_fbo_handle() { return fbo_handle; }
		//inline unsigned int get_rbo_handle() { return rbo_handle; }

		//void Deserialize(const std::string src) {
		//	BasicTexture::Deserialize(src);
		//	ResizeRBO(size);
		//}
	};

	/*
	class FramebufferOld{
	private:
		class RestrictedTexture : public Texture {
		private:
			inline RestrictedTexture &operator=(LLJGFX::Texture::Handle handle){
				handle_ = handle;
				return *this;
			}
		public:
			RestrictedTexture &operator=(Texture &&cpy) = delete;
			inline ~RestrictedTexture() = default;
		};

		LLJGFX::FBuff::Handle handle_ = { LLJGFX::INVALID_HANDLE };

		bool owns_the_texture = false;
		inline void BeforeModification() { if(handle_.handle == LLJGFX::INVALID_HANDLE) handle_ = LLJGFX::FBuff::Make(); }
		//inline Framebuffer& Init() { handle_ = LLJGFX::FBuff::Make(); return *this;}
	public:
		inline FramebufferOld& Resize(pos2dus new_size) {
			LLJGFX::FBuff::Resize(handle_, new_size);
			return *this;
		}
		inline pos2dus size() const { return LLJGFX::FBuff::GetSize(handle_); }
		inline FramebufferOld& AttachTexture(LLJGFX::Texture::Handle attachment, bool transfer_ownership = false){
			BeforeModification();
			if(owns_the_texture)
				LLJGFX::Texture::Delete(LLJGFX::FBuff::GetOutputTexture(handle_));
			LLJGFX::FBuff::AttachOutputTexture(handle_, attachment);
			owns_the_texture = transfer_ownership;
			return *this;
		}
		inline FramebufferOld& AttachTexture(const Texture& attachment)
			{ AttachTexture(attachment.handle(), false); return *this; }
		inline FramebufferOld& AttachTexture(Texture&& attachment){
			RestrictedTexture tmp{static_cast<RestrictedTexture&&>(attachment)};
			AttachTexture(tmp.handle(), true);
			return *this;
		}
		inline FramebufferOld& CreateAndAttachTexture(pos2dus size){
			BeforeModification();
			if(owns_the_texture)
				LLJGFX::Texture::Delete(LLJGFX::FBuff::GetOutputTexture(handle_));
			LLJGFX::FBuff::CreateAndAttachOutputTexture(handle_, size);
			owns_the_texture = true;
			return *this;
		}
		inline RestrictedTexture attached_texture()
			{ return { LLJGFX::FBuff::GetOutputTexture(handle_) }; }
		inline const RestrictedTexture& attached_texture() const
			{ return { LLJGFX::FBuff::GetOutputTexture(handle_) }; }

		inline Texture&& RetrieveTexture(bool remove = true) {
			Texture tmp = (Texture)attached_texture();

			if(remove)
				LLJGFX::FBuff::RemoveTextureAttachment(handle_);

			owns_the_texture = false;

			return std::move(tmp);
		}
		inline FramebufferOld& RemoveTextureAttachment(){
			BeforeModification();
			LLJGFX::FBuff::RemoveTextureAttachment(handle_);
			return *this;
		}


		inline FramebufferOld() = default;
		inline FramebufferOld(pos2dus size) { CreateAndAttachTexture(size); }
		inline FramebufferOld(LLJGFX::Texture::Handle attachment) { AttachTexture(attachment); }
		inline FramebufferOld(const Texture& attachment) { AttachTexture(attachment); }
		inline FramebufferOld(Texture&& attachment) { AttachTexture(attachment); }

		inline FramebufferOld& operator=(LLJGFX::FBuff::Handle handle) {
			this->FramebufferOld();
			handle_ = handle;
			return *this;
		}
		inline FramebufferOld(LLJGFX::FBuff::Handle handle){ operator=(handle); }

		inline FramebufferOld& operator=(const FramebufferOld& cpy){
			owns_the_texture = cpy.owns_the_texture;
			if (owns_the_texture){
				Texture tmp = (const Texture&)cpy.attached_texture();
				AttachTexture(std::move(tmp));
			}
			else AttachTexture(LLJGFX::FBuff::GetOutputTexture(cpy.handle_));
			return *this;
		}
		inline FramebufferOld(const FramebufferOld& cpy) { operator=(cpy); }

		inline FramebufferOld& operator=(FramebufferOld&& cpy) noexcept {
			this->FramebufferOld();
			std::swap(handle_, cpy.handle_);
			std::swap(owns_the_texture, cpy.owns_the_texture);
			return *this;
		}
		inline FramebufferOld(FramebufferOld&& cpy)  noexcept { operator=(std::move(cpy)); }

		inline ~FramebufferOld(){
			if(owns_the_texture)
				LLJGFX::Texture::Delete(LLJGFX::FBuff::GetOutputTexture(handle_));
			LLJGFX::FBuff::Delete(handle_);

			handle_ = { LLJGFX::INVALID_HANDLE };
			owns_the_texture = false;
		}
	};*/
}/*
	class Framebuffer : public Texture {
	private:
		unsigned int fbo_handle = 0;
		unsigned int rbo_handle = 0;

		inline void ResizeRBO(pos2dus new_size) const {
			glBindRenderbuffer(GL_RENDERBUFFER, rbo_handle);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, new_size.x, new_size.y);
		}

	public:
		void Init(){
			glGenFramebuffers(1, &fbo_handle);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);

			BasicTexture::Init();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle_, 0);

			glGenRenderbuffers(1, &rbo_handle);
			ResizeRBO({0, 0});
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
									  GL_RENDERBUFFER, rbo_handle);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		inline Framebuffer() : BasicTexture(false) { Init(); }
		inline Framebuffer(bool) : BasicTexture(false) {}
		inline Framebuffer(pos2dus size) {
			Init();
			Resize(size);
		}
		inline Framebuffer(const std::string& raw, pos2dus size) : BasicTexture(false) {
			Init();
			LoadRaw(raw, size);
		}
		inline Framebuffer(const std::string& compr_src) : BasicTexture(false) {
			Init();
			LoadCompressed(compr_src);
		}
		inline Framebuffer(const std::string& path, bool) : BasicTexture(false) {
			Init();
			LoadFile(path);
		}

		inline Framebuffer(const Framebuffer& cpy) : BasicTexture(false) {
			Init();
			this->LoadRaw(cpy.Dump(), cpy.size());
		}
		inline Framebuffer(const BasicTexture& cpy) : BasicTexture(false) {
			Init();
			this->LoadRaw(cpy.Dump(), cpy.size());
		}
		Framebuffer& operator=(const Framebuffer& cpy) {
			LoadRaw(cpy.Dump(), cpy.size());
			return *this;
		}
		Framebuffer& operator=(const BasicTexture& cpy) {
			LoadRaw(cpy.Dump(), cpy.size());
			return *this;
		}

		Framebuffer(Framebuffer&& cpy) noexcept : BasicTexture(false) {
			handle_ = cpy.handle_;
			size_ = cpy.size_;

			rbo_handle = cpy.rbo_handle;
			fbo_handle = cpy.fbo_handle;
		}
		Framebuffer& operator=(Framebuffer&& cpy) noexcept {
			std::swap(handle_, cpy.handle_);
			size_ = cpy.size_;
			std::swap(rbo_handle, cpy.rbo_handle);
			std::swap(fbo_handle, cpy.fbo_handle);
		}

		operator BasicTexture() { return *dynamic_cast<BasicTexture*>(this); }

		~Framebuffer() {
			glDeleteRenderbuffers(1, &rbo_handle);
			glDeleteFramebuffers(1, &fbo_handle);
		}

		void LoadRaw(const std::string& raw, pos2dus size) {
			BasicTexture::LoadRaw(raw, size);
			ResizeRBO(this->size_);
		} void LoadCompressed(const std::string& compr_src) {
			BasicTexture::LoadCompressed(compr_src);
			ResizeRBO(this->size_);
		} void LoadFile(const std::string& path) {
			BasicTexture::LoadFile(path);
			ResizeRBO(this->size_);
		}

		void Resize(pos2dus new_size){
			size_ = new_size;
			ResizeRBO(new_size);

			glBindTexture(GL_TEXTURE_2D, handle_);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_size.x, new_size.y, 0,
						 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		void Bind() const {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);
			glViewport(0, 0, size_.x, size_.y);
		}

		inline glm::dmat4 MakeProjectionMatrix() const
			{ return glm::ortho<double>(0, size_.x, 0, size_.y); }

		inline unsigned int get_fbo_handle() { return fbo_handle; }
		inline unsigned int get_rbo_handle() { return rbo_handle; }

		//void Deserialize(const std::string src) {
		//	BasicTexture::Deserialize(src);
		//	ResizeRBO(size);
		//}
	};


	namespace TextureProcessing {
		struct Cropper {
			pos2dus pos, size;
		};

		std::vector<Cropper> GenerateCroplist(pos2dus source_size, int n, pos2dus element_size = {16, 16 },
											   pos2dus start_pos = {0, 0 },
											   int horisontal_spacing = 0, int vertical_spacing = 0) {

			pos2dus elem_pos = start_pos;
			std::vector<Cropper> croplist;
			croplist.reserve(n);

			for (int i = 0; i < n; i++) {
				if ((elem_pos.x + element_size.x) <= source_size.x &&
					(elem_pos.y + element_size.y) <= source_size.y) //Если все нормально

					croplist.push_back({ elem_pos, element_size });

				if ((elem_pos.x + element_size.x + horisontal_spacing + element_size.x <= source_size.x) && //Выбрать место для размещенияь
					(elem_pos.y + element_size.y <= source_size.y))
						elem_pos.x += (element_size.x + horisontal_spacing);
				else if ((elem_pos.x + element_size.x + horisontal_spacing + element_size.x > source_size.x) && //Если нет места в строке
						 (elem_pos.y + element_size.y + vertical_spacing + element_size.y <= source_size.y) &&
						 (start_pos.x + element_size.x <= source_size.x))
					elem_pos.x = start_pos.x,
					elem_pos.y += (element_size.y + vertical_spacing);
				else break;

			}
			return croplist;
		}

		std::vector<std::string> CroplistCrop(const std::string& src_texture, pos2dus size, const std::vector<Cropper>& croplist) {
			std::vector<std::string> res;
			res.reserve(croplist.size());

			for (const Cropper& i : croplist) {
				std::string cropped_elem(i.size.x * i.size.y * PIXEL_BINARY_SIZE, 0);

				for (size_t row = 0; row < i.size.y; row++) {
					const size_t src_elem_row_begin_loc = ((i.pos.y + row) * size.x + i.pos.x) * PIXEL_BINARY_SIZE;
					const size_t output_elem_row_begin_loc = (i.size.x * row) * PIXEL_BINARY_SIZE;

					memcpy(cropped_elem.data() + output_elem_row_begin_loc,
						   src_texture.data() + src_elem_row_begin_loc,
						   i.size.x * PIXEL_BINARY_SIZE);
				}
				res.push_back(std::move(cropped_elem));
			}

			return res;
		}

		inline std::vector<std::string> CroplistCrop(const BasicTexture& src, const std::vector<Cropper>& croplist)
			{ return CroplistCrop(src.Dump(), src.size(), croplist); }
	}*/
//}