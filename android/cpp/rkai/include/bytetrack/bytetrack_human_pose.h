//
// Created by tanpv on 14/12/2022.
//

#ifndef SDK_BYTETRACK_HUMAN_POSE_H
#define SDK_BYTETRACK_HUMAN_POSE_H

#include "rkai_type.h"
#include "BaseSTrack.h"

class BTrackHumanPose : public BaseSTrack {
public:
    BTrackHumanPose(std::vector<float> tlwh_, float score) : BaseSTrack(tlwh_, score) {};

    ~BTrackHumanPose();

//    void update_info(BTrackHumanPose newtrack);

    void static multi_predict(std::vector<BTrackHumanPose *> &stracks, byte_kalman::KalmanFilter &kalman_filter);

    void re_activate(BTrackHumanPose &new_track, int frame_id, bool new_id = false);

    void update(BTrackHumanPose &new_track, int frame_id);

//    rkai_pose_det_t human_pose;
};

#endif //SDK_BYTETRACK_HUMAN_POSE_H
