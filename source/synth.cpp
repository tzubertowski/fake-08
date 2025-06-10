//
//  ZEPTO-8 — Fantasy console emulator
//
//  Copyright © 2017—2020 Sam Hocevar <sam@hocevar.net>
//
//  This program is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#include "synth.h"
#include "audioOptimizations.h"

//#include <lol/noise> // lol::perlin_noise
#include <cmath>     // std::fabs, std::fmod
#include <algorithm> //std::min, std::max

//temp for printf debugging
//#include <stdio.h>

namespace z8
{

float synth::waveform(int instrument, float advance)
{
    using std::fabs;
    using std::fmod;

    //const from picolove:
    //local function note_to_hz(note)
	//  return 440 * 2 ^ ((note - 33) / 12)
    //end
    //local tscale = note_to_hz(63) / __sample_rate
    const float tscale = 0.11288053831187f;

    float t = fast_fmod(advance, 1.f);
    float ret = 0.f;

    // Multipliers were measured from PICO-8 WAV exports. Waveforms are
    // inferred from those exports by guessing what the original formulas
    // could be.
    switch (instrument)
    {
        case INST_TRIANGLE:
            // Optimized triangle wave using fast math
            return fast_div2(fast_fabs(fast_mul4(t) - 2.0f) - 1.0f);
        case INST_TILTED_SAW:
        {
            static float const a = 0.9f;
            static float const inv_a = 1.111111f; // 1/0.9
            static float const inv_1_minus_a = 10.0f; // 1/(1-0.9)
            ret = t < a ? fast_mul2(t) * inv_a - 1.0f
                        : fast_mul2(1.0f - t) * inv_1_minus_a - 1.0f;
            return fast_div2(ret);
        }
        case INST_SAW:
            return 0.653f * (t < 0.5f ? t : t - 1.0f);
        case INST_SQUARE:
            return t < 0.5f ? 0.25f : -0.25f;
        case INST_PULSE:
            return t < 0.33333333f ? 0.25f : -0.25f;
        case INST_ORGAN:
            ret = t < 0.5f ? 3.0f - fast_fabs(24.0f * t - 6.0f)
                           : 1.0f - fast_fabs(16.0f * t - 12.0f);
            return fast_div9(ret);
        case INST_NOISE:
        {
            // Spectral analysis indicates this is some kind of brown noise,
            // but losing almost 10dB per octave. I thought using Perlin noise
            // would be fun, but it’s definitely not accurate.
            //
            // This may help us create a correct filter:
            // http://www.firstpr.com.au/dsp/pink-noise/

            //TODO: not even doing zepto 8 noise here

            //static lol::perlin_noise<1> noise;
            //for (float m = 1.75f, d = 1.f; m <= 128; m *= 2.25f, d *= 0.75f)
            //    ret += d * noise.eval(lol::vec_t<float, 1>(m * advance));

            //ret = ((float)rand() / (float)RAND_MAX);

            //return ret * 0.4f;

            //picolove noise function in lua
            //zepto8 phi == picolove oscpos (x parameter in picolove generator func, advance in synth.cpp waveform function)
            //-- noise
            //osc[6] = function()
            //    local lastx = 0
            //    local sample = 0
            //    local lsample = 0
            //    local tscale = note_to_hz(63) / __sample_rate
            //1,041.8329
            //    return function(x)
            //        local scale = (x - lastx) / tscale
            //        lsample = sample
            //        sample = (lsample + scale * (math.random() * 2 - 1)) / (1 + scale)
            //        lastx = x
            //        return math.min(math.max((lsample + sample) * 4 / 3 * (1.75 - scale), -1), 1) *
            //            0.7
            //    end
            //end
            //printf("tscale: %f\n", tscale);
            //printf("advance: %f\n", advance);

            float scale = (advance - lastadvance) / tscale;
            //printf("scale: %f\n", scale);
            lsample = sample;
            sample = (lsample + scale * (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f)) / (1.0f + scale);
            //printf("sample: %f\n", sample);
            lastadvance = advance;
            float temp_val = (lsample + sample) * 1.33333333f * (1.75f - scale);
            float endval = std::min(std::max(temp_val, -1.0f), 1.0f) * 0.2f;
            //printf("endval: %f\n", endval);
            return endval;
        }
        case INST_PHASER:
        {   // This one has a subfrequency of freq/128 that appears
            // to modulate two signals using a triangle wave
            // FIXME: amplitude seems to be affected, too
            float k = fast_fabs(fast_mul2(fast_fmod(advance * 0.0078125f, 1.0f)) - 1.0f); // 1/128 = 0.0078125
            float u = fast_fmod(t + fast_div2(k), 1.0f);
            ret = fast_fabs(fast_mul4(u) - 2.0f) - fast_fabs(8.0f * t - 4.0f);
            return fast_div6(ret);
        }
    }

    return 0.0f;
}

} // namespace z8

