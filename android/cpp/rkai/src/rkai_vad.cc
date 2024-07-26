//
// Created by tannn on 1/8/24.
//

#include <stdlib.h>
#include <string.h>
#include "utils/util.h"
#include "utils/logger.h"
#include "rkai.h"
#include "android_porting/android_fopen.h"
#include <vector>
#include "rkai_vad.h"

#define VAD_MODEL_PATH "model/vad/vad.rknn"
#define VAD_MODEL_INFORMATION_PATH "model/vad/vad_config.txt"

rkai_melspectrogram_config_t vad_model_config_t;

extern "C" rkai_ret_t rkai_init_vad_model(rkai_handle_t handle){
    rkai_ret_t ret = load_config_file(VAD_MODEL_INFORMATION_PATH, &vad_model_config_t);
    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot load config file for vad model \n");
        return RKAI_RET_COMMON_FAIL;
    }
    return model_init(handle, VAD_MODEL_PATH);
}

extern "C" rkai_ret_t rkai_init_vad_android_model(rkai_handle_t handle, AAssetManager *asset_mgr){
    rkai_ret_t ret = load_config_file(VAD_MODEL_INFORMATION_PATH, &vad_model_config_t);
    if (ret != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot load config file for vad model \n");
        return RKAI_RET_COMMON_FAIL;
    }
    return model_init(handle, VAD_MODEL_PATH);
}

extern "C" rkai_melspectrogram_config_t rkai_get_vad_config() {
    return vad_model_config_t;
}

extern "C" rkai_ret_t rkai_vad_detect(rkai_handle_t handle, rkai_audio_t *audio,
                                      rkai_melspectrogram_config_t vad_model_config,
                                      rkai_vad_result_t *vad_result,
                                      float low_threshold, float high_threshold) {
    int rknn_ret_code;
    rkai_ret_t rkai_ret_code = RKAI_RET_SUCCESS;

    // Convert audio waveform to mel spectrogram for vad model
    rkai_melspectrogram_t melspectrogram;
    rkai_ret_code = rkai_audio_to_melspectrogram(audio, &melspectrogram, vad_model_config);
    if (rkai_ret_code != RKAI_RET_SUCCESS){
        LOG_ERROR("Cannot convert audio to melspectrogram \n");
        return rkai_ret_code;
    }

    // Setup input for rknn model
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_FLOAT32;
    inputs[0].size = vad_model_config.output_size * sizeof(float);
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = melspectrogram.data;

    rknn_ret_code = rknn_inputs_set(handle->context, handle->io_num.n_input, inputs);
    if (rknn_ret_code != RKNN_SUCC) {
        LOG_WARN("Failed to Init vad detection input data. Return code of function rknn_input_set = $d\n", rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        rkai_audio_melspectrogram_release(&melspectrogram);
        return rkai_ret_code;
    }

    // model inference
    rknn_ret_code = rknn_run(handle->context, NULL);
    if (rknn_ret_code != RKNN_SUCC) {
        LOG_WARN("Failed to run vad detection model. Return code of function rknn_run = $d\n", rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        rkai_audio_melspectrogram_release(&melspectrogram);
        return rkai_ret_code;
    }

    // Get model output
    rknn_output outputs[handle->io_num.n_output];
    memset(outputs, 0, sizeof(outputs));
    for (int i = 0; i < handle->io_num.n_output; ++i) {
        outputs[i].want_float = true;
    }

    rknn_ret_code = rknn_outputs_get(handle->context, handle->io_num.n_output, outputs, NULL);
    if (rknn_ret_code != RKNN_SUCC) {
        LOG_WARN("Failed to get vad detection model output. Return code of function rknn_outputs_get = $d\n", rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        rkai_audio_melspectrogram_release(&melspectrogram);
        return rkai_ret_code;
    }

    float *vad_output = (float *) outputs[0].buf;
    rkai_ret_code = rkai_vad_postprocess(vad_output, vad_result, low_threshold, high_threshold);
    if (rkai_ret_code != RKAI_RET_SUCCESS) {
        LOG_ERROR("Cannot postprocess vad detection model output \n");
        rkai_audio_melspectrogram_release(&melspectrogram);
        return rkai_ret_code;
    }

    rkai_audio_melspectrogram_release(&melspectrogram);
    return rkai_ret_code;
}

std::vector<int> find_contiguous_regions(std::vector<bool> locations) {
    std::vector<int> regions;
    std::vector<bool> tmp1 = {locations.begin() + 1, locations.end()};
    std::vector<bool> tmp2 = {locations.begin(), locations.end() -1};

    for (int i = 0; i < tmp1.size(); ++i) {
        if (tmp1.at(i) xor tmp2.at(i)) {
            regions.push_back(i+1);
        }
    }

    if (locations.at(0)) {
        regions.insert(regions.begin(), 0);
    }

    if (locations.at(locations.size() - 1)) {
        regions.push_back(locations.size());
    }

    return regions;
}

rkai_ret_t rkai_vad_postprocess(float *output,
                                rkai_vad_result_t *vad_result,
                                float low_threshold, float high_threshold) {
    std::vector<float> score_time_stamp;
    for (int i = 0; i < 51; ++i) {
        score_time_stamp.push_back(output[2*i + 1]);
    }
    std::vector<int> high_locations;
    std::vector<bool> locations;

    for (int i = 0; i < score_time_stamp.size(); ++i) {
        if(score_time_stamp[i] > high_threshold) {
            high_locations.push_back(i);
        }

        if(score_time_stamp[i] > low_threshold) {
            locations.push_back(true);
        } else {
            locations.push_back(false);
        }
    }
    // Find the contiguous regions (continuous true values)
    std::vector<int> change_indexes = find_contiguous_regions(locations);
    std::vector<int> filtered_list;
    for (int i = 0; i < change_indexes.size(); i+=2) {
        int pair0 = change_indexes.at(i);
        int pair1 = change_indexes.at(i + 1);
        for (int j = 0; j < high_locations.size(); ++j) {
            if (pair0 <= high_locations.at(j) && high_locations.at(j) <= pair1) {
                filtered_list.push_back(pair0);
                filtered_list.push_back(pair1);
                break;
            }
        }
    }
    int total_voice = 0;
    for (int i = 0; i < filtered_list.size(); i = i + 2)
    {
        total_voice += filtered_list.at(i + 1) - filtered_list.at(i);
    }
    vad_result->conf = (float) total_voice / 51.0;
    vad_result->is_speech = 1 ? vad_result->conf > 0.4 : 0;
    return RKAI_RET_SUCCESS;
}