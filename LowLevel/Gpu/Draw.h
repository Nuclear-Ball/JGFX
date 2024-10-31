#pragma once
#include <string>
#include <stdexcept>
#include <glad/glad.h>

#include "../Common.h"


namespace LLJGFX{
	namespace Opt{
		enum OptFtr{ //Made as a regular enum to make less boilerplate
			BLEND,
			CULL,
			DEPTH_CLAMP,
			DEPTH_TEST,
			SCISSOR_TEST,
			STENCIL_TEST
		};
	}
	namespace Internal{
		bool opt_modes[] = {
				false, //blend
				false, //cull
				false, //depth_clamp
				false, //depth_test
				false, //scissor_test
				false  //stencil_test
		};
		namespace Gpu{
			namespace Opengl33{
				void Enable(Opt::OptFtr feature){
					const GLenum features[] = { GL_BLEND, GL_CULL_FACE, GL_DEPTH_CLAMP, GL_DEPTH_TEST,
					                            GL_SCISSOR_TEST, GL_STENCIL_TEST};

					glEnable(features[(int32)feature]);
				}
				void Disable(Opt::OptFtr feature){
					const GLenum features[] = { GL_BLEND, GL_CULL_FACE, GL_DEPTH_CLAMP, GL_DEPTH_TEST,
					                            GL_SCISSOR_TEST, GL_STENCIL_TEST};

					glDisable(features[(int32)feature]);
				}
			}
			namespace PreInit{
				void Enable(Opt::OptFtr feature){}
				void Disable(Opt::OptFtr feature){}
			}
			void(*Enable)(Opt::OptFtr feature) = PreInit::Enable;
			void(*Disable)(Opt::OptFtr feature) = PreInit::Disable;
		}
	}
}