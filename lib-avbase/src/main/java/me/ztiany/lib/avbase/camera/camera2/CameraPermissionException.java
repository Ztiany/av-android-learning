package me.ztiany.lib.avbase.camera.camera2;

/**
 * 操作摄像头时由于没有权限而抛出的异常。
 */
public class CameraPermissionException extends Exception implements Camera2Exception {

    public CameraPermissionException(String message) {
        super(message);
    }

    public CameraPermissionException(String message, Throwable cause) {
        super(message, cause);
    }

    public CameraPermissionException(Throwable cause) {
        super(cause);
    }

    public CameraPermissionException() {
        super("Camera permission is denied.");
    }

}