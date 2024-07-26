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

set(RKAI_UTILS_SOURCE_FILES rkai/src/utils/util.c
        rkai/src/utils/logger.c
        rkai/src/utils/rkai_postprocess.cc)

set(RKAI_UTIL_SOURCE_FILES ${RKAI_SOURCE_FILES}
                      ${RKAI_UTILS_SOURCE_FILES} 
                      PARENT_SCOPE)
