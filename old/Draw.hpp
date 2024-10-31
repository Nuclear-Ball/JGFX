#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "WindowHelper.h"
#include "Shader.h"
#include "Mesh.hpp"
#include "Texture.h"

bool InitFunc() {
	if (!gladLoadGLLoader((GLADloadproc)glfw::getProcAddress)) {
		std::cout << "Failed to initialize GLAD\n";
		std::abort();
	}
	LLJGFX::Internal::InitializeTextures();
	LLJGFX::Internal::InitializeShaders();
	return true;
}

namespace LLJGFX{
	namespace Internal{
		void BindFramebuffer(int handle, pos2du16 size){
			glBindFramebuffer(GL_FRAMEBUFFER, handle);
			glViewport(0, 0, size.x, size.y);
		}
	}

	struct TemporaryRT { //"rt" means "rendering target"
		VRef::Handle vr_handle;
		LLJGFX::Texture::Handle texture_handle = { nullptr };
		LLJGFX::ShaderProgram::Handle sh_handle = { nullptr };
		size_t instances = 1;
	};

	void Draw(TemporaryRT target){
		Internal::UnbindBuffers();

		if(target.texture_handle.data != nullptr)
			glBindTexture(GL_TEXTURE_2D, target.texture_handle.data->texture_gpu_handle);
		else glBindTexture(GL_TEXTURE_2D, 0);

		glBindVertexArray(target.vr_handle.handle);

		const Internal::VRefDataHolder& dat = Internal::find_vr_data(target.vr_handle);
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)dat.ibuff_data.size(), GL_UNSIGNED_INT,
								nullptr, (GLsizei)target.instances);

		glBindVertexArray(0);
	}
	namespace Texture{
		void DrawTo(Handle txt_handle, TemporaryRT target){
			Internal::BindFramebuffer(GetFramebufferHandle(txt_handle), GetSize(txt_handle));

			Draw(target);
		}
	}
}

namespace JGFX {
	inline void Draw(const VertexReferencer& vref,
	                 LLJGFX::Texture::Handle texture_handle = { nullptr },
					 LLJGFX::ShaderProgram::Handle prg = {nullptr}, size_t instances = 1){
		LLJGFX::Draw({vref.handle(), texture_handle, prg, instances});
	}
	inline void Draw(const VertexCollection& vcl,
	                 LLJGFX::Texture::Handle texture_handle = { nullptr },
					 LLJGFX::ShaderProgram::Handle prg = {nullptr}, size_t instances = 1){
		LLJGFX::Draw({vcl.handle(), texture_handle, prg, instances});
	}

	inline Texture& Texture::operator<<(LLJGFX::TemporaryRT target){
		LLJGFX::Texture::DrawTo(handle_, target);
		return *this;
	}

	inline void SetDefaultClearColor(rgb color) {
		const glm::vec4 c_color = color;
		glClearColor(c_color.r,
		             c_color.g,
		             c_color.b,
		             c_color.a);
	}

	inline void Clear(rgb color) {
		SetDefaultClearColor(color);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	inline void Clear() { glClear(GL_COLOR_BUFFER_BIT); }
	inline void Clear(LLJGFX::Texture::Handle txt_handle, rgb color) {
		LLJGFX::Internal::BindFramebuffer(GetFramebufferHandle(txt_handle), GetSize(txt_handle));
		Clear(color);
	}
	inline void Clear(Texture& txt, rgb color) { Clear(txt.handle(), color); }

	enum class OptionalFeature{
		BLEND = GL_BLEND,
		CULL = GL_CULL_FACE,
		DEPTH_CLAMP = GL_DEPTH_CLAMP,
		DEPTH_TEST = GL_DEPTH_TEST,
		SCISSOR_TEST = GL_SCISSOR_TEST,
		STENCIL_TEST = GL_STENCIL_TEST
	};

	inline void SetMode(OptionalFeature feature, bool mode){
		mode ? glEnable((GLenum)feature) : glDisable((GLenum)feature);
	}
}


	/*
	inline glm::dmat4 MakeMatrix(pos2dd pos, pos2dus texture_size, const Common::TransfProps& props = {}) {
		glm::dmat4 model = glm::translate<double>(glm::dmat4(1), {pos, 0.});
		model = props.MakeMatrix(model);
		model = glm::scale<double>(model, {texture_size, 1.});

		return model;
	}


	inline void InstancedList(const Common::Mesh& mesh, const Common::BasicTexture& txt, const std::vector<glm::mat4>& list, float transparency = 1.f){
		Common::res.txt_sh().Use();
		Common::res.txt_sh().SetTransparency(1.f - transparency);

		txt.Use();
		mesh.Draw(list);
	}
	inline void InstancedList(const Common::BasicTexture& txt, const std::vector<glm::mat4>& list, float transparency = 1.f){
		InstancedList(Common::res.qd(), txt, list, transparency);
	}
	inline void Sprite(const Common::BasicTexture& txt, pos2dd pos, const Common::TransfProps& props = {}, float transparency = 1.f) {
		InstancedList(txt, { MakeMatrix(pos,txt.size(), props)}, transparency);
	}

	void Line(pos2dd start, pos2dd end, int thickness = 1, rgb color = { 255, 255, 255 }) {
		const double ct_x = (end.x - start.x), ct_y = (end.y - start.y);
		const double l = std::sqrt(std::pow(ct_x, 2) + std::pow(ct_y, 2));
		const double ct_sin = ct_x / l;
		constexpr double pi = 3.14159265358979323846;
		const double rotation = asinf(ct_sin) * (ct_y >= 0 ? 1 : -1) - (pi / 2) + (ct_y >= 0 ? 0 : pi);

		const double offset = static_cast<float>(thickness) / 2;

		glm::dmat4 model = glm::translate<double>(glm::dmat4(1), glm::dvec3(start.x, start.y, 0.0f));

		model = glm::rotate<double>(model, -rotation, glm::dvec3(0.0f, 0.0f, 1.0f));
		model = glm::translate<double>(model, glm::dvec3(-offset, -offset, 0.0));
		model = glm::scale<double>(model, glm::dvec3(l + thickness, thickness, 1.0f));

		Common::res.flat_sh().Use();
		Common::res.flat_sh().SetColor(color);

		Common::res.qd().Draw({model});
	}

	inline void Line(pos2d start, pos2d end, int thickness = 1, rgb color = { 255, 255, 255, 255 }) {
		Line((pos2dd)start,
		     (pos2dd)end, thickness, color);
	}

	void Rect(pos2dd start, pos2dd end, rgb color = {255, 255, 255, 255}) {
		end.x--;
		start.y++;

		pos2dd pos1, pos2;

		pos1 = start, pos2 = { start.x, end.y };
		Line(pos1, pos2, 1, color);

		pos1 = pos2, pos2 = { end.x, end.y };
		Line(pos1, pos2, 1, color);

		pos1 = pos2, pos2 = { end.x, start.y };
		Line(pos1, pos2, 1, color);

		pos1 = pos2, pos2 = { start.x, start.y };
		Line(pos1, pos2, 1, color);
	}

	void Rect(pos2d start, pos2d end, rgb color) {
		Rect((pos2dd)start,
		     (pos2dd)end, color);
	}

	void DisplayFramebuffer(const Common::Framebuffer& frm, const Ghelp::JWindow& win) {
		win.Bind();

		frm.Use();
		Common::res.fb_sh().Bind();
		Common::res.qd().DrawNoInstancing();
	}*/

