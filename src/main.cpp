#include "audio/wav_reader.h"
#include "dsp/yin.h"
#include "music/pitch.h"
#include "music/segment.h"
#include "music/solfege.h"
#include "music/smooth.h"

#include <cmath>
#include <cstring>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static bool arg_eq(const char* a, const char* b) { return std::strcmp(a, b) == 0; }

static void usage() {
std::cout
    << "Usage:\n"
    << "  hum2sol --in <file.wav> --mode movable|fixed [--tonic C|C#|Db|...]\n"
    << "Options:\n"
    << "  --frameMs 30      analysis window length\n"
    << "  --hopMs 10        analysis hop length\n"
    << "  --stableMs 60     min time before a note is emitted\n"
    << "  --minHz 50 --maxHz 1200\n"
    << "  --yinThresh 0.15\n"
    << "  --rmsGate 0.01\n"
    << "  --medianWin 5     median smoothing window (odd number recommended)\n";
}

int main(int argc, char** argv) {
try {
    std::string inPath;
    std::string mode = "movable";
    std::string tonic = "C";

    int frameMs = 30;
    int hopMs = 10;
    int stableMs = 60;
    float minHz = 50.0f;
    float maxHz = 1200.0f;
    float yinThresh = 0.15f;
    float rmsGate = 0.01f;
    int medianWin = 5;

    for (int i = 1; i < argc; ++i) {
        if (arg_eq(argv[i], "--in") && i + 1 < argc) inPath = argv[++i];
        else if (arg_eq(argv[i], "--mode") && i + 1 < argc) mode = argv[++i];
        else if (arg_eq(argv[i], "--tonic") && i + 1 < argc) tonic = argv[++i];
        else if (arg_eq(argv[i], "--frameMs") && i + 1 < argc) frameMs = std::stoi(argv[++i]);
        else if (arg_eq(argv[i], "--hopMs") && i + 1 < argc) hopMs = std::stoi(argv[++i]);
        else if (arg_eq(argv[i], "--stableMs") && i + 1 < argc) stableMs = std::stoi(argv[++i]);
        else if (arg_eq(argv[i], "--minHz") && i + 1 < argc) minHz = std::stof(argv[++i]);
        else if (arg_eq(argv[i], "--maxHz") && i + 1 < argc) maxHz = std::stof(argv[++i]);
        else if (arg_eq(argv[i], "--yinThresh") && i + 1 < argc) yinThresh = std::stof(argv[++i]);
        else if (arg_eq(argv[i], "--rmsGate") && i + 1 < argc) rmsGate = std::stof(argv[++i]);
        else if (arg_eq(argv[i], "--medianWin") && i + 1 < argc) medianWin = std::stoi(argv[++i]);
        else if (arg_eq(argv[i], "--help") || arg_eq(argv[i], "-h")) { usage(); return 0; }
        else {
        std::cerr << "Unknown arg: " << argv[i] << "\n";
        usage();
        return 1;
    }
    }

    if (inPath.empty()) { usage(); return 1; }

    const bool fixedDo = (mode == "fixed");
    const bool movableDo = (mode == "movable");
    if (!fixedDo && !movableDo) throw std::runtime_error("mode must be 'fixed' or 'movable'");

    int tonicPc = note_name_to_pc(tonic);
    if (movableDo && tonicPc < 0) throw std::runtime_error("Unknown tonic: " + tonic);
    if (fixedDo) tonicPc = 0; // C = Do

    if (medianWin < 1) medianWin = 1;
    if (medianWin % 2 == 0) medianWin += 1; // force odd window

    AudioBuffer audio = read_wav_mono_f32(inPath);

    const int sr = audio.sampleRate;
    const int frameSize = std::max(64, (int)std::lround(sr * (frameMs / 1000.0)));
    const int hopSize = std::max(16, (int)std::lround(sr * (hopMs / 1000.0)));

    const double hopSec = hopMs / 1000.0;
StableNoteSegmenter segmenter(hopSec, stableMs, 500); // 500ms neighbor hold

    std::vector<float> frame(frameSize);

    std::deque<int> midiHistory;

    for (int start = 0; start + frameSize <= (int)audio.mono.size(); start += hopSize) {
        for (int i = 0; i < frameSize; ++i) frame[i] = audio.mono[start + i];

        double sumSq = 0.0;
      for (float x : frame) sumSq += (double)x * (double)x;
        const float rms = (float)std::sqrt(sumSq / frameSize);
        const bool rmsVoiced = (rms >= rmsGate);

        const double t = (double)start / (double)sr;

    YinResult y = rmsVoiced ? yin_pitch(frame.data(), frameSize, sr, minHz, maxHz, yinThresh)
                            : YinResult{};

    const int midiRaw = hz_to_midi(y.hz);
    const bool voiced = rmsVoiced && (y.hz > 0.0f) && (midiRaw >= 0);

    int midi = midiRaw;

      // Median smoothing to reduce semitone jitter
    if (voiced) {
        midiHistory.push_back(midiRaw);
        while ((int)midiHistory.size() > medianWin) midiHistory.pop_front();

        std::vector<int> tmp(midiHistory.begin(), midiHistory.end());
        midi = median_int(std::move(tmp));
    } else {
        midiHistory.clear();
    }

    auto ev = segmenter.update(t, y.hz, midi, voiced);
    if (!ev) continue;

    const int pc = ((ev->midi % 12) + 12) % 12;
    const int rel = (pc - tonicPc + 12) % 12;
    const std::string sol = pc_to_solfege_chromatic(rel);

    std::cout << "t=" << ev->timeSec << "s"
                << "  Hz=" << ev->hz
                << "  MIDI=" << ev->midi
                << "  solfege=" << sol
                << "\n";
    }

    return 0;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}
}
