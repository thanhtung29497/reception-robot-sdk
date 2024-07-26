#
# Created on Fri Apr 15 2022
#
# Copyright (c) 2022 Rikkei AI.  All rights reserved.
#
# The material in this file is confidential and contains trade secrets
# of Rikkei AI. This is proprietary information owned by Rikkei AI. No
# part of this work may be disclosed, reproduced, copied, transmitted,
# or used in any way for any purpose,without the express written 
# permission of Rikkei AI
#

set(RKAI_SOURCE_FILE_LOCAL ${RKAI_SOURCE_FILES}
        rkai/src/rkai.c
        rkai/src/rkai_facedetect.cc
        rkai/src/rkai_trigger_word.cc
        rkai/src/rkai_vad.cc
        rkai/src/rkai_image.cc
        rkai/src/rkai_audio.cc
        rkai/src/android_porting/android_fopen.c)

set(RKAI_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../include
        ${CMAKE_CURRENT_LIST_DIR}/../include/bytetrack
        ${CMAKE_CURRENT_LIST_DIR}/../include/tracking
        ${CMAKE_CURRENT_LIST_DIR}/../include/utils
        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/stb
        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/eigen3
        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/ncnn/include
        ${CMAKE_CURRENT_LIST_DIR}/../include/android_porting
        PARENT_SCOPE)
# Add Utils subdirectory
add_subdirectory(utils)
set(RKAI_SOURCE_FILE_LOCAL ${RKAI_SOURCE_FILE_LOCAL}
        ${RKAI_UTIL_SOURCE_FILES})

add_subdirectory(bytetrack)
set(RKAI_SOURCE_FILE_LOCAL  ${RKAI_SOURCE_FILE_LOCAL}
        ${RKAI_BYTETRACK_SOURCE_FILES})

add_subdirectory(tracking)
set(RKAI_SOURCE_FILE_LOCAL  ${RKAI_SOURCE_FILE_LOCAL}
        ${RKAI_TRACKING_SOURCE_FILES})

set(RKAI_SOURCE_FILES ${RKAI_SOURCE_FILES}
        ${RKAI_SOURCE_FILE_LOCAL}
        PARENT_SCOPE)