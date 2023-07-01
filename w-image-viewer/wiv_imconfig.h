#pragma once

#define IM_VEC2_CLASS_EXTRA  \
	constexpr ImVec2& operator+=(const ImVec2& other) \
	{ \
		x += other.x; \
		y += other.y; \
		return *this; \
	}