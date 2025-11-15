package me.ztiany.lib.avbase.camera.camera2;

import android.graphics.Matrix;
import android.graphics.RectF;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

import timber.log.Timber;

class PreviewTransformer {


    /**
     * Configures the necessary {@link Matrix} transformation to `mTextureView`.
     * This method should be called after the camera preview size is determined and the
     * size of `mTextureView` is fixed.
     *
     * <p>
     * This method's purpose is to correctly display the camera preview on the screen.
     * </p>
     * <p>
     * It solves two main problems:
     *
     * <ol>
     *     <li>Orientation Mismatch: The camera sensor has a fixed orientation (usually landscape),
     *     but the phone can be held in any orientation (portrait, landscape, upside down). This
     *     method applies the necessary rotation to the camera preview so that it appears upright to
     *     the user. For instance, if the phone is held in portrait (ROTATION_90), it rotates the
     *     preview stream by -90 or 270 degrees.</li>
     *     <li>Aspect Ratio Mismatch: The resolution of the camera preview (cameraPreviewSize) often has
     *     a different aspect ratio than the TextureView displaying it on the screen. To prevent the
     *     image from looking stretched or squashed, this method calculates a transformation matrix.
     *     Specifically, it scales the preview to completely fill the view, which may involve cropping
     *     parts of the image that don't fit. This is often called a "center-crop" effect.</li>
     * </ol>
     *
     * @param viewWidth  The width of `mTextureView`
     * @param viewHeight The height of `mTextureView`
     */
    static void configureTransform(
            TextureView textureView,
            Size cameraPreviewSize,
            int sensorOrientation,
            int rotation,
            String cameraId,
            int viewWidth,
            int viewHeight
    ) {
        if (null == textureView || null == cameraPreviewSize) {
            return;
        }

        Matrix matrix = new Matrix();
        RectF viewRect = new RectF(0, 0, viewWidth, viewHeight);
        RectF bufferRect = new RectF(0, 0, cameraPreviewSize.getHeight(), cameraPreviewSize.getWidth());

        float centerX = viewRect.centerX();
        float centerY = viewRect.centerY();
        bufferRect.offset(centerX - bufferRect.centerX(), centerY - bufferRect.centerY());
        matrix.setRectToRect(viewRect, bufferRect, Matrix.ScaleToFit.FILL);
        float scale = Math.max(
                (float) viewHeight / cameraPreviewSize.getHeight(),
                (float) viewWidth / cameraPreviewSize.getWidth()
        );

        if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
            matrix.postScale(scale, scale, centerX, centerY);
            matrix.postRotate((90 * (rotation - 2)) % 360, centerX, centerY);
            Timber.d(
                    "configureTransform when 90/270, scale = %f, rotate = %d",
                    scale,
                    (90 * (rotation - 2)) % 360
            );
        } else if (Surface.ROTATION_180 == rotation) {
            matrix.postScale(scale, scale, centerX, centerY);
            matrix.postRotate(180, centerX, centerY);
            Timber.d("configureTransform when 180, scale = %f, rotate = 180", scale);
        } else if (Surface.ROTATION_0 == rotation) {
            matrix.postScale(scale, scale, centerX, centerY);
            Timber.d("configureTransform when 0, scale = %f, rotate = 0", scale);
        }

        Timber.d("camera orientation = %d, degree = %d",
                getCameraOrientation(sensorOrientation, rotation, cameraId),
                rotation * 90
        );
        textureView.setTransform(matrix);
    }

    static int getCameraOrientation(int sensorOrientation, int rotation, String cameraId) {
        Timber.d("getCameraOrientation() called with: rotation = ["
                + rotation + "], cameraId = [" + cameraId + "]");

        int degrees = rotation * 90;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
            default:
                break;
        }
        int result;

        if (CameraId.FRONT.equals(cameraId)) {
            result = (sensorOrientation + degrees) % 360;
            result = (360 - result) % 360;
        } else {
            result = (sensorOrientation - degrees + 360) % 360;
        }

        Timber.d("getCameraOrientation: rotation = " + rotation + " result = "
                + result + " sensorOrientation = " + sensorOrientation);

        return result;
    }

}
