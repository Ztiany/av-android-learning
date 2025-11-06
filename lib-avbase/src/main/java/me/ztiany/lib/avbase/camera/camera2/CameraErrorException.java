package me.ztiany.lib.avbase.camera.camera2;

/**
 * 摄像头相关的错误异常类。
 */
public class CameraErrorException extends Exception implements Camera2Exception {

    private final boolean isFatal;

    public CameraErrorException(String message) {
        this(message, true);
    }

    public CameraErrorException(String message, Throwable cause) {
        this(message, cause, true);
    }

    public CameraErrorException(String message, boolean isFatal) {
        super(message);
        this.isFatal = isFatal;
    }

    public CameraErrorException(String message, Throwable cause, boolean isFatal) {
        super(message, cause);
        this.isFatal = isFatal;
    }

    public CameraErrorException(boolean isFatal) {
        super("An error occurred with the camera. isFatal: " + isFatal);
        this.isFatal = isFatal;
    }

    public boolean isFatal() {
        return isFatal;
    }

}