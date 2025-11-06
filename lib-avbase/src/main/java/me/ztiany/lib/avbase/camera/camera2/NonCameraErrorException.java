package me.ztiany.lib.avbase.camera.camera2;

/**
 * 非摄像头错误异常。
 * 该异常用于标识在摄像头操作中遇到的非摄像头相关的错误。
 */
public class NonCameraErrorException extends Exception implements Camera2Exception {

    public NonCameraErrorException(String message) {
        super(message);
    }

    public NonCameraErrorException(String message, Throwable cause) {
        super(message, cause);
    }

    public NonCameraErrorException(Throwable cause) {
        super(cause);
    }

    public NonCameraErrorException() {
        super("An error occurred that is not related to camera operations.");
    }

}