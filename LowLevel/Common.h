#pragma once
#include <unordered_set>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../pos2d.h"

namespace LLJGFX {
	typedef GLuint HandleType;
	constexpr HandleType INVALID_HANDLE = 0;

	namespace Internal{
		struct TxtDataHolder;

		struct WindowDataHolder {
			GLFWwindow* win = nullptr;
			std::unordered_set<TxtDataHolder*> local_framebuffers;
			TxtDataHolder* icon_handle;
			pos2du16 lower_size_limit = {0, 0};
			pos2du16 upper_size_limit = {65535, 65535};
			//Some additional data
		};

		WindowDataHolder* curr_context = nullptr;
	}
}

namespace LLJGFX {
	struct TemporaryRT;
}