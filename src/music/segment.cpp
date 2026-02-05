#include "music/segment.h"
#include <algorithm>
#include <cmath>

StableNoteSegmenter::StableNoteSegmenter(double hopSec, int stableMs, int neighborHoldMs)
    : hopSec_(hopSec > 0.0 ? hopSec : 0.01),
    stableMs_(std::max(1, stableMs)),
    neighborHoldMs_(std::max(0, neighborHoldMs)) {}

int StableNoteSegmenter::framesForMs_(int ms) const {
    const double sec = ms / 1000.0;
    return std::max(1, (int)std::lround(sec / hopSec_));
}

std::optional<SegmentEvent> StableNoteSegmenter::update(double timeSec,
                                                        float hz,
                                                        int midi,
                                                        bool voiced) {
if (!voiced || midi < 0) {
    candidateMidi_ = -9999;
    candidateCount_ = 0;
    return std::nullopt;
}

  // candidate persistence
if (midi == candidateMidi_) {
    candidateCount_++;
} else {
    candidateMidi_ = midi;
    candidateCount_ = 1;
}

  // If no current note yet, accept after base stability
if (currentMidi_ == -9999) {
    const int need = framesForMs_(stableMs_);
    if (candidateCount_ >= need) {
        currentMidi_ = candidateMidi_;
        return SegmentEvent{timeSec, hz, currentMidi_};
    }
    return std::nullopt;
}

if (candidateMidi_ == currentMidi_) {
    return std::nullopt; // no change
}

const int diff = std::abs(candidateMidi_ - currentMidi_);

  // Base requirement for any change
int need = framesForMs_(stableMs_);

  // If it's a ±1 semitone neighbor change, require much longer persistence
if (diff == 1) {
    need = std::max(need, framesForMs_(neighborHoldMs_));
}

if (candidateCount_ >= need) {
    currentMidi_ = candidateMidi_;
    return SegmentEvent{timeSec, hz, currentMidi_};
}

return std::nullopt;
}
