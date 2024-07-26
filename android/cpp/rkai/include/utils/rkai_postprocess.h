/******************************************************************************
*    Created on Tue Apr 5 2022
*
*    Copyright (c) 2022 Rikkei AI.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Rikkei AI. This is proprietary information owned by Rikkei AI. No
*    part of this work may be disclosed, reproduced, copied, transmitted,
*    or used in any way for any purpose,without the express written
*    permission of Rikkei AI
******************************************************************************/

#ifndef _SMARTROBOT_RKAI_POSTPROCESS_H_
#define _SMARTROBOT_RKAI_POSTPROCESS_H_

#include "stdio.h"
#include "string.h"
#include "rkai_type.h"

#define NUM_CLASSES 2
#define CONF_THRESH 0.5
#define NMS_THRESH 0.3
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compute the iou between two bounding boxes
 * 
 * @param xmin0 x1 of box0
 * @param ymin0 y1 of box0
 * @param xmax0 x2 of box0
 * @param ymax0 y2 of box0
 * @param xmin1 x1 of box1
 * @param ymin1 y1 of box1
 * @param xmax1 x2 of box1
 * @param ymax1 y2 of box1
 * @return float iou value
 */
float compute_iou(float xmin0, float ymin0, float xmax0, float ymax0,
                  float xmin1, float ymin1, float xmax1, float ymax1);

/**
 * @brief Post process for face detection model
 * 
 * @param anchors predefined anchors 
 * @param loc [in] locattion regression to determine the bounding box
 * @param conf [in] confidence scores of the bounding box
 * @param landmarks [in] landmark regression to determine the five point landmarks
 * @param variances [in] variances output to determine the bounding box
 * @param out_dets [out] output bounding boxes with five point landmarks
 * @param image_width
 * @param image_height
 * @return rkai_ret_t 
 */
rkai_ret_t postprocess(anchors_t &anchors, float *loc,
                                   float *conf, float *landmarks,
                                   float *variances,
                                   rkai_det_array_t *out_dets,
                                   int image_width,
                                   int image_height);

void convert_cordinate(rkai_det_array_t *out_dets,
                                   int image_width,
                                   int image_height,
                                   int ori_image_width,
                                   int ori_image_height);
#ifdef __cplusplus
}
#endif
#endif //_SMARTROBOT_RKAI_POSTPROCESS_H
