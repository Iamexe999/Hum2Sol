#include "music/solfege.h"

std::string pc_to_solfege_chromatic(int pc_rel) {
    static const char* names[12] = {
    "Do","Di","Re","Ri","Mi","Fa","Fi","So","Si","La","Li","Ti"
};
int pc = (pc_rel % 12 + 12) % 12;
return names[pc];
}
