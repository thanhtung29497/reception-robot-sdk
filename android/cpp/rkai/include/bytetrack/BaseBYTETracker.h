#ifndef BYTE_TRACKER_H
#define BYTE_TRACKER_H

#include <vector>
#include "opencv2/opencv.hpp"
#include "BaseSTrack.h"
#include <fstream>
#include <queue>
#include <iostream>
#include "lapjv_bytetrack.h"
#include <map>
#include <iostream>


template<template<typename> typename T, typename U>
class BaseBYTETracker {
public:
    BaseBYTETracker(int frame_rate = 30, int track_buffer = 30);
    ~BaseBYTETracker();

    void update(const std::vector <U> &objects, std::vector <T<U>> &output_stracks, int track_frame);
    cv::Scalar get_color(int idx);
    std::vector <T<U>> tracked_stracks;
    void Flush();

private:

    std::vector<T<U> *> joint_stracks(std::vector<T<U> *> &tlista, std::vector <T<U>> &tlistb);

    std::vector <T<U>> joint_stracks(std::vector <T<U>> &tlista, std::vector <T<U>> &tlistb);

    std::vector <T<U>> sub_stracks(std::vector <T<U>> &tlista, std::vector <T<U>> &tlistb);

    void remove_duplicate_stracks(std::vector <T<U>> &resa, std::vector <T<U>> &resb, std::vector <T<U>> &stracksa,
                                  std::vector <T<U>> &stracksb);

    void
    linear_assignment(std::vector <std::vector<float>> &cost_matrix, int cost_matrix_size,
                      int cost_matrix_size_size, float thresh, std::vector <std::vector<int>> &matches,
                      std::vector<int> &unmatched_a, std::vector<int> &unmatched_b);

    std::vector <std::vector<float>>
    iou_distance(std::vector<T<U> *> &atracks, std::vector <T<U>> &btracks, int &dist_size, int &dist_size_size);

    std::vector <std::vector<float>> iou_distance(std::vector <T<U>> &atracks,
                                                  std::vector <T<U>> &btracks);

    std::vector <std::vector<float>> ious(std::vector <std::vector<float>> &atlbrs,
                                          std::vector <std::vector<float>> &btlbrs);

    double lapjv(const std::vector <std::vector<float>> &cost, std::vector<int> &rowsol, std::vector<int> &colsol,
                 bool extend_cost = false, float cost_limit = LONG_MAX, bool return_cost = true);


private:

    float track_thresh;
    float high_thresh;
    float match_thresh;
    int frame_id;
    int max_time_lost;

    std::vector <T<U>> lost_stracks;
    std::vector <T<U>> removed_stracks;

    byte_kalman::KalmanFilter kalman_filter;
};

//source file

template<template<typename> typename T, typename U>
BaseBYTETracker<T, U>::BaseBYTETracker(int frame_rate, int track_buffer) {
    track_thresh = 0.5;
    high_thresh = 0.6;
    match_thresh = 0.8;

    frame_id = 0;
    max_time_lost = int(frame_rate / 30.0 * track_buffer);
}

template<template<typename> typename T, typename U>
BaseBYTETracker<T, U>::~BaseBYTETracker() {
}

template<template<typename> typename T, typename U>
void BaseBYTETracker<T,U>::Flush() {
    for (auto &strack : this->tracked_stracks) {
        strack.mark_removed();
    }
    this->lost_stracks.clear();
    this->tracked_stracks.clear();
    this->removed_stracks.clear();
}

template<template<typename> typename T, typename U>
void BaseBYTETracker<T,U>::update(const std::vector <U> &objects, std::vector <T<U>> &output_stracks, int track_frame) {

    ////////////////// Step 1: Get detections //////////////////
    this->frame_id++;
    std::vector <T<U>> activated_stracks;
    std::vector <T<U>> refind_stracks;
    std::vector <T<U>> removed_stracks;
    std::vector <T<U>> lost_stracks;
    std::vector <T<U>> detections;
    std::vector <T<U>> detections_low;

    std::vector <T<U>> detections_cp;
    std::vector <T<U>> tracked_stracks_swap;
    std::vector <T<U>> resa, resb;

    std::vector <T<U> *> unconfirmed;
    std::vector <T<U> *> tracked_stracks;
    std::vector <T<U> *> strack_pool;
    std::vector <T<U> *> r_tracked_stracks;
    std::vector <T<U> *> lost_track_out;

    if (objects.size() > 0) {
        for (int i = 0; i < objects.size(); i++) {

            float score = objects[i].score;

            T<U> strack(objects[i]);
            if (score >= track_thresh) {
                detections.push_back(strack);
            } else {
                detections_low.push_back(strack);
            }
        }
    }

    // Add newly detected tracklets to tracked_stracks
    for (int i = 0; i < this->tracked_stracks.size(); i++) {
        if (!this->tracked_stracks[i].is_activated)
            unconfirmed.push_back(&this->tracked_stracks[i]);
        else
            tracked_stracks.push_back(&this->tracked_stracks[i]);
    }

    ////////////////// Step 2: First association, with IoU //////////////////
    strack_pool = joint_stracks(tracked_stracks, this->lost_stracks);
    T<U>::multi_predict(strack_pool, this->kalman_filter);

    std::vector <std::vector<float>> dists;
    int dist_size = 0, dist_size_size = 0;
    dists = iou_distance(strack_pool, detections, dist_size, dist_size_size);

    std::vector <std::vector<int>> matches;
    std::vector<int> u_track, u_detection;
    linear_assignment(dists, dist_size, dist_size_size, match_thresh, matches, u_track, u_detection);

    for (int i = 0; i < matches.size(); i++) {
        T<U> *track = strack_pool[matches[i][0]];
        T<U> *det = &detections[matches[i][1]];
        if (track->state == TrackState::Tracked) {

            track->update(*det, this->frame_id);
            activated_stracks.push_back(*track);
        } else {
            track->re_activate(*det, this->frame_id, false);
            refind_stracks.push_back(*track);
        }
    }

    ////////////////// Step 3: Second association, using low score dets //////////////////
    for (int i = 0; i < u_detection.size(); i++) {
        detections_cp.push_back(detections[u_detection[i]]);
    }
    detections.clear();
    detections.assign(detections_low.begin(), detections_low.end());

    for (int i = 0; i < u_track.size(); i++) {
        if (strack_pool[u_track[i]]->state == TrackState::Tracked) {
            r_tracked_stracks.push_back(strack_pool[u_track[i]]);
        }
    }

    dists.clear();
    dists = iou_distance(r_tracked_stracks, detections, dist_size, dist_size_size);

    matches.clear();
    u_track.clear();
    u_detection.clear();
    linear_assignment(dists, dist_size, dist_size_size, 0.5, matches, u_track, u_detection);

    for (int i = 0; i < matches.size(); i++) {
        T<U> *track = r_tracked_stracks[matches[i][0]];
        T<U> *det = &detections[matches[i][1]];
        if (track->state == TrackState::Tracked) {
            track->update(*det, this->frame_id);
            activated_stracks.push_back(*track);
        } else {
            track->re_activate(*det, this->frame_id, false);
            refind_stracks.push_back(*track);
        }
    }

    for (int i = 0; i < u_track.size(); i++) {
        T<U> *track = r_tracked_stracks[u_track[i]];
        if (track->state != TrackState::Lost) {
            track->mark_lost();
            lost_stracks.push_back(*track);
        }
    }

    // Deal with unconfirmed tracks, usually tracks with only one beginning frame
    detections.clear();
    detections.assign(detections_cp.begin(), detections_cp.end());

    dists.clear();
    dists = iou_distance(unconfirmed, detections, dist_size, dist_size_size);

    matches.clear();
    std::vector<int> u_unconfirmed;
    u_detection.clear();
    linear_assignment(dists, dist_size, dist_size_size, 0.7, matches, u_unconfirmed, u_detection);

    for (int i = 0; i < matches.size(); i++) {
        unconfirmed[matches[i][0]]->update(detections[matches[i][1]], this->frame_id);
        activated_stracks.push_back(*unconfirmed[matches[i][0]]);
    }

    for (int i = 0; i < u_unconfirmed.size(); i++) {
        T<U> *track = unconfirmed[u_unconfirmed[i]];
        track->mark_removed();
        removed_stracks.push_back(*track);
    }

    ////////////////// Step 4: Init new stracks //////////////////
    for (int i = 0; i < u_detection.size(); i++) {
        T<U> *track = &detections[u_detection[i]];
        if (track->score < this->high_thresh)
            continue;
        track->activate(this->kalman_filter, this->frame_id);
        activated_stracks.push_back(*track);
    }

    ////////////////// Step 5: Update state //////////////////
    for (int i = 0; i < this->lost_stracks.size(); i++) {
        if (this->frame_id - this->lost_stracks[i].end_frame() > this->max_time_lost) {
            this->lost_stracks[i].mark_removed();
            removed_stracks.push_back(this->lost_stracks[i]);
        }
    }

    for (int i = 0; i < this->tracked_stracks.size(); i++) {
        if (this->tracked_stracks[i].state == TrackState::Tracked) {
            tracked_stracks_swap.push_back(this->tracked_stracks[i]);
        }
    }
    this->tracked_stracks.clear();
    this->tracked_stracks.assign(tracked_stracks_swap.begin(), tracked_stracks_swap.end());

    this->tracked_stracks = joint_stracks(this->tracked_stracks, activated_stracks);
    this->tracked_stracks = joint_stracks(this->tracked_stracks, refind_stracks);

    this->lost_stracks = sub_stracks(this->lost_stracks, this->tracked_stracks);
    for (int i = 0; i < lost_stracks.size(); i++) {
        this->lost_stracks.push_back(lost_stracks[i]);
    }

    this->lost_stracks = sub_stracks(this->lost_stracks, this->removed_stracks);
    this->removed_stracks.clear();
    for (int i = 0; i < removed_stracks.size(); i++) {
        this->removed_stracks.push_back(removed_stracks[i]);
    }

    remove_duplicate_stracks(resa, resb, this->tracked_stracks, this->lost_stracks);

    this->tracked_stracks.clear();
    this->tracked_stracks.assign(resa.begin(), resa.end());
    this->lost_stracks.clear();
    this->lost_stracks.assign(resb.begin(), resb.end());

    
    for (int i = 0; i < this->lost_stracks.size(); i++) {
        if (this->frame_id - this->lost_stracks[i].end_frame() < track_frame + 1) {
            this->lost_stracks[i].mean[7] = 0;
            this->kalman_filter.predict(this->lost_stracks[i].mean, this->lost_stracks[i].covariance);
            this->lost_stracks[i].static_tlwh();
            this->lost_stracks[i].static_tlbr();
            output_stracks.push_back(this->lost_stracks[i]);
        }
    }

    for (int i = 0; i < this->tracked_stracks.size(); i++) {
        if (this->tracked_stracks[i].is_activated) {
            output_stracks.push_back(this->tracked_stracks[i]);
        }
    }
}

using namespace std;

template<template<typename> typename T, typename U>
vector<T<U> *> BaseBYTETracker<T, U>::joint_stracks(vector<T<U> *> &tlista, vector <T<U>> &tlistb) {
    map<int, int> exists;
    vector < T<U> * > res;
    for (int i = 0; i < tlista.size(); i++) {
        exists.insert(pair<int, int>(tlista[i]->track_id, 1));
        res.push_back(tlista[i]);
    }
    for (int i = 0; i < tlistb.size(); i++) {
        int tid = tlistb[i].track_id;
        if (!exists[tid] || exists.count(tid) == 0) {
            exists[tid] = 1;
            res.push_back(&tlistb[i]);
        }
    }
    return res;
}

template<template<typename> typename T, typename U>
vector <T<U>> BaseBYTETracker<T, U>::joint_stracks(vector <T<U>> &tlista, vector <T<U>> &tlistb) {
    map<int, int> exists;
    vector <T<U>> res;
    for (int i = 0; i < tlista.size(); i++) {
        exists.insert(pair<int, int>(tlista[i].track_id, 1));
        res.push_back(tlista[i]);
    }
    for (int i = 0; i < tlistb.size(); i++) {
        int tid = tlistb[i].track_id;
        if (!exists[tid] || exists.count(tid) == 0) {
            exists[tid] = 1;
            res.push_back(tlistb[i]);
        }
    }
    return res;
}

template<template<typename> typename T, typename U>
vector <T<U>> BaseBYTETracker<T, U>::sub_stracks(vector <T<U>> &tlista, vector <T<U>> &tlistb) {
    map<int, T<U>> stracks;
    for (int i = 0; i < tlista.size(); i++) {
        stracks.insert(pair<int, T<U>>(tlista[i].track_id, tlista[i]));
    }
    for (int i = 0; i < tlistb.size(); i++) {
        int tid = tlistb[i].track_id;
        if (stracks.count(tid) != 0) {
            stracks.erase(tid);
        }
    }

    vector <T<U>> res;
    typename map<int, T<U>>::iterator it ;
    for (it = stracks.begin(); it != stracks.end(); ++it) {
        res.push_back(it->second);
    }

    return res;
}

template<template<typename> typename T, typename U>
void BaseBYTETracker<T, U>::remove_duplicate_stracks(vector <T<U>> &resa, vector <T<U>> &resb, vector <T<U>> &stracksa,
                                                     vector <T<U>> &stracksb) {
    vector <vector<float>> pdist = iou_distance(stracksa, stracksb);
    vector <pair<int, int>> pairs;
    for (int i = 0; i < pdist.size(); i++) {
        for (int j = 0; j < pdist[i].size(); j++) {
            if (pdist[i][j] < 0.15) {
                pairs.push_back(pair<int, int>(i, j));
            }
        }
    }

    vector<int> dupa, dupb;
    for (int i = 0; i < pairs.size(); i++) {
        int timep = stracksa[pairs[i].first].frame_id - stracksa[pairs[i].first].start_frame;
        int timeq = stracksb[pairs[i].second].frame_id - stracksb[pairs[i].second].start_frame;
        if (timep > timeq)
            dupb.push_back(pairs[i].second);
        else
            dupa.push_back(pairs[i].first);
    }

    for (int i = 0; i < stracksa.size(); i++) {
        vector<int>::iterator iter = find(dupa.begin(), dupa.end(), i);
        if (iter == dupa.end()) {
            resa.push_back(stracksa[i]);
        }
    }

    for (int i = 0; i < stracksb.size(); i++) {
        vector<int>::iterator iter = find(dupb.begin(), dupb.end(), i);
        if (iter == dupb.end()) {
            resb.push_back(stracksb[i]);
        }
    }
}

template<template<typename> typename T, typename U>
void
BaseBYTETracker<T,U>::linear_assignment(vector <vector<float>> &cost_matrix, int cost_matrix_size, int cost_matrix_size_size,
                                        float thresh,
                                        vector <vector<int>> &matches, vector<int> &unmatched_a, vector<int> &unmatched_b) {
    if (cost_matrix.size() == 0) {
        for (int i = 0; i < cost_matrix_size; i++) {
            unmatched_a.push_back(i);
        }
        for (int i = 0; i < cost_matrix_size_size; i++) {
            unmatched_b.push_back(i);
        }
        return;
    }

    vector<int> rowsol;
    vector<int> colsol;
    float c = lapjv(cost_matrix, rowsol, colsol, true, thresh);
    for (int i = 0; i < rowsol.size(); i++) {
        if (rowsol[i] >= 0) {
            vector<int> match;
            match.push_back(i);
            match.push_back(rowsol[i]);
            matches.push_back(match);
        } else {
            unmatched_a.push_back(i);
        }
    }

    for (int i = 0; i < colsol.size(); i++) {
        if (colsol[i] < 0) {
            unmatched_b.push_back(i);
        }
    }
}

template<template<typename> typename T, typename U>
vector <vector<float>> BaseBYTETracker<T,U>::ious(vector <vector<float>> &atlbrs, vector <vector<float>> &btlbrs) {
    vector <vector<float>> ious;
    if (atlbrs.size() * btlbrs.size() == 0)
        return ious;

    ious.resize(atlbrs.size());
    for (int i = 0; i < ious.size(); i++) {
        ious[i].resize(btlbrs.size());
    }

    //bbox_ious
    for (int k = 0; k < btlbrs.size(); k++) {
        float box_area = (btlbrs[k][2] - btlbrs[k][0] + 1) * (btlbrs[k][3] - btlbrs[k][1] + 1);
        for (int n = 0; n < atlbrs.size(); n++) {
            float iw = min(atlbrs[n][2], btlbrs[k][2]) - max(atlbrs[n][0], btlbrs[k][0]) + 1;
            if (iw > 0) {
                float ih = min(atlbrs[n][3], btlbrs[k][3]) - max(atlbrs[n][1], btlbrs[k][1]) + 1;
                if (ih > 0) {
                    float ua =
                            (atlbrs[n][2] - atlbrs[n][0] + 1) * (atlbrs[n][3] - atlbrs[n][1] + 1) + box_area - iw * ih;
                    ious[n][k] = iw * ih / ua;
                } else {
                    ious[n][k] = 0.0;
                }
            } else {
                ious[n][k] = 0.0;
            }
        }
    }
    return ious;
}

template<template<typename> typename T, typename U>
vector <vector<float>>
BaseBYTETracker<T,U>::iou_distance(vector<T<U> *> &atracks, vector <T<U>> &btracks, int &dist_size, int &dist_size_size) {
    vector <vector<float>> cost_matrix;
    if (atracks.size() * btracks.size() == 0) {
        dist_size = atracks.size();
        dist_size_size = btracks.size();
        return cost_matrix;
    }
    vector <vector<float>> atlbrs, btlbrs;
    for (int i = 0; i < atracks.size(); i++) {
        atlbrs.push_back(atracks[i]->tlbr);
    }
    for (int i = 0; i < btracks.size(); i++) {
        btlbrs.push_back(btracks[i].tlbr);
    }

    dist_size = atracks.size();
    dist_size_size = btracks.size();

    vector <vector<float>> _ious = ious(atlbrs, btlbrs);

    for (int i = 0; i < _ious.size(); i++) {
        vector<float> _iou;
        for (int j = 0; j < _ious[i].size(); j++) {
            _iou.push_back(1 - _ious[i][j]);
        }
        cost_matrix.push_back(_iou);
    }

    return cost_matrix;
}

template<template<typename> typename T, typename U>
vector <vector<float>> BaseBYTETracker<T,U>::iou_distance(vector <T<U>> &atracks, vector <T<U>> &btracks) {
    vector <vector<float>> atlbrs, btlbrs;
    for (int i = 0; i < atracks.size(); i++) {
        atlbrs.push_back(atracks[i].tlbr);
    }
    for (int i = 0; i < btracks.size(); i++) {
        btlbrs.push_back(btracks[i].tlbr);
    }

    vector <vector<float>> _ious = ious(atlbrs, btlbrs);
    vector <vector<float>> cost_matrix;
    for (int i = 0; i < _ious.size(); i++) {
        vector<float> _iou;
        for (int j = 0; j < _ious[i].size(); j++) {
            _iou.push_back(1 - _ious[i][j]);
        }
        cost_matrix.push_back(_iou);
    }

    return cost_matrix;
}

template<template<typename> typename T, typename U>
double BaseBYTETracker<T,U>::lapjv(const vector <vector<float>> &cost, vector<int> &rowsol, vector<int> &colsol,
                                   bool extend_cost, float cost_limit, bool return_cost) {
    vector <vector<float>> cost_c;
    cost_c.assign(cost.begin(), cost.end());

    vector <vector<float>> cost_c_extended;

    int n_rows = cost.size();
    int n_cols = cost[0].size();
    rowsol.resize(n_rows);
    colsol.resize(n_cols);

    int n = 0;
    if (n_rows == n_cols) {
        n = n_rows;
    } else {
        if (!extend_cost) {
            cout << "set extend_cost=True" << endl;
            (void)system("pause");
            exit(0);
        }
    }

    if (extend_cost || cost_limit < LONG_MAX) {
        n = n_rows + n_cols;
        cost_c_extended.resize(n);
        for (int i = 0; i < cost_c_extended.size(); i++)
            cost_c_extended[i].resize(n);

        if (cost_limit < LONG_MAX) {
            for (int i = 0; i < cost_c_extended.size(); i++) {
                for (int j = 0; j < cost_c_extended[i].size(); j++) {
                    cost_c_extended[i][j] = cost_limit / 2.0;
                }
            }
        } else {
            float cost_max = -1;
            for (int i = 0; i < cost_c.size(); i++) {
                for (int j = 0; j < cost_c[i].size(); j++) {
                    if (cost_c[i][j] > cost_max)
                        cost_max = cost_c[i][j];
                }
            }
            for (int i = 0; i < cost_c_extended.size(); i++) {
                for (int j = 0; j < cost_c_extended[i].size(); j++) {
                    cost_c_extended[i][j] = cost_max + 1;
                }
            }
        }

        for (int i = n_rows; i < cost_c_extended.size(); i++) {
            for (int j = n_cols; j < cost_c_extended[i].size(); j++) {
                cost_c_extended[i][j] = 0;
            }
        }
        for (int i = 0; i < n_rows; i++) {
            for (int j = 0; j < n_cols; j++) {
                cost_c_extended[i][j] = cost_c[i][j];
            }
        }

        cost_c.clear();
        cost_c.assign(cost_c_extended.begin(), cost_c_extended.end());
    }

    double **cost_ptr;
    cost_ptr = new double *[sizeof(double *) * n];
    for (int i = 0; i < n; i++)
        cost_ptr[i] = new double[sizeof(double) * n];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cost_ptr[i][j] = cost_c[i][j];
        }
    }

    int *x_c = new int[sizeof(int) * n];
    int *y_c = new int[sizeof(int) * n];

    int ret = lapjv_internal(n, cost_ptr, x_c, y_c);
    if (ret != 0) {
        cout << "Calculate Wrong!" << endl;
        (void)system("pause");
        exit(0);
    }

    double opt = 0.0;

    if (n != n_rows) {
        for (int i = 0; i < n; i++) {
            if (x_c[i] >= n_cols)
                x_c[i] = -1;
            if (y_c[i] >= n_rows)
                y_c[i] = -1;
        }
        for (int i = 0; i < n_rows; i++) {
            rowsol[i] = x_c[i];
        }
        for (int i = 0; i < n_cols; i++) {
            colsol[i] = y_c[i];
        }

        if (return_cost) {
            for (int i = 0; i < rowsol.size(); i++) {
                if (rowsol[i] != -1) {
                    //cout << i << "\t" << rowsol[i] << "\t" << cost_ptr[i][rowsol[i]] << endl;
                    opt += cost_ptr[i][rowsol[i]];
                }
            }
        }
    } else if (return_cost) {
        for (int i = 0; i < rowsol.size(); i++) {
            opt += cost_ptr[i][rowsol[i]];
        }
    }

    for (int i = 0; i < n; i++) {
        delete[]cost_ptr[i];
    }
    delete[]cost_ptr;
    delete[]x_c;
    delete[]y_c;

    return opt;
}

template<template<typename> typename T, typename U>
cv::Scalar BaseBYTETracker<T,U>::get_color(int idx) {
    idx += 3;
    return cv::Scalar(37 * idx % 255, 17 * idx % 255, 29 * idx % 255);
}

#endif