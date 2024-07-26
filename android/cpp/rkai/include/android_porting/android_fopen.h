//
// Created by hungnv4 on 11/12/2023.
//

#ifndef SMARTROBOT_ANDROID_FOPEN_H
#define SMARTROBOT_ANDROID_FOPEN_H

#include <stdio.h>
#include <android/asset_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

/* hijack fopen and route it through the android asset system so that
   we can pull things out of our packagesk APK */

void android_fopen_set_asset_manager(AAssetManager* manager);
AAsset*  android_fopen(const char* fname, const char* mode);
int android_close(void* cookie);
int android_ftell(void *cookie);
int android_read(void* cookie, char* buf, int size);

#define fopen(name, mode) android_fopen(name, mode)

#ifdef __cplusplus
}
#endif

#endif //SMARTROBOT_ANDROID_FOPEN_H
