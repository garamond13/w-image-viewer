#ifndef __HELPERS_HLSLI__
#define __HELPERS_HLSLI__

// Safe floating point comparations.
//

#define WIV_FLT_EPS 1e-6

inline bool is_equal(float a, float b)
{
	return abs(a - b) < WIV_FLT_EPS;
}

inline bool is_zero(float a)
{
	return abs(a) < WIV_FLT_EPS;
}

inline bool is_not_zero(float a)
{
	return abs(a) >= WIV_FLT_EPS;
}

//

// Rotates given texture coordinates by a given angle theta in degrees.
inline void rotate_texcoord(inout float2 texcoord, float theta)
{
    texcoord -= 0.5;
    theta = radians(theta);
    texcoord = float2(texcoord.x * cos(theta) + texcoord.y * sin(theta), -texcoord.x * sin(theta) + texcoord.y * cos(theta)) + 0.5;
}

#endif // __HELPERS_HLSLI__
