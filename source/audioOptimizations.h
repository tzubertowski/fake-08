#pragma once

// Performance optimizations for MIPS platforms without hardware FPU
// Enable with -DENABLE_AUDIO_OPTIMIZATIONS

// Force enable optimizations for SF2000 platform
#ifdef SF2000
#ifndef ENABLE_AUDIO_OPTIMIZATIONS
#define ENABLE_AUDIO_OPTIMIZATIONS
#endif
#endif

#ifdef ENABLE_AUDIO_OPTIMIZATIONS

#include <cstdint>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Fixed-point configuration for performance
// Using Q16.16 format (16 bits integer, 16 bits fractional)
#define FIXED_POINT_SHIFT 16
#define FIXED_POINT_ONE (1 << FIXED_POINT_SHIFT)

typedef int32_t fixed_t;

// Conversion macros
#define FLOAT_TO_FIXED(f) ((fixed_t)((f) * FIXED_POINT_ONE))
#define FIXED_TO_FLOAT(x) ((float)(x) / FIXED_POINT_ONE)
#define INT_TO_FIXED(i) ((fixed_t)(i) << FIXED_POINT_SHIFT)
#define FIXED_TO_INT(x) ((int)((x) >> FIXED_POINT_SHIFT))

// Fixed-point arithmetic
#define FIXED_MUL(a, b) (((int64_t)(a) * (b)) >> FIXED_POINT_SHIFT)
#define FIXED_DIV(a, b) (((int64_t)(a) << FIXED_POINT_SHIFT) / (b))

// Fast modulo operations using bit operations where possible
#define FAST_MOD_2(x) ((x) & 1)
#define FAST_MOD_4(x) ((x) & 3)
#define FAST_MOD_8(x) ((x) & 7)
#define FAST_MOD_16(x) ((x) & 15)

// Fast fmod for small integer values (0.0 to 1.0 range)
inline float fast_fmod_1(float x) {
    return x - (int)x;
}

// Performance optimization class for caching expensive calculations
class PerformanceOptimizations {
private:
    // Pre-computed frequency table for all PICO-8 notes (0-63)
    static fixed_t s_freqLUT[64];
    
    // Pre-computed common fmod values for audio effects (0.0 to 1.0, step 0.01)
    static float s_fmodLUT[101];
    
    // Pre-computed fabs values for common audio range (-1.0 to 1.0, step 0.01)  
    static float s_fabsLUT[201];
    
    // Pre-computed division lookup tables for common audio constants
    static fixed_t s_div7LUT[8];      // Division by 7 for volume (0-7)
    static fixed_t s_div183_LUT[256]; // Division by 183 for audio timing
    
    // Pre-computed sine lookup table for vibrato (256 entries, 0-2π)
    static fixed_t s_sineLUT[256];
    
    // Graphics optimization lookup tables
    // Nibble manipulation tables for pixel operations
    static uint8_t s_setNibbleLowLUT[256][16];  // [original_byte][new_nibble] -> result
    static uint8_t s_setNibbleHighLUT[256][16]; // [original_byte][new_nibble] -> result
    static uint8_t s_getNibbleLowLUT[256];      // [byte] -> low nibble
    static uint8_t s_getNibbleHighLUT[256];     // [byte] -> high nibble
    
    static bool s_initialized;
    
public:
    // Initialize all lookup tables once
    static void Initialize();
    
    // Fast frequency lookup (avoids exp2 calculation)
    static inline fixed_t GetFrequency(uint8_t key) {
        if (!s_initialized) Initialize();
        return (key < 64) ? s_freqLUT[key] : 0;
    }
    
    // Convert fixed to float for compatibility with existing code
    static inline float GetFrequencyFloat(uint8_t key) {
        return FIXED_TO_FLOAT(GetFrequency(key));
    }
    
    // Fast fmod for values in range [0, 1] with 0.01 precision
    static inline float FastFmod1(float x) {
        if (!s_initialized) Initialize();
        if (x < 0.0f || x > 1.0f) return fmod(x, 1.0f); // fallback
        int index = (int)(x * 100.0f);
        if (index >= 101) index = 100;
        return s_fmodLUT[index];
    }
    
    // Fast fabs for values in range [-1, 1] with 0.01 precision
    static inline float FastFabs(float x) {
        if (!s_initialized) Initialize();
        if (x < -1.0f || x > 1.0f) return fabs(x); // fallback
        int index = (int)((x + 1.0f) * 100.0f);
        if (index >= 201) index = 200;
        if (index < 0) index = 0;
        return s_fabsLUT[index];
    }
    
    // Linear interpolation in fixed point
    static inline fixed_t Lerp(fixed_t a, fixed_t b, fixed_t t) {
        return a + FIXED_MUL(b - a, t);
    }
    
    // Float lerp for compatibility
    static inline float LerpFloat(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    // Fast integer division by common values
    static inline int FastDiv2(int x) { return x >> 1; }
    static inline int FastDiv4(int x) { return x >> 2; }
    static inline int FastDiv8(int x) { return x >> 3; }
    static inline int FastDiv16(int x) { return x >> 4; }
    
    // Fast division by 7 for volume calculations (0-7 input range)
    static inline float FastDiv7(uint8_t volume) {
        if (!s_initialized) Initialize();
        return (volume <= 7) ? FIXED_TO_FLOAT(s_div7LUT[volume]) : (volume / 7.0f);
    }
    
    // Fast division by 183 for audio timing
    static inline float FastDiv183(uint8_t speed) {
        if (!s_initialized) Initialize();
        return (speed < 256) ? FIXED_TO_FLOAT(s_div183_LUT[speed]) : (speed / 183.0f);
    }
    
    // Fast sine lookup for vibrato effects (input: 0-1 range)
    static inline float FastSine(float x) {
        if (!s_initialized) Initialize();
        int index = (int)(x * 255.0f);
        if (index >= 256) index = 255;
        if (index < 0) index = 0;
        return FIXED_TO_FLOAT(s_sineLUT[index]);
    }
    
    // Fast floor function for integer values
    static inline int FastFloor(float x) {
        return (int)x - (x < (int)x ? 1 : 0);
    }
    
    // Ultra-fast nibble operations using lookup tables
    static inline uint8_t FastSetNibbleLow(uint8_t original_byte, uint8_t new_nibble) {
        if (!s_initialized) Initialize();
        return s_setNibbleLowLUT[original_byte][new_nibble & 0x0F];
    }
    
    static inline uint8_t FastSetNibbleHigh(uint8_t original_byte, uint8_t new_nibble) {
        if (!s_initialized) Initialize();
        return s_setNibbleHighLUT[original_byte][new_nibble & 0x0F];
    }
    
    static inline uint8_t FastGetNibbleLow(uint8_t byte) {
        if (!s_initialized) Initialize();
        return s_getNibbleLowLUT[byte];
    }
    
    static inline uint8_t FastGetNibbleHigh(uint8_t byte) {
        if (!s_initialized) Initialize();
        return s_getNibbleHighLUT[byte];
    }
    
    // Fast multiplication by common audio constants
    static inline float FastMul2(float x) { return x + x; }
    static inline float FastMul4(float x) { return (x + x) + (x + x); }
    static inline float FastMul0_5(float x) { return x * 0.5f; }
    static inline float FastMul0_25(float x) { return x * 0.25f; }
    
    // Fast division by small integers using multiplication
    static inline float FastDiv2(float x) { return x * 0.5f; }
    static inline float FastDiv3(float x) { return x * 0.33333333f; }
    static inline float FastDiv6(float x) { return x * 0.16666667f; }
    static inline float FastDiv9(float x) { return x * 0.11111111f; }
    
    // Optimized min/max for audio range
    static inline float FastMin(float a, float b) { return (a < b) ? a : b; }
    static inline float FastMax(float a, float b) { return (a > b) ? a : b; }
    static inline float FastClamp(float x, float min, float max) { 
        return (x < min) ? min : ((x > max) ? max : x); 
    }
};

// Compatibility aliases for existing code
using AudioOptimizations = PerformanceOptimizations;

// Optimized key_to_freq function
inline float key_to_freq_optimized(float key) {
    if (key == (int)key && key >= 0 && key < 64) {
        // Use lookup table for integer keys
        return PerformanceOptimizations::GetFrequencyFloat((uint8_t)key);
    } else {
        // Fall back to original calculation for non-integer keys
        return 440.0f * std::pow(2.0f, (key - 33.0f) / 12.0f);
    }
}

// Optimized math function replacements
#define fast_fmod(x, y) ((y) == 1.0f ? PerformanceOptimizations::FastFmod1(x) : fmod(x, y))
#define fast_fabs(x) PerformanceOptimizations::FastFabs(x)
#define fast_div7(volume) PerformanceOptimizations::FastDiv7(volume)
#define fast_div183(speed) PerformanceOptimizations::FastDiv183(speed)
#define fast_sine(x) PerformanceOptimizations::FastSine(x)
#define fast_floor(x) PerformanceOptimizations::FastFloor(x)
#define fast_mul2(x) PerformanceOptimizations::FastMul2(x)
#define fast_mul4(x) PerformanceOptimizations::FastMul4(x)
#define fast_div2(x) PerformanceOptimizations::FastDiv2(x)
#define fast_div3(x) PerformanceOptimizations::FastDiv3(x)
#define fast_div6(x) PerformanceOptimizations::FastDiv6(x)
#define fast_div9(x) PerformanceOptimizations::FastDiv9(x)
#define fast_min(a, b) PerformanceOptimizations::FastMin(a, b)
#define fast_max(a, b) PerformanceOptimizations::FastMax(a, b)
#define fast_clamp(x, min, max) PerformanceOptimizations::FastClamp(x, min, max)

#else
// Basic optimizations even without full optimization suite
#define key_to_freq_optimized(key) (440.0f * std::pow(2.0f, ((key) - 33.0f) / 12.0f))
#define FAST_MOD_2(x) ((x) & 1)
#define FAST_MOD_4(x) ((x) & 3) 
#define FAST_MOD_8(x) ((x) & 7)
#define FAST_MOD_16(x) ((x) & 15)
#define fast_fmod(x, y) fmod(x, y)
#define fast_fabs(x) fabs(x)
#define fast_div7(volume) ((volume) / 7.0f)
#define fast_div183(speed) ((speed) / 183.0f)
#define fast_sine(x) sin((x) * 2.0f * M_PI)
#define fast_floor(x) floor(x)
#define fast_mul2(x) ((x) * 2.0f)
#define fast_mul4(x) ((x) * 4.0f)
#define fast_div2(x) ((x) * 0.5f)
#define fast_div3(x) ((x) / 3.0f)
#define fast_div6(x) ((x) / 6.0f)
#define fast_div9(x) ((x) / 9.0f)
#define fast_min(a, b) std::min(a, b)
#define fast_max(a, b) std::max(a, b)
#define fast_clamp(x, min, max) std::min(std::max(x, min), max)
#endif // ENABLE_AUDIO_OPTIMIZATIONS