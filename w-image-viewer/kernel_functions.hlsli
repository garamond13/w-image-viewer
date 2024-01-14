#ifndef __KERNEL_FUNCTIONS_HLSLI__
#define __KERNEL_FUNCTIONS_HLSLI__

#include "corecrt_math_defines.hlsli"
#include "helpers.hlsli"

// Before including define USE_JINC_BASE macro if you want to use jinc based kernels.

// Math functions
//

// Bessel function of the first kind, order one. J1.
#define bessel_J1(x) ((x) < 2.2931157 ? (x) / 2.0 - (x) * (x) * (x) / 16.0 + (x) * (x) * (x) * (x) * (x) / 384.0 - (x) * (x) * (x) * (x) * (x) * (x) * (x) / 18432.0 : sqrt(M_2_PI / (x)) * (1.0 + 0.1875 / ((x) * (x)) - 0.1933594 / ((x) * (x) * (x) * (x))) * cos((x) - 3.0 * M_PI_4 + 0.375 / (x) - 0.1640625 / ((x) * (x) * (x))))

// Modified Bessel function of the first kind, order zero. I0.
#define bessel_I0(x) ((x) < 4.9706658 ? 1.0 + (x) * (x) / 4.0 + (x) * (x) * (x) * (x) / 64.0 + (x) * (x) * (x) * (x) * (x) * (x) / 2304.0 + (x) * (x) * (x) * (x) * (x) * (x) * (x) * (x) / 147456.0 : rsqrt(2.0 * M_PI * (x)) * exp(x))

//

// For all functions we assume x = abs(x).

// Base functions
//

// (b) is the kernel blur.

#ifdef USE_JINC_BASE
    // Jinc
    // Used for cylindrical resampling.
    // Normalized version: x == 0.0 ? 1.0 : 2.0 * bessel_J1(M_PI * x / blur) / (M_PI * x / blur).
    #define base(x,b) (is_zero(x) ? M_PI_2 / (b) : bessel_J1(M_PI / (b) * (x)) / (x))
#else
    // Sinc
    // Used for orhogonal resampling.
    // Normalized version: x == 0.0 ? 1.0 : sin(M_PI * x / blur) / (M_PI * x / blur).
    #define base(x,b) (is_zero(x) ? M_PI / (b) : sin(M_PI / (b) * (x)) / (x))
#endif

//


// Window functions
//

// (r) is the kernel radius.

// Normalized version sinc: x == 0.0 ? 1.0 : sin(M_PI * x / r) / (M_PI * x / r).
#define sinc(x,r) (is_zero(x) ? M_PI / (r) : sin(M_PI / (r) * (x)) / (x))

// Normalized version: x == 0.0 ? 1.0 : 2.0 * bessel_J1(M_PI * x / (radius * FIRST_JINC_ZERO)) / (M_PI * x / (radius * FIRST_JINC_ZERO)).
#define jinc(x,r) (is_zero(x) ? M_PI_2 / 1.2196699 / (r) : bessel_J1(M_PI / 1.2196699 / (r) * (x)) / (x))

#define hamming(x,r) (0.54 + 0.46 * cos(M_PI / (r) * (x)))

// Has to be satisfied: n >= 0.
// n = 0: Box window.
// n = 1: Cosine window.
// n = 2: Hann window.
#define power_of_cosine(x,r,n) (pow(cos(M_PI_2 / (r) * (x)), (n)))

// Kaiser(x, R, beta) == Kaiser(x, R, -beta).
// Normalized version is divided by bessel_I0(beta).
#define kaiser(x,r,beta) (bessel_I0((beta) * sqrt(1.0 - (x) * (x) / ((r) * (r)))))

// Has to be satisfied: n >= 0, m >= 0.
// If n == 0, should explicitly return 1 (Box window).
// n = 1.0, m = 1.0: Linear window.
// n = 2.0, m = 1.0: Welch window.
// n -> inf, m <= 1.0: Box window.
// m = 0.0: Box window.
// m = 1.0: Garamond window.
#define power_of_garamond(x,r,n,m) (pow(1.0 - pow((x) / (r), (n)), (m)))

// Has to be satisfied: n >= 0.
// Has to be satisfied: if n != 1, a <= 0.16.
// a = 0.0, n = 1.0: Hann window.
// a = 0.0, n = 0.5: Cosine window.
// n = 1.0: Blackman window.
// n = 0.0: Box window.
#define power_of_blackman(x,r,a,n) (pow((1.0 - (a)) / 2.0 + 0.5 * cos(M_PI / (r) * (x)) + (a) / 2.0 * cos(2.0 * M_PI / (r) * (x)), (n)))

// Has to be satisfied: s != 0, n >= 0.
// n = 2: Gaussian window.
// n -> inf: Box window.
#define generalized_normal_window(x,s,n) (exp(-pow((x) / (s), (n))))

// Has to be satisfied: eta != 2.
#define said(x,eta,chi) (cosh(sqrt(2.0 * (eta)) * M_PI * (chi) / (2.0 - (eta)) * (x)) * exp(-M_PI * M_PI * (chi) * (chi) / ((2.0 - (eta)) * (2.0 - (eta))) * (x) * (x)))

//

// Kernel functions
//

// Fixed radius 1.0.
#ifdef USE_JINC_BASE
    #define nearest_neighbor(x) ((x) > 0.5 * M_SQRT2 ? 0.0 : 1.0)
#else
    #define nearest_neighbor(x) ((x) > 0.5 ? 0.0 : 1.0)
#endif

#ifdef USE_JINC_BASE
    // Fixed radius sqrt(2.0).
    #define linear_kernel(x) (1.0 - (x) / M_SQRT2)
#else
    // Fixed radius 1.0.
    #define linear_kernel(x) (1.0 - (x))
#endif

// Fixed radius 2.0.
#define bicubic(x,a) ((x) < 1.0 ? ((a) + 2.0) * (x) * (x) * (x) - ((a) + 3.0) * (x) * (x) + 1.0 : (a) * (x) * (x) * (x) - 5.0 * (a) * (x) * (x) + 8.0 * (a) * (x) - 4.0 * (a))

// Fixed radius 2.0.
// Has to be satisfied: b != 0, b != 2, c != 0.
// c = 1.0: FSR kernel.
#define modified_fsr_kernel(x,b,c) ((1.0 / (2.0 * (b) - (b) * (b)) * ((b) / ((c) * (c)) * (x) * (x) - 1.0) * ((b) / ((c) * (c)) * (x) * (x) - 1.0) - (1.0 / (2.0 * (b) - (b) * (b)) - 1.0)) * (0.25 * (x) * (x) - 1.0) * (0.25 * (x) * (x) - 1.0))

// Fixed radius 2.0.
// Keys kernels: B + 2C = 1.
// B = 1.0, C = 0.0: Spline kernel.
// B = 0.0, C = 0.0: Hermite kernel.
// B = 0.0, C = 0.5: Catmull-Rom kernel.
// B = 1 / 3, C = 1 / 3: Mitchell kernel.
// B = 12 / (19 + 9 * sqrt(2)), C = 113 / (58 + 216 * sqrt(2)): Robidoux kernel.
// B = 6 / (13 + 7 * sqrt(2)), C = 7 / (2 + 12 * sqrt(2)): RobidouxSharp kernel.
// B = (9 - 3 * sqrt(2)) / 7, C = 0.1601886205085204: RobidouxSoft kernel.
// Normalized version is divided by 6, both functions.
#define bc_spline(x,b,c) ((x) < 1.0 ? (12.0 - 9.0 * (b) - 6.0 * (c)) * (x) * (x) * (x) + (-18.0 + 12.0 * (b) + 6.0 * (c)) * (x) * (x) + (6.0 - 2.0 * (b)) : (-(b) - 6.0 * (c)) * (x) * (x) * (x) + (6.0 * (b) + 30.0 * (c)) * (x) * (x) + (-12.0 * (b) - 48.0 * (c)) * (x) + (8.0 * (b) + 24.0 * (c)))

//

#endif // __KERNEL_FUNCTIONS_HLSLI__
