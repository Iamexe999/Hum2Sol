#include "dsp/yin.h"
#include <algorithm>
#include <cmath>
#include <vector>

static inline float parabolic_interpolate(const std::vector<float>& v, int i) {
  // refine position around i using neighbors
    const int n = (int)v.size();
    if (i <= 0 || i >= n - 1) return (float)i;
    float x0 = v[i - 1], x1 = v[i], x2 = v[i + 1];
  float denom = (x0 - 2.0f * x1 + x2);
    if (std::fabs(denom) < 1e-12f) return (float)i;
  float delta = 0.5f * (x0 - x2) / denom;
    return (float)i + delta;
}

YinResult yin_pitch(const float* frame, int frameSize, int sampleRate,
                    float minHz, float maxHz, float threshold) {
    YinResult res{};

if (!frame || frameSize < 64 || sampleRate <= 0 || minHz <= 0 || maxHz <= 0 || minHz >= maxHz) {
        return res;
}

int maxTau = std::min(frameSize / 2, (int)std::floor((float)sampleRate / minHz));
    int minTau = std::max(2, (int)std::floor((float)sampleRate / maxHz));
    if (minTau >= maxTau) return res;

    std::vector<float> diff(maxTau + 1, 0.0f);
    std::vector<float> cmnd(maxTau + 1, 1.0f);

  // Difference function d(tau)
for (int tau = 1; tau <= maxTau; ++tau) {
    double sum = 0.0;
    for (int i = 0; i < frameSize - tau; ++i) {
        float d = frame[i] - frame[i + tau];
      sum += (double)d * (double)d;
    }
    diff[tau] = (float)sum;
}

  // Cumulative mean normalized difference function
double running = 0.0;
cmnd[0] = 1.0f;
for (int tau = 1; tau <= maxTau; ++tau) {
    running += diff[tau];
    cmnd[tau] = (running > 0.0) ? (float)(diff[tau] * tau / running) : 1.0f;
}

  // Absolute threshold search
int tauEstimate = -1;
for (int tau = minTau; tau <= maxTau; ++tau) {
    if (cmnd[tau] < threshold) {
      // pick local minimum
        while (tau + 1 <= maxTau && cmnd[tau + 1] < cmnd[tau]) tau++;
        tauEstimate = tau;
        break;
    }
}
if (tauEstimate < 0) return res;

float refinedTau = parabolic_interpolate(cmnd, tauEstimate);
if (refinedTau <= 0.0f) return res;

res.hz = (float)sampleRate / refinedTau;

  // crude confidence: how far under threshold we got
float v = cmnd[tauEstimate];
  res.confidence = std::max(0.0f, 1.0f - v); // not “real” probability, but useful
return res;
}
