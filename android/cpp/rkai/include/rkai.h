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



#ifndef _SMARTROBOT_RKAI_H_
#define _SMARTROBOT_RKAI_H_

#include "rkai_type.h"
#include "rkai_image.h"
#include "rkai_audio.h"
#include "rkai_trigger_word.h"
#include "rkai_vad.h"
#include "rkai_facedetect.h"
#include "utils/logger.h"
/**
 * @mainpage RIKKEI AI SDK FOR ROBOT
 *
 * @section Overview
 *
 * RKAI SDK provides a series of functions related to face recognition analysis, and makes full use of the NPU of the RV1126 Platform. The SDK provides API functions such as face detection, face recognition, human pose estimation...
 *
 * 
 * @section  How to Use
 *
 * 
 * @subsection  Create code
 *
 * RKAI handle `rkai_create_handle` create the handle object to store rknn context, input, output properties
 * 
 * ```
 *  rkai_ret_t ret;
 *  rkai_handle_t face_handle = rkai_create_handle();
 * ```
 * 
 * After create handle, you init to detector or others to load model, init input, output attrbute
 * ```
 * ret = rkai_init_detector(face_handle);
 * if (ret != RKAI_RET_SUCCESS) {
 *     printf("Error: init detector error %d!", ret);
 *     return ret;
 * }
 * ```
 * 
 * Then, you can use the detect, recoginize... function to run the model
 * 
 * ```
 * rkai_det_array_t detected_face_array;
 * rkai_ret_t ret = rkai_face_detect(face_handle, input_image, &detected_face_array);
 * ```
 * 
 * Finally, if you don't need to continue to use it, you can call the `rkai_release_handle` function to release it. The schematic code is as follows:
 * 
 * ```
 * rkai_release_handle(face_handle);
 * ```
 *
 * @subsection API Function List
 *
 * All of API functions provided by SDK will be listed bellow:
 *
 * Function	                         | Initialization function               | Describe           | Defination location |
 * ----------------------------------|---------------------------------------|--------------------|---------------------|
 * @ref rkai_face_detect             | @ref rkai_init_detector               | Face Detection     | rkai_facedetect.h   |
 * @ref rkai_feature_extracture      | @ref rkai_init_recognizer             | Face Recognition   | rkai_recognition.h  |
 * @ref rkai_feature_compare         | @ref rkai_init_recognizer             | Face Recognition   | rkai_recognition.h  |
 */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create new @ref rkai_handle_t
 * 
 * @return @ref rkai_handle_t 
 */
rkai_handle_t rkai_create_handle();

/**
 * @brief Release unused handle. This function must be call before exit the application to release resource
 * 
 * @param handle handle to be released
 * @return @ref rkai_ret_t 
 */
rkai_ret_t rkai_release_handle(rkai_handle_t handle);

/**
 * @brief Setting logger for this library
 * 
 * @param setting setting for logger
 */
void rkai_setting_logger(rkai_logger_t setting);

#ifdef __cplusplus
}
#endif
#endif //_SMARTROBOT_RKAI_H_

