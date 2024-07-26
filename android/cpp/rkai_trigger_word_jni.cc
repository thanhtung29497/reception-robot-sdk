//
// Created by tannn on 1/5/24.
//
#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include "rkai.h"
#include "android_fopen.h"

rkai_handle_t trigger_word_bc_handle;
rkai_handle_t trigger_word_conv_handle;
rkai_melspectrogram_config_t triggerword_config_bc;
rkai_melspectrogram_config_t triggerword_config_conv;

static jclass objCls = NULL;
static jmethodID constructortorId;
static jfieldID scoreId;
static jfieldID passLowThresholdId;
static jfieldID passHighThresholdId;


extern "C"
{
JNIEXPORT jstring JNICALL Java_org_rikkei_smartrobot_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject, /* this */
        jobject assetManager) {
    std::string hello = "Please load audio file then press run";
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT void JNICALL Java_com_example_smart_1robot_TriggerWord_initModel(
        JNIEnv *env,
        jobject,
        jobject assetManager) {
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    android_fopen_set_asset_manager(mgr);
    trigger_word_bc_handle = rkai_create_handle();
    trigger_word_conv_handle = rkai_create_handle();
    rkai_init_trigger_word_android_bc_model(trigger_word_bc_handle, mgr);
    rkai_init_trigger_word_android_conv_model(trigger_word_conv_handle, mgr);
    triggerword_config_bc = rkai_get_bc_config();
    triggerword_config_conv = rkai_get_conv_config();

    jclass localObjCls = env->FindClass("com/example/smart_robot/TriggerWord$Obj");
    objCls = reinterpret_cast<jclass>(env->NewGlobalRef(localObjCls));

    constructortorId = env->GetMethodID(objCls, "<init>", "(Lcom/example/smart_robot/TriggerWord;)V");

    scoreId = env->GetFieldID(objCls, "score", "F");
    passLowThresholdId = env->GetFieldID(objCls, "passLowThreshold", "Z");
    passHighThresholdId = env->GetFieldID(objCls, "passHighThreshold", "Z");
    return;
}

JNIEXPORT jobject JNICALL Java_com_example_smart_1robot_TriggerWord_bcModelDetect(
        JNIEnv *env,
        jobject thiz,
        jfloatArray audio) {
    rkai_trigger_word_result_t bc_detected_trigger_word_result;
    //prepare audio
    rkai_audio_t input_audio;
    input_audio.data = env->GetFloatArrayElements(audio, 0);
    input_audio.size = env->GetArrayLength(audio);
    input_audio.sample_rate = 8000;
    input_audio.format = RKAI_AUDIO_FORMAT_FLOAT;
    input_audio.n_channels = 1;
    rkai_ret_t ret = rkai_trigger_word_detect(trigger_word_bc_handle, &input_audio,
                                              triggerword_config_bc,
                                              &bc_detected_trigger_word_result, 0.3, 0.6);
    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot run inference session for trigger word bc model  \n");
        return NULL;
    }
    jobject jObj = env->NewObject(objCls, constructortorId, thiz);
    env->SetFloatField(jObj, scoreId, bc_detected_trigger_word_result.score);
    env->SetBooleanField(jObj, passLowThresholdId, bc_detected_trigger_word_result.pass_low_conf);
    env->SetBooleanField(jObj, passHighThresholdId, bc_detected_trigger_word_result.pass_high_conf);
    return jObj;
}

JNIEXPORT jobject JNICALL Java_com_example_smart_1robot_TriggerWord_convModelDetect(
        JNIEnv *env,
        jobject thiz,
        jfloatArray audio) {
    rkai_trigger_word_result_t conv_detected_trigger_word_result;
    //prepare audio
    rkai_audio_t input_audio;
    input_audio.data = env->GetFloatArrayElements(audio, 0);
    input_audio.size = env->GetArrayLength(audio);
    input_audio.sample_rate = 8000;
    input_audio.format = RKAI_AUDIO_FORMAT_FLOAT;
    input_audio.n_channels = 1;
    rkai_ret_t ret = rkai_trigger_word_detect(trigger_word_conv_handle, &input_audio,
                                              triggerword_config_conv,
                                              &conv_detected_trigger_word_result, 0.6, 0.7);
    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot run inference session for trigger word conv model \n");
        return NULL;
    }
    jobject jObj = env->NewObject(objCls, constructortorId, thiz);
    env->SetFloatField(jObj, scoreId, conv_detected_trigger_word_result.score);
    env->SetBooleanField(jObj, passLowThresholdId, conv_detected_trigger_word_result.pass_low_conf);
    env->SetBooleanField(jObj, passHighThresholdId,
                         conv_detected_trigger_word_result.pass_high_conf);
    return jObj;
}
}
