#ifndef __HELPERS_HLSLI__
#define __HELPERS_HLSLI__

//safe floating comparations
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

#endif // __HELPERS_HLSLI__
