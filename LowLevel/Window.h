#pragma once
#include <GLFW/glfw3.h>
#include <unordered_set>
#include <stdexcept>

#include "Common.h"
#include "Texture.h"
#include "Gpu/Draw.h"

bool InitFunc();
void DestroyFunc();

namespace LLJGFX{
	namespace Internal{
		const WindowDataHolder default_win = { nullptr };

		std::unordered_set<WindowDataHolder*> all_window_handles;
	}

	namespace Window{
		struct Handle{
			Internal::WindowDataHolder* data = nullptr;
		};

		bool IsValid(Handle window) { return Internal::all_window_handles.contains(window.data); }
	}

	namespace Internal{
		inline void CheckWindowValidity(Window::Handle win_handle){
#ifndef NDEBUG
			if(!IsValid(win_handle)){
				throw std::runtime_error(win_handle.data == nullptr ?
				                         "Non-existent window was requested using uninitialized handle" :
				                         "Deleted window was requested");
			}
#endif
		}
		inline const WindowDataHolder& get_ro_win_data(Window::Handle win_handle) {//"ro" means "read only"
			if(win_handle.data == nullptr)
				return default_win;
			Internal::CheckWindowValidity(win_handle);
			return *win_handle.data;
		}

		fl64 delta_time = 0;
		fl64 fps = 0;
		fl64 last_frame = 0;

//		inline void ApplyToAllWindows(void(*func)()){
//			WindowDataHolder* const old_ctx = curr_context;
//			for(WindowDataHolder* i : all_window_handles){
//				glfwMakeContextCurrent(i->win);
//				func();
//			}
//			glfwMakeContextCurrent(old_ctx->win);
//			curr_context = old_ctx;
//		}
	}
	namespace Window{
		inline void SetSize(Handle win_handle, pos2du16 size){
			Internal::CheckWindowValidity(win_handle);
			glfwSetWindowSize(win_handle.data->win, size.x, size.y);
		}

#ifdef ADVANCED_WINDOW_ACTIONS
		inline void SetPos(Handle win_handle, pos2d16 pos){
			Internal::CheckWindowValidity(win_handle);
			glfwSetWindowPos(win_handle.data->win, pos.x, pos.y);
		}
#endif
		inline void SetTitle(Handle win_handle, const std::string& title){
			Internal::CheckWindowValidity(win_handle);
			glfwSetWindowTitle(win_handle.data->win, title.c_str());
		}
		inline void SetIcon(Handle win_handle, Texture::Handle icon){
			if(icon.data == nullptr) {
				glfwSetWindowIcon(win_handle.data->win, 0, nullptr);
				return;
			}

			Internal::CheckWindowValidity(win_handle);
			Internal::CheckTextureValidity(icon);
			Internal::WindowDataHolder& dat = *win_handle.data;

			if(dat.icon_handle == nullptr){
				LLJGFX::Texture::Handle tmp = LLJGFX::Texture::Make();
				dat.icon_handle = tmp.data;
			}

			//Copying the texture
			LLJGFX::Texture::Handle ic_obj_handle = { dat.icon_handle };
			LLJGFX::Texture::SetData(ic_obj_handle, LLJGFX::Texture::GetData(icon),
									 LLJGFX::Texture::GetSize(icon));
			LLJGFX::Texture::SetModes(ic_obj_handle, LLJGFX::Texture::GetFilteringMode(icon),
			                          LLJGFX::Texture::GetWrapMode(icon));

			//Actually setting it inside the GLFW
			const std::string icon_data = LLJGFX::Texture::GetData(icon);
			const pos2du16 icon_size = LLJGFX::Texture::GetSize(icon);
			const GLFWimage glfw_icon = { icon_size.x, icon_size.y, (unsigned char*)icon_data.c_str() };
			glfwSetWindowIcon(dat.win, 1, &glfw_icon);
		}
		inline void ResetIconToDefault(Handle win_handle)
			{ SetIcon(win_handle, {nullptr}); }

		inline void SetLowerSizeLimit(Handle win_handle, pos2du16 limit){
			Internal::CheckWindowValidity(win_handle);

			pos2du16 upper_limit = win_handle.data->upper_size_limit;
			if(limit.x > upper_limit.x || limit.y > upper_limit.y)
				throw std::runtime_error("Invalid lower size limit(aka lower limit is greater than the upper limit)");

			glfwSetWindowSizeLimits(win_handle.data->win, limit.x, limit.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
			win_handle.data->lower_size_limit = limit;
		}
		inline void SetUpperSizeLimit(Handle win_handle, pos2du16 limit){
			Internal::CheckWindowValidity(win_handle);

			pos2du16 lower_limit = win_handle.data->lower_size_limit;
			if(lower_limit.x > limit.x || lower_limit.y > limit.y)
				throw std::runtime_error("Invalid upper size limit(aka upper limit is smaller than the lower limit)");

			glfwSetWindowSizeLimits(win_handle.data->win, GLFW_DONT_CARE, GLFW_DONT_CARE, limit.x, limit.y);
			win_handle.data->upper_size_limit = limit;
		}
		inline void SetSizeLimits(Handle win_handle, pos2du16 lower_limit, pos2du16 upper_limit){
			Internal::CheckWindowValidity(win_handle);

			if(lower_limit.x > upper_limit.x || lower_limit.y > upper_limit.y)
				throw std::runtime_error("Invalid size limits(aka lower limit is greater than the upper)");

			glfwSetWindowSizeLimits(win_handle.data->win, lower_limit.x, lower_limit.y, upper_limit.x, upper_limit.y);
			win_handle.data->lower_size_limit = lower_limit;
			win_handle.data->upper_size_limit = upper_limit;
		}

		inline void ResetUpperSizeLimit(Handle win_handle)
			{ SetUpperSizeLimit(win_handle, {65535, 65535}); }
		inline void ResetLowerSizeLimit(Handle win_handle)
			{ SetLowerSizeLimit(win_handle, {0, 0}); }
		inline void ResetSizeLimits(Handle win_handle)
			{ SetSizeLimits(win_handle, {0, 0}, {65535, 65535}); }

#ifdef ADVANCED_WINDOW_ACTIONS
		inline void Iconify(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			glfwIconifyWindow(win_handle.data->win);
		}
		inline void UnIconify(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			bool was_maximized = glfwGetWindowAttrib(win_handle.data->win, GLFW_MAXIMIZED);

			glfwRestoreWindow(win_handle.data->win);
			if(was_maximized)
				glfwMaximizeWindow(win_handle.data->win);
		}

		inline void Maximize(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			glfwMaximizeWindow(win_handle.data->win);
		}
		inline void Minimize(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			bool was_iconified = glfwGetWindowAttrib(win_handle.data->win, GLFW_ICONIFIED);;

			glfwRestoreWindow(win_handle.data->win);
			if(was_iconified)
				glfwIconifyWindow(win_handle.data->win);
		}

		inline void Hide(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			glfwHideWindow(win_handle.data->win);
		}
		inline void Show(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			glfwShowWindow(win_handle.data->win);
		}

		inline void Restore(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			glfwRestoreWindow(win_handle.data->win);
			glfwShowWindow(win_handle.data->win);
		}
#endif
		inline void RequestAttention(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			glfwRequestWindowAttention(win_handle.data->win);
		}
#ifdef ADVANCED_WINDOW_ACTIONS
		inline void BringToFocus(Handle win_handle){
			Internal::CheckWindowValidity(win_handle);
			glfwFocusWindow(win_handle.data->win);
		}
#endif

		inline pos2du16 GetSize(Handle win_handle){
			int32 x_adapter;
			int32 y_adapter;
			glfwGetWindowSize(Internal::get_ro_win_data(win_handle).win, &x_adapter, &y_adapter);
			return { x_adapter, y_adapter };
		}
		inline pos2d16 GetPos(Handle win_handle){
			int32 x_adapter;
			int32 y_adapter;
			glfwGetWindowPos(Internal::get_ro_win_data(win_handle).win, &x_adapter, &y_adapter);
			return { x_adapter, y_adapter };
		}
		inline std::string GetTitle(Handle win_handle)
			{ return glfwGetWindowTitle(Internal::get_ro_win_data(win_handle).win); }
		inline Texture::Handle GetIcon(Handle win_handle)
			{ return { Internal::get_ro_win_data(win_handle).icon_handle }; }

		inline pos2du16 GetLowerSizeLimit(Handle win_handle)
			{ return Internal::get_ro_win_data(win_handle).lower_size_limit; }
		inline pos2du16 GetUpperSizeLimit(Handle win_handle)
			{ return Internal::get_ro_win_data(win_handle).upper_size_limit; }

		inline bool IsIconified(Handle win_handle)
			{ return glfwGetWindowAttrib(Internal::get_ro_win_data(win_handle).win, GLFW_ICONIFIED); }
		inline bool IsMaximized(Handle win_handle)
			{ return glfwGetWindowAttrib(Internal::get_ro_win_data(win_handle).win, GLFW_MAXIMIZED); }
		inline bool IsVisible(Handle win_handle)
			{ return glfwGetWindowAttrib(Internal::get_ro_win_data(win_handle).win, GLFW_VISIBLE); }
		inline bool IsNormal(Handle win_handle)
			{ return !IsIconified(win_handle) && !IsMaximized(win_handle) && IsVisible(win_handle); }

		inline bool IsFocused(Handle win_handle)
			{ return glfwGetWindowAttrib(Internal::get_ro_win_data(win_handle).win, GLFW_FOCUSED); }

		inline bool ShouldClose(Handle win_handle)
			{ return glfwWindowShouldClose(Internal::get_ro_win_data(win_handle).win); }

		inline Handle Make(pos2du16 size = {600, 600}, const std::string& title = {"JGFX Window"}, Texture::Handle icon = {nullptr}) {
			Handle handle = { new Internal::WindowDataHolder{} };

			Internal::WindowDataHolder& dat = *handle.data;

			GLFWwindow* prev_window = nullptr;
			if(!Internal::all_window_handles.empty())
				prev_window = (*Internal::all_window_handles.begin())->win;
			else glfwInit();

			dat.win = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, prev_window);


			if(Internal::all_window_handles.empty()){
				glfwMakeContextCurrent(dat.win);
				Internal::curr_context = &dat;
				InitFunc();
			}

			Internal::all_window_handles.insert(handle.data);

			SetIcon(handle, icon);

			for(int i = 0; i < Opt::STENCIL_TEST; i++){
				if(Internal::opt_modes[i]) Internal::Gpu::Enable((Opt::OptFtr)i);
				else Internal::Gpu::Disable((Opt::OptFtr)i);
			}

			return handle;
		}
		inline void Delete(Handle win_handle){
			if(win_handle.data == nullptr) return;

			Internal::CheckWindowValidity(win_handle);
			Internal::WindowDataHolder& dat = *win_handle.data;

			glfwDestroyWindow(dat.win);

			for(Internal::TxtDataHolder* i : win_handle.data->local_framebuffers)
				i->context_framebuffers.erase(win_handle.data);

			if(Internal::curr_context == win_handle.data) Internal::curr_context = nullptr;

			Internal::all_window_handles.erase(win_handle.data);
			delete win_handle.data;

			//if(Internal::all_window_handles.empty()) DestroyFunc();
		}
	}

	inline void NewFrame() {
		glfwPollEvents();

		for(Internal::WindowDataHolder* i : Internal::all_window_handles)
			glfwSwapBuffers(i->win);

		if(Internal::bound_framebuffer_texture.data == nullptr)
			Internal::Gpu::SetViewportSize(Window::GetSize({Internal::curr_context}));

		const fl64 currentFrame = glfwGetTime();
		Internal::delta_time = currentFrame - Internal::last_frame;
		Internal::last_frame = currentFrame;
		Internal::fps = 1. / Internal::delta_time;
	}

	inline fl64 GetDeltaTimeValue() { return Internal::delta_time; }
	inline fl64 GetFpsValue() { return Internal::fps; }

	inline void BindFramebuffer(Window::Handle win_handle){
		Internal::CheckWindowValidity(win_handle);

		glfwMakeContextCurrent(win_handle.data->win);
		Internal::Gpu::BindFramebuffer(0, Window::GetSize(win_handle));
		Internal::curr_context = win_handle.data;
		Internal::bound_framebuffer_texture = { nullptr };
	}
}