//
// Created by tannn on 19/01/2024.
//
#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <time.h>

#include "rkai.h"
#include "android_fopen.h"

rkai_handle_t face_detect_handle;

static jclass objCls = NULL;
static jmethodID constructortorId;
static jfieldID xId;
static jfieldID yId;
static jfieldID wId;
static jfieldID hId;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_smart_1robot_FaceDetect_initModel(JNIEnv *env, jobject thiz,
                                                   jobject asset_manager) {
    AAssetManager* mgr = AAssetManager_fromJava(env, asset_manager);
    android_fopen_set_asset_manager(mgr);
    face_detect_handle = rkai_create_handle();
    rkai_init_detector_android(face_detect_handle, mgr);
    LOG_DEBUG("Success");

    jclass localObjCls = env->FindClass("com/example/smart_robot/FaceDetect$Obj");
    objCls = reinterpret_cast<jclass>(env->NewGlobalRef(localObjCls));


    constructortorId = env->GetMethodID(objCls, "<init>", "(Lcom/example/smart_robot/FaceDetect;)V");

    xId = env->GetFieldID(objCls, "x", "F");
    yId = env->GetFieldID(objCls, "y", "F");
    wId = env->GetFieldID(objCls, "w", "F");
    hId = env->GetFieldID(objCls, "h", "F");

}
extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_example_smart_1robot_FaceDetect_detectModel(JNIEnv *env, jobject thiz, jobject bitmap) {
    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env, bitmap, &info);

    if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888){
        // Return
    }

    int width = info.width;
    int height = info.height;
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
        return NULL;

    void *data;
    AndroidBitmap_lockPixels(env, bitmap, &data);
    rkai_image_t image;
    rkai_image_from_rgba_buffer((const char *)data, &image, width, height);
    AndroidBitmap_unlockPixels(env, bitmap);

    rkai_size_t diserted_size = {.w=640, .h=480};
    rkai_image_t resized_image;
    rkai_image_resize_rgb(&image, diserted_size, &resized_image);
    LOG_ERROR("Origin Image width - heigh -stride(%d %d %d)\n", width, height, info.stride);
    rkai_det_array_t detected_face_array;

    LOG_ERROR("Image width - heigh (%d %d)\n", resized_image.width, resized_image.height);
    clock_t begin = clock();
    rkai_ret_t ret = rkai_face_detect(face_detect_handle, &resized_image, &detected_face_array);
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    LOG_ERROR("Number of face: %d - error code: %d execute time: %f\n", detected_face_array.count, (int)ret, time_spent);

    jobjectArray jObjArray = env->NewObjectArray(detected_face_array.count, objCls, NULL);

    float slop= 1;

    float original_ratio = (float)image.width/image.height;
    float resize_ratio = (float)resized_image.width/resized_image.height;
    if(original_ratio > resize_ratio){
        // Padding height. Change slope
        slop = (float)image.width/resized_image.width;
    } else {
        // Padding width
        slop = (float)image.height/resized_image.height;
    }

    for (size_t i=0; i< detected_face_array.count; i++)
    {
        jobject jObj = env->NewObject(objCls, constructortorId, thiz);

        LOG_DEBUG("Detect info: (x y w h): (%d %d %d %d)\n", detected_face_array.face[i].box.left,
                  detected_face_array.face[i].box.top,
                  detected_face_array.face[i].box.right,
                  detected_face_array.face[i].box.bottom);
        env->SetFloatField(jObj, xId, detected_face_array.face[i].box.left*slop);
        env->SetFloatField(jObj, yId, detected_face_array.face[i].box.top*slop);
        env->SetFloatField(jObj, wId, detected_face_array.face[i].box.right*slop);
        env->SetFloatField(jObj, hId, detected_face_array.face[i].box.bottom*slop);

        env->SetObjectArrayElement(jObjArray, i, jObj);
    }

    rkai_image_release(&image);
    rkai_image_release(&resized_image);
    return jObjArray;
}