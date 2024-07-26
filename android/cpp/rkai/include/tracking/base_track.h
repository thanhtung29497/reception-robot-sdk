#ifndef BASE_TRACK_H
#define BASE_TRACK_H

#include <opencv2/core.hpp>
#include "rkai_type.h"
#include "tracking/kalman_filter.h"

using namespace hmma;
template <typename T, typename U> class BaseTrack {
static_assert(std::is_same<T, rkai_det_t>::value || std::is_same<T, rkai_pose_det_t>::value, "T must be rkai_det_t or rkai_pose_det_t");
static_assert(std::is_same<U, rkai_point_t>::value || std::is_same<U, rkai_keypoint_t>::value, "U must be rkai_point_t or rkai_keypoint_t");

public:
    //Constructor
    BaseTrack();
    BaseTrack(int n_points);
    ~BaseTrack() = default;

    
    static const int n_points_limit = 17;
    void Init(T det);
    void Predict();
    void Update(T det);
    cv::Rect GetStateAsBbox() const;
    float GetNIS() const;

    int is_updated = 0;
    float score = 0.;
    int coast_cycles_ = 0, hit_streak_ = 0;
    U keypoints[n_points_limit];

protected:
    int n_points_;
    KalmanFilter kf_;

    DDMatrix ConvertBboxToObservation(const cv::Rect& bbox) const;
    cv::Rect ConvertStateToBbox(DDMatrix state) const;
    void InitializeKF();
};

template<> BaseTrack<rkai_det_t, rkai_point_t>::BaseTrack();
template<> void BaseTrack<rkai_det_t, rkai_point_t>::Init(rkai_det_t det);
template<> void BaseTrack<rkai_det_t, rkai_point_t>::Update(rkai_det_t det);
template<> BaseTrack<rkai_pose_det_t, rkai_keypoint_t>::BaseTrack();
template<> void BaseTrack<rkai_pose_det_t, rkai_keypoint_t>::Init(rkai_pose_det_t det);
template<> void BaseTrack<rkai_pose_det_t, rkai_keypoint_t>::Update(rkai_pose_det_t det);

#endif