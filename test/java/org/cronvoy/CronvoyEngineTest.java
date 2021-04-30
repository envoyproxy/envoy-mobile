package org.cronvoy;

import static org.assertj.core.api.Assertions.assertThat;

import android.content.Context;
import androidx.test.core.app.ApplicationProvider;
import io.envoyproxy.envoymobile.engine.AndroidJniLibrary;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.AbstractMap.SimpleImmutableEntry;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import okhttp3.mockwebserver.MockResponse;
import okhttp3.mockwebserver.MockWebServer;
import org.chromium.net.CronetException;
import org.chromium.net.ExperimentalUrlRequest;
import org.chromium.net.UploadDataProviders;
import org.chromium.net.UrlRequest;
import org.chromium.net.UrlResponseInfo;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class CronvoyEngineTest {

  private static final String TEST_URL_PATH = "get/flowers";

  private static CronvoyEngine cronvoyEngine;

  private final MockWebServer mockWebServer = new MockWebServer();

  @BeforeClass
  public static void loadJniLibrary() {
    AndroidJniLibrary.loadTestLibrary();
  }

  @AfterClass
  public static void shutdown() {
    if (cronvoyEngine != null) {
      cronvoyEngine.shutdown();
    }
  }

  @Before
  public void setUp() {
    if (cronvoyEngine == null) {
      Context appContext = ApplicationProvider.getApplicationContext();
      cronvoyEngine =
          new CronvoyEngine(new CronvoyEngineBuilderImpl(appContext).setUserAgent("Cronvoy"));
    }
  }

  @After
  public void shutdownMockWebServer() throws IOException {
    mockWebServer.shutdown();
  }

  @Test
  public void simpleGet() throws Exception {
    mockWebServer.enqueue(new MockResponse().setBody("hello, world"));
    mockWebServer.start();
    Request request = new Request().addResponseBuffer(12);

    Response response = sendRequest(request);

    assertThat(response.getBodyAsString()).isEqualTo("hello, world");
  }

  @Test
  public void simpleGet_noBody() throws Exception {
    mockWebServer.enqueue(new MockResponse().setResponseCode(200));
    mockWebServer.start();
    Request request = new Request().addResponseBuffer(1); // At least one byte must be available.

    Response response = sendRequest(request);

    assertThat(response.getResponseCode()).isEqualTo(200);
    assertThat(response.getBodyAsString()).isEmpty();
    assertThat(response.getCronetException()).isNull();
  }

  @Test
  public void simpleGet_withSmallBuffers() throws Exception {
    mockWebServer.enqueue(new MockResponse().setBody("hello, world"));
    mockWebServer.start();
    Request request = new Request().addResponseBuffer(4).addResponseBuffer(3).addResponseBuffer(5);

    Response response = sendRequest(request);

    assertThat(response.getBodyAsString()).isEqualTo("hello, world");
  }

  @Test
  public void simpleGet_withNotEnoughBuffer() throws Exception {
    mockWebServer.enqueue(new MockResponse().setBody("hello, world"));
    mockWebServer.start();
    Request request = new Request().addResponseBuffer(11);

    Response response = sendRequest(request);

    assertThat(response.getCronetException()).hasCauseInstanceOf(IllegalStateException.class);
    assertThat(response.getCronetException().getCause()).hasMessageContaining("No more buffer");
    assertThat(response.getBodyAsString()).isEqualTo("hello, worl"); // One byte short.
  }

  private Response sendRequest(Request request) {
    ConcurrentLinkedQueue<ByteBuffer> buffers = new ConcurrentLinkedQueue<>(request.responseBody);
    LinkedBlockingQueue<Response> blockedResponse = new LinkedBlockingQueue<>(1);

    ExperimentalUrlRequest.Builder builder = cronvoyEngine.newUrlRequestBuilder(
        mockWebServer.url(request.urlPath).toString(), new UrlRequest.Callback() {
          @Override
          public void onRedirectReceived(UrlRequest urlRequest, UrlResponseInfo info,
                                         String newLocationUrl) {
            throw new UnsupportedOperationException("Not yet supported");
          }

          @Override
          public void onResponseStarted(UrlRequest urlRequest, UrlResponseInfo info) {
            ByteBuffer buffer = buffers.peek();
            if (buffer == null) {
              throw new IllegalStateException("No response buffer provided.");
            }
            urlRequest.read(buffer);
          }

          @Override
          public void onReadCompleted(UrlRequest urlRequest, UrlResponseInfo info,
                                      ByteBuffer byteBuffer) {
            ByteBuffer buffer = buffers.peek();
            if (buffer == null) {
              throw new IllegalStateException("Can't happen...");
            }
            if (!buffer.hasRemaining()) {
              buffers.poll();
              buffer = buffers.peek();
              if (buffer == null) {
                throw new IllegalStateException("No more buffer");
              }
            }
            urlRequest.read(buffer);
          }

          @Override
          public void onSucceeded(UrlRequest urlRequest, UrlResponseInfo info) {
            try {
              blockedResponse.put(new Response(info).setBody(request.responseBody));
            } catch (InterruptedException e) {
              throw new RuntimeException(e);
            }
          }

          @Override
          public void onFailed(UrlRequest urlRequest, UrlResponseInfo info, CronetException error) {
            try {
              blockedResponse.put(
                  new Response(info).setCronetException(error).setBody(request.responseBody));
            } catch (InterruptedException e) {
              throw new RuntimeException(e);
            }
          }

          @Override
          public void onCanceled(UrlRequest urlRequest, UrlResponseInfo info) {
            try {
              blockedResponse.put(new Response(info).setBody(request.responseBody).setCancelled());
            } catch (InterruptedException e) {
              throw new RuntimeException(e);
            }
          }
        }, Executors.newSingleThreadExecutor());

    if (request.requestBody != null) {
      builder.setUploadDataProvider(UploadDataProviders.create(request.requestBody),
                                    Executors.newSingleThreadExecutor());
    }

    for (Map.Entry<String, String> entry : request.header) {
      builder.addHeader(entry.getKey(), entry.getValue());
    }

    builder.setHttpMethod(request.httpMethod).build().start();

    try {
      return blockedResponse.take();
    } catch (InterruptedException e) {
      throw new RuntimeException(e);
    }
  }

  private static class Request {
    String httpMethod = "GET";
    String urlPath = TEST_URL_PATH;
    byte[] requestBody = null;
    final List<Map.Entry<String, String>> header = new ArrayList<>();
    final List<ByteBuffer> responseBody = new ArrayList<>();

    Request setHttpMethod(String httpMethod) {
      this.httpMethod = httpMethod;
      return this;
    }

    Request setUrlPath(String urlPath) {
      this.urlPath = urlPath;
      return this;
    }

    Request setRequestBody(byte[] requestBody) {
      this.requestBody = requestBody;
      return this;
    }

    Request addHeader(String key, String value) {
      header.add(new SimpleImmutableEntry<>(key, value));
      return this;
    }

    Request addResponseBuffer(int size) {
      responseBody.add(ByteBuffer.allocateDirect(size));
      return this;
    }
  }

  private static class Response {

    private final UrlResponseInfo urlResponseInfo;
    private CronetException cronetException;
    private byte[] body;
    private boolean cancelled = false;

    Response(UrlResponseInfo urlResponseInfo) { this.urlResponseInfo = urlResponseInfo; }

    Response setCronetException(CronetException cronetException) {
      this.cronetException = cronetException;
      return this;
    }

    Response setBody(List<ByteBuffer> responseBody) {
      int totalSize = responseBody.stream().mapToInt(ByteBuffer::position).sum();
      body = new byte[totalSize];
      int pos = 0;
      for (ByteBuffer buffer : responseBody) {
        int bytesToRead = buffer.position();
        buffer.rewind();
        buffer.get(body, pos, bytesToRead);
        pos += bytesToRead;
      }
      return this;
    }

    Response setCancelled() {
      cancelled = true;
      return this;
    }

    CronetException getCronetException() { return cronetException; }

    UrlResponseInfo getUrlResponseInfo() { return urlResponseInfo; }

    boolean isCancelled() { return cancelled; }

    int getResponseCode() { return urlResponseInfo.getHttpStatusCode(); }

    String getBodyAsString() { return new String(body); }
  }
}
