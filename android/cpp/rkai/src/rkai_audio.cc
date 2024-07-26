//
// Created by tannn on 1/5/24.
//

#include <stdlib.h>
#include "librosa_api.h"
#include "utils/util.h"
#include "utils/logger.h"
#include "rkai_type.h"
#include "rkai_audio.h"

rkai_ret_t rkai_audio_release(rkai_audio_t *audio) {
    if (audio->data != NULL) {
        free(audio->data);
        audio->data = NULL;
    }
    return RKAI_RET_SUCCESS;
}

rkai_ret_t rkai_audio_melspectrogram_release(rkai_melspectrogram_t *melspectrogram) {
    if (melspectrogram->data != NULL) {
        free(melspectrogram->data);
        melspectrogram->data = NULL;
    }
    return RKAI_RET_SUCCESS;
}

rkai_ret_t rkai_audio_to_melspectrogram(rkai_audio_t *audio, rkai_melspectrogram_t *melspectrogram,
                                        rkai_melspectrogram_config_t config) {
    rkai_ret_t ret = RKAI_RET_SUCCESS;
    if (audio->format != RKAI_AUDIO_FORMAT_FLOAT) {
        LOG_WARN("Only support float format for audio \n");
        return RKAI_RET_INVALID_INPUT_PARAM;
    }
    // Convert to std::vector for input of librosa
    std::vector<float> audio_vector(audio->data, audio->data + audio->size);
    std::vector<std::vector<float>> melspectrogram_vector = clibrosa::melspectrogram(audio_vector,
                                                                                     config.sample_rate,
                                                                                     config.n_fft,
                                                                                     config.win_length,
                                                                                     config.hop_length,
                                                                                     "hann", true,
                                                                                     "reflect", 2.0,
                                                                                     config.n_mels,
                                                                                     0,
                                                                                     config.f_max,
                                                                                     (bool) config.htk,
                                                                                     (bool) config.norm,
                                                                                     (bool) config.norm_mel,
                                                                                     config.transpose,
                                                                                     config.log_mel);
    // Convert back to rkai_melspectrogram_t
    melspectrogram->size = melspectrogram_vector.size() * melspectrogram_vector[0].size();
    melspectrogram->n_mels = config.transpose == 0 ? melspectrogram_vector.size() : melspectrogram_vector[0].size() ;
    melspectrogram->n_frames = config.transpose ==0 ? melspectrogram_vector[0].size() : melspectrogram_vector.size();
    melspectrogram->data = (float *) malloc(melspectrogram->size * sizeof(float));
    if (melspectrogram->data == NULL) {
        LOG_ERROR("Cannot allocate memory for melspectrogram \n");
        return RKAI_RET_COMMON_FAIL;
    }
    for (int i = 0; i < melspectrogram_vector.size(); ++i) {
        for (int j = 0; j < melspectrogram_vector[i].size(); ++j) {
            melspectrogram->data[i * melspectrogram_vector[i].size() + j] = melspectrogram_vector[i][j];
        }
    }
    return ret;
}
