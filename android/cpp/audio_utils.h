//
// Created by tannn on 1/8/24.
//

#ifndef SMARTROBOT_AUDIO_UTILS_H
#define SMARTROBOT_AUDIO_UTILS_H

#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>
#include <sstream>
#include <ios>
#include "logging_macros.h"


template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 64)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

template <typename K>
void fillArrayWithZeros(K *data, int32_t length) {

    size_t bufferSize = length * sizeof(K);
//    LOGI("Utils:: %s", "fillArrayWithZeros(): bufferSize = ");
//    LOGI("Utils:: %s", to_string_with_precision(bufferSize).c_str());
    memset(data, 0, bufferSize);
}

#endif //SMARTROBOT_AUDIO_UTILS_H
