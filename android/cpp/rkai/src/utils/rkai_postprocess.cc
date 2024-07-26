/******************************************************************************
*    Created on Tue Apr 05 2022
*
*    Copyright (c) 2022 Rikkei AI.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Rikkei AI. This is proprietary information owned by Rikkei AI. No
*    part of this work may be disclosed, reproduced, copied, transmitted,
*    or used in any way for any purpose,without the express written
*    permission of Rikkei AI
******************************************************************************/
#include "rkai_postprocess.h"
#include <vector>
#include <cmath>
#include "utils/logger.h"

/**
 * @brief Filter the location where the confidence score is greater than CONF_THRESHOLD
 * 
 * @param conf_scores [in] confidence scores output from model
 * @param conf_thresh predefined threshold
 * @param valid_detection_indices [out] indices of valid detections and its scores
 * @return rkai_ret_t 
 */
rkai_ret_t filter_valid_detection(float *conf_scores, float conf_thresh, 
                                std::vector<std::pair<int, float>> *valid_detection_indices, int num_anchors) 
{
  int num_valid_detection = 0;
  float min_score = conf_thresh;
  for (int i = 0; i < num_anchors; i++) {
    if (conf_scores[i * NUM_CLASSES + 1] > min_score) {
      std::pair<int, float> pair;
      pair.first = i;
      pair.second = conf_scores[i * NUM_CLASSES + 1];
      valid_detection_indices->push_back(pair);
      num_valid_detection++;
    }
  }
  return RKAI_RET_SUCCESS;
}

// Based on the predicted locations and the anchors to generate the bounding
// boxes Here the locs is already filtered by the confidence threshold, so do
// anchors
/**
 * @brief Generate the bounding boxes from the predicted locations and the anchors
 * 
 * @param anchors [in] predefined anchors
 * @param locs [in] predicted locations
 * @param valid_detection_indices [in] indices of valid detections and its scores
 * @param out_dets [out] output bounding boxes with five point landmarks
 * @return rkai_ret_t 
 */
rkai_ret_t decode_bounding_boxes(float *locs, anchors_t &anchors, float *variances,
    std::vector<std::pair<int, float>> *valid_detection_indices, int width,
    int height) 
{
  // Only processing the valid detections
  for (size_t v_i = 0; v_i < valid_detection_indices->size(); v_i++) {
    int i = valid_detection_indices->at(v_i).first;
    locs[i * 4 + 0] = anchors.data[i * 4 + 0] +
                      locs[i * 4 + 0] * variances[0] * anchors.data[i * 4 + 2];
    locs[i * 4 + 1] = anchors.data[i * 4 + 1] +
                      locs[i * 4 + 1] * variances[0] * anchors.data[i * 4 + 3];
    locs[i * 4 + 2] = anchors.data[i * 4 + 2] * exp(locs[i * 4 + 2] * variances[1]);
    locs[i * 4 + 3] = anchors.data[i * 4 + 3] * exp(locs[i * 4 + 3] * variances[1]);

    locs[i * 4 + 0] = (locs[i * 4 + 0] - locs[i * 4 + 2] / 2);
    locs[i * 4 + 1] = (locs[i * 4 + 1] - locs[i * 4 + 3] / 2);
    locs[i * 4 + 2] = (locs[i * 4 + 0] + locs[i * 4 + 2]);
    locs[i * 4 + 3] = (locs[i * 4 + 1] + locs[i * 4 + 3]);

    locs[i * 4 + 0] = locs[i * 4 + 0] * width;
    locs[i * 4 + 1] = locs[i * 4 + 1] * height;
    locs[i * 4 + 2] = locs[i * 4 + 2] * width;
    locs[i * 4 + 3] = locs[i * 4 + 3] * height;
  }
  return RKAI_RET_SUCCESS;
}

/**
 * @brief Generate the landmarks from the predicted locations and the anchors
 * 
 * @param anchors [in] predefined anchors
 * @param locs [in] predicted locations
 * @param valid_detection_indices [in] indices of valid detections and its scores
 * @param out_dets [out] output bounding boxes with five point landmarks
 * @return rkai_ret_t 
 */
rkai_ret_t
decode_landmarks(float *landmarks, anchors_t &anchors,
                 float *variances,
                 std::vector<std::pair<int, float>> *valid_detection_indices,
                 int width, int height) 
{
  // float *tmp_landmarks = (float
  // *)malloc(sizeof(float)*10*num_valid_detections);
  for (int v_i = 0; v_i < valid_detection_indices->size(); v_i++) {
    // Processing the valid detections only
    int i = valid_detection_indices->at(v_i).first;
    for (int j = 0; j < 5; j++) {
      // For x coord
      landmarks[i * 10 + 2 * j] =
          anchors.data[i * 4 + 0] + landmarks[i * 10 + 2 * j] * variances[0] * anchors.data[i * 4 + 2];
      // For y coord
      landmarks[i * 10 + 2 * j + 1] =
          anchors.data[i * 4 + 1] + landmarks[i * 10 + 2 * j + 1] * variances[0] * anchors.data[i * 4 + 3];
    
      landmarks[i * 10 + 2 * j] = landmarks[i * 10 + 2 * j] * width;
      landmarks[i * 10 + 2 * j + 1] = landmarks[i * 10 + 2 * j + 1] * height;
    }
  }
  return RKAI_RET_SUCCESS;
}

/**
 * @brief Convinient function to sort the second element of the pair
 * 
 * @param a pair a
 * @param b pair b
 * @return true 
 * @return false 
 */
bool sortbysec(const std::pair<int, float> &a, const std::pair<int, float> &b) 
{
  return (a.second > b.second);
}

float compute_iou(float xmin0, float ymin0, float xmax0, float ymax0,
                  float xmin1, float ymin1, float xmax1, float ymax1) 
{
  // Compute the intersection area
  float w = fmax(0.f, fmin(xmax0, xmax1) - fmax(xmin0, xmin1));
  float h = fmax(0.f, fmin(ymax0, ymax1) - fmax(ymin0, ymin1));
  float i = w * h;
  // Compute the union area
  float u =
      (xmax0 - xmin0) * (ymax0 - ymin0) + (xmax1 - xmin1) * (ymax1 - ymin1) - i;

  return u <= 0.f ? 0.f : (i / u);
}

/**
 * @brief Non maximum suppression
 * 
 * @param loc [in] predicted locations 
 * @param valid_detections [out] valid detections (locations where conf scores > predefined threshold) 
 * @param nms_thresh predefined nms threshold
 * @return rkai_ret_t 
 */
rkai_ret_t nms(float *loc, std::vector<std::pair<int, float>> *valid_detections,
               float nms_thresh)
// printf("Num valid detections %d\n", valid_detections->size());
{
  // Sort first
  sort(valid_detections->begin(), valid_detections->end(), sortbysec);
  for (int i = 0; i < valid_detections->size(); i++) {
    int index = valid_detections->at(i).first;
    float score = valid_detections->at(i).second;
  }
  for (size_t i = 0; i < valid_detections->size(); i++) {
    int idx_a = valid_detections->at(i).first;
    if (idx_a == -1) {
      continue;
    }

    for (size_t j = i + 1; j < valid_detections->size(); j++) {
      int idx_b = valid_detections->at(j).first;

      float iou = compute_iou(loc[idx_a * 4 + 0], loc[idx_a * 4 + 1],
                              loc[idx_a * 4 + 2], loc[idx_a * 4 + 3],
                              loc[idx_b * 4 + 0], loc[idx_b * 4 + 1],
                              loc[idx_b * 4 + 2], loc[idx_b * 4 + 3]);
      if (iou > nms_thresh) {
        valid_detections->at(j).first = -1;
      }
    }
  }
  return RKAI_RET_SUCCESS;
}

rkai_ret_t postprocess(anchors_t &anchors, float *loc, float *conf, float *landmarks,
                                   float *variances, rkai_det_array_t *out_dets, int image_width,int image_height) 
{
  int rkai_ret_code;
  std::vector<std::pair<int, float>> *valid_detection_indices =
      new std::vector<std::pair<int, float>>();
      
  // Filter the valid detections (having the conf score > conf_thresh)
  filter_valid_detection(conf, CONF_THRESH, valid_detection_indices, anchors.num_anchors);
  
  //Decode the bounding boxes
  rkai_ret_code = decode_bounding_boxes(loc, anchors, variances,
                                        valid_detection_indices, image_width, image_height);

  if (rkai_ret_code != RKAI_RET_SUCCESS) {
    LOG_ERROR("Decode bounding boxes failed\n");
    free(anchors.data);
    free(valid_detection_indices);
    return RKAI_RET_COMMON_FAIL;
  }

  rkai_ret_code =
      decode_landmarks(landmarks, anchors, variances, valid_detection_indices, image_width, image_height);
  if (rkai_ret_code != RKAI_RET_SUCCESS) {
    LOG_ERROR("Decode landmarks failed\n");
    free(anchors.data);
    free(valid_detection_indices);
    return RKAI_RET_COMMON_FAIL;
  }

  // Map current coord to image coord

  rkai_ret_code = nms(loc, valid_detection_indices, NMS_THRESH);
  if (rkai_ret_code != RKAI_RET_SUCCESS) {
    LOG_ERROR("NMS failed\n");
    free(anchors.data);
    free(valid_detection_indices);
    return RKAI_RET_COMMON_FAIL;
  }

  // Copy the valid detections to the output array
  int count = 0;
  for (size_t i = 0; i < valid_detection_indices->size(); i++) {
    int idx = valid_detection_indices->at(i).first;
    if (idx == -1) {
      continue;
    }
    out_dets->face[count].box.left = std::max(0,(int) loc[idx * 4 + 0]);
    out_dets->face[count].box.top = std::max(0,(int) loc[idx * 4 + 1]);
    out_dets->face[count].box.right = std::min((int) loc[idx * 4 + 2],image_width);
    out_dets->face[count].box.bottom = std::min((int) loc[idx * 4 + 3],image_height);
    out_dets->face[count].score = valid_detection_indices->at(i).second;

    for (int i =0;i<5;i++)
    {
      out_dets->face[count].landmarks[i].x = landmarks[idx * 10 + i * 2] < 0 ? 0 : landmarks[idx * 10 + i * 2];
      out_dets->face[count].landmarks[i].y = landmarks[idx * 10 + i * 2 + 1] < 0 ? 0 : landmarks[idx * 10 + i * 2 + 1];
      out_dets->face[count].landmarks[i].x = out_dets->face[count].landmarks[i].x > image_width ? image_width : out_dets->face[count].landmarks[i].x;
      out_dets->face[count].landmarks[i].y = out_dets->face[count].landmarks[i].y > image_height ? image_height : out_dets->face[count].landmarks[i].y;
    }
    out_dets->face[count].landmark_count = 5;
    out_dets->face[count].id = idx;
    count++;
  }

  out_dets->count = count;

  free(valid_detection_indices);
  return RKAI_RET_SUCCESS;
}

void convert_cordinate(rkai_det_array_t *out_dets,
                                   int image_width,
                                   int image_height,
                                   int ori_image_width,
                                   int ori_image_height)
{
  if ((image_width == ori_image_width) && image_height == ori_image_height){
    return;
  }

  float x_slope = ((float)ori_image_width)/image_width;
  float y_slope = ((float)ori_image_height)/image_height;

  for (int idx = 0; idx < out_dets->count; ++idx){
    out_dets->face[idx].box.left = out_dets->face[idx].box.left*x_slope;
    out_dets->face[idx].box.right = out_dets->face[idx].box.right*x_slope;
    out_dets->face[idx].box.top = out_dets->face[idx].box.top*y_slope;
    out_dets->face[idx].box.bottom = out_dets->face[idx].box.bottom*y_slope;

    for (int lm_idx=0; lm_idx < 5; ++lm_idx){
      out_dets->face[idx].landmarks[lm_idx].x = out_dets->face[idx].landmarks[lm_idx].x*x_slope;
      out_dets->face[idx].landmarks[lm_idx].y = out_dets->face[idx].landmarks[lm_idx].y*y_slope;
    }
  }
}