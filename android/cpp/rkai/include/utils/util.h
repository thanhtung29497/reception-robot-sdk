/******************************************************************************
*    Created on Fri Apr 01 2022
*   
*    Copyright (c) 2022 Rikkei AI.  All rights reserved.
*   
*    The material in this file is confidential and contains trade secrets
*    of Rikkei AI. This is proprietary information owned by Rikkei AI. No
*    part of this work may be disclosed, reproduced, copied, transmitted,
*    or used in any way for any purpose,without the express written 
*    permission of Rikkei AI
******************************************************************************/



#ifndef _SMARTROBOT_UTIL_H_
#define _SMARTROBOT_UTIL_H_

#include <stdbool.h>
#include <sys/time.h>
#include "rkai_type.h"
#include "rkai_image.h"
#include "rkai_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief load model from the file and return the model in binary format
 * 
 * @param model_path [in] path to model in the system memory
 * @param model_size [out] size of model binary array
 * @return rkai_ret_t [out] pointer to binary array
 */
unsigned char *rkai_util_load_model_from_file(char *model_path, int *model_size);

/**
 * @brief Common function for init model
 * 
 * @param handle Resource keeper
 * @param model_path Path of the model
 * @return rkai_ret_t 
 */
rkai_ret_t model_init(rkai_handle_t handle, const char *model_path);

rkai_ret_t init_yuv420p_table();

/**
 * @brief Convert YUV420SP_NV12 to RGB888
 * 
 * @param yuvbuffer [in][unsigned char *] buffer contain image in YUV420SP_NV12 pixel format
 * @param rga_buffer [out][unsigned char *] buffer to contain the convert output in RGB888 pixel format
 * @param width [int] image width
 * @param height [int] image height
 */
rkai_ret_t nv12_to_rgb24(unsigned char *yuvbuffer, unsigned char *rga_buffer, int width, int height);

/**
 * @brief Convert image in others pixel format back to RGB888
 * 
 * @param image [in][rkai_image_t *] Input image
 * @param rgb_buff [out][uint8_t *] buffer contain image in RGB888 pixel format
 */
rkai_ret_t convert_to_rgb24(rkai_image_t *image, uint8_t *rgb_buff);

/**
 * @brief Convert image in others pixel format back to RGB888
 * 
 * @param image [in][rkai_image_t *] Input image
 * @param rgb_buff [out][uint8_t *] buffer contain image in RGB888 pixel format
 */
rkai_ret_t convert_to_rgb24_v2(rkai_image_t *image, rkai_image_t *converted_image);

/**
 * @brief Resize RGB888 image
 * 
 * @param input_rgb [in][unsigned char *] Input image (RGB888 pixel format)
 * @param output_rgb [out][unsigned char *] Output image (RGB888 pixel format)
 * @param width [int] Image input width
 * @param height [int] Image input height
 * @param out_width [int] Desired image width
 * @param out_height [int] Desired image height
 * @return int
 */
rkai_ret_t rgb24_resize(unsigned char *input_rgb, unsigned char *output_rgb, int width, int height, int out_width, int out_height);

/**
 * @brief Process input image (Convert to RGB888 format and resize)
 * 
 * @param image [in][rkai_image_t *] Input image
 * @param output_resized_rgb [out][unsigned char *] Preprocessed image
 * @param req_width [int] Desired image width
 * @param req_height [int] Desired image height
 */
rkai_ret_t preprocess(rkai_image_t *image, unsigned char *output_resized_rgb, int req_width, int req_height);

/**
 * @brief Process input image (Convert to RGB888 format and resize) v2 
 *
 * @param image [in][rkai_image_t *] Input image
 * @param output_image [out][rkai_image_t] Preprocessed image
 * @param req_width [int] Desired image width
 * @param req_height [int] Desired image height
 */
rkai_ret_t preprocess_v2(rkai_image_t *image, rkai_image_t *output_image, int req_width, int req_height);

/**
 * @brief  Check if two float number are equal
 * 
 * @param a float number a
 * @param b float number b
 * @return true 
 * @return false 
 */
bool fequal(float a, float b);

/**
 * @brief Get new box give the current box with its new width and new height
 * @param current_box [in] Input box 
 * @param new_box [out] Output box
 * @param new_width  New width
 * @param new_height New height
 */
rkai_ret_t crop_new_box(rkai_rect_t *current_box, rkai_rect_t *new_box, int new_width, int new_height , int image_width, int image_height);


static inline int64_t get_current_time_us()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

rkai_ret_t load_config_file(const char *file_name, rkai_melspectrogram_config_t *config);

#ifdef __cplusplus
}
#endif
#endif //_SMARTROBOT_UTIL_H_
