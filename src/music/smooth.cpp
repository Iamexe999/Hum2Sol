#include "music/smooth.h"
#include <algorithm>

int median_int(std::vector<int> v) {
    if (v.empty()) return -1;
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}
