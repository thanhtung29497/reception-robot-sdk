/******************************************************************************
*    Created on Wed Apr 20 2022
*   
*    Copyright (c) 2022 Rikkei AI.  All rights reserved.
*   
*    The material in this file is confidential and contains trade secrets
*    of Rikkei AI. This is proprietary information owned by Rikkei AI. No
*    part of this work may be disclosed, reproduced, copied, transmitted,
*    or used in any way for any purpose,without the express written 
*    permission of Rikkei AI
******************************************************************************/

#include <stdlib.h>
#include <opencv2/opencv.hpp>

#include "rkai_image.h"
#include "utils/logger.h"
#include "utils/util.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

//Private definition
#ifdef RKAI_IMAGE_DEBUG_INFO
void debug_print_image_info(rkai_image_t *image)
{
    LOG_INFO("Image Buffer size: %d\n", image->size);
    LOG_INFO("Image Is prealloc Buf: %d\n", image->is_prealloc_buf);
    LOG_INFO("Image Pixel Format: %d\n", image->pixel_format);
    LOG_INFO("Image Width: %d\n", image->width);
    LOG_INFO("Image Heigh: %d\n", image->height);
    LOG_INFO("Image Original Ratio: %d\n", image->original_ratio);
}
#endif
/**
 * Read image file (must be called after use @ref rkai_image_release to free allocated memory)
 * 
 * @param img_path [in] Image file path
 * @param image [out] Read Image
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_read(const char *img_path, rkai_image_t *image)
{
    rkai_ret_t error_code = RKAI_RET_SUCCESS;
    int IMAGE_CHANNEL_NUMBER = 3;
    int w = 0, h = 0, n = 0;

    unsigned char *data = stbi_load(img_path, &w, &h, &n , IMAGE_CHANNEL_NUMBER); // Only load r, g, b component

    if (data == NULL) {
        // Query failed message
        LOG_WARN("Failed to load image from %s, Error message %s\n", img_path, stbi_failure_reason());
        return RKAI_RET_THIRD_PARTY_FAIL;
    }

    image->data = data;
    image->is_prealloc_buf = 1;
    image->pixel_format = RKAI_PIXEL_FORMAT_RGB888;
    image->width = w;
    image->height = h;
    image->original_ratio = 1;
    image->size = w*h*IMAGE_CHANNEL_NUMBER;

#ifdef RKAI_IMAGE_DEBUG_INFO
    debug_print_image_info(image);
#endif

    return error_code;
}

/**
 * Save the raw data of the image as an RGB24 file
 * 
 * @param path [in] File path to write
 * @param image [in] Image to write
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_write(const char *path, rkai_image_t *image)
{
    rkai_ret_t error_code = RKAI_RET_SUCCESS;
    uint8_t IMAGE_CHANNEL_NUMBER = 3;
    uint8_t is_preallocation = 0;
    unsigned char *rgb_buf = NULL;
    if (image->pixel_format == RKAI_PIXEL_FORMAT_RGB888){
        //Save image
        rgb_buf = image->data;
    } else if(image->pixel_format == RKAI_PIXEL_FORMAT_YUV420SP_NV12)
    {
        rgb_buf = (unsigned char*)malloc(image->width*image->height*IMAGE_CHANNEL_NUMBER);
        is_preallocation = 1;
        if (rgb_buf == NULL)
        {
            LOG_WARN("Not enough memory to dynamically allocation for rgb buf");
            return RKAI_RET_COMMON_FAIL;
        }

        error_code = convert_to_rgb24(image, rgb_buf);

        if (error_code != RKAI_RET_SUCCESS)
        {
            LOG_WARN("Failed to convert from NV12 format to RGB24\n");
            free(rgb_buf);
            return RKAI_RET_COMMON_FAIL;
        }
    } else 
    {
        LOG_WARN("Passing invalid image format \n");
        // Not support others format
        return RKAI_RET_INVALID_INPUT_PARAM;
    }

    int write_error_code = stbi_write_jpg(path, image->width, image->height, 
                                            IMAGE_CHANNEL_NUMBER, rgb_buf, 100);
    // Free buffer if it is dynamically allocated in this function
    if (write_error_code == 0 ) {
        LOG_WARN("Cannot write image to disk with stbi_write_jpg. Error code %d\n", write_error_code);
        error_code = RKAI_RET_COMMON_FAIL;
    }

    if (is_preallocation){
        free(rgb_buf);
    }

    return error_code;
}


/**
 * Save the raw data of the image as an RGB24 file
 * 
 * @param path [in] File path to write
 * @param image [in] Image to write
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_write_raw(const char *path, rkai_image_t *image)
{
    FILE *fp = fopen(path, "wb");

    if (fp == NULL) {
        LOG_WARN("Failed to open file %s to write biniary data\n", path);
        return RKAI_RET_COMMON_FAIL;
    }
    fwrite(image->data, image->size, 1, fp);

    fclose(fp);

    return RKAI_RET_SUCCESS;
}

rkai_ret_t rkai_image_read_raw(const char *path, rkai_image_t *image, int width, int height)
{
    FILE *fp = fopen(path, "rb");

    if (fp == NULL) {
        LOG_WARN("Failed to open file %s to read biniary data\n", path);
        return RKAI_RET_COMMON_FAIL;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);  /* same as rewind(f); */

    unsigned char *data = (unsigned char *)malloc(fsize);
    if (data == NULL) {
        LOG_WARN("Failed to allocate memory for image data\n");
        return RKAI_RET_COMMON_FAIL;
    }
    fread(data, fsize, 1, fp);
    fclose(fp);

    image->data = data;
    image->is_prealloc_buf = 1;
    image->pixel_format = RKAI_PIXEL_FORMAT_RGB888;
    image->width = width;
    image->height = height;
    image->original_ratio = 1;
    image->size = fsize;
    return RKAI_RET_SUCCESS;
}

rkai_ret_t rkai_image_from_rgba_buffer(const char *raw, rkai_image_t *image, int width, int height)
{
    int fsize = width*height*3;
    unsigned char *data = (unsigned char *)malloc(fsize);
    if (data == NULL) {
        LOG_WARN("Failed to allocate memory for image data\n");
        return RKAI_RET_COMMON_FAIL;
    }

    for(int i = 0; i < height; ++i){
        for(int j = 0; j < width; ++j){
            data[i*width*3 + j*3 + 0] = raw[i*width*4 + j*4 + 0];
            data[i*width*3 + j*3 + 1] = raw[i*width*4 + j*4 + 1];
            data[i*width*3 + j*3 + 2] = raw[i*width*4 + j*4 + 2];
        }
    }

    image->data = data;
    image->is_prealloc_buf = 1;
    image->pixel_format = RKAI_PIXEL_FORMAT_RGB888;
    image->width = width;
    image->height = height;
    image->original_ratio = 1;
    image->size = fsize;
    return RKAI_RET_SUCCESS;
}
rkai_ret_t rkai_image_rgb_argb(rkai_image_t *src, rkai_image_t *image)
{
    int fsize = src->width*src->height*4;
    unsigned char *data = (unsigned char *)malloc(fsize);
    if (data == NULL) {
        LOG_WARN("Failed to allocate memory for image data\n");
        return RKAI_RET_COMMON_FAIL;
    }

    int width = src->width;
    int height = src->height;
    for(int i = 0; i < height; ++i){
        for(int j = 0; j < width; ++j){
            data[i*width*4 + j*4 + 0] = 255;
            data[i*width*4 + j*4 + 1] = src->data[i*width*3 + j*3 + 1];
            data[i*width*4 + j*4 + 2] = src->data[i*width*3 + j*3 + 2];
            data[i*width*4 + j*4 + 3] = 255;
        }
    }

    image->data = data;
    image->is_prealloc_buf = 1;
    image->pixel_format = RKAI_PIXEL_FORMAT_RGB888;
    image->width = width;
    image->height = height;
    image->original_ratio = 1;
    image->size = fsize;
    return RKAI_RET_SUCCESS;
}

/**
 * Free the dynamically allocated data in the @ref rkai_image_t object
 * 
 * @param image object to be freed
 * @return @ref rkai_ret_t 
 */
rkai_ret_t rkai_image_release(rkai_image_t *image)
{
    if (image->is_prealloc_buf)
    {
        free(image->data);
        return RKAI_RET_SUCCESS;
    } else {
        return RKAI_RET_INVALID_INPUT_PARAM;
    }
}


/**
 * Get the region submap of the image
 * 
 * @param image [in] Input image
 * @param box [in] crop area
 * @param roi_img [out] croped image
 * @return @ref rkai_ret_t
 */
// rkai_ret_t rkai_image_roi(rkai_image_t *image, rkai_rect_t *box, rkai_image_t *roi_img)
// {

//     // Check input box
//     if (box->top > image->height || box->bottom > image->height || box->top < 0 || box->bottom < 0||
//         box->left > image->width || box->right > image->width || box->left < 0 || box->right < 0) {
//             return RKAI_RET_INVALID_INPUT_PARAM;
//     }

//     // Check the input format
//     unsigned char *rgb_buffer = (unsigned char *)malloc(image->width*image->height*3);
//     if (rgb_buffer == NULL) {
//         LOG_ERROR("Failed to dynamic allocate buffer to covert to rgb format \n");
//         return RKAI_RET_COMMON_FAIL;
//     }

//     if (image->pixel_format != RKAI_PIXEL_FORMAT_RGB888){
//         rkai_ret_t return_code = convert_to_rgb24(image, rgb_buffer);

//         if (return_code != RKAI_RET_SUCCESS){
//             LOG_ERROR("Failed to convert to rgb format \n");
//             free(rgb_buffer);
//             return return_code;
//         }
//     } else {
//         memcpy(rgb_buffer, image->data, image->width * image->height * 3);
//     }
    
//     unsigned char *output_buffer = (unsigned char*)(malloc)((box->right - box->left) * (box->bottom - box->top) * 3);

//     for (int row = box->top; row < box->bottom; row ++){
//         memcpy(output_buffer + 3*(row - box->top)*(box->right - box->left), rgb_buffer + row*3*image->width + box->left*3, (box->right - box->left)*3);
//     }

//     roi_img->data = output_buffer;
//     roi_img->is_prealloc_buf = 1;
//     roi_img->original_ratio = 0;
//     roi_img->width = box->right - box->left;
//     roi_img->height = box->bottom - box->top;
//     roi_img->size = roi_img->width * roi_img->height *3;
//     roi_img->pixel_format = RKAI_PIXEL_FORMAT_RGB888;

//     free(rgb_buffer);
//     return RKAI_RET_SUCCESS;
// }

rkai_ret_t rkai_image_roi(rkai_image_t *image, rkai_rect_t *box, rkai_image_t *roi_img)
{
    // Check input box
    if (box->top > image->height || box->bottom > image->height || box->top < 0 || box->bottom < 0||
        box->left > image->width || box->right > image->width || box->left < 0 || box->right < 0) {
            return RKAI_RET_INVALID_INPUT_PARAM;
    }

    // Check the input format
    unsigned char *rgb_buffer = NULL;
    if (image->pixel_format != RKAI_PIXEL_FORMAT_RGB888){
        rgb_buffer = (unsigned char *)malloc(image->width*image->height*3);
        if (rgb_buffer == NULL) {
            LOG_ERROR("Failed to dynamic allocate buffer to covert to rgb format \n");
            return RKAI_RET_COMMON_FAIL;
        }
        rkai_ret_t return_code = convert_to_rgb24(image, rgb_buffer);

        if (return_code != RKAI_RET_SUCCESS){
            LOG_ERROR("Failed to convert to rgb format \n");
            free(rgb_buffer);
            return return_code;
        }
    } else {
        rgb_buffer = image->data;
    }
    
    unsigned char *output_buffer = (unsigned char*)(malloc)((box->right - box->left) * (box->bottom - box->top) * 3);

    for (int row = box->top; row < box->bottom; row ++){
        memcpy(output_buffer + 3*(row - box->top)*(box->right - box->left), rgb_buffer + row*3*image->width + box->left*3, (box->right - box->left)*3);
    }

    roi_img->data = output_buffer;
    roi_img->is_prealloc_buf = 1;
    roi_img->original_ratio = 0;
    roi_img->width = box->right - box->left;
    roi_img->height = box->bottom - box->top;
    roi_img->size = roi_img->width * roi_img->height *3;
    roi_img->pixel_format = RKAI_PIXEL_FORMAT_RGB888;

    if (rgb_buffer != image->data){
        free(rgb_buffer);
    }
    return RKAI_RET_SUCCESS;
}
//Crop face with new wdith and new height
rkai_ret_t rkai_image_crop_with_padding(rkai_image_t *image, rkai_rect_t *current_box, int new_width, int new_height, rkai_image_t *cropped_image)
{
    
    rkai_ret_t ret; 
    rkai_rect_t cut_box;
    ret = crop_new_box(current_box, &cut_box, new_width, new_height, image->width, image->height);
    if (ret != RKAI_RET_SUCCESS){
        LOG_WARN("Failed to crop new box\n");
        return ret;
    }
    ret = rkai_image_roi(image, &cut_box, cropped_image);
    if(ret != RKAI_RET_SUCCESS)
    {
        LOG_WARN("Failed to crop image\n");
        return ret;
    }
    return RKAI_RET_SUCCESS;
}

rkai_ret_t rkai_image_horizontal_flip(rkai_image_t *image)
{
    // Make sure it is rgb image
    if (image->pixel_format!=RKAI_PIXEL_FORMAT_RGB888)
    {
        LOG_WARN("Only support RGB888 format\n");
        return RKAI_RET_INVALID_INPUT_PARAM;
    }
    
    // Flip image horizontally
    unsigned char *rgb_buffer = NULL;
    int rows = image->height;
    int cols = image->width;
    int channels = 3;

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols/2; col++)
        {
            for (int channel = 0; channel < channels; channel++)
            {
                int index = row*cols*channels + col*channels + channel;
                int index2 = row*cols*channels + (cols - col - 1)*channels + channel;
                unsigned char temp = image->data[index];
                image->data[index] = image->data[index2];
                image->data[index2] = temp;
            }
        }
    }
    return RKAI_RET_SUCCESS;
}

// Fill a part of image with black
rkai_ret_t rkai_image_fill_row(rkai_image_t *image, int starting_row, rkai_image_t *dst_image)
{
    if (starting_row < 0 || starting_row >= image->height)
    {
        LOG_WARN("Invalid starting row\n");
        return RKAI_RET_COMMON_FAIL;
    }

    // Create new image with all black
    unsigned char *black_image = (unsigned char *)malloc(image->width*image->height*3);
    memset(black_image, 0, image->width*image->height*3);
    
    // int keeping_row = image->height - starting_row;
    memcpy(black_image, image->data, starting_row*image->width*3);

    // Reassign the image data
    dst_image->width = image->width;
    dst_image->height = image->height;
    dst_image->size = image->size;
    dst_image->pixel_format = image->pixel_format;
    dst_image->data = (unsigned char *)malloc(image->size);
    memcpy(dst_image->data, black_image, image->size);
    free(black_image);

    return RKAI_RET_SUCCESS;
}


/**
 * @brief Convert nv12 or others format to rgb format. This function will allocate new buffer for converted image. So, please release it after use
 * 
 * @param image [in] source image
 * @param converted_image [out] destination image in rgb format
 * @return rkai_ret_t 
 */
rkai_ret_t rkai_image_convert_to_rgb(rkai_image_t *image, rkai_image_t *converted_image)
{
    return convert_to_rgb24_v2(image, converted_image);
}

/**
 * @brief Resize RGB image
 * 
 * @param image [in] source image
 * @param desert_size [in] Desert output size
 * @param resized_image [out] destination image in rgb format
 * @return rkai_ret_t 
 */
rkai_ret_t rkai_image_resize_rgb(rkai_image_t *image, rkai_size_t desert_size, rkai_image_t *resized_image)
{
    //First of all, check the input pixel format
    if (image->pixel_format != RKAI_PIXEL_FORMAT_RGB888){
        return RKAI_RET_COMMON_FAIL;
    }
    float resize_ratio = desert_size.w/(float)desert_size.h;
    //Padding the image to ratio 4:3
    int padding_width;
    int padding_height;

    if (image->width  >= image->height * resize_ratio){
        padding_width = image->width;
        padding_height = (int)(image->width * 1/resize_ratio);
    } else {
        padding_width = (int)(image->height * resize_ratio);
        padding_height = image->height;
    }

    int padding_size = padding_width*padding_height*3;
    uint8_t *padding_buffer = (uint8_t*)malloc(padding_size);
    memset(padding_buffer, 0, padding_size);

    //Padding image to 4:3 image
    for (int i = 0; i < padding_height; ++i){
        if (i >= image->height) continue;
        memcpy(padding_buffer + i*padding_width*3, image->data + i*image->width*3, image->width*3);
    }

    resized_image->size = 3 * desert_size.w * desert_size.h;
    resized_image->data = (uint8_t*)malloc(resized_image->size);

    int error_code = stbir_resize(padding_buffer, padding_width, padding_height, 0,
                                resized_image->data, desert_size.w, desert_size.h, 0,
                                STBIR_TYPE_UINT8,
                                3, // Number of channel
                                STBIR_ALPHA_CHANNEL_NONE, //alpha_channel
                                0, //Flag
                                STBIR_EDGE_REFLECT, STBIR_EDGE_REFLECT,  //stbir edge
                                STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT,
                                STBIR_COLORSPACE_LINEAR, NULL);
    resized_image->width = desert_size.w;
    resized_image->height = desert_size.h;
    resized_image->is_prealloc_buf = 1;
    resized_image->pixel_format = RKAI_PIXEL_FORMAT_RGB888;
    resized_image->original_ratio = 1;
    resized_image->scale_ratio = (float) image->width / image->height;
    free(padding_buffer);
    if (error_code == 1){
        return RKAI_RET_SUCCESS;
    } 

    return RKAI_RET_COMMON_FAIL;
}

/**
 * Read image file (must be called after use @ref rkai_image_release to free allocated memory)
 * 
 * @param img_path [in] Image file path
 * @param image [out] Read Image
 * @return @ref rkai_ret_t
 */
//rkai_ret_t rkai_image_read_opencv_wrap(const char *img_path, rkai_image_t *image)
//{
//    cv::Mat input_image, input_image_rgb;
//    input_image = cv::imread(img_path);
//    cv::cvtColor(input_image, input_image_rgb, cv::COLOR_BGR2RGB);
//
//    image->pixel_format = RKAI_PIXEL_FORMAT_RGB888;
//    image->is_prealloc_buf = 1;
//    image->original_ratio = 1;
//    image->width = input_image_rgb.size().width;
//    image->height = input_image_rgb.size().height;
//    image->size = image->width * image->height * 3;
//    image->data = (unsigned char*)malloc(image->size);
//    memcpy(image->data, input_image_rgb.data, image->size);
//
//    return RKAI_RET_SUCCESS;
//}

//rkai_ret_t rkai_image_write_opencv_wrap(const char *img_path, rkai_image_t *image)
//{
//    // cv::Mat brg_img;
//    cv::Mat convertedImage(image->height, image->width, CV_8UC3, image->data);
//    // cv::cvtColor(convertedImage, brg_img,cv::COLOR_RGB2BGR);
//    cv::imwrite(img_path, convertedImage);
//    return RKAI_RET_SUCCESS;
//}
//extern "C" rkai_ret_t rkai_image_read_opencv(const char *img_path, rkai_image_t *image)
//{
//    return rkai_image_read_opencv_wrap(img_path, image);
//}
//
//extern "C" rkai_ret_t rkai_image_write_opencv(const char *img_path, rkai_image_t *image)
//{
//    return rkai_image_write_opencv_wrap(img_path, image);
//}


rkai_ret_t rkai_image_rgb2bgr(rkai_image_t *image)
{
    for(int i = 0; i < image->height; ++i){
        for(int j=0; j < image->width; ++j){
            //RGB = BRG
            // printf("%d \n", (i*image->width + j)*3 + 2);
            uint8_t tmp = image->data[(i*image->width + j)*3];
            image->data[(i*image->width + j)*3] = image->data[(i*image->width + j)*3 + 2];
            image->data[(i*image->width + j)*3 + 2] = tmp;
        }
    }

    return RKAI_RET_SUCCESS;
}

rkai_ret_t rkai_image_convert_to_rgb_opencv_wrap(rkai_image_t *image, rkai_image_t *converted_image)
{    
    cv::Mat output_image;
    if(image->pixel_format == RKAI_PIXEL_FORMAT_YUV420SP_NV12)
    {
        cv::Mat input_image(image->height*3/2, image->width, CV_8UC1, image->data);
        cv::cvtColor(input_image, output_image, cv::COLOR_YUV2RGB_NV12);
    } else {
        LOG_WARN("Not support this format\n");
        return RKAI_RET_INVALID_INPUT_PARAM;
    }
    
    converted_image->pixel_format = RKAI_PIXEL_FORMAT_RGB888;
    converted_image->is_prealloc_buf = 1;
    converted_image->original_ratio = 1;
    converted_image->width = output_image.size().width;
    converted_image->height = output_image.size().height;
    converted_image->size = converted_image->width * converted_image->height * 3;
    converted_image->data = (unsigned char*)malloc(converted_image->size);
    memcpy(converted_image->data, output_image.data, converted_image->size);

    return RKAI_RET_SUCCESS;
}

extern "C" rkai_ret_t rkai_image_convert_to_rgb_opencv(rkai_image_t *image, rkai_image_t *converted_image)
{
    return rkai_image_convert_to_rgb_opencv_wrap(image, converted_image);
}
