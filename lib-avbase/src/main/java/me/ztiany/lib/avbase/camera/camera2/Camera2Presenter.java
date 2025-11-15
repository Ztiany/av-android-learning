package me.ztiany.lib.avbase.camera.camera2;

import static me.ztiany.lib.avbase.camera.camera2.PreviewTransformer.configureTransform;
import static me.ztiany.lib.avbase.camera.camera2.PreviewTransformer.getCameraOrientation;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Handler;
import android.os.HandlerThread;
import android.text.TextUtils;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.content.ContextCompat;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import timber.log.Timber;

public class Camera2Presenter {

    private final int mRotation;

    private final boolean mIsMirror;

    private Size mPreviewSize;

    @NonNull
    private final SizeSelector mSizeSelector;

    @Nullable
    private OutputProvider mOutputProvider;

    @CameraId
    private String mCameraId;

    @CameraId
    private String mSpecifiedCameraId;

    private final CameraSelector mCameraSelector;

    private Camera2Listener mCamera2Listener;

    @Nullable
    private TextureView mTextureView;

    private final boolean mFitPreview;

    private Context mContext;

    /**
     * A {@link CameraCaptureSession } for camera preview.
     */
    private CameraCaptureSession mCaptureSession;

    /**
     * A reference to the opened {@link CameraDevice}.
     */
    private CameraDevice mCameraDevice;

    private Camera2Presenter(Builder builder) {
        mTextureView = builder.previewView;
        mFitPreview = builder.fitPreview && mTextureView != null;


        mSpecifiedCameraId = builder.specifiedCameraId;
        mCameraSelector = builder.cameraSelector;

        mRotation = builder.rotation;
        mIsMirror = builder.isMirror;

        mOutputProvider = builder.outputProvider;
        mCamera2Listener = builder.mCamera2Listener;
        mSizeSelector = builder.sizeSelector;

        mContext = builder.context;

        if (mTextureView != null && mIsMirror) {
            mTextureView.setScaleX(-1);
        }

        Timber.d("camera builder %s", builder.toString());
    }

    public void switchCamera() {
        if (CameraId.BACK.equals(mCameraId)) {
            mSpecifiedCameraId = CameraId.FRONT;
        } else if (CameraId.FRONT.equals(mCameraId)) {
            mSpecifiedCameraId = CameraId.BACK;
        }
        stop();
        start();
    }

    private final CameraDevice.StateCallback mDeviceStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            Timber.d("StateCallback.onOpened()");

            mCameraOpenCloseLock.release();

            // This method is called when the camera is opened. We start camera preview here.
            mCameraDevice = cameraDevice;
            createPreviewSession(null, null);
            if (mCamera2Listener != null) {
                Camera2Listener.CameraData cameraData = new Camera2Listener.CameraData(
                        cameraDevice,
                        mCameraId,
                        mPreviewSize,
                        getCameraOrientation(mSensorOrientation, mRotation, mCameraId),
                        mIsMirror
                );
                mCamera2Listener.onCameraOpened(cameraData);
            }
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            Timber.d("StateCallback.onDisconnected()");

            mCameraOpenCloseLock.release();

            closeCameraSession();
            closeCameraDevice();
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            Timber.d("StateCallback.onError(): error = %d", error);

            mCameraOpenCloseLock.release();

            closeCameraSession();
            closeCameraDevice();

            if (mCamera2Listener != null) {
                mCamera2Listener.onCameraError(new CameraErrorException("error occurred, code is " + error));
            }
        }

    };

    private final Camera2Handle mCamera2Handle = new Camera2Handle() {

        @Override
        public void startCapturingCameraSession(
                @NonNull Surface surface,
                CameraCaptureSession.StateCallback stateCallback
        ) {
            createPreviewSession(surface, stateCallback);
        }

        @Override
        public void stopCapturingCameraSession() {
            createPreviewSession(null, null);
        }
    };

    /**
     * An additional thread for running tasks that shouldn't block the UI.
     */
    private HandlerThread mBackgroundThread;

    /**
     * A {@link Handler} for running tasks in the background.
     */
    private Handler mBackgroundHandler;

    /**
     * {@link CaptureRequest.Builder} for the camera preview
     */
    private CaptureRequest.Builder mPreviewRequestBuilder;

    /**
     * A {@link Semaphore} to prevent the app from exiting before closing the camera.
     */
    private final Semaphore mCameraOpenCloseLock = new Semaphore(1);

    /**
     * Orientation of the camera sensor
     */
    private int mSensorOrientation;

    public synchronized void start() {
        if (mCameraDevice != null) {
            Timber.d("already started: mCameraDevice is not null!");
            return;
        }
        startBackgroundThread();
        if (mTextureView == null) {
            openCamera();
            return;
        }

        // When the screen is turned off and turned back on, the SurfaceTexture is already
        // available, and "onSurfaceTextureAvailable" will not be called. In that case, we can open
        // a camera and start preview from here (otherwise, we wait until the surface is ready in
        // the SurfaceTextureListener).
        if (mTextureView.isAvailable()) {
            openCamera();
        } else {
            mTextureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {

                @Override
                public void onSurfaceTextureAvailable(@NonNull SurfaceTexture texture, int width, int height) {
                    Timber.d("onSurfaceTextureAvailable: %d, %d", width, height);
                    openCamera();
                }

                @Override
                public void onSurfaceTextureSizeChanged(@NonNull SurfaceTexture texture, int width, int height) {
                    Timber.d("onSurfaceTextureSizeChanged: %d, %d", width, height);
                    if (mFitPreview) {
                        configureTransform(mTextureView, mPreviewSize, mSensorOrientation, mRotation, mCameraId, width, height);
                    }
                }

                @Override
                public boolean onSurfaceTextureDestroyed(@NonNull SurfaceTexture texture) {
                    Timber.d("onSurfaceTextureDestroyed: ");
                    return true;
                }

                @Override
                public void onSurfaceTextureUpdated(@NonNull SurfaceTexture texture) {
                }

            });
        }
    }

    public synchronized void release() {
        stop();
        mTextureView = null;
        mCamera2Listener = null;
        mContext = null;
    }

    public synchronized void stop() {
        closeCamera();
        stopBackgroundThread();
    }

    private boolean setUpCameraOutputs(CameraManager cameraManager) throws CameraAccessException {
        if (mCameraSelector != null) {
            String[] cameraIdList = cameraManager.getCameraIdList();
            String targetId = mCameraSelector.selectCamera(Arrays.asList(cameraIdList));
            if (configCameraParams(cameraManager, targetId)) {
                return true;
            }
        }

        if (!TextUtils.isEmpty(mSpecifiedCameraId) && configCameraParams(cameraManager, mSpecifiedCameraId)) {
            return true;
        }
        for (String cameraId : cameraManager.getCameraIdList()) {
            if (configCameraParams(cameraManager, cameraId)) {
                return true;
            }
        }
        return false;
    }

    private boolean configCameraParams(
            CameraManager manager,
            @CameraId String cameraId
    ) throws CameraAccessException, IllegalArgumentException {
        CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);

        StreamConfigurationMap configurationMap = characteristics.get(
                CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP
        );
        if (configurationMap == null) {
            return false;
        }

        Integer sensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
        if (sensorOrientation != null) {
            mSensorOrientation = sensorOrientation;
        }
        this.mCameraId = cameraId;

        ArrayList<Size> sizes = new ArrayList<>(Arrays.asList(configurationMap.getOutputSizes(SurfaceTexture.class)));
        mPreviewSize = mSizeSelector.getBestSupportedSize(sizes);

        if (mOutputProvider != null) {
            mOutputProvider.onAttach(mCamera2Handle, new OutputProvider.Components() {
                {
                    put(OutputProvider.ORIENTATION, getCameraOrientation(mSensorOrientation, mRotation, cameraId));
                    put(OutputProvider.PREVIEW_SIZE, mPreviewSize);
                    put(OutputProvider.WORKER, mBackgroundHandler);
                    put(OutputProvider.STREAM_CONFIGURATION, configurationMap);
                }
            });
        }
        return true;
    }

    private void openCamera() {
        if (ContextCompat.checkSelfPermission(mContext, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            Timber.e("openCamera failed, no camera permission!");
            if (mCamera2Listener != null) {
                mCamera2Listener.onCameraError(new CameraPermissionException());
            }
            return;
        }

        CameraManager cameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                Timber.e("Time out waiting to lock camera opening.");
                if (mCamera2Listener != null) {
                    mCamera2Listener.onCameraError(new NonCameraErrorException("Time out waiting to lock camera opening."));
                }
                return;
            }

            if (setUpCameraOutputs(cameraManager)) {
                Timber.d("do open camera: %s", mCameraId);
                if (mFitPreview && mTextureView != null) {
                    configureTransform(
                            mTextureView,
                            mPreviewSize,
                            mSensorOrientation,
                            mRotation,
                            mCameraId,
                            mTextureView.getWidth(),
                            mTextureView.getHeight()
                    );
                }
                cameraManager.openCamera(mCameraId, mDeviceStateCallback, mBackgroundHandler);
            } else {
                mCameraOpenCloseLock.release();
                if (mCamera2Listener != null) {
                    mCamera2Listener.onCameraError(new CameraErrorException("setUpCameraOutputs!"));
                }
            }

        } catch (CameraAccessException | SecurityException | IllegalArgumentException exception) {
            Timber.e(exception, "openCamera");
            mCameraOpenCloseLock.release();
            if (mCamera2Listener != null) {
                mCamera2Listener.onCameraError(new CameraErrorException("openCamera failed: "
                        + exception.getMessage(), exception));
            }
        } catch (InterruptedException exception) {
            Timber.e(exception, "openCamera");
            if (mCamera2Listener != null) {
                mCamera2Listener.onCameraError(new NonCameraErrorException("openCamera failed: "
                        + exception.getMessage(), exception));
            }
        }
    }

    /**
     * Closes the current {@link CameraDevice}.
     */
    private void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();

            closeCameraSession();

            if (null != mOutputProvider) {
                mOutputProvider.onDetach();
                mOutputProvider = null;
            }

            closeCameraDevice();

        } catch (InterruptedException exception) {
            if (mCamera2Listener != null) {
                mCamera2Listener.onCameraError(new CameraErrorException("closeCamera failed: "
                        + exception.getMessage(), exception, false));
            }
        } finally {
            mCameraOpenCloseLock.release();
        }
    }

    /**
     * Starts a background thread and its {@link Handler}.
     */
    private void startBackgroundThread() {
        stopBackgroundThread();

        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    /**
     * Stops the background thread and its {@link Handler}.
     */
    private void stopBackgroundThread() {
        if (mBackgroundThread == null) {
            return;
        }
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException interruptedException) {
            Timber.e(interruptedException, "stopBackgroundThread");
        }
    }

    /**
     * Creates a new {@link CameraCaptureSession} for camera preview.
     */
    private void createPreviewSession(
            @Nullable Surface outputSurface,
            @Nullable CameraCaptureSession.StateCallback callback
    ) {
        closeCameraSession();
        List<Surface> targets = new ArrayList<>();

        try {
            if (mTextureView != null) {
                SurfaceTexture texture = mTextureView.getSurfaceTexture();
                Timber.d("texture is null! no preview session will be created!");
                if (texture != null) {
                    // We configure the size of default buffer to be the size of camera preview we want.
                    texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
                    // This is the output Surface we need to start preview.
                    Surface surface = new Surface(texture);
                    targets.add(surface);
                }
            }
            // added through CameraHandle
            if (outputSurface != null) {
                targets.add(outputSurface);
            }
            // added through OutputProvider
            if (mOutputProvider != null) {
                Surface providedSurface = mOutputProvider.provideSurface();
                if (providedSurface != null && !targets.contains(providedSurface)) {
                    targets.add(providedSurface);
                }
            }
            if (targets.isEmpty()) {
                return;
            }

            // We set up a CaptureRequest.Builder with the output Surface.
            mPreviewRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewRequestBuilder.set(
                    CaptureRequest.CONTROL_AF_MODE,
                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE
            );
            for (Surface target : targets) {
                mPreviewRequestBuilder.addTarget(target);
            }
            // Here, we create a CameraCaptureSession for camera preview.
            CameraCaptureSession.StateCallback configureFailed = new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(@NonNull CameraCaptureSession session) {
                    Timber.d("StateCallback.onConfigured()");
                    // The camera is already closed
                    if (null == mCameraDevice) {
                        return;
                    }
                    startPreview(session);
                    if (callback != null) {
                        callback.onConfigured(session);
                    }
                }

                @Override
                public void onConfigureFailed(@NonNull CameraCaptureSession session) {
                    Timber.d("StateCallback.onConfigureFailed()");

                    if (mCamera2Listener != null) {
                        mCamera2Listener.onCameraError(new CameraErrorException("configureFailed"));
                    }

                    if (callback != null) {
                        callback.onConfigureFailed(session);
                    }
                }
            };

            mCameraDevice.createCaptureSession(targets, configureFailed, mBackgroundHandler);
        } catch (CameraAccessException cameraAccessException) {
            Timber.e(cameraAccessException, "createCameraPreviewSession");
            if (mCamera2Listener != null) {
                mCamera2Listener.onCameraError(new CameraErrorException("createCaptureSession failed: "
                        + cameraAccessException.getMessage(), cameraAccessException));
            }
        }
    }

    private void startPreview(@NonNull CameraCaptureSession cameraCaptureSession) {
        if (mCameraDevice == null) {
            Timber.w("startPreview is called but camera is closed.");
            return;
        }

        // When the session is ready, we start displaying the preview.
        mCaptureSession = cameraCaptureSession;

        try {
            mCaptureSession.setRepeatingRequest(
                    mPreviewRequestBuilder.build(),
                    new CameraCaptureSession.CaptureCallback() {
                        @Override
                        public void onCaptureFailed(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull CaptureFailure failure) {
                            Timber.d("onCaptureFailed");
                        }
                    },
                    mBackgroundHandler
            );
        } catch (Exception exception) {
            Timber.e(exception, "setRepeatingRequest");
            if (mCamera2Listener != null) {
                mCamera2Listener.onCameraError(new CameraErrorException("startPreview failed: "
                        + exception.getMessage(), exception));
            }
        }
    }

    /**
     * close camera session.
     */
    private void closeCameraSession() {
        if (mCaptureSession != null) {
            try {
                mCaptureSession.close();
            } catch (Exception exception) {
                Timber.e(exception, "closeCameraSession");
            }
            mCaptureSession = null;
        }
    }

    private void closeCameraDevice() {
        if (null == mCameraDevice) {
            Timber.d("mCameraDevice is already null.");
            return;
        }
        try {
            mCameraDevice.close();
        } catch (Exception e) {
            Timber.d("closeCameraDevice");
        }
        mCameraDevice = null;

        if (mCamera2Listener != null) {
            mCamera2Listener.onCameraClosed();
        }
    }

    public static final class Builder {

        /**
         * 上下文，用于获取 CameraManager。
         */
        private Context context;

        /**
         * 传入 getWindowManager().getDefaultDisplay().getRotation() 的值即可。
         */
        private int rotation;

        /**
         * 是否镜像显示，只支持 textureView。
         */
        private boolean isMirror;

        /**
         * 预览显示的 view，目前仅支持 textureView。
         */
        private TextureView previewView;

        /**
         * transform preview so it correctly displays the camera preview on the screen.
         */
        private boolean fitPreview = true;

        /**
         * 相机选择器。
         */
        private CameraSelector cameraSelector;

        /**
         * 指定的相机 ID。
         */
        @CameraId
        private String specifiedCameraId;

        /**
         * 事件回调
         */
        private Camera2Listener mCamera2Listener;

        private OutputProvider outputProvider;

        private SizeSelector sizeSelector;

        public Builder() {
        }

        public Builder previewOn(TextureView textureView) {
            this.previewView = textureView;
            return this;
        }

        public Builder fitPreview(boolean fitPreview) {
            this.fitPreview = fitPreview;
            return this;
        }

        public Builder isMirror(boolean isMirror) {
            this.isMirror = isMirror;
            return this;
        }

        public Builder rotation(int rotation) {
            this.rotation = rotation;
            return this;
        }

        public Builder cameraId(@CameraId String cameraId) {
            this.specifiedCameraId = cameraId;
            return this;
        }

        public Builder cameraSelector(CameraSelector cameraSelector) {
            this.cameraSelector = cameraSelector;
            return this;
        }

        public Builder cameraListener(Camera2Listener val) {
            this.mCamera2Listener = val;
            return this;
        }

        public Builder outputProvider(OutputProvider outputProvider) {
            this.outputProvider = outputProvider;
            return this;
        }

        public Builder sizeSelector(SizeSelector sizeSelector) {
            this.sizeSelector = sizeSelector;
            return this;
        }

        public Builder context(Context val) {
            this.context = val;
            return this;
        }

        public Camera2Presenter build() {
            if (mCamera2Listener == null) {
                Timber.w("camera2Listener is null, callback will not be called!");
            }
            if (sizeSelector == null) {
                throw new NullPointerException("you must provide a sizeSelector!");
            }
            return new Camera2Presenter(this);
        }

        @NonNull
        @Override
        public String toString() {
            return "Builder{" +
                    "previewDisplayView=" + previewView +
                    ", isMirror=" + isMirror +
                    ", specificCameraId='" + specifiedCameraId + '\'' +
                    ", rotation=" + rotation +
                    ", context=" + context +
                    '}';
        }

    }

}