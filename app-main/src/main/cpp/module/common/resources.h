#ifndef ANDROID_AV_RESOURCES_H
#define ANDROID_AV_RESOURCES_H

#include <jni.h>

void initAssetManager(JNIEnv *env, jobject assetManager);

unsigned char *loadAssetContent(const char *path, size_t &fileSize);

unsigned char *loadFileContent(const char *path, size_t &fileSize);

#endif //ANDROID_AV_RESOURCES_H