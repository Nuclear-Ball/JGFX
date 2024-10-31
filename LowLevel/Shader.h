#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>

#include "Gpu/Shader.h"

namespace LLJGFX{
	namespace Internal {
		struct ProgramDataHolder {
			HandleType gpu_handle = INVALID_HANDLE;

			std::string bound_vs_src;
			HandleType bound_vs_gpu_handle = INVALID_HANDLE;

			std::string bound_fs_src;
			HandleType bound_fs_gpu_handle = INVALID_HANDLE;

			bool is_linked = false;
		};
		const ProgramDataHolder default_program = {};

		std::unordered_set<ProgramDataHolder *> all_program_handles;
	}

	namespace ShaderProgram {
		struct Handle {
			Internal::ProgramDataHolder* data;
		};

		inline bool IsValid(Handle handle) { return Internal::all_program_handles.contains(handle.data); }
	}
	namespace Internal {
		ShaderProgram::Handle bound_shader = { nullptr };

		void CheckProgramValidity(ShaderProgram::Handle handle){
			if(!IsValid(handle)){
				throw std::runtime_error(handle.data == nullptr ?
				                         "Non-existent shader was requested using uninitialized handle" :
				                         "Deleted shader was requested");
			}
		}
		inline const ProgramDataHolder& get_ro_pr_data(ShaderProgram::Handle handle) {//"ro" means "read only"
			if(handle.data == nullptr) return default_program;
#ifndef NDEBUG
			Internal::CheckProgramValidity(handle);
#endif
			return *handle.data;
		}
//		inline size_t program_hash(ShaderProgram::Handle handle){
//			const ProgramDataHolder& dat = get_ro_pr_data(handle);
//			return std::hash<std::string>{}(dat.bound_vs_src + dat.bound_fs_src);
//		}

		inline void RebindBoundShader(){
			Gpu::BindShaderProgram(get_ro_pr_data(bound_shader).gpu_handle);
		}
		void InitializeShaders(){
			Gpu::MakeShader = Gpu::Opengl33::MakeShader;
			Gpu::DeleteShader = Gpu::Opengl33::DeleteShader;
			Gpu::CompileShader = Gpu::Opengl33::CompileShader;

			Gpu::MakeShaderProgram = Gpu::Opengl33::MakeShaderProgram;
			Gpu::DeleteShaderProgram = Gpu::Opengl33::DeleteShaderProgram;
			Gpu::LinkShaderProgram = Gpu::Opengl33::LinkShaderProgram;

			Gpu::SetUniform = Gpu::Opengl33::SetUniform;

			for(ProgramDataHolder* i : all_program_handles){
				const HandleType old_handle = i->gpu_handle;

				i->gpu_handle = Gpu::MakeShaderProgram();

				i->bound_vs_gpu_handle = Gpu::MakeShader(Gpu::ShaderType::VERTEX);
				i->bound_fs_gpu_handle = Gpu::MakeShader(Gpu::ShaderType::FRAGMENT);

				if(i->is_linked) {
					Gpu::CompileShader(i->bound_vs_gpu_handle, i->bound_vs_src);
					Gpu::CompileShader(i->bound_fs_gpu_handle, i->bound_fs_src);
					Gpu::LinkShaderProgram(i->gpu_handle, i->bound_vs_gpu_handle, i->bound_fs_gpu_handle);

					const Gpu::PreInit::PI_SP& pi_dat = Gpu::PreInit::pi_sps.find(old_handle)->second;

					for(const std::pair<const std::string, Gpu::PreInit::PI_UF>& c_uf : pi_dat.uniforms)
						Gpu::SetUniform(i->gpu_handle, c_uf.first, c_uf.second.type,
										(const char*)c_uf.second.data.data(),
										c_uf.second.data.size() / Gpu::type_size_table[(int32)c_uf.second.type]);
				}

				RebindBoundShader();
				Gpu::PreInit::pi_sps.clear();
				Gpu::PreInit::sh_last_handle = 1;
			}
		}
	}

	namespace ShaderProgram {
		inline void Compile(Handle program_handle, const std::string& vertex_shader_source, const std::string& fragment_shader_source){
#ifndef NDEBUG
			Internal::CheckProgramValidity(program_handle);
#endif
			Internal::ProgramDataHolder& dat = *program_handle.data;

			dat.is_linked = true;
			dat.bound_vs_src = vertex_shader_source;
			dat.bound_fs_src = fragment_shader_source;

			Internal::Gpu::CompileShader(dat.bound_vs_gpu_handle, vertex_shader_source);
			Internal::Gpu::CompileShader(dat.bound_fs_gpu_handle, fragment_shader_source);
			Internal::Gpu::LinkShaderProgram(dat.gpu_handle, dat.bound_vs_gpu_handle, dat.bound_fs_gpu_handle);
		}

		inline Handle Make() {
			Handle handle = { new Internal::ProgramDataHolder{} };
			Internal::all_program_handles.insert(handle.data);
			Internal::ProgramDataHolder& dat = *handle.data;

			dat.gpu_handle = Internal::Gpu::MakeShaderProgram();
			dat.bound_vs_gpu_handle = Internal::Gpu::MakeShader(Internal::Gpu::ShaderType::VERTEX);
			dat.bound_fs_gpu_handle = Internal::Gpu::MakeShader(Internal::Gpu::ShaderType::FRAGMENT);
			return handle;
		}
		inline Handle Make(const std::string& vertex_shader_source, const std::string& fragment_shader_source) {
			Handle program_handle = Make();
			Compile(program_handle, vertex_shader_source, fragment_shader_source);
			return program_handle;
		}

		inline void Delete(Handle program_handle) {
#ifndef NDEBUG
			Internal::CheckProgramValidity(program_handle);
#endif
			Internal::ProgramDataHolder& dat = *program_handle.data;
			Internal::Gpu::DeleteShaderProgram(dat.gpu_handle);
			Internal::Gpu::DeleteShader(dat.bound_vs_gpu_handle);
			Internal::Gpu::DeleteShader(dat.bound_fs_gpu_handle);

			delete program_handle.data;
		}

		inline void Bind(Handle program_handle) {
			Internal::bound_shader = program_handle;
			Internal::Gpu::BindShaderProgram(Internal::get_ro_pr_data(program_handle).gpu_handle);
		}

		inline std::string GetAttachedVertexShader(Handle program_handle)
			{ return Internal::get_ro_pr_data(program_handle).bound_vs_src; }
		inline std::string GetAttachedFragmentShader(Handle program_handle)
			{ return Internal::get_ro_pr_data(program_handle).bound_fs_src; }

		template<typename T> inline void SetUniform(Handle program_handle, const std::string& uniform, const T* data, size_t element_count) {
#ifndef NDEBUG
			Internal::CheckProgramValidity(program_handle);
#endif
			Internal::ProgramDataHolder& dat = *program_handle.data;

			if(!dat.is_linked)
				throw std::runtime_error("Trying to set uniform for a shader program that wasn't compiled");

			Internal::Gpu::SetUniform(dat.gpu_handle, uniform, Internal::Gpu::DeduceUfType<T>(), (const void*)data, element_count);
		}
		template<typename T> inline void SetUniform(Handle program_handle, const std::string& uniform, const T& data)
			{ SetUniform(program_handle, uniform, &data, 1); }
		template<typename T> inline void SetUniform(Handle program_handle, const std::string& uniform, const std::vector<T>& data)
			{ SetUniform(program_handle, uniform, data.data(), data.size()); }
	}
}