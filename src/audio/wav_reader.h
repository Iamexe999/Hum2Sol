#pragma once
#include <string>
#include <vector>

struct AudioBuffer {
    int sampleRate = 0;
  std::vector<float> mono; // [-1, 1]
};

AudioBuffer read_wav_mono_f32(const std::string& path);
