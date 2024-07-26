#ifndef  STRACK_H
#define  STRACK_H
#include <vector>
#include "kalmanFilter.h"
#include "rkai_type.h"
#include "dataType.h"
enum TrackState { New = 0, Tracked, Lost, Removed };

template<typename T>
class BaseSTrack
{
public:
    BaseSTrack(T object);
	~BaseSTrack();

	void static multi_predict(std::vector<BaseSTrack*> &stracks, byte_kalman::KalmanFilter &kalman_filter);
	void mark_lost();
	void mark_removed();
	int end_frame();
	void activate(byte_kalman::KalmanFilter &kalman_filter, int frame_id);
	void re_activate(BaseSTrack &new_track, int frame_id, bool new_id = false);
	void update(BaseSTrack &new_track, int frame_id);
    void static_tlwh();
    void static_tlbr();

public:
    bool is_updated;
	bool is_activated;
	int track_id;
	int state;

    T rkai_tracking_object;
	std::vector<float> _tlwh;
	std::vector<float> tlwh;
	std::vector<float> tlbr;
	int frame_id;
	int tracklet_len;
	int start_frame;

	KAL_MEAN mean;
	KAL_COVA covariance;
	float score;

private:
    std::vector<float> static tlbr_to_tlwh(std::vector<float> &tlbr);
    void update_info(const BaseSTrack &new_track);
    std::vector<float> tlwh_to_xyah(const std::vector<float> &tlwh_tmp);
    std::vector<float> to_xyah();
    int next_id();

	byte_kalman::KalmanFilter kalman_filter;
};


template<typename T>
BaseSTrack<T>::BaseSTrack(T object) {
    _tlwh.resize(4);
    _tlwh[0] = object.box.left;
    _tlwh[1] = object.box.top;
    _tlwh[2] = object.box.right - object.box.left;
    _tlwh[3] = object.box.bottom - object.box.top;

    is_activated = true;
    is_updated = false;
    track_id = 0;
    state = TrackState::New;

    tlwh.resize(4);
    tlbr.resize(4);

    static_tlwh();
    static_tlbr();
    rkai_tracking_object = object;

    frame_id = 0;
    tracklet_len = 0;
    this->score = object.score;
    start_frame = 0;
}

template<typename T>
BaseSTrack<T>::~BaseSTrack() {}

template<typename T>
void BaseSTrack<T>::activate(byte_kalman::KalmanFilter &kalman_filter, int frame_id) {
    this->kalman_filter = kalman_filter;
    this->track_id = this->next_id();

    std::vector<float> _tlwh_tmp(4);
    _tlwh_tmp[0] = this->_tlwh[0];
    _tlwh_tmp[1] = this->_tlwh[1];
    _tlwh_tmp[2] = this->_tlwh[2];
    _tlwh_tmp[3] = this->_tlwh[3];
    std::vector<float> xyah = tlwh_to_xyah(_tlwh_tmp);
    DETECTBOX xyah_box;
    xyah_box[0] = xyah[0];
    xyah_box[1] = xyah[1];
    xyah_box[2] = xyah[2];
    xyah_box[3] = xyah[3];
    auto mc = this->kalman_filter.initiate(xyah_box);
    this->mean = mc.first;
    this->covariance = mc.second;

    static_tlwh();
    static_tlbr();

    this->tracklet_len = 0;
    this->state = TrackState::Tracked;
    if (frame_id == 1) {
        this->is_activated = true;
    }
    //this->is_activated = true;
    this->frame_id = frame_id;
    this->start_frame = frame_id;
}

template<typename T>
void BaseSTrack<T>::re_activate(BaseSTrack &new_track, int frame_id, bool new_id) {
    std::vector<float> xyah = tlwh_to_xyah(new_track.tlwh);
    DETECTBOX xyah_box;
    xyah_box[0] = xyah[0];
    xyah_box[1] = xyah[1];
    xyah_box[2] = xyah[2];
    xyah_box[3] = xyah[3];
    auto mc = this->kalman_filter.update(this->mean, this->covariance, xyah_box);
    this->mean = mc.first;
    this->covariance = mc.second;
    this->is_updated = true;

    static_tlwh();
    static_tlbr();
    update_info(new_track);
    this->tracklet_len = 0;
    this->state = TrackState::Tracked;
    this->is_activated = true;
    this->frame_id = frame_id;
    this->score = new_track.score;
    if (new_id){
        this->track_id = next_id();
    }
}

template<typename T>
void BaseSTrack<T>::update(BaseSTrack &new_track, int frame_id) {
    this->frame_id = frame_id;
    this->tracklet_len++;

    std::vector<float> xyah = tlwh_to_xyah(new_track.tlwh);
    DETECTBOX xyah_box;
    xyah_box[0] = xyah[0];
    xyah_box[1] = xyah[1];
    xyah_box[2] = xyah[2];
    xyah_box[3] = xyah[3];

    auto mc = this->kalman_filter.update(this->mean, this->covariance, xyah_box);
    this->mean = mc.first;
    this->covariance = mc.second;

    static_tlwh();
    static_tlbr();
    update_info(new_track);
    this->state = TrackState::Tracked;
    this->is_activated = true;
    this->is_updated = true;

    this->score = new_track.score;
}

template<typename T>
void BaseSTrack<T>::update_info(const BaseSTrack &new_track)
{
    this->rkai_tracking_object = new_track.rkai_tracking_object;
}

template<typename T>
void BaseSTrack<T>::static_tlwh() {
    if (this->state == TrackState::New) {
        tlwh[0] = _tlwh[0];
        tlwh[1] = _tlwh[1];
        tlwh[2] = _tlwh[2];
        tlwh[3] = _tlwh[3];
        return;
    }

    tlwh[0] = mean[0];
    tlwh[1] = mean[1];
    tlwh[2] = mean[2];
    tlwh[3] = mean[3];

    tlwh[2] *= tlwh[3];
    tlwh[0] -= tlwh[2] / 2;
    tlwh[1] -= tlwh[3] / 2;
}

template<typename T>
void BaseSTrack<T>::static_tlbr() {
    tlbr.clear();
    tlbr.assign(tlwh.begin(), tlwh.end());
    tlbr[2] += tlbr[0];
    tlbr[3] += tlbr[1];
}

template<typename T>
std::vector<float> BaseSTrack<T>::tlwh_to_xyah(const std::vector<float> &tlwh_tmp) {
    std::vector<float> tlwh_output = tlwh_tmp;
    tlwh_output[0] += tlwh_output[2] / 2;
    tlwh_output[1] += tlwh_output[3] / 2;
    tlwh_output[2] /= tlwh_output[3];
    return tlwh_output;
}

template<typename T>
std::vector<float> BaseSTrack<T>::to_xyah() {
    return tlwh_to_xyah(tlwh);
}

template<typename T>
std::vector<float> BaseSTrack<T>::tlbr_to_tlwh(std::vector<float> &tlbr) {
    tlbr[2] -= tlbr[0];
    tlbr[3] -= tlbr[1];
    return tlbr;
}

template<typename T>
void BaseSTrack<T>::mark_lost() {
    state = TrackState::Lost;
}

template<typename T>
void BaseSTrack<T>::mark_removed() {
    state = TrackState::Removed;
}

template<typename T>
int BaseSTrack<T>::next_id() {
    static int _count = 0;
    _count++;
    return _count;
}

template<typename T>
int BaseSTrack<T>::end_frame() {
    return this->frame_id;
}

template<typename T>
void BaseSTrack<T>::multi_predict(std::vector<BaseSTrack*> &stracks, byte_kalman::KalmanFilter &kalman_filter)
{
    for (int i = 0; i < stracks.size(); i++)
    {
        if (stracks[i]->state != TrackState::Tracked)
        {
            stracks[i]->mean[7] = 0;
        }
        kalman_filter.predict(stracks[i]->mean, stracks[i]->covariance);
        stracks[i]->is_updated = false;
        stracks[i]->static_tlwh();
        stracks[i]->static_tlbr();
    }
}

#endif