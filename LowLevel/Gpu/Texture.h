#pragma once
#include <unordered_map>
#include <unordered_set>
#include <cstring>

#include "../Common.h"

namespace JGFX{
	constexpr unsigned char PIXEL_BINARY_SIZE = 4;

	enum class TxtFiltMode{
		NEAREST,
		LINEAR
	};
	enum class TxtWrapMode{
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};
}

namespace LLJGFX {
	namespace Internal {
		namespace Gpu{
			//Actual declarations
			namespace Opengl33{
				HandleType MakeTexture(){
					HandleType handle;
					glGenTextures(1, &handle);
					return handle;
				}
				void DeleteTexture(HandleType txt_handle)
					{ glDeleteTextures(1, &txt_handle);}
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
					const GLint modes[] = { GL_NEAREST, GL_LINEAR };
					glBindTexture(GL_TEXTURE_2D, txt_handle);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, modes[(int)mode]);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, modes[(int)mode]);
				}
				void SetTextureWrapMode(HandleType txt_handle, JGFX::TxtWrapMode mode){
					const GLint modes[] = { GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER};
					glBindTexture(GL_TEXTURE_2D, txt_handle);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, modes[(int)mode]);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, modes[(int)mode]);
				}

				HandleType MakeFramebuffer(){
					HandleType handle;
					glGenFramebuffers(1, &handle);
					return handle;
				}
				void DeleteFramebuffer(HandleType fb_handle)
					{ glDeleteFramebuffers(1, &fb_handle); }
				HandleType MakeRenderbuffer(){
					HandleType handle;
					glGenRenderbuffers(1, &handle);
					return handle;
				}
				void DeleteRenderbuffer(HandleType rb_handle) { glDeleteRenderbuffers(1, &rb_handle); }
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
			namespace PreInit{
				struct PI_TextureData{
					std::string data;
				};
				std::unordered_map<HandleType, PI_TextureData> txt_data;
				HandleType txt_last_handle = 1;

				HandleType MakeTexture(){
					txt_data[txt_last_handle];
					return txt_last_handle++;
				}
				void DeleteTexture(HandleType txt_handle) { txt_data.erase(txt_handle); }
				void SetTextureData(HandleType txt_handle, const uint8* data, pos2du16 size) {
					txt_data.find(txt_handle)->second.data =
							{(const char*)data, (size_t)(size.x * size.y * JGFX::PIXEL_BINARY_SIZE)};
				}
				void GetTextureData(HandleType txt_handle, uint8* dest){
					const std::string& data = txt_data.find(txt_handle)->second.data;
					memcpy(dest, data.data(), data.size());
				}
				void SetTextureFilteringMode(HandleType txt_handle, JGFX::TxtFiltMode mode){}
				void SetTextureWrapMode(HandleType txt_handle, JGFX::TxtWrapMode mode){}

				HandleType MakeFramebuffer(){
					return INVALID_HANDLE;
				}
				void DeleteFramebuffer(HandleType fb_handle){}
				HandleType MakeRenderbuffer(){
					return INVALID_HANDLE;
				}
				void DeleteRenderbuffer(HandleType rb_handle) {}
				void ResizeRenderbuffer(HandleType rb_handle, pos2du16 new_size){}
				void BindRenderbufferToFramebuffer(HandleType fb_handle, HandleType rb_handle){}
				void BindTextureToFramebuffer(HandleType fb_handle, HandleType txt_attachment){}
			}

			//Function pointers
			HandleType (*MakeTexture)() = PreInit::MakeTexture;
			void (*DeleteTexture)(HandleType) = PreInit::DeleteTexture;
			void (*SetTextureData)(HandleType, const uint8*, pos2du16) = PreInit::SetTextureData;
			void (*GetTextureData)(HandleType, uint8*) = PreInit::GetTextureData;
			void (*SetTextureFilteringMode)(HandleType, JGFX::TxtFiltMode) = PreInit::SetTextureFilteringMode;
			void (*SetTextureWrapMode)(HandleType, JGFX::TxtWrapMode) = PreInit::SetTextureWrapMode;


			HandleType (*MakeFramebuffer)() = PreInit::MakeFramebuffer;
			void (*DeleteFramebuffer)(HandleType) = PreInit::DeleteFramebuffer;
			HandleType (*MakeRenderbuffer)() = PreInit::MakeRenderbuffer;
			void (*DeleteRenderbuffer)(HandleType) = PreInit::DeleteRenderbuffer;
			void (*ResizeRenderbuffer)(HandleType, pos2du16) = PreInit::ResizeRenderbuffer;
			void (*BindRenderbufferToFramebuffer)(HandleType, HandleType) = PreInit::BindRenderbufferToFramebuffer;
			void (*BindTextureToFramebuffer)(HandleType, HandleType) = PreInit::BindTextureToFramebuffer;


			void BindFramebuffer(int handle, pos2du16 size){
				glBindFramebuffer(GL_FRAMEBUFFER, handle);
				glViewport(0, 0, size.x, size.y);
			}
			void SetViewportSize(pos2du16 size){
				glViewport(0, 0, size.x, size.y);
			}
		}
	}
}