#include <jni.h>
#include <vector>
#include <log.h>
#include "common/GLRenderer.h"
#include "sample2/BackgroundRenderer.hpp"
#include "sample2/TriangleRenderer.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemoryLeak"

long createNativeRenderer(jint type) {
    GLRenderer *result = nullptr;
    if (type == BackgroundRenderer::TYPE) {
        auto *renderer = new BackgroundRenderer();
        result = renderer;
    }
    if (type == TriangleRenderer::TYPE) {
        auto *renderer = new TriangleRenderer();
        result = renderer;
    }

    return reinterpret_cast<long>(result);
}

#pragma clang diagnostic pop


extern "C"
JNIEXPORT jlong JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_createNativeRenderer(
        JNIEnv *env,
        jobject thiz,
        jint type
) {
    LOGD("createNativeRenderer");
#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemoryLeak"
    return createNativeRenderer(type);
#pragma clang diagnostic pop
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_onSurfaceCreated(
        JNIEnv *env,
        jobject thiz,
        jlong handle
) {
    LOGD("onSurfaceCreated");
    if (handle == 0) {
        LOGD("onSurfaceCreated: handle == 0, ignored");
        return;
    }
    auto *renderer = reinterpret_cast<GLRenderer *>(handle);
    renderer->onSurfaceCreated();
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_onViewportChanged(
        JNIEnv *env,
        jobject thiz,
        jlong handle,
        jint width,
        jint height
) {
    LOGD("onSurfaceCreated, width = %d, height = %d", width, height);
    if (handle == 0) {
        LOGD("onSurfaceCreated: handle == 0, ignored");
        return;
    }
    auto *renderer = reinterpret_cast<GLRenderer *>(handle);
    renderer->onSurfaceChanged(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_onDrawFrame(
        JNIEnv *env,
        jobject thiz,
        jlong handle
) {
    if (handle == 0) {
        LOGD("onDrawFrame: handle == 0, ignored");
        return;
    }
    auto *renderer = reinterpret_cast<GLRenderer *>(handle);
    renderer->onDrawFrame(nullptr);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_onSurfaceDestroy(
        JNIEnv *env,
        jobject thiz,
        jlong handle
) {
    LOGD("onSurfaceDestroy");
    if (handle == 0) {
        LOGD("onSurfaceDestroy: handle == 0, ignored");
        return;
    }
    auto *renderer = reinterpret_cast<GLRenderer *>(handle);
    renderer->onSurfaceDestroy();
    delete renderer;
}
