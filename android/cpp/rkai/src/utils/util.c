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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <rga/im2d.h>
#include <rga/rga.h>
#include "util.h"
#include "logger.h"
#include "rkai_image.h"
#include "rkai_type.h"

#include "android_porting/android_fopen.h"

unsigned char *rkai_util_load_model_from_file(char *model_path, int *model_size)
{
    // 1. Check if model exist or not
    AAsset *fp = android_fopen(model_path, "r");

    if (fp != NULL)
    {
        unsigned int model_len = android_ftell(fp);
        unsigned char *model = (unsigned char *)malloc(model_len);

        if (model_len != android_read(fp, model, model_len))
        {
            // TODO print log here
            LOG_ERROR("Failed to read binary data from model: %s \n", model_path);
            if (model != NULL)
            {
                free(model);
            }
            return NULL;
        }
        *model_size = model_len;
        if (fp)
        {
            android_close(fp);
        }
        return model;
    }
    else
    {
        LOG_ERROR("Model at %s is not exist. Please copy and rename model to %s", model_path, model_path);
        return NULL;
    }
}

rkai_ret_t model_init(rkai_handle_t handle, const char *model_path)
{
    int model_size;
    unsigned char *model_data = rkai_util_load_model_from_file(model_path, &model_size);

    if (model_data == NULL)
    {
        LOG_ERROR("Model data is null. \n");
        return RKAI_RET_COMMON_FAIL;
    }
    // Create context
    int rknn_ret_code;
    rknn_ret_code = rknn_init(&handle->context, model_data, model_size, RKNN_FLAG_PRIOR_HIGH, NULL);

    // Free model here
    if (model_data != NULL)
    {
        free(model_data);
    }

    if (rknn_ret_code != RKNN_SUCC)
    {
        LOG_ERROR("Cannot init model context, model path %s\n", model_path);
        return RKAI_RET_COMMON_FAIL;
    }

//     rknn_core_mask core_mask = RKNN_NPU_CORE_0_1_2;
//     rknn_ret_code = rknn_set_core_mask(handle->context, core_mask);
//     if (rknn_ret_code != RKNN_SUCC)
//     {
//         LOG_ERROR("Failed to set core mask . \n");
//         rknn_destroy(handle->context);
//         return RKAI_RET_COMMON_FAIL;
//     }
    // Query number of input and output
    rknn_ret_code = rknn_query(handle->context, RKNN_QUERY_IN_OUT_NUM, &handle->io_num, sizeof(handle->io_num));
    if (rknn_ret_code != RKNN_SUCC)
    {
        LOG_ERROR("Failed to query model input and output. Model path %s\n", model_path);
        rknn_destroy(handle->context);
        return RKAI_RET_COMMON_FAIL;
    }

    // Init the input tensor
    handle->input_tensor_attr_size = sizeof(rknn_tensor_attr) * handle->io_num.n_input;
    handle->input_tensor_attr = (rknn_tensor_attr *)malloc(handle->input_tensor_attr_size);
    memset(handle->input_tensor_attr, 0, handle->input_tensor_attr_size);

    for (int i = 0; i < handle->io_num.n_input; ++i)
    {   
        handle->input_tensor_attr[i].index = i;
        rknn_ret_code = rknn_query(handle->context, RKNN_QUERY_INPUT_ATTR, handle->input_tensor_attr + i,
                                   sizeof(rknn_tensor_attr));
        if (rknn_ret_code != RKNN_SUCC)
        {
            LOG_WARN("Query Input Tensor Attribute failed at %d: \n", i);
        }
    }

    // Init output tensor
    handle->output_tensor_attr_size = sizeof(rknn_tensor_attr) * handle->io_num.n_output;
    handle->output_tensor_attr = (rknn_tensor_attr *)malloc(handle->output_tensor_attr_size);
    memset(handle->output_tensor_attr, 0, handle->output_tensor_attr_size);

    for (int i = 0; i < handle->io_num.n_output; ++i)
    {   
        handle->output_tensor_attr[i].index = i;
        rknn_ret_code = rknn_query(handle->context, RKNN_QUERY_OUTPUT_ATTR, handle->output_tensor_attr + i,
                                   sizeof(rknn_tensor_attr));
        if (rknn_ret_code != RKNN_SUCC)
        {
            LOG_WARN("Query Output Tensor Attribute failed at %d \n", i);
        }
    }

    return RKAI_RET_SUCCESS;
}

static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024]; // for clip in CCIR601

rkai_ret_t init_yuv420p_table()
{
    long int crv, cbu, cgu, cgv;
    int i, ind;
    static int init = 0;

    if (init == 1)
        return RKAI_RET_COMMON_FAIL;

    crv = 104597;
    cbu = 132201; /* fra matrise i global.h */
    cgu = 25675;
    cgv = 53279;

    for (i = 0; i < 256; i++)
    {
        crv_tab[i] = (i - 128) * crv;
        cbu_tab[i] = (i - 128) * cbu;
        cgu_tab[i] = (i - 128) * cgu;
        cgv_tab[i] = (i - 128) * cgv;
        tab_76309[i] = 76309 * (i - 16);
    }

    for (i = 0; i < 384; i++)
        clp[i] = 0;
    ind = 384;
    for (i = 0; i < 256; i++)
        clp[ind++] = i;
    ind = 640;
    for (i = 0; i < 384; i++)
        clp[ind++] = 255;

    init = 1;
    return RKAI_RET_SUCCESS;
}

rkai_ret_t nv12_to_rgb24(unsigned char *yuvbuffer, unsigned char *rga_buffer,
                   int width, int height) {
  int y1, y2, u, v;
  unsigned char *py1, *py2;
  int i, j, c1, c2, c3, c4;
  unsigned char *d1, *d2;
  unsigned char *src_u;

  src_u = yuvbuffer + width * height; // u

  py1 = yuvbuffer; // y
  py2 = py1 + width;
  d1 = rga_buffer;
  d2 = d1 + 3 * width;

  init_yuv420p_table();

  for (j = 0; j < height; j += 2) {
    for (i = 0; i < width; i += 2) {
      u = *src_u++;
      v = *src_u++; // v immediately follows u, in the next position of u

      c4 = crv_tab[v];
      c2 = cgu_tab[u];
      c3 = cgv_tab[v];
      c1 = cbu_tab[u];

      // up-left
      y1 = tab_76309[*py1++];
      *d1++ = clp[384 + ((y1 + c4) >> 16)];
      *d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
      *d1++ = clp[384 + ((y1 + c1) >> 16)];

      // down-left
      y2 = tab_76309[*py2++];
      *d2++ = clp[384 + ((y2 + c4) >> 16)];
      *d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
      *d2++ = clp[384 + ((y2 + c1) >> 16)];
      // up-right
      y1 = tab_76309[*py1++];
      *d1++ = clp[384 + ((y1 + c4) >> 16)];
      *d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
      *d1++ = clp[384 + ((y1 + c1) >> 16)];

      // down-right
      y2 = tab_76309[*py2++];
      *d2++ = clp[384 + ((y2 + c4) >> 16)];
      *d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
      *d2++ = clp[384 + ((y2 + c1) >> 16)];
    }
    d1 += 3 * width;
    d2 += 3 * width;
    py1 += width;
    py2 += width;
  }
  return RKAI_RET_SUCCESS;
}

rkai_ret_t convert_to_rgb24(rkai_image_t *image, uint8_t *rgb_buff)
{
    rkai_ret_t ret = RKAI_NOT_SUPPORT; // Return if the pixel format is not supported
    switch (image->pixel_format)
    {
    case RKAI_PIXEL_FORMAT_YUV420SP_NV12:
        // nv12 to rbg24
        ret = nv12_to_rgb24(image->data, rgb_buff, image->width, image->height);
        return ret;
    case RKAI_PIXEL_FORMAT_RGB888:
        printf("image->width = %d, image->height = %d\n", image->width, image->height);
        memcpy(rgb_buff, image->data, image->width*image->height*3);
        return RKAI_RET_SUCCESS;
        break;
    case RKAI_PIXEL_FORMAT_BGR888:
        break;
    case RKAI_PIXEL_FORMAT_YUV420P_YU12:
        break;
    case RKAI_PIXEL_FORMAT_YUV420P_YV12:
        break;
    case RKAI_PIXEL_FORMAT_YUV420SP_NV21:
        break;
    case RKAI_PIXEL_FORMAT_YUV422P_YU16:
        break;
    case RKAI_PIXEL_FORMAT_YUV422P_YV16:
        break;
    case RKAI_PIXEL_FORMAT_YUV422SP_NV16:
        break;
    case RKAI_PIXEL_FORMAT_YUV422SP_NV61:
        break;
    default:
        break;
    }
}

/**
 * @brief Convert image in others pixel format back to RGB888
 * 
 * @param image [rkai_image_t *] Input image
 * @param rgb_buff [uint8_t] buffer contain image in RGB888 pixel format
 */
rkai_ret_t convert_to_rgb24_v2(rkai_image_t *image, rkai_image_t *converted_image)
{
    uint8_t *rgb_buff = (uint8_t*)malloc(image->width*image->height*3); // RGB Format
    if (rgb_buff == NULL)
    {
        LOG_ERROR("Memory allocation failed for rgb_buff\n");
        return RKAI_RET_COMMON_FAIL;
    }
    rkai_ret_t error_code = convert_to_rgb24(image, rgb_buff);

    if (error_code != RKAI_RET_SUCCESS)
    {
        free(rgb_buff);
        return error_code;
    }

    converted_image->data = rgb_buff;
    converted_image->is_prealloc_buf = 1;
    converted_image->width = image->width;
    converted_image->height = image->height;
    converted_image->size = image->width * image->height * 3;
    converted_image->pixel_format = RKAI_PIXEL_FORMAT_RGB888;
    converted_image->original_ratio = 1;
    return RKAI_RET_SUCCESS;
}

rkai_ret_t rgb24_resize(unsigned char *input_rgb, unsigned char *output_rgb, int width, int height, int out_width, int out_height)
{   
    rga_buffer_t src =
        wrapbuffer_virtualaddr(input_rgb, width, height, RK_FORMAT_RGB_888);
    rga_buffer_t dst = wrapbuffer_virtualaddr(output_rgb, out_width, out_height,
                                              RK_FORMAT_RGB_888);
    rga_buffer_t pat = {0};
    im_rect src_rect = {0, 0, width, height};
    im_rect dst_rect = {0, 0, out_width, out_height};
    im_rect pat_rect = {0};
    IM_STATUS STATUS = improcess(src, dst, pat, src_rect, dst_rect, pat_rect, 0);
    if (STATUS != IM_STATUS_SUCCESS)
    {
        LOG_ERROR("imcrop failed: %s\n", imStrError(STATUS));
        return RKAI_RET_COMMON_FAIL;
    }
    return RKAI_RET_SUCCESS;
}

rkai_ret_t preprocess(rkai_image_t *image, unsigned char *output_resized_rgb, int req_width, int req_height)
{
    rkai_ret_t ret = RKAI_RET_SUCCESS;
    if (image->width == req_width && image->height == req_height) 
    {
        // Input and output is same size, so we don't need to resize any more
        ret = convert_to_rgb24(image, output_resized_rgb);
        return ret;
    } else
    {
        int rgb_buffer_size = image->width * image->height * 3;
        unsigned char *rgb_buffer = malloc(rgb_buffer_size);
        if (rgb_buffer == NULL)
        {
            LOG_ERROR("Memory allocation failed for rgb_buffer\n");
            return RKAI_RET_COMMON_FAIL;
        }

        ret = convert_to_rgb24(image, rgb_buffer);
        if (ret != RKAI_RET_SUCCESS)
        {
            LOG_ERROR("Convert to RGB888 Failed \n");
            free(rgb_buffer);
            return ret;
        }
        // Resize
        rgb24_resize(rgb_buffer, output_resized_rgb, image->width, image->height, req_width, req_height);
        free(rgb_buffer);
        return ret;
    }
}
rkai_ret_t preprocess_v2(rkai_image_t *image, rkai_image_t *output_image, int req_width, int req_height)
{
    rkai_ret_t ret = RKAI_RET_SUCCESS;
    // Checking the case when the input image is RGB888 and also width == req_width and height == req_height
    if (image->width == req_width && image->height == req_height) 
    {
        // Input and output is same size, so we don't need to resize any more
        
        ret = convert_to_rgb24(image, output_image->data);
        output_image->size = image->size;
        output_image->width = image->width;
        output_image->height = image->height;
        output_image->pixel_format =RKAI_PIXEL_FORMAT_BGR888;
        output_image->is_prealloc_buf = 1;
        return ret;
    } else
    {
        int rgb_buffer_size = image->width * image->height * 3;
        unsigned char *rgb_buffer = malloc(rgb_buffer_size);
        if (rgb_buffer == NULL)
        {
            LOG_ERROR("Memory allocation failed for rgb_buffer\n");
            return RKAI_RET_COMMON_FAIL;
        }
        ret = convert_to_rgb24(image, rgb_buffer);
        if (ret != RKAI_RET_SUCCESS)
        {
            LOG_ERROR("Convert to RGB888 Failed \n");
            free(rgb_buffer);
            return ret;
        }
        
        // Convert to rgb
        rkai_image_t rgb_image;
        rgb_image.height = image->height;
        rgb_image.width = image->width;
        rgb_image.size = image->size;
        rgb_image.pixel_format = RKAI_PIXEL_FORMAT_RGB888;
        rgb_image.data = rgb_buffer;

        memcpy(rgb_image.data, rgb_buffer, rgb_buffer_size);
        rkai_size_t req_size = {.h = req_height, .w = req_width};
        rkai_image_resize_rgb(&rgb_image, req_size, output_image);

        free(rgb_buffer);
        return ret;
    }
}

bool fequal(float a, float b)
{
static float epsilon = 0.00001;
 return fabs(a-b) < epsilon;
}

rkai_ret_t crop_new_box(rkai_rect_t *current_box, rkai_rect_t *new_box, int new_width, int new_height , int image_width, int image_height)
{
    int box_width = current_box->right - current_box->left;
    int box_height = current_box->bottom - current_box->top;
    if (new_width > image_width|| new_height > image_height || new_width <= 0 || new_height <= 0)
    {
        LOG_WARN("Invalid new width or new height which is %d %d for box xmin %d ymin %d xmax %d ymax %d \n", new_width, new_height, current_box->left, 
                                                                                                            current_box->top, current_box->right, current_box->bottom);
        return RKAI_RET_INVALID_INPUT_PARAM;
    }
    if (box_width > new_width || box_height > new_height){
        LOG_WARN("The new width and height %d %d must be larger than the original box %d %d\n", new_width, new_height, image_width, image_height);
        return RKAI_RET_INVALID_INPUT_PARAM;
    }

    int left = current_box->left - (new_width - box_width)/2;
    int right = current_box->right + (new_width - box_width)/2;
    int top = current_box->top - (new_height - box_height)/2;
    int bottom = current_box->bottom + (new_height - box_height)/2;


    new_box->left = left > 0 ? left : 0;
    new_box->right = right < image_width ? right : image_width;
    new_box->top = top > 0 ? top : 0;
    new_box->bottom = bottom < image_height ? bottom : image_height;

    int cut_box_width = new_box->right - new_box->left;
    int cut_box_height = new_box->bottom - new_box->top;

    if (cut_box_width < new_width)
    {
        if (new_box->left == 0){
            new_box->right = new_box->right + (new_width - cut_box_width);
        } else {
            new_box->left = new_box->left - (new_width - cut_box_width);
        }
    }
    
    if (cut_box_height < new_height)
    {
        if (new_box->top == 0){
            new_box->bottom = new_box->bottom + (new_height - cut_box_height);
        } else {
            new_box->top = new_box->top - (new_height - cut_box_height);
        }
    }
    return RKAI_RET_SUCCESS;
}


rkai_ret_t load_config_file(const char *file_name, rkai_melspectrogram_config_t *config) {
    AAsset *fp = android_fopen(file_name, "r");
    char line[1024000];
    int count = 0;
    if (fp == NULL) {
        LOG_ERROR("Cannot open file %s \n", file_name);
        return RKAI_RET_COMMON_FAIL;
    }

    if (android_read(fp, line, 1024000) == -1) {
        LOG_ERROR("Cannot read file %s \n", file_name);
        return RKAI_RET_COMMON_FAIL;
    }

    char *token;
    token = strtok(line, "\n");
    while (token != NULL) {
        LOG_INFO("Read data from file --------- %s \n", token);
        // Check if start with #, it is comment so skip
        if (token[0] == '#') {
            token = strtok(NULL, "\n");
            count++;
            continue;
        }
        int sample_rate, n_fft, f_max, n_mels, win_len, n_hop, output_length, transpose_mel, htk, norm, norm_mel;
        double log_mel;
        if (sscanf(token, "%d %d %d %d %d %d %d %d %d %d %d %le", &sample_rate, &n_fft, &f_max, &n_mels, &win_len,
                   &n_hop, &output_length, &transpose_mel, &htk, &norm, &norm_mel, &log_mel) == 0) {
            LOG_ERROR("Cannot read data from file %s \n", file_name);
            return RKAI_RET_COMMON_FAIL;
        }
        config->sample_rate = sample_rate;
        config->n_fft = n_fft;
        config->f_max = f_max;
        config->n_mels = n_mels;
        config->win_length = win_len;
        config->hop_length = n_hop;
        config->output_size = output_length;
        config->transpose = transpose_mel;
        config->norm = norm;
        config->htk = htk;
        config->norm_mel = norm_mel;
        config->log_mel = log_mel;

        LOG_INFO("Read data from file --------- %d %d %d %d %d %d %d %d %d %d %d %le \n", sample_rate, n_fft, f_max,
                 n_mels, win_len, n_hop, output_length, transpose_mel, htk, norm, norm_mel, log_mel);
        token = strtok(NULL, "\n");
        count++;
    }
    android_close(fp);
    return RKAI_RET_SUCCESS;
}

