#include "music/pitch.h"
#include <cmath>
#include <map>

int hz_to_midi(float hz) {
if (hz <= 0.0f) return -1;
  float m = 69.0f + 12.0f * std::log2(hz / 440.0f);
return (int)std::lround(m);
}

int note_name_to_pc(const std::string& s) {
static const std::map<std::string, int> m = {
    {"C",0},{"B#",0},
    {"C#",1},{"Db",1},
    {"D",2},
    {"D#",3},{"Eb",3},
    {"E",4},{"Fb",4},
    {"F",5},{"E#",5},
    {"F#",6},{"Gb",6},
    {"G",7},
    {"G#",8},{"Ab",8},
    {"A",9},
    {"A#",10},{"Bb",10},
    {"B",11},{"Cb",11},
};
auto it = m.find(s);
return (it == m.end()) ? -1 : it->second;
}
