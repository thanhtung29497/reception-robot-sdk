//
// Created by tannn on 1/5/24.
//

#ifndef SMARTROBOT_RKAI_AUDIO_H
#define SMARTROBOT_RKAI_AUDIO_H

#include "rkai_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read audio file (must be called after use @ref rkai_audio_release to free allocated memory)
 * @param audio
 * @return
 */
rkai_ret_t rkai_audio_release(rkai_audio_t *audio);

/**
 * @brief
 * @param melspectrogram
 * @return
 */
rkai_ret_t rkai_audio_melspectrogram_release(rkai_melspectrogram_t *melspectrogram);

/**
 * @brief Convert audio waveform to mel spectrogram
 * @param audio
 * @param melspectrogram
 * @param config
 * @return
 */

rkai_ret_t rkai_audio_to_melspectrogram(rkai_audio_t *audio, rkai_melspectrogram_t *melspectrogram, rkai_melspectrogram_config_t config);
#ifdef __cplusplus
};
#endif
#endif //SMARTROBOT_RKAI_AUDIO_H
