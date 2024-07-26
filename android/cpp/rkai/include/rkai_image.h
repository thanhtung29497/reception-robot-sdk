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

#ifndef _SMARTROBOT_RKAI_IMAGE_H_
#define _SMARTROBOT_RKAI_IMAGE_H_

#include "rkai_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Read image file (must be called after use @ref rkai_image_release to free allocated memory)
 * 
 * @param img_path [in] Image file path
 * @param image [out] Read Image
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_read(const char *img_path, rkai_image_t *image);

/**
 * Save image as image file (support jpg)
 * 
 * @param path [in] File path to write
 * @param image [in] Image to write. Only support RGB pixel format
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_write(const char *path, rkai_image_t *image);

/**
 * Save the raw data of the image as an RGB24 file
 * 
 * @param path [in] File path to write
 * @param image [in] Image to write
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_write_raw(const char *path, rkai_image_t *image);

/**
 * Read the raw data of the image as an RGB24 file
 * 
 * @param path [in] File path to write
 * @param image [in] Image to write
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_read_raw(const char *path, rkai_image_t *image, int width, int height);
rkai_ret_t rkai_image_from_rgba_buffer(const char *raw, rkai_image_t *image, int width, int height);

/**
 * Free the dynamically allocated data in the @ref rkai_image_t object
 * 
 * @param image object to be freed
 * @return @ref rkai_ret_t 
 */
rkai_ret_t rkai_image_release(rkai_image_t *image);

/**
 * rotate, flip channels on images
 * 
 * @param src [in] Source image
 * @param dst [out] Destination image
 * @param mode [in] transform mode
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_convert(rkai_image_t *src, rkai_image_t *dst, rkai_image_transform_mode mode);
rkai_ret_t rkai_image_rgb_argb(rkai_image_t *src, rkai_image_t *dst);


/**
 * Get the memory size of the image
 * 
 * @param image [in] Image
 * @return @ref rkai_ret_t
 */
int rkai_image_size(rkai_image_t *image);

/**
 * Get the region submap of the image
 * 
 * @param image [in] Input image
 * @param box [in] crop area
 * @param roi_img [out] croped image
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_roi(rkai_image_t *image, rkai_rect_t *box, rkai_image_t *roi_img);

/**
 * @brief Crop the a region of the image with new size for the boungidng box
 * 
 * @param image [in] Input image
 * @param current_box Input bounding box
 * @param new_width New width of the cropped image
 * @param new_height New height of the cropped image
 * @param cropped_image [out] Cropped image
 * @return rkai_ret_t 
 */
rkai_ret_t rkai_image_crop_with_padding(rkai_image_t *image, rkai_rect_t *current_box, int new_width, int new_height, rkai_image_t *cropped_image);

/**
 * @brief Flip the image horizontally
 * 
 * @param image [in] Input image
 * @param dst [out] Flipped image
 * @return rkai_ret_t 
 */
rkai_ret_t rkai_image_horizontal_flip(rkai_image_t *image);


rkai_ret_t rkai_image_fill_row(rkai_image_t *image, int starting_row, rkai_image_t *dst_image);

/**
 * Draw a circle on the image (only RGB/GRAY8 images are supported)
 * 
 * @param image [in] Input image
 * @param point [in] The coordinates of the center of the circle
 * @param radius [in] radius
 * @param color [in] circle color
 * @param thickness [in] line thickness
 * @return @ref rkai_ret_t
 */

rkai_ret_t rkai_image_draw_circle(rkai_image_t *image, rkai_point_t point, int radius, rkai_color_t color, int thickness);

/**
 * Draw a rectangle on the image (only RGB/GRAY8 images are supported)
 * 
 * @param image [in] Input image
 * @param box [in] drawing area
 * @param color [in] drawing color
 * @param thickness [in] line thickness
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_draw_rect(rkai_image_t *image, rkai_rect_t *box, rkai_color_t color, int thickness);

/**
 * Draw text on images (only RGB/GRAY8 images are supported)
 * 
 * @param image [in] Input image
 * @param text [in] text
 * @param pt [in] text starting coordinates
 * @param color [in] 
 * @param thickness [in] line thickness
 * @return @ref rkai_ret_t
 */
rkai_ret_t rkai_image_draw_text(rkai_image_t *image, const char *text, rkai_point_t pt,
                                         rkai_color_t color, int thickness);

/**
 * @brief Convert nv12 or others format to rgb format. This function will allocate new buffer for converted image. So, please release it after use
 * 
 * @param image [in] source image
 * @param converted_image [out] destination image in rgb format
 * @return rkai_ret_t 
 */
rkai_ret_t rkai_image_convert_to_rgb(rkai_image_t *image, rkai_image_t *converted_image);

/**
 * @brief Convert nv12 or others format to rgb format. This function will allocate new buffer for converted image. So, please release it after use
 * 
 * @param image [in] source image
 * @param converted_image [out] destination image in rgb format
 * @return rkai_ret_t 
 */
rkai_ret_t rkai_image_convert_to_rgb_opencv(rkai_image_t *image, rkai_image_t *converted_image);

/**
 * @brief Resize RGB image.  This function will allocate new buffer for converted image. So, please release it after use
 * 
 * @param image [in] source image
 * @param desert_size [in] Desert output size
 * @param resized_image [out] destination image in rgb format
 * @return rkai_ret_t 
 */
rkai_ret_t rkai_image_resize_rgb(rkai_image_t *image, rkai_size_t desert_size, rkai_image_t *resized_image);

/**
 * Read image file (must be called after use @ref rkai_image_release to free allocated memory)
 * 
 * @param img_path [in] Image file path
 * @param image [out] Read Image
 * @return @ref rkai_ret_t
 */
//rkai_ret_t rkai_image_read_opencv(const char *img_path, rkai_image_t *image);
//
///**
// * @brief Write image file using opencv
// *
// * @param img_path  [in] Image file path
// * @param image [in] Image to write
// * @return rkai_ret_t
// */
//rkai_ret_t rkai_image_write_opencv(const char *img_path, rkai_image_t *image);

rkai_ret_t rkai_image_rgb2bgr(rkai_image_t *image);

#ifdef __cplusplus
}
#endif
#endif //_SMARTROBOT_RKAI_IMAGE_H_

