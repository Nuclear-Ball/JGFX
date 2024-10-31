#pragma once
#include <string>
#include <stdexcept>
#include <glad/glad.h>

#include "../Common.h"


namespace LLJGFX{
	namespace Internal{
		namespace Gpu{
			enum class ShaderType {
				VERTEX,
				FRAGMENT
			};

			enum class UniformType {
				UINT32,
				INT32,
				FL32,

				VEC2_UINT32,
				VEC2_INT32,
				VEC2_FL32,

				VEC3_UINT32,
				VEC3_INT32,
				VEC3_FL32,

				VEC4_UINT32,
				VEC4_INT32,
				VEC4_FL32,

				MAT2x2,
				MAT2x3,
				MAT2x4,

				MAT3x2,
				MAT3x3,
				MAT3x4,

				MAT4x2,
				MAT4x3,
				MAT4x4,

				INVALID
			};

			template<typename T> consteval UniformType DeduceUfType() { return UniformType::INVALID; }

			template<> consteval UniformType DeduceUfType<uint32>() { return UniformType::UINT32; }
			template<> consteval UniformType DeduceUfType<int32>() { return UniformType::INT32; }
			template<> consteval UniformType DeduceUfType<fl32>() { return UniformType::FL32; }

			template<> consteval UniformType DeduceUfType<glm::vec<2, uint32, glm::highp>>() { return UniformType::VEC2_UINT32; }
			template<> consteval UniformType DeduceUfType<glm::vec<2, int32, glm::highp>>() { return UniformType::VEC2_INT32; }
			template<> consteval UniformType DeduceUfType<glm::vec<2, fl32, glm::highp>>() { return UniformType::VEC2_FL32; }

			template<> consteval UniformType DeduceUfType<glm::vec<3, uint32, glm::highp>>() { return UniformType::VEC3_UINT32; }
			template<> consteval UniformType DeduceUfType<glm::vec<3, int32, glm::highp>>() { return UniformType::VEC3_INT32; }
			template<> consteval UniformType DeduceUfType<glm::vec<3, fl32, glm::highp>>() { return UniformType::VEC3_FL32; }

			template<> consteval UniformType DeduceUfType<glm::vec<4, uint32, glm::highp>>() { return UniformType::VEC4_UINT32; }
			template<> consteval UniformType DeduceUfType<glm::vec<4, int32, glm::highp>>() { return UniformType::VEC4_INT32; }
			template<> consteval UniformType DeduceUfType<glm::vec<4, fl32, glm::highp>>() { return UniformType::VEC4_FL32; }


			template<> consteval UniformType DeduceUfType<glm::mat2x2>() { return UniformType::MAT2x2; }
			template<> consteval UniformType DeduceUfType<glm::mat2x3>() { return UniformType::MAT2x3; }
			template<> consteval UniformType DeduceUfType<glm::mat2x4>() { return UniformType::MAT2x4; }

			template<> consteval UniformType DeduceUfType<glm::mat3x2>() { return UniformType::MAT3x2; }
			template<> consteval UniformType DeduceUfType<glm::mat3x3>() { return UniformType::MAT3x3; }
			template<> consteval UniformType DeduceUfType<glm::mat3x4>() { return UniformType::MAT3x4; }

			template<> consteval UniformType DeduceUfType<glm::mat4x2>() { return UniformType::MAT4x2; }
			template<> consteval UniformType DeduceUfType<glm::mat4x3>() { return UniformType::MAT4x3; }
			template<> consteval UniformType DeduceUfType<glm::mat4x4>() { return UniformType::MAT4x4; }


			const size_t type_size_table[] = {
				sizeof(uint32),
				sizeof(int32),
				sizeof(fl32),

				sizeof(glm::vec<2, uint32, glm::highp>),
				sizeof(glm::vec<2, int32, glm::highp>),
				sizeof(glm::vec<2, fl32, glm::highp>),

				sizeof(glm::vec<3, uint32, glm::highp>),
				sizeof(glm::vec<3, int32, glm::highp>),
				sizeof(glm::vec<3, fl32, glm::highp>),

				sizeof(glm::vec<4, uint32, glm::highp>),
				sizeof(glm::vec<4, int32, glm::highp>),
				sizeof(glm::vec<4, fl32, glm::highp>),


				sizeof(glm::mat2x2),
				sizeof(glm::mat2x3),
				sizeof(glm::mat2x4),

				sizeof(glm::mat3x2),
				sizeof(glm::mat3x3),
				sizeof(glm::mat3x4),

				sizeof(glm::mat4x2),
				sizeof(glm::mat4x3),
				sizeof(glm::mat4x4)
			};

			namespace Opengl33{
				namespace Uniforms {
					using UniformFuncPtr = void (*)(int uniform_location, int count, const void* value);

					void UniformFunctionMatrixWrapper2x2(int uniform_location, int count, const void* value)
						{ glUniformMatrix2fv(uniform_location, count, false, (float*)value); }
					void UniformFunctionMatrixWrapper2x3(int uniform_location, int count, const void* value)
						{ glUniformMatrix2x3fv(uniform_location, count, false, (float*)value); }
					void UniformFunctionMatrixWrapper2x4(int uniform_location, int count, const void* value)
						{ glUniformMatrix2x4fv(uniform_location, count, false, (float*)value); }

					void UniformFunctionMatrixWrapper3x2(int uniform_location, int count, const void* value)
						{ glUniformMatrix3x2fv(uniform_location, count, false, (float*)value); }
					void UniformFunctionMatrixWrapper3x3(int uniform_location, int count, const void* value)
						{ glUniformMatrix3fv(uniform_location, count, false, (float*)value); }
					void UniformFunctionMatrixWrapper3x4(int uniform_location, int count, const void* value)
						{ glUniformMatrix3x4fv(uniform_location, count, false, (float*)value); }

					void UniformFunctionMatrixWrapper4x2(int uniform_location, int count, const void* value)
						{ glUniformMatrix4x2fv(uniform_location, count, false, (float*)value); }
					void UniformFunctionMatrixWrapper4x3(int uniform_location, int count, const void* value)
						{ glUniformMatrix4x2fv(uniform_location, count, false, (float*)value); }
					void UniformFunctionMatrixWrapper4x4(int uniform_location, int count, const void* value)
						{ glUniformMatrix4fv(uniform_location, count, false, (float*)value); }


					inline UniformFuncPtr GetUniformFunction(UniformType type) {
						switch(type){
							case UniformType::INVALID: throw std::runtime_error("Invalid uniform type");

							case UniformType::UINT32: return (UniformFuncPtr)glUniform1uiv;
							case UniformType::INT32: return (UniformFuncPtr)glUniform1iv;
							case UniformType::FL32: return (UniformFuncPtr)glUniform1fv;

							case UniformType::VEC2_UINT32: return (UniformFuncPtr)glUniform2uiv;
							case UniformType::VEC2_INT32: return (UniformFuncPtr)glUniform2iv;
							case UniformType::VEC2_FL32: return (UniformFuncPtr)glUniform2fv;

							case UniformType::VEC3_UINT32: return (UniformFuncPtr)glUniform3uiv;
							case UniformType::VEC3_INT32: return (UniformFuncPtr)glUniform3iv;
							case UniformType::VEC3_FL32: return (UniformFuncPtr)glUniform3fv;

							case UniformType::VEC4_UINT32: return (UniformFuncPtr)glUniform4uiv;
							case UniformType::VEC4_INT32: return (UniformFuncPtr)glUniform4iv;
							case UniformType::VEC4_FL32: return (UniformFuncPtr)glUniform4fv;


							case UniformType::MAT2x2: return UniformFunctionMatrixWrapper2x2;
							case UniformType::MAT2x3: return UniformFunctionMatrixWrapper2x3;
							case UniformType::MAT2x4: return UniformFunctionMatrixWrapper2x4;

							case UniformType::MAT3x2: return UniformFunctionMatrixWrapper3x2;
							case UniformType::MAT3x3: return UniformFunctionMatrixWrapper3x3;
							case UniformType::MAT3x4: return UniformFunctionMatrixWrapper3x4;

							case UniformType::MAT4x2: return UniformFunctionMatrixWrapper4x2;
							case UniformType::MAT4x3: return UniformFunctionMatrixWrapper4x3;
							case UniformType::MAT4x4: return UniformFunctionMatrixWrapper4x4;
						}
					}
				}

				HandleType bound_shader = INVALID_HANDLE;

				HandleType MakeShader(ShaderType type) {
					const GLenum sh_types[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
					return glCreateShader(sh_types[(int32)type]);
				}
				void DeleteShader(HandleType handle) { glDeleteShader(handle); }
				void CompileShader(HandleType handle, const std::string& source){
					const char *tmp = source.c_str();
					const GLint size = source.size();
					glShaderSource(handle, 1, &tmp, &size);
					glCompileShader(handle);
#ifndef NDEBUG
					char deb_log[1024];
					int success = 0;
					glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
					if (!success) {
						glGetShaderInfoLog(handle, 1024, nullptr, deb_log);
						throw std::runtime_error(std::string("Shader compilation failed: " + std::string(deb_log)).c_str());
					}
#endif
				}

				HandleType MakeShaderProgram() { return glCreateProgram(); }
				void DeleteShaderProgram(HandleType handle) { glDeleteProgram(handle); }
				void LinkShaderProgram(HandleType program_handle, HandleType v_sh_handle, HandleType f_sh_handle){
					glAttachShader(program_handle, v_sh_handle);
					glAttachShader(program_handle, f_sh_handle);
					glLinkProgram(program_handle);
#ifndef NDEBUG
					char deb_log[1024];
					int success = 0;
					glGetProgramiv(program_handle, GL_LINK_STATUS, &success);
					if (!success) {
						glGetProgramInfoLog(program_handle, 1024, nullptr, deb_log);
						throw std::runtime_error(std::string("Shader program linking failed: " + std::string(deb_log)).c_str());
					}
#endif
				}

				void SetUniform(HandleType prg, const std::string& u_name, UniformType type, const void* data, size_t elem_count) {
					glUseProgram(prg);

					const GLint loc = glGetUniformLocation(prg, u_name.c_str());
					Uniforms::GetUniformFunction(type)(loc, (GLint)elem_count, data);

					glUseProgram(bound_shader);
				}

				void BindSP(HandleType prg){ bound_shader = prg; glUseProgram(prg); }
			}
			namespace PreInit{
				HandleType sh_last_handle = 1;

				struct PI_UF{
					UniformType type;
					std::string data;
				};

				struct PI_SP{
					std::unordered_map<std::string, PI_UF> uniforms;
				};

				std::unordered_map<HandleType, PI_SP> pi_sps;

				HandleType MakeShader(ShaderType type) { return INVALID_HANDLE; }
				void DeleteShader(HandleType handle) {}
				void CompileShader(HandleType handle, const std::string& source){}

				HandleType MakeShaderProgram() { pi_sps[sh_last_handle]; return sh_last_handle++; }
				void DeleteShaderProgram(HandleType handle) { pi_sps.erase(handle); }
				void LinkShaderProgram(HandleType program_handle, HandleType vertex_sh_handle, HandleType fragment_sh_handle){}

				void SetUniform(HandleType prg, const std::string& u_name, UniformType type, const void* data, size_t elem_count){
					pi_sps.find(prg)->second.uniforms[u_name] = {type, std::string((const char*)data, type_size_table[(int32)type] * elem_count)};
				}
			}
			HandleType (*MakeShader)(ShaderType) = PreInit::MakeShader;
			void (*DeleteShader)(HandleType) = PreInit::DeleteShader;
			void (*CompileShader)(HandleType, const std::string&) = PreInit::CompileShader;

			HandleType (*MakeShaderProgram)() = PreInit::MakeShaderProgram;
			void (*DeleteShaderProgram)(HandleType) = PreInit::DeleteShaderProgram;
			void (*LinkShaderProgram)(HandleType, HandleType, HandleType) = PreInit::LinkShaderProgram;

			void (*SetUniform)(HandleType prg, const std::string& u_name, UniformType type, const void* data, size_t elem_count) = PreInit::SetUniform;

			void (*BindShaderProgram)(HandleType prg) = Opengl33::BindSP;
		}
	}
}