#include "tracking/base_track.h"

template <typename T, typename U> void BaseTrack<T,U>::InitializeKF() {
    /*** Define constant velocity model ***/
    // state - center_x, center_y, width, height, v_cx, v_cy, v_width, v_height
    // kf_.F_ <<
    //        1, 0, 0, 0, 1, 0, 0, 0,
    //         0, 1, 0, 0, 0, 1, 0, 0,
    //         0, 0, 1, 0, 0, 0, 1, 0,
    //         0, 0, 0, 1, 0, 0, 0, 1,
    //         0, 0, 0, 0, 1, 0, 0, 0,
    //         0, 0, 0, 0, 0, 1, 0, 0,
    //         0, 0, 0, 0, 0, 0, 1, 0,
    //         0, 0, 0, 0, 0, 0, 0, 1;

    kf_.F_ (0, 0) = 1.;
    kf_.F_ (1, 1) = 1.;
    kf_.F_ (2, 2) = 1.;
    kf_.F_ (3, 3) = 1.;
    kf_.F_ (4, 4) = 1.;
    kf_.F_ (5, 5) = 1.;
    kf_.F_ (6, 6) = 1.;
    kf_.F_ (0, 4) = 1.;
    kf_.F_ (1, 5) = 1.;
    kf_.F_ (2, 6) = 1.;
        
    // Give high uncertainty to the unobservable initial velocities
    //kf_.P_ <<
    //       10, 0, 0, 0, 0, 0, 0, 0,
    //        0, 10, 0, 0, 0, 0, 0, 0,
    //        0, 0, 10, 0, 0, 0, 0, 0,
    //        0, 0, 0, 10, 0, 0, 0, 0,
    //        0, 0, 0, 0, 10000, 0, 0, 0,
    //        0, 0, 0, 0, 0, 10000, 0, 0,
    //        0, 0, 0, 0, 0, 0, 10000, 0,
    //        0, 0, 0, 0, 0, 0, 0, 10000;
    // Original values
    kf_.P_ (0, 0) = 1.;
    kf_.P_ (1, 1) = 1.;
    kf_.P_ (2, 2) = 1.;
    kf_.P_ (3, 3) = 1.;
    kf_.P_ (4, 4) = 1.;
    kf_.P_ (5, 5) = 1.;
    kf_.P_ (6, 6) = 1.;

        

    // kf_.H_ << measurement matrix
    //        1, 0, 0, 0, 0, 0, 0, 0,
    //         0, 1, 0, 0, 0, 0, 0, 0,
    //         0, 0, 1, 0, 0, 0, 0, 0,
    //         0, 0, 0, 1, 0, 0, 0, 0;

    kf_.H_ (0, 0) = 1.;
    kf_.H_ (1, 1) = 1.;
    kf_.H_ (2, 2) = 1.;
    kf_.H_ (3, 3) = 1.;

    // kf_.Q_ << process noise covariance matrix
    //        1, 0, 0, 0, 0, 0, 0, 0,
    //         0, 1, 0, 0, 0, 0, 0, 0,
    //         0, 0, 1, 0, 0, 0, 0, 0,
    //         0, 0, 0, 1, 0, 0, 0, 0,
    //         0, 0, 0, 0, 0.01, 0, 0, 0,
    //         0, 0, 0, 0, 0, 0.01, 0, 0,
    //         0, 0, 0, 0, 0, 0, 0.0001, 0,
    //         0, 0, 0, 0, 0, 0, 0, 0.0001;
    kf_.Q_ (0, 0) = 0.02;
    kf_.Q_ (1, 1) = 0.02;
    kf_.Q_ (2, 2) = 0.02;
    kf_.Q_ (3, 3) = 0.02;
    kf_.Q_ (4, 4) = 0.02;
    kf_.Q_ (5, 5) = 0.02;
    kf_.Q_ (6, 6) = 0.02;

    // kf_.R_ << Measurement noise covariance
    //        1, 0, 0,  0,
    //         0, 1, 0,  0,
    //         0, 0, 10, 0,
    //         0, 0, 0,  10;
    kf_.R_ (0, 0) = 0.1;
    kf_.R_ (1, 1) = 0.1;
    kf_.R_ (2, 2) = 0.1;
    kf_.R_ (3, 3) = 0.1;

}

template <typename T, typename U> BaseTrack<T,U>::BaseTrack() : kf_(7, 4) {
    InitializeKF();
    n_points_ = n_points_limit;
}

template <typename T, typename U> BaseTrack<T,U>::BaseTrack(int n_points) :kf_(7, 4) {
    InitializeKF();
    n_points_ = n_points;
}

// Create and initialize new trackers for unmatched detections, with initial bounding box
template <typename T, typename U> void BaseTrack<T,U>::Init(T det) {
    cv::Rect bbox(det.box.left,det.box.top,det.box.right-det.box.left,det.box.bottom-det.box.top);

    DDMatrix head = ConvertBboxToObservation(bbox);

    kf_.x_ (0,0) = head (0,0);
    kf_.x_ (1,0) = head (1,0);
    kf_.x_ (2,0) = head (2,0);
    kf_.x_ (3,0) = head (3,0);

    hit_streak_++;
    score = det.score;
}

// Update matched trackers with assigned detections
template <typename T, typename U> void BaseTrack<T,U>::Update(T det) {
    // get measurement update, reset coast cycle count
    coast_cycles_ = 0;
    // accumulate hit streak count
    hit_streak_++;

    cv::Rect bbox(det.box.left,det.box.top, det.box.right-det.box.left, det.box.bottom-det.box.top);

    // observation - center_x, center_y, area, ratio
    DDMatrix observation = ConvertBboxToObservation(bbox);
    kf_.Update(observation);
    
    //Get updated
    is_updated = 1;
    score = det.score;
}


// Get predicted locations from existing trackers
// dt is time elapsed between the current and previous measurements
template <typename T, typename U> void BaseTrack<T,U>::Predict() {
    kf_.Predict();

    // hit streak count will be reset
    if (coast_cycles_ > 0) {
        hit_streak_ = 0;
    }
    //not update yet
    is_updated = 0;
    // accumulate coast cycle count
    coast_cycles_++;
}

/**
 * Returns the current bounding box estimate
 * @return
 */
template <typename T, typename U> cv::Rect BaseTrack<T,U>::GetStateAsBbox() const {
    return ConvertStateToBbox(kf_.x_);
}

template <typename T, typename U> float BaseTrack<T,U>::GetNIS() const {
    return kf_.NIS_;
}

/**
 * Takes a bounding box in the form [x, y, width, height] and returns z in the form
 * [x, y, s, r] where x,y is the centre of the box and s is the scale/area and r is
 * the aspect ratio
 *
 * @param bbox
 * @return
 */
template <typename T, typename U> DDMatrix BaseTrack<T,U>::ConvertBboxToObservation(const cv::Rect& bbox) const{
    DDMatrix observation(4, 1, 0);
    auto width = static_cast<float>(bbox.width);
    auto height = static_cast<float>(bbox.height);
    float center_x = bbox.x + width / 2;
    float center_y = bbox.y + height / 2;
    float area = width * height;
    float ratio = width / height;
    observation (0, 0) = center_x;
    observation (1, 0) = center_y;
    observation (2, 0) = area;
    observation (3, 0) = ratio;

    return observation;
}


/**
 * Takes a bounding box in the centre form [x,y,s,r] and returns it in the form
 * [x1,y1,x2,y2] where x1,y1 is the top left and x2,y2 is the bottom right
 *
 * @param state
 * @return
 */
template <typename T, typename U> cv::Rect BaseTrack<T,U>::ConvertStateToBbox(DDMatrix state) const {
    // state - center_x, center_y, width, height, v_cx, v_cy, v_width, v_height
    auto s = state(2,0);
    auto r = state(3,0);
    auto cx = state(0,0);
    auto cy = state(1,0);
    float w = sqrt(s * r);
    float h = s / w;
    float x = (cx - w / 2);
    float y = (cy - h / 2);
    if (x < 0 && cx > 0)
    {
        x = 0;
    }
    if (y < 0 && cy > 0)
    {
        y = 0;
    }
    cv::Rect rect(cv::Point(x, y), cv::Size(w, h));
    return rect;
}


template<> BaseTrack<rkai_det_t, rkai_point_t>::BaseTrack() : kf_(7, 4) {
    InitializeKF();
    n_points_ = 5;
}

template<> void BaseTrack<rkai_det_t, rkai_point_t>::Init(rkai_det_t det){
    cv::Rect bbox(det.box.left,det.box.top,det.box.right-det.box.left,det.box.bottom-det.box.top);

    DDMatrix head = ConvertBboxToObservation(bbox);

    kf_.x_ (0,0) = head (0,0);
    kf_.x_ (1,0) = head (1,0);
    kf_.x_ (2,0) = head (2,0);
    kf_.x_ (3,0) = head (3,0);

    for (int i = 0; i < n_points_; i++) {
        keypoints[i].x = det.landmarks[i].x;
        keypoints[i].y = det.landmarks[i].y;
    }
    hit_streak_++;
    score = det.score;
}

template<> void BaseTrack<rkai_det_t, rkai_point_t>::Update(rkai_det_t det){
    // get measurement update, reset coast cycle count
    coast_cycles_ = 0;
    // accumulate hit streak count
    hit_streak_++;

    cv::Rect bbox(det.box.left,det.box.top, det.box.right-det.box.left, det.box.bottom-det.box.top);

    // observation - center_x, center_y, area, ratio
    DDMatrix observation = ConvertBboxToObservation(bbox);
    kf_.Update(observation);
    //Get updated
    is_updated = 1;
    score = det.score;
    //Update landmark position
    for (int i = 0; i< 5;i++){
        keypoints[i].x = det.landmarks[i].x;
        keypoints[i].y = det.landmarks[i].y;
    }
}

template<> BaseTrack<rkai_pose_det_t, rkai_keypoint_t>::BaseTrack() : kf_(7,4) {
    InitializeKF();
    n_points_ = 17;
}

template<> void BaseTrack<rkai_pose_det_t, rkai_keypoint_t>::Init(rkai_pose_det_t det){
    cv::Rect bbox(det.box.left,det.box.top,det.box.right-det.box.left,det.box.bottom-det.box.top);

    DDMatrix head = ConvertBboxToObservation(bbox);

    kf_.x_ (0,0) = head (0,0);
    kf_.x_ (1,0) = head (1,0);
    kf_.x_ (2,0) = head (2,0);
    kf_.x_ (3,0) = head (3,0);

    for (int i = 0; i < n_points_; i++) {
        keypoints[i].x = det.keypoints[i].x;
        keypoints[i].y = det.keypoints[i].y;
        keypoints[i].kpt_score = det.keypoints[i].kpt_score;
    }
    hit_streak_++;
    score = det.score;
}

template<> void BaseTrack<rkai_pose_det_t, rkai_keypoint_t>::Update(rkai_pose_det_t det){
    // get measurement update, reset coast cycle count
    coast_cycles_ = 0;
    // accumulate hit streak count
    hit_streak_++;

    cv::Rect bbox(det.box.left,det.box.top, det.box.right-det.box.left, det.box.bottom-det.box.top);

    // observation - center_x, center_y, area, ratio
    DDMatrix observation = ConvertBboxToObservation(bbox);
    kf_.Update(observation);
    //Get updated
    is_updated = 1;
    score = det.score;
    //Update landmark position
    for (int i = 0; i< 17;i++){
        keypoints[i].x = det.keypoints[i].x;
        keypoints[i].y = det.keypoints[i].y;
        keypoints[i].kpt_score = det.keypoints[i].kpt_score;
    }
}

// Define specializations for the template class
template class BaseTrack<rkai_det_t, rkai_point_t>;
template class BaseTrack<rkai_pose_det_t, rkai_keypoint_t>;
