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

#include "logger.h"
#include "rkai_type.h"

rkai_logger_t logger_setting = {.log_level = RKAI_LOGGER_LEVEL_INFO, .log_output_type = RKAI_LOGGER_TYPE_FILE};