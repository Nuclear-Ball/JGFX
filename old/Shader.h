#pragma once
#include <unordered_set>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>

#include "Shared.h"
#include "pos2d.h"

namespace JGFX {
	enum class ShaderType {
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER
	};
}
namespace LLJGFX{
	namespace Internal{
		struct ShaderDataHolder{
			std::string source;
			std::unordered_set<HandleType> bound_to;
			JGFX::ShaderType type = JGFX::ShaderType::VERTEX;
		};
		std::unordered_map<HandleType, ShaderDataHolder> shader_data;

		struct ProgramDataHolder {
			HandleType bound_vertex_shader = INVALID_HANDLE;
			HandleType bound_fragment_shader = INVALID_HANDLE;
			bool is_linked = false;
		};
		std::unordered_map<HandleType, ProgramDataHolder> program_data;
	}

	namespace Shader {
		struct Handle{
			HandleType handle;
		};

		inline Handle Make(JGFX::ShaderType type) {
			HandleType handle = glCreateShader((GLint)type);
			Internal::shader_data[handle].type = type;
			return { handle };
		}
		inline void Delete(Handle shader_handle) {
			if(Internal::shader_data[shader_handle.handle].bound_to.empty())
				glDeleteShader(shader_handle.handle);
		}

		void Compile(Handle shader_handle, const std::string &source) {
			const char *tmp = source.c_str();
			const GLint size = source.size();
			glShaderSource(shader_handle.handle, 1, &tmp, &size);
			glCompileShader(shader_handle.handle);
#ifndef NDEBUG
			char deb_log[1024];
			int success = 0;
			glGetShaderiv(shader_handle.handle, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader_handle.handle, 1024, nullptr, deb_log);
				throw std::runtime_error(std::string("Shader compilation failed: " + std::string(deb_log)).c_str());
			}
#endif
			Internal::shader_data[shader_handle.handle].source = source;
		}
		inline Handle Make(JGFX::ShaderType type, const std::string &source) {
			Handle shader_handle = Make(type);
			Compile(shader_handle, source);
			return shader_handle;
		}

		inline JGFX::ShaderType GetType(Handle shader_handle)
			{ return Internal::shader_data.find(shader_handle.handle)->second.type; }
		inline std::string GetSource(Handle shader_handle)
			{ return Internal::shader_data.find(shader_handle.handle)->second.source;}

		inline bool Verify(Handle shader_handle) { return Internal::shader_data.contains(shader_handle.handle); }
	}
	namespace Program{
		struct Handle{ HandleType handle; };

		inline Handle Make() {
			HandleType program_handle = glCreateProgram();
			Internal::program_data[program_handle].is_linked = false;
			return { program_handle };
		}
		inline void Delete(Handle program_handle) {
			glDeleteProgram(program_handle.handle);
			if(Internal::program_data.contains(program_handle.handle)){
				const Internal::ProgramDataHolder& dat = Internal::program_data.find(program_handle.handle)->second;
				if(dat.is_linked){
					Internal::shader_data[dat.bound_vertex_shader].bound_to.erase(program_handle.handle);
					Internal::shader_data[dat.bound_fragment_shader].bound_to.erase(program_handle.handle);
				}
				Internal::program_data.erase(program_handle.handle);
			}
		}

		inline void Link(Handle program_handle, Shader::Handle vertex_shader_handle, Shader::Handle fragment_shader_handle){
			glAttachShader(program_handle.handle, vertex_shader_handle.handle);
			glAttachShader(program_handle.handle, fragment_shader_handle.handle);
			glLinkProgram(program_handle.handle);
#ifndef NDEBUG
			char deb_log[1024];
			int success = 0;
			glGetProgramiv(program_handle.handle, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(program_handle.handle, 1024, nullptr, deb_log);
				throw std::runtime_error(std::string("Shader program linking failed: " + std::string(deb_log)).c_str());
			}
#endif
			Internal::program_data[program_handle.handle].is_linked = true;
			Internal::program_data[program_handle.handle].bound_vertex_shader = vertex_shader_handle.handle;
			Internal::program_data[program_handle.handle].bound_fragment_shader = fragment_shader_handle.handle;

			Internal::shader_data[vertex_shader_handle.handle].bound_to.insert(program_handle.handle);
			Internal::shader_data[fragment_shader_handle.handle].bound_to.insert(program_handle.handle);
		}
		inline Handle Make(Shader::Handle vertex_shader_handle, Shader::Handle fragment_shader_handle) {
			Handle program_handle = Make();
			Link(program_handle, vertex_shader_handle, fragment_shader_handle);
			return program_handle;
		}

		//Will be deprecated in the future
		inline void Bind(Handle program_handle) { glUseProgram(program_handle.handle); }

		inline Shader::Handle GetBoundVertexShader(Handle program_handle)
			{ return {Internal::program_data.find(program_handle.handle)->second.bound_vertex_shader}; }
		inline Shader::Handle GetBoundFragmentShader(Handle program_handle)
			{ return {Internal::program_data.find(program_handle.handle)->second.bound_fragment_shader}; }
	}

	namespace Uniform {
		struct Handle{
			HandleType uniform_handle = INVALID_HANDLE;
			HandleType program_handle = INVALID_HANDLE;
		};
		namespace FuncPtrs {
			template<typename T> using UniformFuncPtr = void (*)(int, int, const T *);

			template<typename T> void UniformFunctionMatrixWrapper(int uniform_location, int count, const T *value)
				{ throw std::runtime_error("Invalid type"); }
			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat2 *value)
				{ glUniformMatrix2fv(uniform_location, count, false, (float *) value); }
			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat2x3 *value)
				{ glUniformMatrix2x3fv(uniform_location, count, false, (float *) value); }
			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat2x4 *value)
				{ glUniformMatrix2x4fv(uniform_location, count, false, (float *) value); }

			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat3 *value)
				{ glUniformMatrix3fv(uniform_location, count, false, (float *) value); }
			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat3x2 *value)
				{ glUniformMatrix3x2fv(uniform_location, count, false, (float *) value); }
			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat3x4 *value)
				{ glUniformMatrix3x4fv(uniform_location, count, false, (float *) value); }

			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat4 *value)
				{ glUniformMatrix4fv(uniform_location, count, false, (float *) value); }
			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat4x2 *value)
				{ glUniformMatrix4x2fv(uniform_location, count, false, (float *) value); }
			template<> void UniformFunctionMatrixWrapper(int uniform_location, int count, const glm::mat4x3 *value)
				{ glUniformMatrix4x3fv(uniform_location, count, false, (float *) value); }


			template<typename T> inline UniformFuncPtr<T> GetUniformFunction() {
				throw std::runtime_error("There is no uniform function for this type");
				//static_assert(false, "There is no uniform function for this type");
				return nullptr;
			}

			template<> inline UniformFuncPtr<uint32> GetUniformFunction()
				{ return glUniform1uiv; }
			template<> inline UniformFuncPtr<int32> GetUniformFunction()
				{ return glUniform1iv; }
			template<> inline UniformFuncPtr<fl32> GetUniformFunction()
				{ return glUniform1fv; }

			template<> inline UniformFuncPtr<glm::vec<2, uint32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<2, uint32, glm::highp>>)glUniform2uiv; }
			template<> inline UniformFuncPtr<glm::vec<2, int32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<2, int32, glm::highp>>)glUniform2iv; }
			template<> inline UniformFuncPtr<glm::vec<2, fl32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<2, fl32, glm::highp>>) glUniform2fv; }

			template<> inline UniformFuncPtr<glm::vec<3, uint32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<3, uint32, glm::highp>>) glUniform3uiv; }
			template<> inline UniformFuncPtr<glm::vec<3, int32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<3, int32, glm::highp>>) glUniform3iv; }
			template<> inline UniformFuncPtr<glm::vec<3, fl32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<3, fl32, glm::highp>>) glUniform3fv; }

			template<> inline UniformFuncPtr<glm::vec<4, uint32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<4, uint32, glm::highp>>) glUniform4uiv; }
			template<> inline UniformFuncPtr<glm::vec<4, int32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<4, int32, glm::highp>>) glUniform4iv; }
			template<> inline UniformFuncPtr<glm::vec<4, fl32, glm::highp>> GetUniformFunction()
				{ return (UniformFuncPtr<glm::vec<4, fl32, glm::highp>>) glUniform4fv; }

			template<> inline UniformFuncPtr<glm::mat2> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat2>; }
			template<> inline UniformFuncPtr<glm::mat2x3> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat2x3>; }
			template<> inline UniformFuncPtr<glm::mat2x4> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat2x4>; }

			template<> inline UniformFuncPtr<glm::mat3> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat3>; }
			template<> inline UniformFuncPtr<glm::mat3x2> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat3x2>; }
			template<> inline UniformFuncPtr<glm::mat3x4> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat3x4>; }

			template<> inline UniformFuncPtr<glm::mat4> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat4>; }
			template<> inline UniformFuncPtr<glm::mat4x2> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat4x2>; }
			template<> inline UniformFuncPtr<glm::mat4x3> GetUniformFunction()
				{ return UniformFunctionMatrixWrapper<glm::mat4x3>; }
		}
		template<typename T> inline void Set(Uniform::Handle uniform_handle, const T* data, size_t size) {
			Program::Bind({uniform_handle.program_handle});
			Uniform::FuncPtrs::GetUniformFunction<T>()(uniform_handle.uniform_handle, size, data);
		}
		template<typename T> inline void Set(Uniform::Handle uniform_handle, const T& data)
			{ Set(uniform_handle, &data, sizeof(T)); }
		template<typename T> inline void Set(Handle program_handle, Uniform::Handle uniform_handle, const std::vector<T>& data)
			{ Set(uniform_handle, data.data(), data.size()); }

		inline Handle GetLocation(Program::Handle program_handle, const std::string& uniform_name) {
			return {(HandleType)glGetUniformLocation(program_handle.handle, uniform_name.c_str()),
					program_handle.handle};
		}
	}
}


namespace JGFX{
	class Shader{
	protected:
		LLJGFX::Shader::Handle handle_ = { LLJGFX::INVALID_HANDLE };
	public:
		void Load(const std::string& data, ShaderType type){
			this->~Shader();
			handle_ = LLJGFX::Shader::Make(type);
			LLJGFX::Shader::Compile(handle_, data);
		}
		inline LLJGFX::Shader::Handle handle() const { return handle_; }
		inline ShaderType type() const { return LLJGFX::Shader::GetType(handle_); }
		inline std::string source() const { return LLJGFX::Shader::GetSource(handle_); }

		Shader() = default;
		Shader(const std::string& data, ShaderType type) { Load(data, type); }

		Shader& operator=(const Shader& cpy) { Load(cpy.source(), cpy.type()); return *this; }
		Shader(const Shader& cpy) { operator=(cpy); }

		Shader& operator=(Shader&& cpy) noexcept {
			this->~Shader();
			std::swap(handle_, cpy.handle_);
			return *this;
		}
		Shader(Shader&& cpy) noexcept { operator=(std::move(cpy)); }

		~Shader() { LLJGFX::Shader::Delete(handle_); handle_ = { LLJGFX::INVALID_HANDLE }; }
	};

	class VertexShader : public Shader{
	public:
		inline void Load(const std::string& data) { Shader::Load(data, ShaderType::VERTEX); }
		void Load(const std::string& data, ShaderType type) = delete;
		ShaderType type() = delete;

		VertexShader() = default;
		explicit VertexShader(const std::string& data) { Load(data); }
		VertexShader(const std::string& data, ShaderType type) = delete;
	};
	class FragmentShader : public Shader{
	public:
		inline void Load(const std::string& data) { Shader::Load(data, ShaderType::FRAGMENT); }
		void Load(const std::string& data, ShaderType type) = delete;
		ShaderType type() = delete;

		FragmentShader() = default;
		explicit FragmentShader(const std::string& data) { Load(data); }
		FragmentShader(const std::string& data, ShaderType type) = delete;
	};

	class ShaderProgram {
	private:
		LLJGFX::Program::Handle handle_ = { LLJGFX::INVALID_HANDLE };
	public:
		inline LLJGFX::Program::Handle handle() const { return handle_; }

		void Load(LLJGFX::Shader::Handle vertex, LLJGFX::Shader::Handle fragment) {
#ifndef NDEBUG
			if(LLJGFX::Shader::GetType(vertex) != ShaderType::VERTEX ||
			   LLJGFX::Shader::GetType(fragment) != ShaderType::FRAGMENT)
				throw std::runtime_error("Provided shader with incorrect type \n");
#endif
			this->~ShaderProgram();
			handle_ = LLJGFX::Program::Make();

			LLJGFX::Program::Link(handle_, vertex, fragment);
		}

		void Load(const Shader& vertex, const Shader& fragment) {
#ifndef NDEBUG
			if(vertex.type() != ShaderType::VERTEX || fragment.type() != ShaderType::FRAGMENT)
				throw std::runtime_error("Provided shader with incorrect type \n");
#endif
			this->~ShaderProgram();
			handle_ = LLJGFX::Program::Make();

			LLJGFX::Program::Link(handle_, vertex.handle(), fragment.handle());
		}
		inline void Load(const VertexShader& vertex, const FragmentShader& fragment)
			{ Load(static_cast<const Shader&>(vertex), static_cast<const Shader&>(fragment)); }
		inline void Load(const std::string& vertex, const std::string& fragment){
			Load(VertexShader{vertex},
				 FragmentShader{fragment});
		}

		inline void Bind() const { LLJGFX::Program::Bind(handle_); }

		inline LLJGFX::Uniform::Handle GetUniform(const std::string& name) const
			{ return LLJGFX::Uniform::GetLocation(handle_, name); }
		template <typename T> inline void SetUniform(LLJGFX::Uniform::Handle uniform_handle, const T& value) const {
#ifndef NDEBUG
			if(uniform_handle.program_handle != handle_.handle)
				throw std::runtime_error("That uniform is not for that shader");
#endif
			LLJGFX::Uniform::Set<T>(uniform_handle, value);
		}
		template <typename T> inline void SetUniform(const std::string& name, const T& value) const
			{ SetUniform(GetUniform(name), value); }

		inline ShaderProgram() = default;
		inline ShaderProgram(const std::string& vertex, const std::string& fragment)
			{ Load(vertex, fragment); }

		inline ShaderProgram& operator=(const ShaderProgram& cpy) {
			Load(LLJGFX::Program::GetBoundVertexShader(cpy.handle_),
			     LLJGFX::Program::GetBoundFragmentShader(cpy.handle_));
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
			{ LLJGFX::Program::Delete(handle_); handle_ = { LLJGFX::INVALID_HANDLE }; }
	};
}