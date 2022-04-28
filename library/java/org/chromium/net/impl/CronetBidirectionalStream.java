package org.chromium.net.impl;

import android.os.ConditionVariable;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import org.chromium.net.BidirectionalStream;
import org.chromium.net.CallbackException;
import org.chromium.net.CronetException;
import org.chromium.net.ExperimentalBidirectionalStream;
import org.chromium.net.NetworkException;
import org.chromium.net.RequestFinishedInfo;
import org.chromium.net.UrlResponseInfo;
import org.chromium.net.impl.Annotations.RequestPriority;
import org.chromium.net.impl.CronetBidirectionalState.Event;
import org.chromium.net.impl.CronetBidirectionalState.NextAction;
import org.chromium.net.impl.UrlResponseInfoImpl.HeaderBlockImpl;

import java.net.MalformedURLException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.concurrent.Executor;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import io.envoyproxy.envoymobile.engine.types.EnvoyFinalStreamIntel;
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;
import io.envoyproxy.envoymobile.engine.types.EnvoyStreamIntel;

/**
 * {@link BidirectionalStream} implementation using Envoy-Mobile stack.
 */
public final class CronetBidirectionalStream
    extends ExperimentalBidirectionalStream implements EnvoyHTTPCallbacks {

  private static final String X_ENVOY = "x-envoy";
  private static final String X_ENVOY_SELECTED_TRANSPORT = "x-envoy-upstream-alpn";
  private static final String USER_AGENT = "User-Agent";
  private static final Executor DIRECT_EXECUTOR = new DirectExecutor();

  private final CronetUrlRequestContext mRequestContext;
  private final Executor mExecutor;
  private final VersionSafeCallbacks.BidirectionalStreamCallback mCallback;
  private final String mInitialUrl;
  private final int mInitialPriority;
  private final String mMethod;
  private final boolean mReadOnly; // if mInitialMethod is GET or HEAD, then this is true.
  private final List<Map.Entry<String, String>> mRequestHeaders;
  private final boolean mDelayRequestHeadersUntilFirstFlush;
  private final Collection<Object> mRequestAnnotations;
  private final boolean mTrafficStatsTagSet;
  private final int mTrafficStatsTag;
  private final boolean mTrafficStatsUidSet;
  private final int mTrafficStatsUid;
  private final String mUserAgent;
  private final CancelProofEnvoyStream mStream = new CancelProofEnvoyStream();
  private final CronetBidirectionalState mState = new CronetBidirectionalState();
  private final AtomicInteger mUserflushConcurrentInvocationCount = new AtomicInteger();
  private final AtomicInteger mflushBufferConcurrentInvocationCount = new AtomicInteger();
  private final AtomicReference<CronetException> mException = new AtomicReference<>();
  private final ConditionVariable mStartBlock = new ConditionVariable();

  // Set by start() upon success.
  private Map<String, List<String>> mEnvoyRequestHeaders;

  // Pending write data.
  private final ConcurrentLinkedDeque<WriteBuffer> mPendingData;

  // Flush data queue that should be pushed to the native stack when the previous
  // writevData completes.
  private final ConcurrentLinkedDeque<WriteBuffer> mFlushData;

  /* Final metrics recorded the the Envoy Mobile Engine. May be null */
  private EnvoyFinalStreamIntel mEnvoyFinalStreamIntel;

  private WriteBuffer mLastWriteBufferSent;
  private ByteBuffer mLatestBufferRead;
  private int mLatestBufferReadInitialPosition;
  private int mLatestBufferReadInitialLimit;

  // Only modified on the network thread.
  private UrlResponseInfoImpl mResponseInfo;

  private Runnable mOnDestroyedCallbackForTesting;

  private final class OnReadCompletedRunnable implements Runnable {
    // Buffer passed back from current invocation of onReadCompleted.
    private ByteBuffer mByteBuffer;
    // End of stream flag from current invocation of onReadCompleted.
    private final boolean mEndOfStream;

    OnReadCompletedRunnable(ByteBuffer mByteBuffer, boolean mEndOfStream) {
      this.mByteBuffer = mByteBuffer;
      this.mEndOfStream = mEndOfStream;
    }

    @Override
    public void run() {
      try {
        // Null out mByteBuffer, to pass buffer ownership to callback or release if done.
        ByteBuffer buffer = mByteBuffer;
        mByteBuffer = null;
        @NextAction
        int nextAction =
            mState.nextAction(mEndOfStream ? Event.LAST_READ_COMPLETED : Event.READ_COMPLETED);
        if (nextAction == NextAction.INVOKE_ON_READ_COMPLETED_CALLBACK) {
          mCallback.onReadCompleted(CronetBidirectionalStream.this, mResponseInfo, buffer,
                                    mEndOfStream);
        }
        if (mEndOfStream &&
            mState.nextAction(Event.READY_TO_FINISH) == NextAction.INVOKE_ON_SUCCEEDED) {
          onSucceededOnExecutor();
        }
      } catch (Exception e) {
        onCallbackException(e);
      }
    }
  }

  private final class OnWriteCompletedRunnable implements Runnable {
    // Buffer passed back from current invocation of onWriteCompleted.
    private ByteBuffer mByteBuffer;
    // End of stream flag from current call to write.
    private final boolean mEndOfStream;

    OnWriteCompletedRunnable(ByteBuffer buffer, boolean endOfStream) {
      mByteBuffer = buffer;
      mEndOfStream = endOfStream;
    }

    @Override
    public void run() {
      try {
        // Null out mByteBuffer, to pass buffer ownership to callback or release if done.
        ByteBuffer buffer = mByteBuffer;
        mByteBuffer = null;

        @NextAction
        int nextAction =
            mState.nextAction(mEndOfStream ? Event.LAST_WRITE_COMPLETED : Event.WRITE_COMPLETED);
        if (nextAction == NextAction.INVOKE_ON_WRITE_COMPLETED_CALLBACK) {
          mCallback.onWriteCompleted(CronetBidirectionalStream.this, mResponseInfo, buffer,
                                     mEndOfStream);
        }
        if (mEndOfStream &&
            mState.nextAction(Event.READY_TO_FINISH) == NextAction.INVOKE_ON_SUCCEEDED) {
          onSucceededOnExecutor();
        }
      } catch (Exception e) {
        onCallbackException(e);
      }
    }
  }

  CronetBidirectionalStream(CronetUrlRequestContext requestContext, String url,
                            @CronetEngineBase.StreamPriority int priority, Callback callback,
                            Executor executor, String userAgent, String httpMethod,
                            List<Map.Entry<String, String>> requestHeaders,
                            boolean delayRequestHeadersUntilNextFlush,
                            Collection<Object> requestAnnotations, boolean trafficStatsTagSet,
                            int trafficStatsTag, boolean trafficStatsUidSet, int trafficStatsUid) {
    mRequestContext = requestContext;
    mInitialUrl = url;
    mInitialPriority = convertStreamPriority(priority);
    mCallback = new VersionSafeCallbacks.BidirectionalStreamCallback(callback);
    mExecutor = executor;
    mUserAgent = userAgent;
    mMethod = httpMethod;
    mRequestHeaders = requestHeaders;
    mDelayRequestHeadersUntilFirstFlush = delayRequestHeadersUntilNextFlush;
    mPendingData = new ConcurrentLinkedDeque<>();
    mFlushData = new ConcurrentLinkedDeque<>();
    mRequestAnnotations = requestAnnotations;
    mTrafficStatsTagSet = trafficStatsTagSet;
    mTrafficStatsTag = trafficStatsTag;
    mTrafficStatsUidSet = trafficStatsUidSet;
    mTrafficStatsUid = trafficStatsUid;
    mReadOnly = !doesMethodAllowWriteData(mMethod);
  }

  @Override
  public void start() {
    validateHttpMethod(mMethod);
    for (Map.Entry<String, String> requestHeader : mRequestHeaders) {
      validateHeader(requestHeader.getKey(), requestHeader.getValue());
    }
    mEnvoyRequestHeaders =
        buildEnvoyRequestHeaders(mMethod, mRequestHeaders, mUserAgent, mInitialUrl);
    // Cronet C++ layer exposes reported errors here with an onError callback. EM does not.
    @Nullable CronetException startUpException = engineSimulatedError(mEnvoyRequestHeaders);
    @Event
    int startingEvent =
        startUpException != null ? Event.ERROR
        : mDelayRequestHeadersUntilFirstFlush
            ? (mReadOnly ? Event.USER_START_READ_ONLY : Event.USER_START)
            : (mReadOnly ? Event.USER_START_WITH_HEADERS_READ_ONLY : Event.USER_START_WITH_HEADERS);
    @NextAction int nextAction = mState.nextAction(startingEvent);
    mRequestContext.onRequestStarted();
    if (nextAction == NextAction.INVOKE_ON_FAILED) {
      mException.set(startUpException);
      failWithException();
      return;
    }
    try {
      mRequestContext.setTaskToExecuteWhenInitializationIsCompleted(new Runnable() {
        @Override
        public void run() {
          mStartBlock.open();
        }
      });
      mStartBlock.block();
      mStream.setStream(
          mRequestContext.getEnvoyEngine().startStream(this, /* explicitFlowCrontrol= */ true));
      if (nextAction == NextAction.FLUSH_HEADERS) {
        mStream.sendHeaders(mEnvoyRequestHeaders, mReadOnly);
      }
      onStreamReady();
    } catch (RuntimeException e) {
      // Will be reported when "onCancel" gets invoked.
      reportException(new CronetExceptionImpl("Startup failure", e));
    }
  }

  /**
   * Returns, potentially, an exception to report through the "onError" callback, even though no
   * stream has been created yet. This awkward error reporting solely exists to mimic Cronet.
   */
  @Nullable
  private static CronetException engineSimulatedError(Map<String, List<String>> requestHeaders) {
    if (requestHeaders.get(":scheme").get(0).equals("http")) {
      return new BidirectionalStreamNetworkException("Exception in BidirectionalStream: "
                                                         + "net::ERR_DISALLOWED_URL_SCHEME",
                                                     11, -301);
    }
    return null;
  }

  @Override
  public void read(ByteBuffer buffer) {
    Preconditions.checkHasRemaining(buffer);
    Preconditions.checkDirect(buffer);
    switch (mState.nextAction(Event.USER_READ)) {
    case NextAction.READ:
      recordReadBuffer(buffer);
      mStream.readData(buffer.remaining());
      break;
    case NextAction.INVOKE_ON_READ_COMPLETED:
      // The final read buffer has already been received, or there was no response body.
      onReadCompleted(buffer, 0, buffer.position(), buffer.limit());
      break;
    case NextAction.CARRY_ON:
      recordReadBuffer(buffer);
      // The response header has not been received yet. Read will occur later.
      break;
    default:
      assert false;
    }
  }

  /**
   * Saves the buffer intended to receive the data from the next read.
   */
  void recordReadBuffer(ByteBuffer buffer) {
    mLatestBufferRead = buffer;
    mLatestBufferReadInitialPosition = buffer.position();
    mLatestBufferReadInitialLimit = buffer.limit();
  }

  @Override
  public void write(ByteBuffer buffer, boolean endOfStream) {
    Preconditions.checkDirect(buffer);
    if (!buffer.hasRemaining() && !endOfStream) {
      throw new IllegalArgumentException("Empty buffer before end of stream.");
    }
    if (mState.nextAction(endOfStream ? Event.USER_LAST_WRITE : Event.USER_WRITE) ==
        NextAction.WRITE) {
      mPendingData.add(new WriteBuffer(buffer, endOfStream));
    }
  }

  @Override
  public void flush() {
    if (mState.nextAction(Event.USER_FLUSH) == NextAction.FLUSH_HEADERS) {
      mStream.sendHeaders(mEnvoyRequestHeaders, /* endStream= */ mReadOnly);
    }
    if (mUserflushConcurrentInvocationCount.getAndIncrement() > 0) {
      // Another Thread is already copying pending buffers - can't be done concurrently.
      // However, the thread which started with a zero count will loop until this count goes back
      // to zero. For all intent and purposes, this has a similar outcome as using synchronized {}
      return;
    }
    do {
      WriteBuffer pendingBuffer;
      // A write operation can occur while this "flush" method is being executed. This might look
      // like a breach of contract with the Cronet implementation given that this is not possible
      // with Cronet - equivalent code is under a synchronized block. However, for all intents and
      // purposes, this does not affect the general contract: the race condition remains
      // conceptually identical. With Cronet, a distinct Thread invoking a "write" can be lucky or
      // unlucky, depending if that "write" occurred just before the "flush" or not. With Cronvoy,
      // the same "luck" factor is present: it depends if the "write" sent by the other Thread
      // happens before the end of this loop, or not. In short, there is not any strong ordering
      // guarantees between the flush and write when executed by different Threads.
      while ((pendingBuffer = mPendingData.poll()) != null) {
        mFlushData.add(pendingBuffer);
      }
      sendFlushedDataIfAny();
    } while (mUserflushConcurrentInvocationCount.decrementAndGet() > 0);
  }

  private void sendFlushedDataIfAny() {
    if (mflushBufferConcurrentInvocationCount.getAndIncrement() > 0) {
      // Another Thread is already attempting to flush data - can't be done concurrently.
      // However, the thread which started with a zero count will loop until this count goes back
      // to zero. For all intent and purposes, this has a similar outcome as using synchronized {}
      return;
    }
    do {
      if (!mFlushData.isEmpty() &&
          mState.nextAction(Event.READY_TO_FLUSH) == NextAction.SEND_DATA) {
        WriteBuffer writeBuffer = mFlushData.poll();
        mLastWriteBufferSent = writeBuffer;
        mStream.sendData(writeBuffer.mByteBuffer, writeBuffer.mEndStream);
        if (writeBuffer.mEndStream) {
          // There is no EM final callback - last write is therefore acknowledged immediately.
          onWriteCompleted(writeBuffer);
        }
      }
    } while (mflushBufferConcurrentInvocationCount.decrementAndGet() > 0);
  }

  /**
   * Returns a read-only copy of {@code mPendingData} for testing.
   */
  @VisibleForTesting
  public List<ByteBuffer> getPendingDataForTesting() {
    List<ByteBuffer> pendingData = new LinkedList<>();
    for (WriteBuffer writeBuffer : mPendingData) {
      pendingData.add(writeBuffer.mByteBuffer.asReadOnlyBuffer());
    }
    return pendingData;
  }

  /**
   * Returns a read-only copy of {@code mFlushData} for testing.
   */
  @VisibleForTesting
  public List<ByteBuffer> getFlushDataForTesting() {
    List<ByteBuffer> flushData = new LinkedList<>();
    for (WriteBuffer writeBuffer : mFlushData) {
      flushData.add(writeBuffer.mByteBuffer.asReadOnlyBuffer());
    }
    return flushData;
  }

  @Override
  public void cancel() {
    switch (mState.nextAction(Event.USER_CANCEL)) {
    case NextAction.CANCEL:
      mStream.cancel();
      break;
    case NextAction.INVOKE_ON_CANCELED:
      onCanceledReceived();
      break;
    case NextAction.CARRY_ON:
    case NextAction.TAKE_NO_MORE_ACTIONS:
      // Has already been cancelled, an error condition already registered, or just too late.
      break;
    default:
      assert false;
    }
  }

  @Override
  public boolean isDone() {
    return mState.isDone();
  }

  private void onSucceeded() {
    postTaskToExecutor(new Runnable() {
      @Override
      public void run() {
        onSucceededOnExecutor();
      }
    });
  }

  /*
   * Runs an onSucceeded callback if both Read and Write sides are closed.
   */
  private void onSucceededOnExecutor() {
    cleanup();
    try {
      mCallback.onSucceeded(CronetBidirectionalStream.this, mResponseInfo);
    } catch (Exception e) {
      Log.e(CronetUrlRequestContext.LOG_TAG, "Exception in onSucceeded method", e);
    }
  }

  private void onStreamReady() {
    postTaskToExecutor(new Runnable() {
      @Override
      public void run() {
        try {
          if (!mState.isTerminating()) {
            mCallback.onStreamReady(CronetBidirectionalStream.this);
          }
        } catch (Exception e) {
          onCallbackException(e);
        }
      }
    });
  }

  /**
   * Called when the final set of headers, after all redirects,
   * is received. Can only be called once for each stream.
   */
  private void onResponseHeadersReceived(int httpStatusCode, String negotiatedProtocol,
                                         Map<String, List<String>> headers,
                                         long receivedByteCount) {
    try {
      mResponseInfo = prepareResponseInfoOnNetworkThread(httpStatusCode, negotiatedProtocol,
                                                         headers, receivedByteCount);
    } catch (Exception e) {
      reportException(new CronetExceptionImpl("Cannot prepare ResponseInfo", null));
      return;
    }
    postTaskToExecutor(new Runnable() {
      @Override
      public void run() {
        try {
          if (mState.isTerminating()) {
            return;
          }
          mCallback.onResponseHeadersReceived(CronetBidirectionalStream.this, mResponseInfo);
        } catch (Exception e) {
          onCallbackException(e);
        }
      }
    });
  }

  private void onReadCompleted(ByteBuffer byteBuffer, int bytesRead, int initialPosition,
                               int initialLimit) {
    if (byteBuffer.position() != initialPosition || byteBuffer.limit() != initialLimit) {
      reportException(new CronetExceptionImpl("ByteBuffer modified externally during read", null));
      return;
    }
    if (bytesRead < 0 || initialPosition + bytesRead > initialLimit) {
      reportException(new CronetExceptionImpl("Invalid number of bytes read", null));
      return;
    }
    byteBuffer.position(initialPosition + bytesRead);
    postTaskToExecutor(new OnReadCompletedRunnable(byteBuffer, bytesRead == 0));
  }

  private void onWriteCompleted(WriteBuffer writeBuffer) {
    boolean endOfStream = writeBuffer.mEndStream;
    // Flush if there is anything in the flush queue mFlushData.
    @Event int event = endOfStream ? Event.LAST_FLUSH_DATA_COMPLETED : Event.FLUSH_DATA_COMPLETED;
    if (mState.nextAction(event) == NextAction.TAKE_NO_MORE_ACTIONS) {
      return;
    }
    ByteBuffer buffer = writeBuffer.mByteBuffer;
    if (buffer.position() != writeBuffer.mInitialPosition ||
        buffer.limit() != writeBuffer.mInitialLimit) {
      reportException(new CronetExceptionImpl("ByteBuffer modified externally during write", null));
      return;
    }
    // Current implementation always writes the complete buffer.
    buffer.position(buffer.limit());
    postTaskToExecutor(new OnWriteCompletedRunnable(buffer, endOfStream));
  }

  private void onResponseTrailersReceived(List<Map.Entry<String, String>> trailers) {
    final UrlResponseInfo.HeaderBlock trailersBlock = new HeaderBlockImpl(trailers);
    postTaskToExecutor(new Runnable() {
      @Override
      public void run() {
        try {
          if (mState.isTerminating()) {
            return;
          }
          mCallback.onResponseTrailersReceived(CronetBidirectionalStream.this, mResponseInfo,
                                               trailersBlock);
        } catch (Exception e) {
          onCallbackException(e);
        }
      }
    });
  }

  private void onErrorReceived(int errorCode, int nativeError, int nativeQuicError,
                               String errorString, long receivedByteCount) {
    if (mResponseInfo != null) {
      mResponseInfo.setReceivedByteCount(receivedByteCount);
    }
    CronetException exception;
    if (errorCode == NetworkException.ERROR_QUIC_PROTOCOL_FAILED ||
        errorCode == NetworkException.ERROR_NETWORK_CHANGED) {
      exception = new QuicExceptionImpl("Exception in BidirectionalStream: " + errorString,
                                        errorCode, nativeError, nativeQuicError);
    } else {
      exception = new BidirectionalStreamNetworkException(
          "Exception in BidirectionalStream: " + errorString, errorCode, nativeError);
    }
    mException.set(exception);
    failWithException();
  }

  /**
   * Called when request is canceled, no callbacks will be called afterwards.
   */
  private void onCanceledReceived() {
    cleanup();
    postTaskToExecutor(new Runnable() {
      @Override
      public void run() {
        try {
          mCallback.onCanceled(CronetBidirectionalStream.this, mResponseInfo);
        } catch (Exception e) {
          Log.e(CronetUrlRequestContext.LOG_TAG, "Exception in onCanceled method", e);
        }
      }
    });
  }

  /**
   * Report metrics to listeners.
   */
  private void onMetricsCollected(long requestStartMs, long dnsStartMs, long dnsEndMs,
                                  long connectStartMs, long connectEndMs, long sslStartMs,
                                  long sslEndMs, long sendingStartMs, long sendingEndMs,
                                  long pushStartMs, long pushEndMs, long responseStartMs,
                                  long requestEndMs, boolean socketReused, long sentByteCount,
                                  long receivedByteCount) {
    // Metrics information. Obtained when request succeeds, fails or is canceled.
    RequestFinishedInfo.Metrics mMetrics = new CronetMetrics(
        requestStartMs, dnsStartMs, dnsEndMs, connectStartMs, connectEndMs, sslStartMs, sslEndMs,
        sendingStartMs, sendingEndMs, pushStartMs, pushEndMs, responseStartMs, requestEndMs,
        socketReused, sentByteCount, receivedByteCount);
    final RequestFinishedInfo requestFinishedInfo =
        new RequestFinishedInfoImpl(mInitialUrl, mRequestAnnotations, mMetrics,
                                    mState.getFinishedReason(), mResponseInfo, mException.get());
    mRequestContext.reportRequestFinished(requestFinishedInfo);
  }

  @VisibleForTesting
  public void setOnDestroyedCallbackForTesting(Runnable onDestroyedCallbackForTesting) {
    mOnDestroyedCallbackForTesting = onDestroyedCallbackForTesting;
  }

  private static boolean doesMethodAllowWriteData(String methodName) {
    return !methodName.equals("GET") && !methodName.equals("HEAD");
  }

  private static int convertStreamPriority(@CronetEngineBase.StreamPriority int priority) {
    switch (priority) {
    case Builder.STREAM_PRIORITY_IDLE:
      return RequestPriority.IDLE;
    case Builder.STREAM_PRIORITY_LOWEST:
      return RequestPriority.LOWEST;
    case Builder.STREAM_PRIORITY_LOW:
      return RequestPriority.LOW;
    case Builder.STREAM_PRIORITY_MEDIUM:
      return RequestPriority.MEDIUM;
    case Builder.STREAM_PRIORITY_HIGHEST:
      return RequestPriority.HIGHEST;
    default:
      throw new IllegalArgumentException("Invalid stream priority.");
    }
  }

  /**
   * Posts task to application Executor. Used for callbacks
   * and other tasks that should not be executed on network thread.
   */
  private void postTaskToExecutor(Runnable task) {
    try {
      mExecutor.execute(task);
    } catch (RejectedExecutionException failException) {
      Log.e(CronetUrlRequestContext.LOG_TAG, "Exception posting task to executor", failException);
      // If already in a failed state this invocation is a no-op.
      reportException(new CronetExceptionImpl("Exception posting task to executor", failException));
    }
  }

  private UrlResponseInfoImpl
  prepareResponseInfoOnNetworkThread(int httpStatusCode, String negotiatedProtocol,
                                     Map<String, List<String>> responseHeaders,
                                     long receivedByteCount) {
    List<Map.Entry<String, String>> headers = new ArrayList<>();
    for (Map.Entry<String, List<String>> headerEntry : responseHeaders.entrySet()) {
      String headerKey = headerEntry.getKey();
      if (headerEntry.getValue().get(0) == null) {
        continue;
      }
      if (!headerKey.startsWith(X_ENVOY) && !headerKey.equals("date")) {
        for (String value : headerEntry.getValue()) {
          headers.add(new AbstractMap.SimpleEntry<>(headerKey, value));
        }
      }
    }
    // proxy and caching are not supported.
    UrlResponseInfoImpl responseInfo =
        new UrlResponseInfoImpl(Arrays.asList(mInitialUrl), httpStatusCode, "", headers, false,
                                negotiatedProtocol, null, receivedByteCount);
    return responseInfo;
  }

  private void cleanup() {
    if (mEnvoyFinalStreamIntel != null) {
      recordFinalIntel(mEnvoyFinalStreamIntel);
    }
    mRequestContext.onRequestDestroyed();
    if (mOnDestroyedCallbackForTesting != null) {
      mOnDestroyedCallbackForTesting.run();
    }
  }

  /**
   * Fails the stream with an exception.
   */
  private void failWithException() {
    assert mException.get() != null;
    cleanup();
    mExecutor.execute(new Runnable() {
      @Override
      public void run() {
        try {
          mCallback.onFailed(CronetBidirectionalStream.this, mResponseInfo, mException.get());
        } catch (Exception failException) {
          Log.e(CronetUrlRequestContext.LOG_TAG, "Exception notifying of failed request",
                failException);
        }
      }
    });
  }

  /**
   * If callback method throws an exception, stream gets canceled
   * and exception is reported via onFailed callback.
   * Only called on the Executor.
   */
  private void onCallbackException(Exception e) {
    CallbackException streamError =
        new CallbackExceptionImpl("CalledByNative method has thrown an exception", e);
    Log.e(CronetUrlRequestContext.LOG_TAG, "Exception in CalledByNative method", e);
    reportException(streamError);
  }

  /**
   * Reports an exception. Can be called on any thread. Only the first call is recorded. The
   * error handler will be invoked once a onError, onCancel, or onComplete, has been processed.
   */
  private void reportException(CronetException exception) {
    mException.compareAndSet(null, exception);
    switch (mState.nextAction(Event.ERROR)) {
    case NextAction.CANCEL:
      mStream.cancel();
      break;
    case NextAction.INVOKE_ON_FAILED:
      failWithException();
      break;
    default:
      Log.e(CronetUrlRequestContext.LOG_TAG,
            "An exception has already been previously recorded. This one is ignored.", exception);
    }
  }

  private void recordFinalIntel(EnvoyFinalStreamIntel intel) {
    if (mRequestContext.hasRequestFinishedListener()) {
      onMetricsCollected(intel.getStreamStartMs(), intel.getDnsStartMs(), intel.getDnsEndMs(),
                         intel.getConnectStartMs(), intel.getConnectEndMs(), intel.getSslStartMs(),
                         intel.getSslEndMs(), intel.getSendingStartMs(), intel.getSendingEndMs(),
                         /* pushStartMs= */ -1, /* pushEndMs= */ -1, intel.getResponseStartMs(),
                         intel.getStreamEndMs(), intel.getSocketReused(), intel.getSentByteCount(),
                         intel.getReceivedByteCount());
    }
  }

  private static void validateHttpMethod(String method) {
    if (method == null) {
      throw new NullPointerException("Method is required.");
    }
    if ("OPTIONS".equalsIgnoreCase(method) || "GET".equalsIgnoreCase(method) ||
        "HEAD".equalsIgnoreCase(method) || "POST".equalsIgnoreCase(method) ||
        "PUT".equalsIgnoreCase(method) || "DELETE".equalsIgnoreCase(method) ||
        "TRACE".equalsIgnoreCase(method) || "PATCH".equalsIgnoreCase(method)) {
      return;
    }
    throw new IllegalArgumentException("Invalid http method " + method);
  }

  private static void validateHeader(String header, String value) {
    if (header == null) {
      throw new NullPointerException("Invalid header name.");
    }
    if (value == null) {
      throw new NullPointerException("Invalid header value.");
    }
    if (!isValidHeaderName(header) || value.contains("\r\n")) {
      throw new IllegalArgumentException("Invalid header " + header + "=" + value);
    }
  }

  private static boolean isValidHeaderName(String header) {
    for (int i = 0; i < header.length(); i++) {
      char c = header.charAt(i);
      switch (c) {
      case '(':
      case ')':
      case '<':
      case '>':
      case '@':
      case ',':
      case ';':
      case ':':
      case '\\':
      case '\'':
      case '/':
      case '[':
      case ']':
      case '?':
      case '=':
      case '{':
      case '}':
        return false;
      default: {
        if (Character.isISOControl(c) || Character.isWhitespace(c)) {
          return false;
        }
      }
      }
    }
    return true;
  }

  private static Map<String, List<String>>
  buildEnvoyRequestHeaders(String initialMethod, List<Map.Entry<String, String>> headerList,
                           String userAgent, String currentUrl) {
    Map<String, List<String>> headers = new LinkedHashMap<>();
    final URL url;
    try {
      url = new URL(currentUrl);
    } catch (MalformedURLException e) {
      throw new IllegalArgumentException("Invalid URL", e);
    }
    // TODO: with an empty string it does not always work. Why?
    String path = url.getFile().isEmpty() ? "/" : url.getFile();
    headers.computeIfAbsent(":authority", unused -> new ArrayList<>()).add(url.getAuthority());
    headers.computeIfAbsent(":method", unused -> new ArrayList<>()).add(initialMethod);
    headers.computeIfAbsent(":path", unused -> new ArrayList<>()).add(path);
    headers.computeIfAbsent(":scheme", unused -> new ArrayList<>()).add(url.getProtocol());
    boolean hasUserAgent = false;
    for (Map.Entry<String, String> header : headerList) {
      if (header.getKey().isEmpty()) {
        throw new IllegalArgumentException("Invalid header =");
      }
      hasUserAgent = hasUserAgent ||
                     (header.getKey().equalsIgnoreCase(USER_AGENT) && !header.getValue().isEmpty());
      headers.computeIfAbsent(header.getKey(), unused -> new ArrayList<>()).add(header.getValue());
    }
    if (!hasUserAgent) {
      headers.computeIfAbsent(USER_AGENT, unused -> new ArrayList<>()).add(userAgent);
    }
    // TODO: support H3
    headers.computeIfAbsent("x-envoy-mobile-upstream-protocol", unused -> new ArrayList<>())
        .add("http2");
    return headers;
  }

  @Override
  public Executor getExecutor() {
    return DIRECT_EXECUTOR;
  }

  @Override
  public void onSendWindowAvailable(EnvoyStreamIntel streamIntel) {
    onWriteCompleted(mLastWriteBufferSent);
    sendFlushedDataIfAny();
  }

  @Override
  public void onHeaders(Map<String, List<String>> headers, boolean endStream,
                        EnvoyStreamIntel streamIntel) {
    List<String> statuses = headers.get(":status");
    int httpStatusCode =
        statuses != null && !statuses.isEmpty() ? Integer.parseInt(statuses.get(0)) : -1;
    List<String> transportValues = headers.get(X_ENVOY_SELECTED_TRANSPORT);
    String negotiatedProtocol =
        transportValues != null && !transportValues.isEmpty() ? transportValues.get(0) : "unknown";
    onResponseHeadersReceived(httpStatusCode, negotiatedProtocol, headers,
                              streamIntel.getConsumedBytesFromResponse());

    switch (mState.nextAction(endStream ? Event.ON_HEADERS_END_STREAM : Event.ON_HEADERS)) {
    case NextAction.READ:
      mStream.readData(mLatestBufferRead.remaining());
      break;
    case NextAction.INVOKE_ON_READ_COMPLETED:
      onReadCompleted(mLatestBufferRead, 0, mLatestBufferRead.position(),
                      mLatestBufferRead.limit());
      break;
    case NextAction.CARRY_ON:
    case NextAction.TAKE_NO_MORE_ACTIONS:
      break;
    default:
      assert false;
    }
  }

  @Override
  public void onData(ByteBuffer data, boolean endStream, EnvoyStreamIntel streamIntel) {
    mResponseInfo.setReceivedByteCount(streamIntel.getConsumedBytesFromResponse());
    if (mState.nextAction(endStream ? Event.ON_DATA_END_STREAM : Event.ON_DATA) ==
        NextAction.INVOKE_ON_READ_COMPLETED) {
      ByteBuffer userBuffer = mLatestBufferRead;
      mLatestBufferRead = null;
      // TODO: copy buffer on network Thread - consider doing on the user Thread.
      userBuffer.mark();
      userBuffer.put(data); // NPE ==> BUG, BufferOverflowException ==> User not behaving.
      userBuffer.reset();
      onReadCompleted(userBuffer, data.capacity(), mLatestBufferReadInitialPosition,
                      mLatestBufferReadInitialLimit);
    }
  }

  @Override
  public void onTrailers(Map<String, List<String>> trailers, EnvoyStreamIntel streamIntel) {
    List<Map.Entry<String, String>> headers = new ArrayList<>();
    for (Map.Entry<String, List<String>> headerEntry : trailers.entrySet()) {
      String headerKey = headerEntry.getKey();
      if (headerEntry.getValue().get(0) == null) {
        continue;
      }
      // TODO: make sure which headers should be posted.
      if (!headerKey.startsWith(X_ENVOY) && !headerKey.equals("date") &&
          !headerKey.startsWith(":")) {
        for (String value : headerEntry.getValue()) {
          headers.add(new AbstractMap.SimpleEntry<>(headerKey, value));
        }
      }
    }
    onResponseTrailersReceived(headers);
  }

  @Override
  public void onError(int errorCode, String message, int attemptCount, EnvoyStreamIntel streamIntel,
                      EnvoyFinalStreamIntel finalStreamIntel) {
    mEnvoyFinalStreamIntel = finalStreamIntel;
    switch (mState.nextAction(Event.ON_ERROR)) {
    case NextAction.INVOKE_ON_ERROR_RECEIVED:
      // TODO: fix error scheme.
      onErrorReceived(errorCode, /* nativeError= */ -1,
                      /* nativeQuicError */ 0, message, finalStreamIntel.getReceivedByteCount());
      break;
    case NextAction.INVOKE_ON_FAILED:
      failWithException();
      break;
    default:
      assert false;
    }
  }

  @Override
  public void onCancel(EnvoyStreamIntel streamIntel, EnvoyFinalStreamIntel finalStreamIntel) {
    mEnvoyFinalStreamIntel = finalStreamIntel;
    switch (mState.nextAction(Event.ON_CANCEL)) {
    case NextAction.INVOKE_ON_CANCELED:
      onCanceledReceived();
      break;
    case NextAction.INVOKE_ON_FAILED:
      failWithException();
      break;
    default:
      assert false;
    }
  }

  @Override
  public void onComplete(EnvoyStreamIntel streamIntel, EnvoyFinalStreamIntel finalStreamIntel) {
    mEnvoyFinalStreamIntel = finalStreamIntel;
    switch (mState.nextAction(Event.ON_COMPLETE)) {
    case NextAction.INVOKE_ON_FAILED:
      failWithException();
      break;
    case NextAction.INVOKE_ON_CANCELED:
      onCanceledReceived();
      break;
    case NextAction.INVOKE_ON_SUCCEEDED:
      onSucceeded();
      break;
    case NextAction.CARRY_ON:
      break;
    default:
      assert false;
    }
  }

  private static class WriteBuffer {
    final ByteBuffer mByteBuffer;
    final boolean mEndStream;
    final int mInitialPosition;
    final int mInitialLimit;

    WriteBuffer(ByteBuffer mByteBuffer, boolean mEndStream) {
      this.mByteBuffer = mByteBuffer;
      this.mEndStream = mEndStream;
      this.mInitialPosition = mByteBuffer.position();
      this.mInitialLimit = mByteBuffer.limit();
    }
  }

  private static class DirectExecutor implements Executor {
    @Override
    public void execute(Runnable runnable) {
      runnable.run();
    }
  }
}
