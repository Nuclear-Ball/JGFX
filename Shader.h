#pragma once
#include "LowLevel/Shader.h"

namespace JGFX{
	constexpr LLJGFX::ShaderProgram::Handle NULL_PRG = { nullptr };

	class ShaderProgram {
	private:
		LLJGFX::ShaderProgram::Handle handle_ = { nullptr };
		void BeforeModification() { if(handle_.data == nullptr) handle_ = LLJGFX::ShaderProgram::Make(); }
	public:
		inline LLJGFX::ShaderProgram::Handle handle() const { return handle_; }

		void Compile(const std::string& vertex_src, const std::string& fragment_src) {
			BeforeModification();
			LLJGFX::ShaderProgram::Compile(handle_, vertex_src, fragment_src);
		}

		inline void Bind() const { LLJGFX::ShaderProgram::Bind(handle_); }

		template <typename T> inline ShaderProgram& SetUniform(const std::string& uniform, const T& value) const {
			const_cast<ShaderProgram&>(*this).BeforeModification();
			LLJGFX::ShaderProgram::SetUniform(handle_, uniform, value);
			return const_cast<ShaderProgram&>(*this);
		}
		template <typename T> inline ShaderProgram& SetUniform(const std::string& uniform, const T* value, size_t element_count) const {
			const_cast<ShaderProgram&>(*this).BeforeModification();
			LLJGFX::ShaderProgram::SetUniform(handle_, uniform, value, element_count);
			return const_cast<ShaderProgram&>(*this);
		}
		template <typename T> inline ShaderProgram& SetUniform(const std::string& uniform, const std::vector<T>& value) const {
			const_cast<ShaderProgram&>(*this).BeforeModification();
			LLJGFX::ShaderProgram::SetUniform(handle_, uniform, value);
			return const_cast<ShaderProgram&>(*this);
		}

		inline ShaderProgram() = default;
		inline ShaderProgram(const std::string& vertex, const std::string& fragment)
			{ Compile(vertex, fragment); }

		inline ShaderProgram& operator=(const ShaderProgram& cpy) {
			Compile(LLJGFX::ShaderProgram::GetAttachedVertexShader(cpy.handle_),
			        LLJGFX::ShaderProgram::GetAttachedFragmentShader(cpy.handle_));
			return *this;
		}
		inline ShaderProgram(const ShaderProgram& cpy) { operator=(cpy); }

		inline ShaderProgram& operator=(ShaderProgram&& cpy) noexcept {
			this->~ShaderProgram();
			std::swap(handle_, cpy.handle_);
			return *this;
		}
		inline ShaderProgram(ShaderProgram&& cpy) noexcept { operator=(std::move(cpy)); }

		inline ~ShaderProgram()
			{ LLJGFX::ShaderProgram::Delete(handle_); handle_ = { nullptr }; }
	};

	namespace Internal{
		struct ShHandleArgumentWrapper : public LLJGFX::ShaderProgram::Handle {
			//LLJGFX::ShaderProgram::Handle handle = { nullptr };
			ShHandleArgumentWrapper() = default;
			ShHandleArgumentWrapper(LLJGFX::ShaderProgram::Handle sh) { data = sh.data; }
			ShHandleArgumentWrapper(const ShaderProgram& sh) { data = sh.handle().data; }
		};
	}
}