# JGFX - a very simple 3D graphics library!

**WARNING!** The library is still in alpha, so be ready for a lots of bugs, and other stuff not working.

## The triangle example:
```C++
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <JGFX/JGFX.h>

const std::string vertex_source = "#version 330 core\n"
                                  "layout (location = 0) in vec3 aPos;\n"
                                  "layout (location = 1) in vec3 aCol;\n"
                                  "out vec3 vCol;\n"
                                  "void main() {\n"
                                  "   gl_Position = vec4(aPos, 1.0);\n"
                                  "   vCol = aCol;\n"
                                  "}";

const std::string fragment_source = "#version 330 core\n"
                                    "out vec4 FragColor;\n"
                                    "in vec3 vCol;\n"

                                    "void main() {\n"
                                    "   FragColor = vec4(vCol, 1.0);\n"
                                    "}";


struct TriangleVertex{
	glm::vec3 pos;
	glm::vec3 uv;
};

const std::vector<TriangleVertex> vertices = {
		{{ 0.9f, -0.9f, 0.0f },  { 1.0f, 0.0f, 0.0f } },
		{{ -0.9f, -0.9f, 0.0f},  { 0.0f, 0.0f, 0.0f } },
		{{ 0.0f,  0.9f, 0.0f },  { 0.5f, 1.0f, 1.0f } }
};

int main() {
	JGFX::Window win = {{600, 600}, "Hello triangle"};
	JGFX::ShaderProgram prg = { vertex_source, fragment_source };
	const JGFX::VertexLayout triangle_layout = {{0, JGFX::AttType<float>(), 3},
	                                            {1, JGFX::AttType<float>(), 3}};
	JGFX::VertexCollection triangle = {{vertices, triangle_layout}, "verts", JGFX::Indexate(3)};

	while(!win.should_close()){
		JGFX::NewFrame();

		JGFX::Clear(100, 100, 100);

		JGFX::Draw(triangle, { nullptr }, prg);
	}
}
```
## Installation
JGFX is a header-only library, so you only need to copy this repo to your include directory. But before using it you also should install following dependencies:
    -GLM
    -GLFW
    -GLAD (specifically for OpenGL 3.3)
    -STB Image

## Requirements 
Compiler: Clang or MSVC,

C++ version: 20 or higher

## User input handling
It hasn't been implemented yet(but will be soon), so for now you can use raw GLFW functions for that. The GLFW window handle can be obtained from a JGFX handle like this:
```C++
GLFWwindow* raw = your_jgfx_win.handle().data->win;
```
Or if you using low level API:
```C++
GLFWwindow* raw = your_lljgfx_win_handle.data->win;
```

## JGFX Example 2
This example shows textures, framebuffers, shader uniforms
```C++
#include "JGFX/JGFX.h"

const std::string vertex_source = "#version 330 core\n"
                                  "layout (location = 0) in vec3 aPos;\n"
                                  "layout (location = 1) in vec3 aUv;\n"
                                  "out vec3 vUv;\n"
                                  "void main() {\n"
                                  "   gl_Position = vec4(aPos, 1.0);\n"
                                  "   vUv = aUv;\n"
                                  "}";

const std::string fragment_source = "#version 330 core\n"
	                                "out vec4 FragColor;\n"
	                                "in vec3 vUv;\n"

                                    "uniform sampler2D txt;\n"
                                    "uniform sampler2D txt2;\n"
                                    "uniform float col;\n"

	                                "void main() {\n"
	                                "   FragColor = mix(vec4(texture(txt, vec2(vUv.x, -vUv.y)).r, col, 0, 1), texture(txt2, vec2(vUv.x, -vUv.y)), 0.6);\n"
	                                "}";


struct TriangleVertex{
	glm::vec3 pos;
	glm::vec3 uv;
};

const std::vector<TriangleVertex> vertices = {
		{{ 0.9f, -0.9f, 0.0f },  { 1.0f, 0.0f, 0.0f } },
		{{ -0.9f, -0.9f, 0.0f},  { 0.0f, 0.0f, 0.0f } },
		{{ 0.0f,  0.9f, 0.0f },  { 0.5f, 1.0f, 1.0f } }
};

int main() {
	JGFX::Opt::Enable(JGFX::Opt::BLEND);
	const JGFX::ShaderProgram prg(vertex_source, fragment_source);
	prg.SetUniform("txt2", 1);

  //Some of the JGFX functionality can be used even before complete initialization.
  //All the data will be transfered to the GPU once the window context is created.
  //(Except for everything in the Mesh.hpp file, trying to do that will crash your program, explanations will be later)
	JGFX::Texture fb = {pos2du16{600, 600}};
	JGFX::Texture tmp;
	JGFX::Texture txt2 = {"SomeTexture.png"};

	JGFX::Window win = { {600, 600}, "Example 2", txt2 };

	uint8 col = 0;
	prg.SetUniform("col", 1.f); //The .SetUniform() is a template function. It also supports GLM containers such as glm::vec3 or glm::mat4

	const JGFX::VertexLayout layout = {{0, JGFX::AttType<float>(), 3},
	                                   {1, JGFX::AttType<float>(), 3}};
	JGFX::VertexCollection vco = {{vertices, layout}, "vbo", JGFX::Indexate(3)};

	while (!win.should_close()) {
		JGFX::NewFrame();

		JGFX::Clear(128, 128, 128);

		tmp = fb;

    //.Bind() function will bind texture or a window as a framebuffer for all draw-related function calls
    //You can also use the JGFX::BindFramebuffer() function for that
		fb.Bind();

		JGFX::Clear(col += 16, 0, 0);
		JGFX::Draw(vco, tmp, prg);

		win.Bind();

    //You can attach multiple textures per draw call
		JGFX::Draw(vco, {{0, fb}, {1, txt2}}, prg);
	}
}
```
