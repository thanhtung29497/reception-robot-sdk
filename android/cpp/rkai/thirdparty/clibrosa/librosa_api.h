//
// Created by tannn on 1/5/24.
//

#ifndef SMARTROBOT_LIBROSA_API_H
#define SMARTROBOT_LIBROSA_API_H

#include <vector>
#include <complex>
#include <librosa.h>

namespace clibrosa {
    std::vector<std::vector<std::complex<float>>> stft(std::vector<float> &x, int n_fft, int win_len,
                                                                         int n_hop, const std::string &win,
                                                                         bool center, const std::string &mode) {
        return librosa::Feature::stft(x, n_fft, win_len, n_hop, win, center, mode);
    }

    std::vector<std::vector<float>> melspectrogram(std::vector<float> &x, int sr,
                                                   int n_fft, int win_len, int n_hop,
                                                   const std::string &win, bool center,
                                                   const std::string &mode, float power,
                                                   int n_mels, int fmin, int fmax,
                                                   bool htk, bool norm, bool mel_norm,
                                                   bool is_convert_transpose, double log_mel) {
        return librosa::Feature::melspectrogram(x, sr, n_fft, win_len, n_hop, win, center, mode,
                                                power, n_mels, fmin, fmax, htk, norm, mel_norm,
                                                is_convert_transpose, log_mel);
    }

    std::vector<std::vector<float>> mfcc(std::vector<float> &x, int sr,
                                         int n_fft, int win_len, int n_hop, const std::string &win,
                                         bool center, const std::string &mode,
                                         float power, int n_mels, int fmin, int fmax,
                                         int n_mfcc, bool norm, int type, bool htk, bool norm_mel) {
        return librosa::Feature::mfcc(x, sr, n_fft, win_len, n_hop, win, center, mode, power,
                                      n_mels,
                                      fmin, fmax, n_mfcc, norm, type, htk, norm_mel);
    }

} // namespace clibrosa

#endif //SMARTROBOT_LIBROSA_API_H
