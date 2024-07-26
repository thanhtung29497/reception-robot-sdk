//
// Created by tannn on 19/01/2024.
//
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

#include <stdlib.h>

#include "utils/rkai_postprocess.h"
#include "utils/util.h"
#include "rkai_facedetect.h"
#include "utils/logger.h"
#include "BaseBYTETracker.h"
#include "base_tracker.h"

#include "android_porting/android_fopen.h"

// Private macro
#define DETECT_MODEL_PATH "model/face_detect/best.rknn"
#define ANCHOR_INFORMATION_PATH "model/face_detect/anchor_information.txt"
#define MAX_COAST_CYCLES 12
rkai_size_t required_detect_size = {.w=640, .h=480};

anchors_t anchors;
// Private tracker declaration
BaseBYTETracker <BaseSTrack, rkai_det_t> rkai_tracker_detect;
rkai_ret_t load_anchor_file(const char *file_name)
{
    AAsset *fp = android_fopen(file_name, "r");
    char line[1024000];
    int count = 0;
    if (fp == NULL)
    {
        LOG_ERROR("Cannot open file %s \n", file_name);
        return RKAI_RET_COMMON_FAIL;
    }

    if( android_read(fp, line, 1024000) < 0){
        LOG_ERROR("Cannot read file %s \n", file_name);
        return RKAI_RET_COMMON_FAIL;
    }

    char *token;

    token = strtok(line, "\n");
    // Read the file to anchors array
    while(token != NULL)
    {
        LOG_DEBUG("Read data from file---------------- %s\n", token);
        if (count == 0)
        {
            if(sscanf(token, "%d %d %d %d", &anchors.num_anchors, &anchors.anchors_array_size, &required_detect_size.w, &required_detect_size.h) == 0)
            {
                LOG_ERROR("Failed to read anchor information from file %s, please check th number of anchors and total size of anchor array\n", file_name);
                return RKAI_RET_COMMON_FAIL;
            }
            anchors.curr_idx = 0;
            anchors.data = (float *)malloc(anchors.anchors_array_size * sizeof(float));
        } else {
            float x, y, w, h;
            if(sscanf(token, "%f %f %f %f", &x, &y, &w, &h) == 0)
            {
                LOG_ERROR("open file %s failed\n", file_name);
                return RKAI_RET_COMMON_FAIL;
            }

            anchors.data[anchors.curr_idx*4+0] = x;
            anchors.data[anchors.curr_idx*4+1] = y;
            anchors.data[anchors.curr_idx*4+2] = w;
            anchors.data[anchors.curr_idx*4+3] = h;
            anchors.curr_idx += 1;
        }

        token = strtok(NULL, "\n");
        count += 1;
    }
    android_close(fp);
    return RKAI_RET_SUCCESS;
}

extern "C" rkai_ret_t rkai_init_detector(rkai_handle_t handle)
{
    // memset(&anchors, 0, sizeof(anchors_t));
    rkai_ret_t ret = load_anchor_file(ANCHOR_INFORMATION_PATH);
    if (ret != RKAI_RET_SUCCESS)
    {
        LOG_ERROR("Load anchor file failed \n");
        return RKAI_RET_COMMON_FAIL;
    }
    return model_init(handle, DETECT_MODEL_PATH);
}

extern "C" rkai_ret_t rkai_init_detector_android(rkai_handle_t handle, AAssetManager *asset_mgr)
{
    rkai_ret_t ret = load_anchor_file(ANCHOR_INFORMATION_PATH);

    if (ret != RKAI_RET_SUCCESS)
    {
        LOG_ERROR("Load anchor file failed \n");
        return RKAI_RET_COMMON_FAIL;
    }
    return model_init(handle, DETECT_MODEL_PATH);
}

extern "C" rkai_ret_t rkai_face_detect(rkai_handle_t handle, rkai_image_t *image, rkai_det_array_t *detected_face_array)
{

    rknn_input inputs[1];

    float variances[2] = {0.1, 0.2};
    // Init model output info
    int rknn_ret_code;
    rkai_ret_t rkai_ret_code = RKAI_RET_SUCCESS;

    // convert to rgb24 and resize
    int req_width = required_detect_size.w;
    int req_height = required_detect_size.h;
    int input_buffer_size = req_width * req_height * 3;
    unsigned char *input_image = (unsigned char *) malloc(input_buffer_size);

    if (input_image == NULL) {
        LOG_ERROR("Malloc input image failed \n");
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        return rkai_ret_code;
    }

    int64_t start_time = get_current_time_us();
    rkai_ret_code = preprocess(image, input_image, req_width, req_height);
    if (rkai_ret_code != RKAI_RET_SUCCESS) {
        free(input_image);
        return rkai_ret_code;
    }
    memset(inputs, 0, sizeof(inputs));

    // Set input data
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = handle->input_tensor_attr[0].size;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = input_image;
    int64_t start_time_input = get_current_time_us();
    rknn_ret_code = rknn_inputs_set(handle->context, handle->io_num.n_input, inputs);
    if (rknn_ret_code != RKNN_SUCC) {
        LOG_WARN("Failed to Init Face detection Input data. Return code of function rknn_input_set = %d\n",
                 rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        free(input_image);
        return rkai_ret_code;
    }

    // Inference
    int64_t start_time_inference = get_current_time_us();
    rknn_ret_code = rknn_run(handle->context, NULL);
    if (rknn_ret_code != RKNN_SUCC)
    {
        LOG_WARN("Failed to run inference. rknn_run return code = %d\n", rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        free(input_image);
        return rkai_ret_code;
    }

    // Get output
    rknn_output outputs[handle->io_num.n_output];
    memset(outputs, 0, sizeof(outputs));
    for (int j = 0; j < handle->io_num.n_output; j++) {
        outputs[j].want_float = 1;
    }
    int64_t start_time_output = get_current_time_us();
    rknn_ret_code = rknn_outputs_get(handle->context, handle->io_num.n_output, outputs, NULL);
    if (rknn_ret_code != RKNN_SUCC) {
        LOG_WARN("Failed to get output after inference, rknn_outputs_get return code %d \n", rknn_ret_code);
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        free(input_image);
        return rkai_ret_code;
    }

    //Extract output
    float *loc = (float *) outputs[0].buf;
    float *conf = (float *) outputs[1].buf;
    float *landmark = (float *) outputs[2].buf;

    //Post process and get detected faces
    int64_t start_time_postprocess = get_current_time_us();
    rkai_ret_code = postprocess(anchors,loc, conf, landmark, variances, detected_face_array,req_width,req_height);
    if (rkai_ret_code != RKAI_RET_SUCCESS)
    {
        LOG_WARN("Failed to postprocess \n");
        rkai_ret_code = RKAI_RET_COMMON_FAIL;
        free(input_image);
        return rkai_ret_code;
    }
    convert_cordinate(detected_face_array, req_width, req_height, image->width, image->height);

    //Release dynamic allocation resource here
    rknn_ret_code = rknn_outputs_release(handle->context, handle->io_num.n_output, outputs);
    if (rknn_ret_code != RKAI_RET_SUCCESS) {
        LOG_WARN("Failed to release dynamic allocation resource %d \n", rknn_ret_code);
    }
    free(input_image);
    return rkai_ret_code;
}

std::vector <rkai_det_t> process_label(rkai_det_array_t *detected_face_array) {
    std::vector <rkai_det_t> detections_per_frame;
    if (detected_face_array == NULL) {
        return detections_per_frame; //empty detection
    }
    for (int i = 0; i < detected_face_array->count; i++) {
        detections_per_frame.push_back(detected_face_array->face[i]);
    }
    return detections_per_frame;
}

// rkai_ret_t rkai_sort_tracking_cplusplus_wrapper(rkai_det_array_t *detections,rkai_det_array_t *out_track, bool enable_autotrack, int track_frame, bool flush_all_id)
// {
//     (void) track_frame;
//     const std::vector<rkai_det_t> box_per_frame = process_label(detections);

//     // Check if the tracks need to be flushed
//     if (flush_all_id)
//     {
//         rkai_tracker_detect.Flush();
//     }

//     // Run Sort Tracker
//     rkai_tracker_detect.Run(box_per_frame, enable_autotrack);
//     memset(out_track, 0, sizeof(rkai_det_array_t));
//     // Get the tracks for output
//     const auto tracks = rkai_tracker_detect.GetTracks();
//     for (auto& track_map: tracks)
//     {
//         //Get id and track
//         int id = track_map.first;
//         auto track = track_map.second;
//         if (track.coast_cycles_ < MAX_COAST_CYCLES)
//         {
// 			// Reach maximum number of tracking face
// 			if (out_track->count >= 128)
// 			{
// 				return RKAI_RET_SUCCESS;
// 			}
//             const auto box = track.GetStateAsBbox();
//             if (box.width <= 0 || box.height <= 0)
//             {
//                 continue;
//             }
//             out_track->face[out_track->count].box.left = box.tl().x;
//             out_track->face[out_track->count].box.top = box.tl().y;
//             out_track->face[out_track->count].box.right = box.x + box.width;
//             out_track->face[out_track->count].box.bottom = box.y + box.height;
//             out_track->face[out_track->count].id = id;
//             for (int i = 0; i < 5; i++)
//             {
//                 out_track->face[out_track->count].landmarks[i].x = track.keypoints[i].x;
//                 out_track->face[out_track->count].landmarks[i].y = track.keypoints[i].y;
//             }
//             out_track->face[out_track->count].landmark_count = 5;
//             out_track->face[out_track->count].is_updated = track.is_updated;
//             out_track->face[out_track->count].score = track.score;
//             out_track->count++;
//         }
//     }
//     return RKAI_RET_SUCCESS;

// }

rkai_ret_t rkai_sort_tracking_cplusplus_wrapper(rkai_det_array_t *detections,rkai_det_array_t *out_track, bool enable_autotrack, int track_frame, bool flush_all_id)
{
    const std::vector<rkai_det_t> box_per_frame = process_label(detections);
    std::vector <BaseSTrack<rkai_det_t>> tracks;
    // Check if the tracks need to be flushed
    // (void) flush_all_id;
    if (flush_all_id)
    {
        rkai_tracker_detect.Flush();
    }

    // Run Sort Tracker
    rkai_tracker_detect.update(box_per_frame, tracks, track_frame);
    memset(out_track, 0, sizeof(rkai_det_array_t));

    for (const auto &track: tracks) {

        if (out_track->count >= 128) {
            return RKAI_RET_SUCCESS;
        }
        out_track->face[out_track->count] = track.rkai_tracking_object;
        out_track->face[out_track->count].id = track.track_id;
        out_track->face[out_track->count].is_updated = track.is_updated;
        out_track->count++;
    }
    return RKAI_RET_SUCCESS;
}

extern "C" rkai_ret_t rkai_object_autotrack(rkai_handle_t handle, rkai_image_t *image, \
                                int time_out, rkai_det_array_t *in_track_object, \
                                rkai_det_array_t *out_track_object, bool enable_autotrack, int track_frame, bool flush_all_id)
{
    (void) time_out;
    rkai_ret_t rkai_ret_code = RKAI_RET_SUCCESS;
    return rkai_sort_tracking_cplusplus_wrapper(in_track_object, out_track_object, enable_autotrack, track_frame, flush_all_id);
}

rkai_ret_t rkai_required_detect_size(rkai_size_t *required_size)
{
    *required_size = required_detect_size;
    return RKAI_RET_SUCCESS;
}


