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



#ifndef _SMARTROBOT_RKAI_TYPE_H_
#define _SMARTROBOT_RKAI_TYPE_H_

#include <stdint.h>
#include <rknn/rknn_api.h>


#ifdef __cplusplus
extern "C" {
#endif

/*! \public
 * @brief rikkeiai handle. This will be used to save the context of the rknn
 * 
 */

typedef struct _rkai_handle_t {
    rknn_context context;
    rknn_input_output_num io_num; /// Save number of input, output
    rknn_tensor_attr *input_tensor_attr;    /// Pointer to input tensor attribute. This will be created dynamically on init function
    uint64_t input_tensor_attr_size;    /// Size of input tensor attribute in byte. sizeof(rknn_tensor_attr)*io_num.n_input
    rknn_tensor_attr *output_tensor_attr;   ///Pointer to output tensor attribute. This will be created dynamically on init function
    uint64_t output_tensor_attr_size; /// Size of output tensor attribute in byte. sizeof(rknn_tensor_attr)*io_num.n_output
} _rkai_handle_t;

/**
 * @brief handle pointer type
 * 
 */
typedef _rkai_handle_t *rkai_handle_t;

/*\public
 * @brief return code
 * 
 */
typedef enum {
    RKAI_RET_SUCCESS = 0, /// Successfully case
    RKAI_RET_COMMON_FAIL = -1, /// For common fail, such as 
    RKAI_RET_INVALID_INPUT_PARAM = -2, /// Invalid
    RKAI_NOT_SUPPORT = -3,
    RKAI_RET_THIRD_PARTY_FAIL = -4,
    RKAI_RET_UNKNOW = -100
} rkai_ret_t;

/*! \public
 * @brief Image Pixel format
 */
typedef enum {
    RKAI_PIXEL_FORMAT_GRAY8 = 0,       ///< Gray8
    RKAI_PIXEL_FORMAT_RGB888,          ///< RGB888
    RKAI_PIXEL_FORMAT_BGR888,          ///< BGR888
    RKAI_PIXEL_FORMAT_RGBA8888,        ///< RGBA8888
    RKAI_PIXEL_FORMAT_BGRA8888,        ///< BGRA8888
    RKAI_PIXEL_FORMAT_YUV420P_YU12,    ///< YUV420P YU12: YYYYYYYYUUVV
    RKAI_PIXEL_FORMAT_YUV420P_YV12,    ///< YUV420P YV12: YYYYYYYYVVUU
    RKAI_PIXEL_FORMAT_YUV420SP_NV12,   ///< YUV420SP NV12: YYYYYYYYUVUV
    RKAI_PIXEL_FORMAT_YUV420SP_NV21,   ///< YUV420SP NV21: YYYYYYYYVUVU
    RKAI_PIXEL_FORMAT_YUV422P_YU16,    ///< YUV422P YU16: YYYYYYYYUUUUVVVV
    RKAI_PIXEL_FORMAT_YUV422P_YV16,    ///< YUV422P YV16: YYYYYYYYVVVVUUUU
    RKAI_PIXEL_FORMAT_YUV422SP_NV16,   ///< YUV422SP NV16: YYYYYYYYUVUVUVUV
    RKAI_PIXEL_FORMAT_YUV422SP_NV61,   ///< YUV422SP NV61: YYYYYYYYVUVUVUVU
    RKAI_PIXEL_FORMAT_GRAY16,          ///< Gray16
    RKAI_PIXEL_FORMAT_MAX,
} rkai_pixel_format;

/**
 * @brief Image Rotate Mode
 */
typedef enum {
    RKAI_IMAGE_TRANSFORM_NONE = 0x00,  ///< Do not transform
    RKAI_IMAGE_TRANSFORM_FLIP_H = 0x01,  ///< Flip image horizontally
    RKAI_IMAGE_TRANSFORM_FLIP_V = 0x02,  ///< Flip image vertically
    RKAI_IMAGE_TRANSFORM_ROTATE_90 = 0x04,  ///< Rotate image 90 degree
    RKAI_IMAGE_TRANSFORM_ROTATE_180 = 0x03,  ///< Rotate image 180 degree
    RKAI_IMAGE_TRANSFORM_ROTATE_270 = 0x07,  ///< Rotate image 270 defree
} rkai_image_transform_mode;

/**
 * @brief Represent the coordinates of a point on a 2D image
 */
typedef struct rkai_point_t {
    int x;      ///< X coordinate
    int y;      ///< Y coordinate
} rkai_point_t;

/**
 * @brief  Represent the coordinates and confidence of a human keypoint  
 */
typedef struct rkai_keypoint_t {
    int x;              ///< X coordinate
    int y;              ///< Y coordinate
    float kpt_score;    ///< Keypoint Score
} rkai_keypoint_t;

/**
 * @brief Represent the size of regtangle in 2D plan
 * 
 */
typedef struct rkai_size_t {
    int w;      ///< w width of rectangle
    int h;      ///< h height of rectangle
} rkai_size_t;

/**
 * @brief RGB color pixel reprentation
 */
typedef struct rkai_color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rkai_color_t;

/**
 * @brief A rectangular area representing a face on a 2D image
 */
typedef struct rkai_rect_t {
    int left;       ///< the leftmost coordinate of the rectangle
    int top;        ///< The coordinates of the top edge of the rectangle
    int right;      ///< the rightmost coordinate of the rectangle
    int bottom;     ///< The coordinates of the bottom of the rectangle
} rkai_rect_t;

/**
 * @brief represents a two-dimensional image
 */
typedef struct rkai_image_t {
    uint8_t *data;                          ///< Image data
    uint32_t size;                          ///< Image size
    uint8_t is_prealloc_buf;                ///< Whether the image data has preallocated memory
    rkai_pixel_format pixel_format;     ///< Image pixel format (@ref rkai_pixel_format)
    uint32_t width;                         ///< Image width
    uint32_t height;                        ///< Image height
    float original_ratio;                   ///< Image original ratio of width & height, default is 1
    float scale_ratio;                      ///< Image scale ratio of width & height, default is 1
} rkai_image_t;

/**
 * @brief Headpose estimation results
 */
typedef struct rkai_headpose_estimation_t {
    float yaw;                                ///< yaw angle
    float pitch;                              ///< pitch angle
    float roll;                               ///< roll angle
} rkai_headpose_estimation_t;

/**
 * @brief Headpose estimation results array
 */
typedef struct rkai_headpose_estimation_array_t {
    int count;
    rkai_headpose_estimation_t headpose[128];
} rkai_headpose_estimation_array_t;

/**
 * @brief Represent the landmark of 2d face
 */
typedef struct rkai_landmark_t {
    int image_width;                    ///< Width of input image
    int image_height;                   ///< Height of input image
    rkai_rect_t face_box;           ///< Detected face area
    int landmarks_count;                ///< Number of landmarks points
    rkai_point_t landmarks[128];    ///< landmarks point array
    float score;                        ///< score
} rkai_landmark_t;

/**
 * @brief Indicates the face angle result
 */
typedef struct rkai_angle_t {
    float pitch;        ///< Pitch angle ( < 0: up, > 0: down)
    float yaw;          ///< yaw ( < 0: left, > 0: right )
    float roll;         ///< roll ( < 0: left, > 0: right )
} rkai_angle_t;

/**
 * @brief Face Recognition Algorithm Version
 * 
 */
typedef enum {
    RKAI_RECOG_NORMAL = 0,  ///< Normal face recognition
    RKAI_RECOG_MASK = 11    ///< Face Recognition on masked face
} rkai_recog_version;

/**
 * @brief Represents the results of facial features
 */
typedef struct rkai_feature_t {
    int version;            ///< Face Recognition Algorithm Version
    int len;                ///< Feature length
    uint8_t feature[512];   ///< Feature data
} rkai_feature_t;

/**
 * @brief Represents the results of facial features
 */
typedef struct rkai_feature_float_t {
    int version;            ///< Face Recognition Algorithm Version
    int len;                ///< Feature length
    float feature[512];     ///< Feature data
} rkai_feature_float_t;

/**
 * @brief Indicates face search results
 */
typedef struct rkai_search_result_t {
    float similarity;           ///< similarity
    void *face_data;            ///< Custom face structure pointer
} rkai_search_result_t;

/**
 * @brief Indicates face search results
 */
typedef struct rkai_search_result_v2_t {
    float similarity;           ///< similarity
    int face_id;            ///< Face ID
    char user_name[256];
} rkai_search_result_v2_t;

/**
 * @brief Indicates face attribute results
 */
typedef struct rkai_attribute_t {
    int gender;         ///< gender
    int age;            ///< age
} rkai_attribute_t;

/**
 * @brief Indicates the result of a live test
 */
typedef struct rkai_liveness_t {
    float fake_score;   ///< fake probability
    float real_score;   ///< real probability
} rkai_liveness_t;

typedef struct rkai_face_quality_attr_t {
    float sharpness;        ///< sharpness
    float brightness;       ///< brightness
    float center_deviation;
} rkai_face_quality_attr_t;

/**
 * @brief Represents a detected face
 */
typedef struct rkai_det_t {
    int is_face;            // <having face>
    int is_mask;
    int id;                     ///< tracking id
    int is_updated;
    int frontal;                ///< 1: frontal, 0: non-frontal
    rkai_rect_t box;        ///< face bounding box
    float score;                ///< confident score
    int landmark_count;         ///< Number of landmarks
    rkai_point_t landmarks[5];    ///< Landmark points
    rkai_headpose_estimation_t headpose; ///< Head pose estimation
    rkai_face_quality_attr_t attr;  ///< face quality
} rkai_det_t;

/**
 * @brief  Represent a detected Human Pose
 */
typedef struct rkai_pose_det_t {
    int id;                         ///< Tracking id
    int is_updated;
    rkai_rect_t box;                ///< Human Bounding Box
    float score;                    ///< Confident score of Human
    rkai_keypoint_t keypoints[17];  ///< Human keypoints

} rkai_pose_det_t;


/**
 * @brief Represents object detect
 */
typedef struct rkai_object_det_t {
    int id;                     ///< tracking id
    rkai_rect_t box;        ///< face bounding box
    float score;                ///< confident score
} rkai_object_det_t;

/**
 * @brief An array representing detected faces
 */
typedef struct rkai_det_array_t {
    int count;                     ///< Number of detected face. (0<= count <= 128)
    rkai_det_t face[128];      ///< detected face array
} rkai_det_array_t;

/**
 * @brief An array representing the detected human id
 */
typedef struct rkai_det_person_array_t {
    int count;                       ///< number of detected face (0 <= count < 128)
    rkai_det_t person[128];      ///< detected person array
} rkai_det_person_array_t;

/**
 * @brief An array representing the detected human having pose id 
 */
typedef struct rkai_det_human_pose_array_t {
    int count;                          ///< number of detected huamn   
    rkai_pose_det_t human_pose[128];    ///<detected human pose array 
} rkai_det_human_pose_array_t;


/**
 * @brief Face mask test results
 */
typedef struct rkai_mask_t {
    rkai_rect_t face_box;       ///< face bounding box
    float score;                    ///< confident score
    int has_mask;                   ///< Indicate detected face wearing mask or not. 0 - not wear mask, 1 - wear mask
} rkai_mask_t;

/**
 * @brief Face mask detection result array
 */
typedef struct rkai_mask_array_t {
    int count;                                ///< Number of face mask
    rkai_mask_t face_masks[128];          ///< Face mask array
} rkai_mask_array_t;

/**
 * @brief Logger type.
 * 
 */
typedef enum rkai_logger_output_t {
    RKAI_LOGGER_TYPE_PRINT = 0,
    RKAI_LOGGER_TYPE_FILE = 1,
    RKAI_LOGGER_TYPE_BOTH = 2
} rkai_logger_output_t;

/**
 * @brief Log level
 * 
 */
typedef enum rkai_logger_level_t {
    RKAI_LOGGER_LEVEL_ERROR = 0,
    RKAI_LOGGER_LEVEL_WARN = 1,
    RKAI_LOGGER_LEVEL_INFO = 2,
    RKAI_LOGGER_LEVEL_DEBUG = 3
} rkai_logger_level_t;

/**
 * @brief type for install logger
 * 
 */
typedef struct rkai_logger_t {
    rkai_logger_output_t log_output_type;
    int log_level;
} rkai_logger_t;


/**
 * @brief Image Quality Enum 
 * 
 */
typedef enum rkai_image_quality_t {
    RKAI_IMAGE_QUALITY_GOOD = 0,
    RKAI_IMAGE_QUALITY_BLUR = 1,
    RKAI_IMAGE_QUALITY_DISTORTION = 2,
    RKAI_IMAGE_QUALITY_SKEW_FACE = 3,
    RKAI_IMAGE_QUALITY_NOT_GODD = 4
} rkai_image_quality_t;

typedef enum rkai_face_class_t {
    RKAI_FACE_NORMAL_FACE = 0,
    RKAI_FACE_MASK = 1,
    RKAI_FACE_OTHERS_OBJECT = 2
} rkai_face_class_t;

typedef struct rkai_face_classify_result_t {
    float score;
    rkai_face_class_t face_class;
} rkai_face_classify_result_t;

typedef enum rkai_gesture_class_t {
    RKAI_GESTURE_NO_GESTURE = 0,
    RKAI_GESTURE_DISLIKE = 1,
    RKAI_GESTURE_FOUR = 2,
    RKAI_GESTURE_LIKE = 3,
    RKAI_GESTURE_OK = 4,
    RKAI_GESTURE_ONE = 5,
    RKAI_GESTURE_PALM = 6,
    RKAI_GESTURE_PEACE = 7,
    RKAI_GESTURE_PEACE_INVERTED = 8,
    RKAI_GESTURE_THREE = 9,
    RKAI_GESTURE_TWO_UP = 10,
    RKAI_GESTURE_TWO_UP_INVERTED = 11
} rkai_gesture_class_t;

typedef struct rkai_gesture_det_t {
    rkai_gesture_class_t gesture_class;
    rkai_rect_t hand_box;
} rkai_gesture_det_t;

typedef struct rkai_gesture_det_array_t {
    /* data */
    int count;
    rkai_gesture_det_t gestures[128];
} rkai_gesture_det_array_t;

typedef struct rkai_object_det_array_t {
    /* object detect array */
    int count;
    rkai_object_det_t objects[128];
} rkai_object_det_array_t;

typedef struct anchors {
    float *data;
    int num_anchors;
    int anchors_array_size;
    int curr_idx;
} anchors_t;

typedef enum rkai_audio_format_t {
    RKAI_AUDIO_FORMAT_INT16 = 0,
    RKAI_AUDIO_FORMAT_FLOAT = 1
} rkai_audio_format_t;

/*
 * @brief Audio data
 */
typedef struct rkai_audio_t {
    int size;
    int sample_rate;
    int n_seconds;
    int n_channels;
    rkai_audio_format_t format;
    float *data;
} rkai_audio_t;

/*
 * @brief trigger word output
 */
typedef struct rkai_trigger_word_result_t {
    int pass_low_conf;
    int pass_high_conf;
    float score;
} rkai_trigger_word_result_t;

typedef struct rkai_vad_result_t {
    float conf;
    int is_speech;
} rkai_vad_result_t;

/**
 * @brief Trigger model config
 */
typedef struct rkai_trigger_word_model_config_t {
    char model_config_name[256];
    int sample_rate;
    int n_fft;
    int f_max;
    int n_mels;
    int hop_length;
    int win_length;
    int output_size;
    int transpose;
    int htk;
    int norm;
    int norm_mel;
    double log_mel;
} rkai_melspectrogram_config_t;

typedef struct rkai_vad_model_config_t {
    int sample_rate;
    int n_fft;
    int f_max;
    int n_mels;
    int hop_length;
    int win_length;
    int output_size;
    int transpose;
    int htk;
    int norm;
    int norm_mel;
    double log_mel;
} rkai_vad_model_config_t;

typedef struct rkai_melspectrogram_t {
    int size;
    int n_mels;
    int n_frames;
    float *data;
} rkai_melspectrogram_t;
#ifdef __cplusplus
}
#endif
#endif //_SMARTROBOT_RKAI_TYPE_H_
