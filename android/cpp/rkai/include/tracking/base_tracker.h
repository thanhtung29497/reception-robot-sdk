#include <map>
#include <opencv2/core.hpp>

#ifndef SORT_BASETRACK_H
#define SORT_BASETRACK_H
#include "base_track.h"
#include "tracking/utils_tracking.h"

template <typename T, typename U> class BaseTracker {
    // Add static assert here if adding other templates
    static_assert(std::is_same<T, rkai_det_t>::value || std::is_same<T, rkai_pose_det_t>::value, "T must be rkai_det_t or rkai_pose_det_t");
    static_assert(std::is_same<U, rkai_point_t>::value || std::is_same<U, rkai_keypoint_t>::value, "U must be rkai_point_t or rkai_keypoint_t");
public:
    BaseTracker();
    ~BaseTracker() = default;

    static float CalculateIou(T det, const BaseTrack<T,U>& track);
    static void AssociateDetectionsToTracker(const std::vector<T>& detection,
                                             std::map<int, BaseTrack<T,U>>& tracks,
                                             std::map<int, T>& matched,
                                             std::vector<T>& unmatched_det,
                                             float iou_threshold = 0.3);
    void Run(const std::vector<T>& detections, bool enable_autotrack);
    void Flush();
    std::map<int, BaseTrack<T,U>> GetTracks();

private:
    // Hash-map between ID and corresponding tracker
    std::map<int, BaseTrack<T,U>> tracks_;
    int frame_index;
    // Assigned ID for each bounding box
    int id_;
};

#endif