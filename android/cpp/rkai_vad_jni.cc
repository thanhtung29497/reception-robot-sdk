//
// Created by tannn on 16/01/2024.
//
#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include "rkai.h"
#include "android_fopen.h"

rkai_handle_t mRkaiVadHandle = nullptr;
rkai_melspectrogram_config_t mVadModelConfig;

static jclass objCls = NULL;
static jmethodID constructortorId;
static jfieldID scoreId;
static jfieldID isSpeech;

extern "C"
{
JNIEXPORT void JNICALL Java_com_example_smart_1robot_VAD_initVADModel(
        JNIEnv *env,
        jobject,
        jobject assetManager) {
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    android_fopen_set_asset_manager(mgr);
    mRkaiVadHandle = rkai_create_handle();
    rkai_init_vad_android_model(mRkaiVadHandle, mgr);
    mVadModelConfig = rkai_get_vad_config();

    jclass localObjCls = env->FindClass("com/example/smart_robot/VAD$Obj");
    objCls = reinterpret_cast<jclass>(env->NewGlobalRef(localObjCls));

    constructortorId = env->GetMethodID(objCls, "<init>", "(Lcom/example/smart_robot/VAD;)V");

    scoreId = env->GetFieldID(objCls, "score", "F");
    isSpeech = env->GetFieldID(objCls, "isSpeech", "Z");

    return;
}

JNIEXPORT jobject JNICALL Java_com_example_smart_1robot_VAD_detectVAD(
        JNIEnv *env,
        jobject thiz,
        jfloatArray audio) {
    rkai_vad_result_t vad_result;
    //prepare audio
    rkai_audio_t input_audio;
    input_audio.data = env->GetFloatArrayElements(audio, 0);
    input_audio.size = env->GetArrayLength(audio);
    input_audio.sample_rate = mVadModelConfig.sample_rate;
    input_audio.format = RKAI_AUDIO_FORMAT_FLOAT;
    input_audio.n_channels = 1;
    rkai_ret_t ret = rkai_vad_detect(mRkaiVadHandle,
                                     &input_audio,
                                     mVadModelConfig,
                                     &vad_result,
                                     0.5,
                                     0.9);

    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot detect voice activity \n");
        rkai_audio_release(&input_audio);
        return nullptr;
    }

    jobject jObj = env->NewObject(objCls, constructortorId, thiz);
    env->SetFloatField(jObj, scoreId, vad_result.conf);
    env->SetBooleanField(jObj, isSpeech, vad_result.is_speech);
    return jObj;
}
}

