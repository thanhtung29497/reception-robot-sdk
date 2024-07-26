//
// Created by tannn on 1/5/24.
//

#include <stdlib.h>
#include <string.h>
#include "utils/util.h"
#include "utils/logger.h"
#include "rkai.h"
#include "android_porting/android_fopen.h"
#include "rkai_trigger_word.h"

//Private marco
#define TRIGGER_WORD_MODEL_PATH_BC "model/trigger_word/bc.rknn"
#define TRIGGER_WORD_MODEL_INFORMATION_PATH_BC "model/trigger_word/bc_config.txt"
#define TRIGGER_WORD_MODEL_PATH_CONV "model/trigger_word/conv.rknn"
#define TRIGGER_WORD_MODEL_INFORMATION_PATH_CONV "model/trigger_word/conv_config.txt"
#define BC_LOW_THRESHOLD 0.3
#define BC_HIGH_THRESHOLD 0.6
#define CONV_LOW_THRESHOLD 0.6
#define CONV_HIGH_THRESHOLD 0.8
rkai_melspectrogram_config_t trigger_word_model_config_bc;
rkai_melspectrogram_config_t trigger_word_model_config_conv;

extern "C" rkai_ret_t rkai_init_trigger_word_bc_model(rkai_handle_t handle) {
    rkai_ret_t ret = load_config_file(TRIGGER_WORD_MODEL_INFORMATION_PATH_BC,
                                      &trigger_word_model_config_bc);
    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot load config file for trigger word model 1 \n");
        return RKAI_RET_COMMON_FAIL;
    }
    return model_init(handle, TRIGGER_WORD_MODEL_PATH_BC);
}

extern "C" rkai_ret_t rkai_init_trigger_word_android_bc_model(rkai_handle_t handle, AAssetManager *asset_mgr) {
    rkai_ret_t ret = load_config_file(TRIGGER_WORD_MODEL_INFORMATION_PATH_BC,
                                      &trigger_word_model_config_bc);
    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot load config file for trigger word model 1 \n");
        return RKAI_RET_COMMON_FAIL;
    }
    return model_init(handle, TRIGGER_WORD_MODEL_PATH_BC);
}

extern "C" rkai_ret_t rkai_init_trigger_word_conv_model(rkai_handle_t handle) {
    rkai_ret_t ret = load_config_file(TRIGGER_WORD_MODEL_INFORMATION_PATH_CONV,
                                      &trigger_word_model_config_conv);
    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot load config file for trigger word model 1 \n");
        return RKAI_RET_COMMON_FAIL;
    }
    return model_init(handle, TRIGGER_WORD_MODEL_PATH_CONV);
}

extern "C" rkai_ret_t rkai_init_trigger_word_android_conv_model(rkai_handle_t handle, AAssetManager *asset_mgr) {
    rkai_ret_t ret = load_config_file(TRIGGER_WORD_MODEL_INFORMATION_PATH_CONV,
                                      &trigger_word_model_config_conv);
    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot load config file for trigger word model 1 \n");
        return RKAI_RET_COMMON_FAIL;
    }
    return model_init(handle, TRIGGER_WORD_MODEL_PATH_CONV);
}

extern "C" rkai_melspectrogram_config_t rkai_get_bc_config() {
    return trigger_word_model_config_bc;
}

extern "C" rkai_melspectrogram_config_t rkai_get_conv_config() {
    return trigger_word_model_config_conv;
}

extern "C" rkai_ret_t
rkai_trigger_word_detect(rkai_handle_t handle, rkai_audio_t *audio,
                         rkai_melspectrogram_config_t trigger_word_model_config,
                         rkai_trigger_word_result_t *detected_trigger_word,
                         float low_threshold, float high_threshold) {
    int rknn_ret_code;
    rkai_ret_t rkai_ret_code = RKAI_RET_SUCCESS;

    // Convert audio waveform to mel spectrogram for bc model
    rkai_melspectrogram_t melspectrogram;
    rkai_ret_code = rkai_audio_to_melspectrogram(audio, &melspectrogram, trigger_word_model_config);
    if (rkai_ret_code != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot convert audio to melspectrogram for bc_model \n");
        rkai_audio_melspectrogram_release(&melspectrogram);
        return rkai_ret_code;
    }

    // Setup input for rknn model
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_FLOAT32;
    inputs[0].size = trigger_word_model_config.output_size * sizeof(float);
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = melspectrogram.data;

    rknn_ret_code = rknn_inputs_set(handle->context, handle->io_num.n_input, inputs);
    if (rknn_ret_code != RKNN_SUCC) {
        LOG_WARN("Failed to Init trigger word detection input data. Return code of function rknn_input_set = $d\n", rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        rkai_audio_melspectrogram_release(&melspectrogram);
        return rkai_ret_code;
    }

    // bc model inference
    rknn_ret_code = rknn_run(handle->context, NULL);
    if (rknn_ret_code != RKNN_SUCC) {
        LOG_WARN("Failed to run inference. rknn_run return code = %d\n", rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        rkai_audio_melspectrogram_release(&melspectrogram);
        return rkai_ret_code;
    }

    //get bc model output
    rknn_output outputs[handle->io_num.n_output];
    memset(outputs, 0, sizeof(outputs));
    for (int i = 0; i < handle->io_num.n_output; i++) {
        outputs[i].want_float = 1;
    }

    rknn_ret_code = rknn_outputs_get(handle->context, handle->io_num.n_output, outputs, NULL);
    if (rknn_ret_code != RKNN_SUCC) {
        LOG_WARN("Failed to get output after inference, rknn_outputs_get return code %d \n", rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        rkai_audio_melspectrogram_release(&melspectrogram);
        return rkai_ret_code;
    }

    float *output_confidence = (float *) outputs[0].buf;
    detected_trigger_word->score = output_confidence[1];
    detected_trigger_word->pass_low_conf = detected_trigger_word->score > low_threshold;
    detected_trigger_word->pass_high_conf = detected_trigger_word->score > high_threshold;

    rkai_ret_code = rkai_audio_melspectrogram_release(&melspectrogram);
    if (rkai_ret_code != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot release melspectrogram \n");
        return rkai_ret_code;
    }
    return rkai_ret_code;
}