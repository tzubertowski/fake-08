#pragma once

// Audio optimizations for MIPS platforms without hardware FPU
// Enable with -DENABLE_AUDIO_OPTIMIZATIONS

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

// Audio optimization class for caching expensive calculations
class AudioOptimizations {
private:
    // Pre-computed frequency table for all PICO-8 notes (0-63)
    static fixed_t s_freqLUT[64];
    static bool s_initialized;
    
public:
    // Initialize lookup tables once
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
    
    // Linear interpolation in fixed point
    static inline fixed_t Lerp(fixed_t a, fixed_t b, fixed_t t) {
        return a + FIXED_MUL(b - a, t);
    }
    
    // Float lerp for compatibility
    static inline float LerpFloat(float a, float b, float t) {
        return a + (b - a) * t;
    }
};

// Optimized key_to_freq function
inline float key_to_freq_optimized(float key) {
    if (key == (int)key && key >= 0 && key < 64) {
        // Use lookup table for integer keys
        return AudioOptimizations::GetFrequencyFloat((uint8_t)key);
    } else {
        // Fall back to original calculation for non-integer keys
        return 440.0f * std::pow(2.0f, (key - 33.0f) / 12.0f);
    }
}

#else
// No optimizations - use original functions
#define key_to_freq_optimized(key) (440.0f * std::pow(2.0f, ((key) - 33.0f) / 12.0f))
#endif // ENABLE_AUDIO_OPTIMIZATIONS