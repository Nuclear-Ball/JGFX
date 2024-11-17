#pragma once
#include "LowLevel/Window.h"
#include "Texture.h"

namespace JGFX{
	class Window{
	private:
		LLJGFX::Window::Handle handle_ = { nullptr };
	public:
		Window& SetSize(pos2du16 size)
			{ LLJGFX::Window::SetSize(handle_, size); return *this; }
#ifdef ADVANCED_WINDOW_ACTIONS
		Window& SetPos(pos2du16 pos){ LLJGFX::Window::SetPos(handle_, pos); return *this; }
#endif
		Window& SetTitle(const std::string& name)
			{ LLJGFX::Window::SetTitle(handle_, name); return *this; }
		Window& SetIcon(Internal::TxtFnArg icon)
			{ LLJGFX::Window::SetIcon(handle_, icon); return *this; }
		Window& ResetIcon()
			{ LLJGFX::Window::ResetIconToDefault(handle_); return *this; }

		inline Window& Bind() const {
			LLJGFX::BindFramebuffer(handle_);
			return (Window&)*this;
		}

		inline pos2du16 size() const { return LLJGFX::Window::GetSize(handle_); }
		inline pos2d16 pos() const { return LLJGFX::Window::GetPos(handle_); }
		inline std::string title() const { return LLJGFX::Window::GetTitle(handle_); }
		inline Texture icon() const { return LLJGFX::Window::GetIcon(handle_); }
		inline LLJGFX::Window::Handle handle() const { return handle_; }

		inline bool should_close() const { return LLJGFX::Window::ShouldClose(handle_); }

		inline Window(pos2du16 size = {600, 600}, const std::string& title = "JGFX Window", Internal::TxtFnArg icon = NULL_TXT)
			{ handle_ = LLJGFX::Window::Make(size, title, icon); }
		inline ~Window() { LLJGFX::Window::Delete(handle_); handle_ = { nullptr }; }
	};

	namespace Internal{
		struct WinFnArg : public LLJGFX::Window::Handle{
			WinFnArg() = default;
			WinFnArg(LLJGFX::Window::Handle txt) { data = txt.data; }
			WinFnArg(const Window& win) { data = win.handle().data; }
		};
	}

	inline void BindFramebuffer(Internal::WinFnArg win){
		LLJGFX::BindFramebuffer(win);
	}

	using LLJGFX::NewFrame;
}