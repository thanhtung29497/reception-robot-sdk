//
// Created by tannn on 1/7/24.
//

#ifndef SMARTROBOT_SOUND_RECORDING_H
#define SMARTROBOT_SOUND_RECORDING_H

#include <cstdint>
#include <array>
#include <atomic>
#include <sndfile.hh>
#include <oboe/Definitions.h>
#include <android/log.h>
#include <string>
#include <ios>
#include <sstream>
#include "rkai.h"
//#include "logging_macros.h"
//#include "Utils.h"

//constexpr int kMaxSamples = 8000; // 1s of audio data @ 8kHz float
constexpr int kMaxSamples = 480000;

class SoundRecording {
public:
    int32_t write(const float *sourceData, int32_t numSamples);

    int32_t read(float *targetData, int32_t numSamples);

    void initiateWritingToFile(const char *outfilename, int32_t outputChannels, int32_t sampleRate);

    SndfileHandle createFile(const char *outfilename, int32_t outputChannels, int32_t sampleRate);

    void writeFile(SndfileHandle sndfileHandle);

    void readFileInfo(const char *fileName);

    bool isFull() const { return (mWriteIndex == kMaxSamples); };

    void setReadPositionToStart() { mReadIndex = 0; };

    void clear() { mWriteIndex = 0; };

    void setLooping(bool isLooping) { mIsLooping = isLooping; };

    int32_t getLength() const { return mWriteIndex; };

    int32_t getTotalSamples() const { return mTotalSamples; };

    void getData(float *targetData, uint start, uint end) {
        for (int i = start; i < end; ++i) {
            targetData[i-start] = mData[i];
        }
    }

    static const int32_t getMaxSamples() { return kMaxSamples; };
private:
    const char *TAG = "SoundRecording:: %s";

    std::atomic<int32_t> mIteration{1};
    std::atomic<int32_t> mWriteIndex{0};
    std::atomic<int32_t> mReadIndex{0};
    std::atomic<int32_t> mTotalSamples{0};
    std::atomic<bool> mIsLooping{false};

    float *mData = new float[kMaxSamples]{0};

    // 6 Decibels gain on audio signal
    int16_t gain_factor = 1;
};

#endif //SMARTROBOT_SOUND_RECORDING_H
