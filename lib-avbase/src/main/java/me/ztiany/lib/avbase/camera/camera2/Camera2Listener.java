package me.ztiany.lib.avbase.camera.camera2;

import android.hardware.camera2.CameraDevice;
import android.util.Size;

import androidx.annotation.NonNull;

public interface Camera2Listener {

    class CameraData {
        public final CameraDevice cameraDevice;
        public final String cameraId;
        public final Size previewSize;
        public final int orientation;
        public final boolean isMirror;

        public CameraData(
                CameraDevice cameraDevice, String cameraId,
                Size previewSize, int orientation, boolean isMirror
        ) {
            this.cameraDevice = cameraDevice;
            this.cameraId = cameraId;
            this.previewSize = previewSize;
            this.orientation = orientation;
            this.isMirror = isMirror;
        }

        @NonNull
        @Override
        public String toString() {
            return "CameraData{" +
                    "cameraDevice=" + cameraDevice +
                    ", cameraId='" + cameraId + '\'' +
                    ", previewSize=" + previewSize +
                    ", orientation=" + orientation +
                    ", isMirror=" + isMirror +
                    '}';
        }
    }

    /**
     * 当相机打开时执行。
     */
    void onCameraOpened(CameraData cameraData);

    /**
     * 当相机关闭时执行。
     */
    void onCameraClosed();

    /**
     * 当出现异常时执行。
     *
     * @param exception 相机相关异常，都实现了 {@link Camera2Exception} 接口。
     */
    void onCameraError(Exception exception);

}