/******************************************************************************
*    Created on Fri Apr 15 2022
*   
*    Copyright (c) 2022 Rikkei AI.  All rights reserved.
*   
*    The material in this file is confidential and contains trade secrets
*    of Rikkei AI. This is proprietary information owned by Rikkei AI. No
*    part of this work may be disclosed, reproduced, copied, transmitted,
*    or used in any way for any purpose,without the express written 
*    permission of Rikkei AI
******************************************************************************/



#ifndef _SMARTROBOT_LOGGER_H_
#define _SMARTROBOT_LOGGER_H_

#include <android/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RKAI"

#include <stdio.h>

#include "rkai_type.h"

extern rkai_logger_t logger_setting;


#ifdef __cplusplus
extern "C" {
#endif

#define LOG_INFO(format, ...)                                                  \
  do {                                                                         \
    if (logger_setting.log_level < RKAI_LOGGER_LEVEL_INFO)                             \
      break;                                                                   \
    if (logger_setting.log_output_type == RKAI_LOGGER_TYPE_FILE                \
         || logger_setting.log_output_type == RKAI_LOGGER_TYPE_BOTH)           \
      __android_log_print(ANDROID_LOG_INFO, "[RKAI]", "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);  \
    if (logger_setting.log_output_type == RKAI_LOGGER_TYPE_PRINT               \
         || logger_setting.log_output_type == RKAI_LOGGER_TYPE_BOTH)           \
      fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__,               \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define LOG_WARN(format, ...)                                                  \
  do {                                                                         \
    if (logger_setting.log_level < RKAI_LOGGER_LEVEL_WARN)                                \
      break;                                                                   \
    if (logger_setting.log_output_type == RKAI_LOGGER_TYPE_FILE                \
         || logger_setting.log_output_type == RKAI_LOGGER_TYPE_BOTH)           \
      __android_log_print(ANDROID_LOG_WARN, "[RKAI]", "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);  \
    if (logger_setting.log_output_type == RKAI_LOGGER_TYPE_PRINT               \
         || logger_setting.log_output_type == RKAI_LOGGER_TYPE_BOTH)           \
      fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__,               \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define LOG_ERROR(format, ...)                                                 \
  do {                                                                         \
    if (logger_setting.log_level < RKAI_LOGGER_LEVEL_ERROR)                               \
      break;                                                                   \
    if (logger_setting.log_output_type == RKAI_LOGGER_TYPE_FILE                \
         || logger_setting.log_output_type == RKAI_LOGGER_TYPE_BOTH)           \
      __android_log_print(ANDROID_LOG_ERROR, "[RKAI]", "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);  \
    if (logger_setting.log_output_type == RKAI_LOGGER_TYPE_PRINT               \
         || logger_setting.log_output_type == RKAI_LOGGER_TYPE_BOTH)           \
      fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__,               \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define LOG_DEBUG(format, ...)                                                 \
  do {                                                                         \
    if (logger_setting.log_level < RKAI_LOGGER_LEVEL_DEBUG)                               \
      break;                                                                   \
    if (logger_setting.log_output_type == RKAI_LOGGER_TYPE_FILE                \
         || logger_setting.log_output_type == RKAI_LOGGER_TYPE_BOTH)           \
      __android_log_print(ANDROID_LOG_DEBUG, "[RKAI]", "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);  \
    if (logger_setting.log_output_type == RKAI_LOGGER_TYPE_PRINT               \
         || logger_setting.log_output_type == RKAI_LOGGER_TYPE_BOTH)           \
      fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__,               \
              ##__VA_ARGS__);                                                  \
  } while (0)


#ifdef __cplusplus
}
#endif
#endif //_SMARTROBOT_LOGGER_H_
