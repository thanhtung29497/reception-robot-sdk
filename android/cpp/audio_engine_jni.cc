//
// Created by tannn on 1/7/24.
//

#include <jni.h>
#include "audio_engine.h"
#include <android/asset_manager_jni.h>
#include "android_fopen.h"
#include "logging_macros.h"

const char *TAG = "AudioEngineJNI:: %s";
static AudioEngine *audioEngine = nullptr;

extern "C" {
JNIEXPORT jboolean JNICALL
Java_org_rikkei_smartrobot_AudioEngine_create(JNIEnv *env, jclass, jobject assetManager) {
    LOGD(TAG, "Create(): ");
    if (audioEngine == nullptr) {

        AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
        android_fopen_set_asset_manager(mgr);
        audioEngine = new AudioEngine(mgr);
    }
    return (audioEngine != nullptr);
}

JNIEXPORT void JNICALL
Java_org_rikkei_smartrobot_AudioEngine_delete(JNIEnv *env, jclass) {
    LOGD(TAG, "Delete(): ");
    delete audioEngine;
    audioEngine = nullptr;
}

JNIEXPORT void JNICALL
Java_org_rikkei_smartrobot_AudioEngine_startRecording(JNIEnv *env, jclass) {
    LOGD(TAG, "StartRecording(): ");
    if (audioEngine == nullptr) {
        LOGE(TAG, "Engine is null, please call create() first");
        return;
    }
    audioEngine->startRecording();
}

JNIEXPORT void JNICALL
Java_org_rikkei_smartrobot_AudioEngine_stopRecording(JNIEnv *env, jclass) {
    LOGD(TAG, "StopRecording(): ");
    if (audioEngine == nullptr) {
        LOGE(TAG, "Engine is null, please call create() first");
        return;
    }
    audioEngine->stopRecording();
}

JNIEXPORT void JNICALL
Java_org_rikkei_smartrobot_AudioEngine_startPlayingRecordedStream(JNIEnv *env, jclass) {
    LOGD(TAG, "StartPlayingRecordedStream(): ");
    if (audioEngine == nullptr) {
        LOGE(TAG, "Engine is null, please call create() first");
        return;
    }
    audioEngine->startPlayingRecordedStream();
}

JNIEXPORT void JNICALL
Java_org_rikkei_smartrobot_AudioEngine_stopPlayingRecordedStream(JNIEnv *env, jclass) {
    LOGD(TAG, "StopPlayingRecordedStream(): ");
    if (audioEngine == nullptr) {
        LOGE(TAG, "Engine is null, please call create() first");
        return;
    }
    audioEngine->stopPlayingRecordedStream();
}

JNIEXPORT void JNICALL
Java_org_rikkei_smartrobot_AudioEngine_writeFile(JNIEnv *env, jclass, jstring filePath) {
    LOGD(TAG, "WriteFile(): ");
    if (audioEngine == nullptr) {
        LOGE(TAG, "Engine is null, please call create() first");
        return;
    }
    const char *path;
    path = env->GetStringUTFChars(filePath, nullptr);
    audioEngine->writeToFile(path);
    env->ReleaseStringUTFChars(filePath, path);
}
JNIEXPORT void JNICALL
Java_org_rikkei_smartrobot_AudioEngine_startPlayingFromFile(JNIEnv *env, jclass, jstring filePath) {
    LOGD(TAG, "StartPlayingFromFile(): ");
    if (audioEngine == nullptr) {
        LOGE(TAG, "Engine is null, please call create() first");
        return;
    }
    const char *path;
    path = env->GetStringUTFChars(filePath, nullptr);
    audioEngine->startPlayingFromFile(path);
    env->ReleaseStringUTFChars(filePath, path);
}

JNIEXPORT void JNICALL
Java_org_rikkei_smartrobot_AudioEngine_stopPlayingFromFile(JNIEnv *env, jclass) {
    LOGD(TAG, "StopPlayingFromFile(): ");
    if (audioEngine == nullptr) {
        LOGE(TAG, "Engine is null, please call create() first");
        return;
    }
    audioEngine->stopPlayingFromFile();
}
}