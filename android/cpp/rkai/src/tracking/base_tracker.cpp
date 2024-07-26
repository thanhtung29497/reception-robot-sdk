
#include "tracking/lapjv.h"
#include "tracking/tracking_matrix.h"
#include "tracking/base_tracker.h"

#define TRACKING_IOU_THESHOLD 0.2
#define MAX_NUM_TRACK 50


template <typename T, typename U> BaseTracker<T,U>::BaseTracker(){
    id_ = 1;
}

template <typename T, typename U> float BaseTracker<T,U>::CalculateIou(T detection, const BaseTrack<T,U>& track) {
    auto trk = track.GetStateAsBbox();
    cv::Rect det(detection.box.left,detection.box.top,detection.box.right-detection.box.left,detection.box.bottom-detection.box.top);

    auto xx1 = std::max(det.tl().x, trk.tl().x);
    auto yy1 = std::max(det.tl().y, trk.tl().y);
    auto xx2 = std::min(det.br().x, trk.br().x);
    auto yy2 = std::min(det.br().y, trk.br().y);
    auto w = std::max(0, xx2 - xx1);
    auto h = std::max(0, yy2 - yy1);

    // calculate area of intersection and union
    float det_area = det.area();
    float trk_area = trk.area();
    auto intersection_area = w * h;
    float union_area = det_area + trk_area - intersection_area;
    auto iou = intersection_area / union_area;
    return iou;
}

template <typename T, typename U> void BaseTracker<T,U>::AssociateDetectionsToTracker(const std::vector<T>& detection,
                                                                    std::map<int, BaseTrack<T,U>>& tracks,
                                                                    std::map<int, T>& matched,
                                                                    std::vector<T>& unmatched_det,
                                                                    float iou_threshold) {
    double *iou_matrix = (double*) malloc(detection.size() * tracks.size() * sizeof(double)); 
    intptr_t nrows = (intptr_t)detection.size();
    intptr_t ncols = (intptr_t)tracks.size(); 
    size_t min_dim = std::min(nrows, ncols);
    int64_t *row_sol = (int64_t*) malloc(nrows * sizeof(int64_t));
    int64_t *col_sol = (int64_t*) malloc(ncols * sizeof(int64_t));

    // row - detection, column - tracks
    for (size_t i = 0; i < detection.size(); i++)
    {
        size_t j = 0;
        for(const auto& trk : tracks)
        {
            iou_matrix[i * tracks.size() + j] = CalculateIou(detection[i], trk.second);
            j++;
        }
    }

    int solver_ret;
    solver_ret = solve_lap(nrows, ncols, iou_matrix,true, row_sol, col_sol);

    // Create a association matrix based on row and column solution
    int *association = (int*) malloc(nrows * ncols * sizeof(int)); 
    memset(association, 0, nrows * ncols * sizeof(int));
    for (size_t i = 0; i <min_dim; i++)
    {
        association[row_sol[i] * ncols + col_sol[i]] = 1;
    }

    // Get the matching track and detection
    
    for (size_t i = 0; i < detection.size(); i++) {
        bool matched_flag = false;
        size_t j = 0;
        for (const auto& trk : tracks) {
            if (1 == association[i * ncols + j]) {
                // Filter out matched with low IOU
                if (iou_matrix[i * ncols + j] >= iou_threshold) {
                    matched[trk.first] = detection[i];
                    matched_flag = true;
                }
                // It builds 1 to 1 association, so we can break from here
                break;
            }
            j++;
        }
        // if detection cannot match with any tracks
        if (!matched_flag) {
            unmatched_det.push_back(detection[i]);
        }
    }
    free(iou_matrix);
    free(row_sol);
    free(col_sol);
}

template<typename T, typename U> void BaseTracker<T,U>::Run(const std::vector<T>& detections, bool enable_autotrack)
{
    for (auto &track: tracks_) {
        track.second.Predict();
    }

        // Hash-map between track ID and associated detection bounding box
    std::map<int, T> matched;
    // vector of unassociated detections
    std::vector<T> unmatched_det;

    // If auto track is enabled, predict only
    if (enable_autotrack) {
        return;
    }
    // If not autotrack, run the correction step

    // return values - matched, unmatched_det
    // Should we associate 
    if (!detections.empty()) {
        AssociateDetectionsToTracker(detections, tracks_, matched, unmatched_det,TRACKING_IOU_THESHOLD);
    }

    // Update tracks with associated bbox 
    for (const auto &match : matched) {
        const auto &ID = match.first;
        tracks_[ID].Update(match.second);
    }


    // Create new tracks for unmatched detections
    for (const auto &det : unmatched_det) {
        
        // Not append if reach max num track
        if (tracks_.size()> MAX_NUM_TRACK)
        {
            break;
        }
        BaseTrack<T,U> tracker;
        tracker.Init(det);
        // Create new track and generate new ID
        tracks_[id_++] = tracker;
    }

    // Delete lose tracked tracks 
    for (auto it = tracks_.begin(); it != tracks_.end();) {
        if (it->second.coast_cycles_ > kMaxCoastCycles) {
            it = tracks_.erase(it);
        } else {
            it++;
        }
    }
}

template <typename T, typename U> std::map<int, BaseTrack<T,U>> BaseTracker<T,U>::GetTracks() {
    return tracks_;
}

template <typename T, typename U> void BaseTracker<T,U>::Flush() {
    tracks_.clear();
}

// Define the specific templates to avoid the linking error
template class BaseTracker<rkai_det_t, rkai_point_t>;
template class BaseTracker<rkai_pose_det_t, rkai_keypoint_t>;
