//
// Created by tannn on 1/5/24.
//

#ifndef SMARTROBOT_RKAI_TRIGGER_WORD_H
#define SMARTROBOT_RKAI_TRIGGER_WORD_H

#include<stdbool.h>
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
rkai_ret_t rkai_init_trigger_word_bc_model(rkai_handle_t handle);
rkai_ret_t rkai_init_trigger_word_android_bc_model(rkai_handle_t handle, AAssetManager *asset_mgr);
rkai_ret_t rkai_init_trigger_word_conv_model(rkai_handle_t handle);
rkai_ret_t rkai_init_trigger_word_android_conv_model(rkai_handle_t handle, AAssetManager *asset_mgr);
rkai_melspectrogram_config_t rkai_get_bc_config();
rkai_melspectrogram_config_t rkai_get_conv_config();

/**
 * @brief This function will run an inference session to detect if the audio contains trigger word
 *
 * @param handle [in] rkai handle
 * @param audio [in] Input audio
 * @param detected_trigger_word [out] Output trigger word
 */
rkai_ret_t rkai_trigger_word_detect(rkai_handle_t handle, rkai_audio_t *audio,
                                    rkai_melspectrogram_config_t config,
                                    rkai_trigger_word_result_t *detected_trigger_word,
                                    float low_threshold, float high_threshold);
#ifdef __cplusplus
};
#endif
#endif //SMARTROBOT_RKAI_TRIGGER_WORD_H
