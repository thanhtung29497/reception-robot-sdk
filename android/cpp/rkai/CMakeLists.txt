#
# Created on Fri Apr 01 2022
#
# Copyright (c) 2022 Rikkei AI.  All rights reserved.
#
# The material in this file is confidential and contains trade secrets
# of Rikkei AI. This is proprietary information owned by Rikkei AI. No
# part of this work may be disclosed, reproduced, copied, transmitted,
# or used in any way for any purpose,without the express written 
# permission of Rikkei AI
#
add_subdirectory(src)

# Add the thirdparty directory
set(RKAI_INCLUDE_DIRS   ${RKAI_INCLUDE_DIRS}
                        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rknpu2/include
                        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rga/include
                        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/c_vector
                        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/matrix
                        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen3/Eigen
                        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen3/unsupported
                        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/clibrosa)

set(APP_SOURCE_FILES ${APP_SOURCE_FILES}
                     ${RKAI_SOURCE_FILES} PARENT_SCOPE)

set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS}
        ${RKAI_INCLUDE_DIRS} PARENT_SCOPE)
