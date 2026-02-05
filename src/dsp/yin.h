#pragma once
#include <vector>

struct YinResult {
  float hz = 0.0f;        // 0 means unvoiced
  float confidence = 0.0f; // higher is better (rough)
};

// frame: pointer to mono samples (length = frameSize)
// sampleRate: Hz
// minHz/maxHz: expected pitch range
// threshold: typical 0.10-0.20 (lower = stricter)
YinResult yin_pitch(const float* frame, int frameSize, int sampleRate,
                    float minHz, float maxHz, float threshold);
