#pragma once
#include <cinttypes>
#include <glm/glm.hpp>
#include <iostream>

typedef int8_t int8;
typedef uint8_t uint8;

typedef int16_t int16;
typedef uint16_t uint16;

typedef int32_t int32;
typedef uint32_t uint32;

typedef int64_t int64;
typedef uint64_t uint64;

typedef float fl32;
typedef double fl64;

constexpr float rgb_ratio = 1.f / 256.f;
struct rgb {
    uint8 r{}, g{}, b{}, a{};

	rgb(uint8 r_ = 0, uint8 g_ = 0, uint8 b_ = 0, uint8 a_ = 255){ r = r_, g = g_, b = b_, a = a_;}

	inline rgb& operator=(uint32 hex){
		r = (hex & 0xff000000) >> 24;
		g = (hex & 0x00ff0000) >> 16;
		b = (hex & 0x0000ff00) >> 8;
		a = (hex & 0x000000ff);
		return *this;
	}
	inline explicit rgb(uint32 hex) { operator=(hex); }

	inline rgb& operator=(glm::vec4 normalized_color){
		r = (uint8)(normalized_color.r / rgb_ratio);
		g = (uint8)(normalized_color.g / rgb_ratio);
		b = (uint8)(normalized_color.b / rgb_ratio);
		a = (uint8)(normalized_color.a / rgb_ratio);
		return *this;
	}
	inline rgb(glm::vec4 normalized_color = { 0.f, 0.f, 0.f, 1.f }){ operator=(normalized_color); }

	inline rgb& operator=(glm::vec3 normalized_color) {
		operator=(glm::vec4{normalized_color.r, normalized_color.b, normalized_color.g, 1.f});
		return *this;
	}
	inline rgb(glm::vec3 normalized_color = { 0.f, 0.f, 0.f}){ operator=(normalized_color); }


	inline operator uint32() const {
		uint32 res = 0;
		res += (uint32)r << 24;
		res += (uint32)g << 16;
		res += (uint32)b << 8;
		res += a;

		return res;
	}
	inline operator glm::vec4() const {
		return {(fl32)r * rgb_ratio,
		        (fl32)g * rgb_ratio,
		        (fl32)b * rgb_ratio,
		        (fl32)a * rgb_ratio};
	}
};

typedef glm::vec<2, int8, glm::defaultp> pos2d8;
typedef glm::vec<2, uint8, glm::defaultp> pos2du8;
typedef glm::vec<2, int16, glm::defaultp> pos2d16;
typedef glm::vec<2, uint16, glm::defaultp> pos2du16;
typedef glm::vec<2, int32, glm::defaultp> pos2d32;
typedef glm::vec<2, uint32, glm::defaultp> pos2du32;
typedef glm::vec<2, int64, glm::defaultp> pos2d64;
typedef glm::vec<2, uint64, glm::defaultp> pos2du64;
typedef glm::vec<2, fl32, glm::defaultp> pos2df32;
typedef glm::vec<2, fl64, glm::defaultp> pos2df64;