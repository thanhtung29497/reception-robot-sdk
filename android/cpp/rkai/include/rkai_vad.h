//
// Created by tannn on 1/8/24.
//

#ifndef SMARTROBOT_RKAI_VAD_H
#define SMARTROBOT_RKAI_VAD_H

#include <stdbool.h>
#include  <android/asset_manager_jni.h>
#include "rkai_type.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief This function will init the content of the handle.
 *
 * @param handle [in] rkai handle
 * @return @ref rkai_ret_t return code.
 */
rkai_ret_t rkai_init_vad_model(rkai_handle_t handle);
rkai_ret_t rkai_init_vad_android_model(rkai_handle_t handle, AAssetManager *asset_mgr);

rkai_melspectrogram_config_t rkai_get_vad_config();
/**
 * @brief This function will run an inference session to detect if the audio contains voice
 *
 * @param handle [in] rkai handle
 * @param audio [in] Input audio
 * @param vad_result [out] Output trigger word
 */
 rkai_ret_t rkai_vad_detect(rkai_handle_t handle, rkai_audio_t *audio,
                          rkai_melspectrogram_config_t vad_model_config,
                          rkai_vad_result_t *vad_result,
                          float low_threshold, float high_threshold);

 rkai_ret_t rkai_vad_postprocess(float *output,
                                 rkai_vad_result_t *vad_result,
                                 float low_threshold, float high_threshold);
#ifdef __cplusplus
};
#endif
#endif //SMARTROBOT_RKAI_VAD_H
