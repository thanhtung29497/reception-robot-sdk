
set(RKAI_TRACKING_SOURCE_FILES rkai/src/tracking/kalman_filter.cpp
                                rkai/src/tracking/munkres.cpp
                                rkai/src/tracking/lapjv.cpp
                                rkai/src/tracking/base_track.cpp
                                rkai/src/tracking/base_tracker.cpp)

set(RKAI_TRACKING_SOURCE_FILES ${RKAI_SOURCE_FILES}
                      ${RKAI_TRACKING_SOURCE_FILES} 
                      PARENT_SCOPE)