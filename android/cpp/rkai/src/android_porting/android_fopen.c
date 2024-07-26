//
// Created by hungnv4 on 11/12/2023.
//

#include <stdio.h>

#include "android_fopen.h"
#include <errno.h>
#include <android/asset_manager.h>

#include "logger.h"

int android_read(void* cookie, char* buf, int size) {
    return AAsset_read((AAsset*)cookie, buf, size);
}

static int android_write(void* cookie, const char* buf, int size) {
    return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek(void* cookie, fpos_t offset, int whence) {
    return AAsset_seek((AAsset*)cookie, offset, whence);
}

int android_close(void* cookie) {
    AAsset_close((AAsset*)cookie);
    return 0;
}

int android_ftell(void *cookie){
    return AAsset_getLength((AAsset *)cookie);
}

// must be established by someone else...
AAssetManager* android_asset_manager;
void android_fopen_set_asset_manager(AAssetManager* manager) {
    android_asset_manager = manager;
}

AAsset* android_fopen(const char* fname, const char* mode) {
    if(mode[0] == 'w') return NULL;

    AAsset* asset = AAssetManager_open(android_asset_manager, fname, 0);
    if(!asset) return NULL;

    return asset;
}