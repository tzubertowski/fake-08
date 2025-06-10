#include "audioOptimizations.h"

#ifdef ENABLE_AUDIO_OPTIMIZATIONS

#include <cmath>

// Static member definitions
fixed_t AudioOptimizations::s_freqLUT[64];
bool AudioOptimizations::s_initialized = false;

void AudioOptimizations::Initialize() {
    if (s_initialized) return;
    
    // Pre-compute frequency lookup table for all PICO-8 notes
    // Original formula: 440.0f * exp2((key - 33.0f) / 12.0f)
    for (int i = 0; i < 64; i++) {
        float freq = 440.0f * std::pow(2.0f, (i - 33.0f) / 12.0f);
        s_freqLUT[i] = FLOAT_TO_FIXED(freq);
    }
    
    s_initialized = true;
}

#endif // ENABLE_AUDIO_OPTIMIZATIONS