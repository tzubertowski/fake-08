#include "audioOptimizations.h"

#ifdef ENABLE_AUDIO_OPTIMIZATIONS

#include <cmath>

// Static member definitions
fixed_t PerformanceOptimizations::s_freqLUT[64];
float PerformanceOptimizations::s_fmodLUT[101];
float PerformanceOptimizations::s_fabsLUT[201];
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
    
    s_initialized = true;
}

#endif // ENABLE_AUDIO_OPTIMIZATIONS