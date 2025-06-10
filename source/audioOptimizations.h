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

#else
// Basic optimizations even without full optimization suite
#define key_to_freq_optimized(key) (440.0f * std::pow(2.0f, ((key) - 33.0f) / 12.0f))
#define FAST_MOD_2(x) ((x) & 1)
#define FAST_MOD_4(x) ((x) & 3) 
#define FAST_MOD_8(x) ((x) & 7)
#define FAST_MOD_16(x) ((x) & 15)
#define fast_fmod(x, y) fmod(x, y)
#define fast_fabs(x) fabs(x)
#endif // ENABLE_AUDIO_OPTIMIZATIONS