
set(RKAI_BYTETRACKING_SOURCE_FILES  rkai/src/bytetrack/kalmanFilter.cpp
                                    rkai/src/bytetrack/lapjv_bytetrack.cpp
                                    )

set(RKAI_BYTETRACK_SOURCE_FILES ${RKAI_SOURCE_FILES}
        ${RKAI_BYTETRACKING_SOURCE_FILES}
        PARENT_SCOPE)