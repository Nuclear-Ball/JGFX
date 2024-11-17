#pragma once
#include "LowLevel/Texture.h"

namespace JGFX {
	constexpr LLJGFX::Texture::Handle NULL_TXT = { nullptr };

	class Texture {
	private:
		LLJGFX::Texture::Handle handle_ = { nullptr };
		void BeforeModification() { if(handle_.data == nullptr) handle_ = LLJGFX::Texture::Make(); }
	public:
		inline Texture& SetData(const uint8 *raw, pos2du16 src_size) {
			BeforeModification();
			LLJGFX::Texture::SetData(handle_, raw, src_size);
			return *this;
		}
		inline Texture& SetData(const std::string &raw, pos2du16 src_size) {
			BeforeModification();
			LLJGFX::Texture::SetData(handle_, raw, src_size);
			return *this;
		}
		inline Texture& LoadCompressed(const std::string &compr_src) {
			this->BeforeModification();
			LLJGFX::Texture::LoadCompressed(handle_, compr_src);
			return *this;
		}
		inline Texture& LoadFile(const std::string &path) {
			BeforeModification();
			LLJGFX::Texture::LoadFromFile(handle_, path);
			return *this;
		}
		inline Texture& Resize(pos2du16 new_size) {
			BeforeModification();
			LLJGFX::Texture::Resize(handle_, new_size);
			return *this;
		}
		inline Texture& Bind() {
			LLJGFX::BindFramebuffer(handle_);
			return *this;
		}
		inline const Texture& Bind() const {
			LLJGFX::BindFramebuffer(handle_);
			return *this;
		}

		inline Texture& SetFilteringMode(TxtFiltMode mode) {
			BeforeModification();
			LLJGFX::Texture::SetFilteringMode(handle_, mode);
			return *this;
		}
		inline Texture& SetWrapMode(TxtWrapMode mode) {
			BeforeModification();
			LLJGFX::Texture::SetWrapMode(handle_, mode);
			return *this;
		}
		inline Texture& SetModes(TxtFiltMode filtering, TxtWrapMode wrap) {
			BeforeModification();
			LLJGFX::Texture::SetModes(handle_, filtering, wrap);
			return *this;
		}

		//inline Texture& operator<<(const LLJGFX::TemporaryRT& target);

		inline std::string Dump() const { return LLJGFX::Texture::GetData(handle_); }
		inline pos2du16 size() const { return LLJGFX::Texture::GetSize(handle_); }
		inline LLJGFX::Texture::Handle handle() const { return handle_; }
		//inline LLJGFX::HandleType framebuffer_handle() const { return LLJGFX::Texture::GetFramebufferHandle(handle_); }
		inline TxtFiltMode filtering() const { return LLJGFX::Texture::GetFilteringMode(handle_); }
		inline TxtWrapMode wrapping() const { return LLJGFX::Texture::GetWrapMode(handle_); }

		inline Texture() = default;
		inline Texture(const uint8 *raw, pos2du16 size, TxtFiltMode filtering = TxtFiltMode::NEAREST,
					                                    TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			SetData(raw, size);
			SetModes(filtering, wrap);
		}
		inline Texture(const std::string &raw, pos2du16 size, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		                                                      TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			SetData(raw, size);
			SetModes(filtering, wrap);
		}
		inline Texture(pos2du16 size, TxtFiltMode filtering = TxtFiltMode::NEAREST,
		                              TxtWrapMode wrap = TxtWrapMode::REPEAT) {
			Resize(size);
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
			handle_ = { nullptr };
		}
	};

	namespace Internal{
		struct TxtFnArg : public LLJGFX::Texture::Handle{
			TxtFnArg() = default;
			TxtFnArg(LLJGFX::Internal::TxtDataHolder* ptr) { data = ptr; }
			TxtFnArg(LLJGFX::Texture::Handle txt) { data = txt.data; }
			TxtFnArg(const Texture& txt) { data = txt.handle().data; }
		};
	}

	inline void BindFramebuffer(Internal::TxtFnArg fb) { LLJGFX::BindFramebuffer(fb); }
}