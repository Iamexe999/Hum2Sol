#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "audio/wav_reader.h"
#include <cstdint>
#include <stdexcept>

AudioBuffer read_wav_mono_f32(const std::string& path) {
    drwav wav{};
    if (!drwav_init_file(&wav, path.c_str(), nullptr)) {
    throw std::runtime_error("Failed to open WAV: " + path);
}

if (wav.channels < 1 || wav.sampleRate == 0) {
    drwav_uninit(&wav);
    throw std::runtime_error("Invalid WAV format.");
}

const uint64_t totalFrames = wav.totalPCMFrameCount;
std::vector<float> interleaved;
  interleaved.resize(static_cast<size_t>(totalFrames) * wav.channels);

uint64_t framesRead = drwav_read_pcm_frames_f32(&wav, totalFrames, interleaved.data());
drwav_uninit(&wav);

AudioBuffer out;
out.sampleRate = static_cast<int>(wav.sampleRate);
out.mono.resize(static_cast<size_t>(framesRead));

  // Downmix to mono
for (uint64_t i = 0; i < framesRead; ++i) {
    double sum = 0.0;
    for (uint32_t ch = 0; ch < wav.channels; ++ch) {
      sum += interleaved[static_cast<size_t>(i) * wav.channels + ch];
    }
    out.mono[static_cast<size_t>(i)] = static_cast<float>(sum / wav.channels);
}

return out;
}
