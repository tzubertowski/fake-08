#include "audioOptimizations.h"

#ifdef ENABLE_AUDIO_OPTIMIZATIONS

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static member definitions
fixed_t PerformanceOptimizations::s_freqLUT[64];
float PerformanceOptimizations::s_fmodLUT[101];
float PerformanceOptimizations::s_fabsLUT[201];
fixed_t PerformanceOptimizations::s_div7LUT[8];
fixed_t PerformanceOptimizations::s_div183_LUT[256];
fixed_t PerformanceOptimizations::s_sineLUT[256];

// Graphics optimization lookup tables
uint8_t PerformanceOptimizations::s_setNibbleLowLUT[256][16];
uint8_t PerformanceOptimizations::s_setNibbleHighLUT[256][16];
uint8_t PerformanceOptimizations::s_getNibbleLowLUT[256];
uint8_t PerformanceOptimizations::s_getNibbleHighLUT[256];

bool PerformanceOptimizations::s_initialized = false;

void PerformanceOptimizations::Initialize() {
    if (s_initialized) return;
    
    // Pre-compute frequency lookup table for all PICO-8 notes
    // Original formula: 440.0f * exp2((key - 33.0f) / 12.0f)
    for (int i = 0; i < 64; i++) {
        float freq = 440.0f * std::pow(2.0f, (i - 33.0f) / 12.0f);
        s_freqLUT[i] = FLOAT_TO_FIXED(freq);
    }
    
    // Pre-compute fmod lookup table for range [0, 1] with 0.01 step
    for (int i = 0; i < 101; i++) {
        float x = i / 100.0f;
        s_fmodLUT[i] = x - (int)x;  // equivalent to fmod(x, 1.0f)
    }
    
    // Pre-compute fabs lookup table for range [-1, 1] with 0.01 step
    for (int i = 0; i < 201; i++) {
        float x = (i / 100.0f) - 1.0f;  // range from -1.0 to 1.0
        s_fabsLUT[i] = (x < 0) ? -x : x;  // equivalent to fabs(x)
    }
    
    // Pre-compute division by 7 lookup table for volume (0-7)
    for (int i = 0; i < 8; i++) {
        s_div7LUT[i] = FLOAT_TO_FIXED(i / 7.0f);
    }
    
    // Pre-compute division by 183 lookup table for audio timing
    for (int i = 0; i < 256; i++) {
        s_div183_LUT[i] = FLOAT_TO_FIXED(i / 183.0f);
    }
    
    // Pre-compute sine lookup table for vibrato (0 to 2π, 256 steps)
    for (int i = 0; i < 256; i++) {
        float angle = (i / 256.0f) * 2.0f * M_PI;
        s_sineLUT[i] = FLOAT_TO_FIXED(std::sin(angle));
    }
    
    // Graphics lookup tables removed - caused performance regression
    
    s_initialized = true;
}

#endif // ENABLE_AUDIO_OPTIMIZATIONS