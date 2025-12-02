#include "resources.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "log.h"

static AAssetManager *sAssetManager = nullptr;

void initAssetManager(JNIEnv *env, jobject assetManager) {
    sAssetManager = AAssetManager_fromJava(env, assetManager);
    LOGD("sAssetManager has been initialized. result = %d", sAssetManager != nullptr);
}

unsigned char *loadAssetContent(const char *path, size_t &fileSize) {
    unsigned char *fileContent = nullptr;
    fileSize = 0;
    AAsset *asset = AAssetManager_open(sAssetManager, path, AASSET_MODE_UNKNOWN);

    if (asset != nullptr) {
        fileSize = AAsset_getLength(asset);
        fileContent = new unsigned char[fileSize + 1];
        AAsset_read(asset, fileContent, fileSize);
        fileContent[fileSize] = 0;
        AAsset_close(asset);
    }

    return fileContent;
}

unsigned char *loadFileContent(const char *path, size_t &fileSize) {
    unsigned char *fileContent = nullptr;
    fileSize = 0;
    FILE *file = fopen(path, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        fileContent = new unsigned char[fileSize + 1];
        fread(fileContent, 1, fileSize, file);
        fileContent[fileSize] = 0;
        fclose(file);
    }
    return fileContent;
}