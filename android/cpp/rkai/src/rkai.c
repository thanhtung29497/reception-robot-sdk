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

#include <stdio.h>
#include <stdlib.h>
#include "rkai.h"
#include "util.h"
#include "logger.h"

//Private macro
extern rkai_logger_t logger_setting;
//Static function defination

//static function declaration


rkai_handle_t rkai_create_handle()
{
    // memory dynamic allocation
    rkai_handle_t handle = (rkai_handle_t)malloc(sizeof(_rkai_handle_t));

    if (handle == NULL) {
        LOG_ERROR("Error while allocate handle\n");
    }

    return handle;
}

rkai_ret_t rkai_release_handle(rkai_handle_t handle)
{
    if (handle == NULL) {
        return RKAI_RET_INVALID_INPUT_PARAM;
    }
    // Check attribute in handle
    if (handle->input_tensor_attr != NULL)
    {
        free(handle->input_tensor_attr);
        handle->input_tensor_attr = NULL;
    }

    // Check attribute in handle
    if (handle->output_tensor_attr != NULL)
    {
        free(handle->output_tensor_attr);
        handle->output_tensor_attr = NULL;
    }

    // Release model context
    rknn_destroy(handle->context);

    //relase the handle
    free(handle);
    handle = NULL;

    return RKAI_RET_SUCCESS;
}

void rkai_setting_logger(rkai_logger_t setting)
{
    logger_setting = setting;
}