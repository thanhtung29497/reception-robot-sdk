# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("smartrobot")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/rkai/thirdparty/opencv-mobile-3.4.20-android/sdk/native/jni)
find_package(OpenCV REQUIRED)


set(APP_SOURCE_FILES rkai_trigger_word_jni.cc
                    rkai_vad_jni.cc
                    audio_engine_jni.cc
                    face_detect_jni.cc
                    audio_engine.cc
                    sound_recording.cc
                    recording_callback.cc
                    playing_callback.cc
                    triggerword_callback.cc
                    vad_callback.cc)
add_subdirectory(rkai)

include_directories(${APP_INCLUDE_DIRS})

add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        ${APP_SOURCE_FILES}
        )

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
#target_link_directories(${CMAKE_PROJECT_NAME} PUBLIC rkai/thirdparty/rknpu2/arm64-v8a)

add_library(rknnrt
        SHARED
        IMPORTED)
set_target_properties( # Specifies the target library.
        rknnrt
        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION
        # Provides the path to the library you want to import.
        ${CMAKE_SOURCE_DIR}/rkai/thirdparty/rknpu2/lib/${ANDROID_ABI}/librknnrt.so)

add_library(rga
        SHARED
        IMPORTED)

set_target_properties( # Specifies the target library.
        rga
        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION
        # Provides the path to the library you want to import.
        ${CMAKE_SOURCE_DIR}/rkai/thirdparty/rga/lib/${ANDROID_ABI}/librga.so)

set(LIB_SND_FILE_DIR ../libsndfile)
add_subdirectory(${LIB_SND_FILE_DIR} ./sndfile)
include_directories(${LIB_SND_FILE_DIR}/src)

find_package(oboe REQUIRED CONFIG)

set(APP_THIRDPARTY_LIBS rknnrt rga oboe PARENT_SCOPE)

target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log
        jnigraphics
        ${OpenCV_LIBS}
        rga
        rknnrt
        oboe::oboe
        sndfile)