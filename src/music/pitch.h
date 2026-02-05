#pragma once
#include <string>

int hz_to_midi(float hz);
int note_name_to_pc(const std::string& s); // C, C#, Db, ...
