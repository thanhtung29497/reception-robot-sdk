//
// Created by tannn on 19/01/2024.
//

#ifndef SMARTROBOT_RKAI_FACEDETECT_H
#define SMARTROBOT_RKAI_FACEDETECT_H

#include <stdbool.h>
#include "rkai_type.h"
#include <android/asset_manager_jni.h>

#define CONF_THRESHOLD 0.6

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function will init the content of the handle.
 *
 * @param handle [in] rkai handle
 * @return @ref rkai_ret_t return code.
 */
rkai_ret_t rkai_init_detector(rkai_handle_t handle);
rkai_ret_t rkai_init_detector_android(rkai_handle_t handle, AAssetManager *asset_mgr);

/**
 * @brief This function will run an inference session to detect the face in the input image and return the array of face
 *
 * @param handle [in] rkai handle
 * @param image [in] Input image
 * @param detected_face_array [out] Output face array.
 * @param req_width Required width that model can process
 * @param req_height Required height that model can process
 * @return rkai_ret_t return code
 */
rkai_ret_t rkai_face_detect(rkai_handle_t handle, rkai_image_t *image, rkai_det_array_t *detected_face_array);

/**
 * @brief This function will run an inference session to detect the person in the input image and return the array of person in image
 *
 * @param handle [in] rkai handle
 * @param image [in] Input image
 * @param detected_person_array [out] //Output person array
 * @return rkai_ret_t return code
 */
rkai_ret_t rkai_person_detect(rkai_handle_t handle, rkai_image_t *image, rkai_det_person_array_t *detected_person_array);

/**
 * @brief Tracking object, used in continuous video frames, will assign a tracking ID to the detection result of
 *        the current scene, and keep the same tracking ID for the same target under continuous frames
 *
 * @param handle [in] rkai handle
 * @param image  [in] rkai_image, this is used to determine size of image frame
 * @param time_out [in] Maximum number of consecutive frames, that object not appear in the frame.
 *                      This is used to avoid create new object when miss detected object
 * @param in_track_object [in] Array of the the detected object
 * @param out_track_object [out] Array of the tracked object. New object will be assign unique ID.
 * @param enable_autotrack [in] If this option is true, autotrack will enable then interpolate the output base on
 *                              history trajectory. Otherwise, output will be interpolate by history and current detection input
 * @return rkai_ret_t
 */
rkai_ret_t rkai_object_autotrack(rkai_handle_t handle, rkai_image_t *image, \
                                int time_out, rkai_det_array_t *in_track_object, \
                                rkai_det_array_t *out_track_object, bool enable_autotrack, int track_frame, bool flush_all_id);

rkai_ret_t rkai_required_detect_size(rkai_size_t *required_size);
#ifdef __cplusplus
}
#endif

#endif //SMARTROBOT_RKAI_FACEDETECT_H
