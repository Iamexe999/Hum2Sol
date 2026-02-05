#pragma once
#include <optional>

struct SegmentEvent {
    double timeSec = 0.0;
    float hz = 0.0f;
    int midi = -1;
};

class StableNoteSegmenter {
public:
  // hopSec: hop size in seconds (e.g., 0.01 for 10ms)
  // stableMs: base stability needed to accept a note
  // neighborHoldMs: extra time required to accept a ±1 semitone change (kills jitter)
StableNoteSegmenter(double hopSec, int stableMs, int neighborHoldMs = 500);

std::optional<SegmentEvent> update(double timeSec,
                                    float hz,
                                    int midi,
                                    bool voiced);

private:
    int framesForMs_(int ms) const;

    double hopSec_;
    int stableMs_;
    int neighborHoldMs_;

    int currentMidi_ = -9999;

    int candidateMidi_ = -9999;
    int candidateCount_ = 0;
};
