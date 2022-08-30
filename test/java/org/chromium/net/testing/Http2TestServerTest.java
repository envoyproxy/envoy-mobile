package org.chromium.net.testing;

import static io.envoyproxy.envoymobile.engine.EnvoyConfiguration.TrustChainVerification;
import static org.assertj.core.api.Assertions.assertThat;
import static org.chromium.net.testing.CronetTestRule.SERVER_CERT_PEM;
import static org.chromium.net.testing.CronetTestRule.SERVER_KEY_PKCS8_PEM;

import android.content.Context;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import org.chromium.net.AndroidNetworkLibrary;
import io.envoyproxy.envoymobile.AndroidEngineBuilder;
import io.envoyproxy.envoymobile.Engine;
import io.envoyproxy.envoymobile.EnvoyError;
import io.envoyproxy.envoymobile.RequestHeaders;
import io.envoyproxy.envoymobile.RequestHeadersBuilder;
import io.envoyproxy.envoymobile.RequestMethod;
import io.envoyproxy.envoymobile.ResponseHeaders;
import io.envoyproxy.envoymobile.ResponseTrailers;
import io.envoyproxy.envoymobile.UpstreamHttpProtocol;
import io.envoyproxy.envoymobile.engine.AndroidJniLibrary;
import io.envoyproxy.envoymobile.engine.JniLibrary;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import java.nio.charset.StandardCharsets;

@RunWith(RobolectricTestRunner.class)
public class Http2TestServerTest {

  private Engine engine;

  @BeforeClass
  public static void loadJniLibrary() {
    AndroidJniLibrary.loadTestLibrary();
    JniLibrary.load();
  }

  @Before
  public void setUp() throws Exception {
    AndroidNetworkLibrary.setFakeCertificateVerificationForTesting(true);
  }

  public void setUpEngine(boolean usePlatformCertValidator,
                          TrustChainVerification trustChainVerification) throws Exception {
    CountDownLatch latch = new CountDownLatch(1);
    Context appContext = ApplicationProvider.getApplicationContext();
    engine =
        new AndroidEngineBuilder(appContext)
            .usePlatformCertValidator(usePlatformCertValidator)
            .setTrustChainVerification(trustChainVerification)
            .setOnEngineRunning(() -> {
              latch.countDown();
              System.out.println("========= OnEngineRunning, " +
                                 AndroidNetworkLibrary.getFakeCertificateVerificationForTesting());
              return null;
            })
            .build();
    Http2TestServer.startHttp2TestServer(appContext, SERVER_CERT_PEM, SERVER_KEY_PKCS8_PEM);
    latch.await(); // Don't launch a request before initialization has completed.
  }

  @After
  public void shutdown() throws Exception {
    engine.terminate();
    Http2TestServer.shutdownHttp2TestServer();
    JniLibrary.callClearTestRootCertificateFromNative();
    AndroidNetworkLibrary.setFakeCertificateVerificationForTesting(false);
  }

  private void getSchemeIsHttps(boolean usePlatformCertValidator,
                                TrustChainVerification trustChainVerification) throws Exception {
    setUpEngine(usePlatformCertValidator, trustChainVerification);

    RequestScenario requestScenario = new RequestScenario()
                                          .setHttpMethod(RequestMethod.GET)
                                          .setUrl(Http2TestServer.getEchoAllHeadersUrl());

    Response response = sendRequest(requestScenario);

    assertThat(response.getHeaders().getHttpStatus()).isEqualTo(200);
    assertThat(response.getBodyAsString()).contains(":scheme: https");
    assertThat(response.getHeaders().value("x-envoy-upstream-alpn")).containsExactly("h2");
    assertThat(response.getEnvoyError()).isNull();
  }

  /*
    @Test
    public void testGetRequest() throws Exception {
      System.out.println("TEST_testGetRequest", ACCEPT_UNTRUSTED);
      getSchemeIsHttps(false);
       }
VERIFY_TRUST_CHAIN

  @Test
  public void testGetRequestWithPlatformCertValidatorSuccess() throws Exception {
    System.out.println("TEST_testGetRequestWithPlatformCertValidator");
    Path caPath = Paths.get(SERVER_CERT_PEM);
    byte[] caBytes = Files.readAllBytes(caPath);
    AndroidNetworkLibrary.addTestRootCertificate(caBytes);
    getSchemeIsHttps(true, TrustChainVerification.VERIFY_TRUST_CHAIN);
  }
  */

  @Test
  public void testGetRequestWithPlatformCertValidatorFail() throws Exception {
    System.out.println("TEST_testGetRequestWithPlatformCertValidator");
    final String fakeCa = new String("fake CA cert");
    AndroidNetworkLibrary.addTestRootCertificate(fakeCa.getBytes());
    getSchemeIsHttps(true, TrustChainVerification.VERIFY_TRUST_CHAIN);
  }

  private Response sendRequest(RequestScenario requestScenario) throws Exception {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<Response> response = new AtomicReference<>(new Response());

    engine.streamClient()
        .newStreamPrototype()
        .setOnResponseHeaders((responseHeaders, endStream, ignored) -> {
          response.get().setHeaders(responseHeaders);
          return null;
        })
        .setOnResponseData((data, endStream, ignored) -> {
          response.get().addBody(data);
          return null;
        })
        .setOnResponseTrailers((trailers, ignored) -> {
          response.get().setTrailers(trailers);
          return null;
        })
        .setOnError((error, ignored) -> {
          response.get().setEnvoyError(error);
          latch.countDown();
          return null;
        })
        .setOnCancel((ignored) -> {
          response.get().setCancelled();
          latch.countDown();
          return null;
        })
        .setOnComplete((ignore) -> {
          latch.countDown();
          return null;
        })
        .start(Executors.newSingleThreadExecutor())
        .sendHeaders(requestScenario.getHeaders(), /* hasRequestBody= */ false);

    latch.await();
    response.get().throwAssertionErrorIfAny();
    return response.get();
  }

  private static class RequestScenario {

    private URL url;
    private RequestMethod method = null;

    RequestHeaders getHeaders() {
      RequestHeadersBuilder requestHeadersBuilder =
          new RequestHeadersBuilder(method, url.getProtocol(), url.getAuthority(), url.getPath());
      return requestHeadersBuilder.addUpstreamHttpProtocol(UpstreamHttpProtocol.HTTP2).build();
    }

    RequestScenario setHttpMethod(RequestMethod requestMethod) {
      this.method = requestMethod;
      return this;
    }

    RequestScenario setUrl(String url) throws MalformedURLException {
      this.url = new URL(url);
      return this;
    }
  }

  private static class Response {
    private final AtomicReference<ResponseHeaders> headers = new AtomicReference<>();
    private final AtomicReference<ResponseTrailers> trailers = new AtomicReference<>();
    private final AtomicReference<EnvoyError> envoyError = new AtomicReference<>();
    private final List<ByteBuffer> bodies = new ArrayList<>();
    private final AtomicBoolean cancelled = new AtomicBoolean(false);
    private final AtomicReference<AssertionError> assertionError = new AtomicReference<>();

    void setHeaders(ResponseHeaders headers) {
      if (!this.headers.compareAndSet(null, headers)) {
        assertionError.compareAndSet(
            null, new AssertionError("setOnResponseHeaders called more than once."));
      }
    }

    void addBody(ByteBuffer body) { bodies.add(body); }

    void setTrailers(ResponseTrailers trailers) {
      if (!this.trailers.compareAndSet(null, trailers)) {
        assertionError.compareAndSet(
            null, new AssertionError("setOnResponseTrailers called more than once."));
      }
    }

    void setEnvoyError(EnvoyError envoyError) {
      if (!this.envoyError.compareAndSet(null, envoyError)) {
        assertionError.compareAndSet(null, new AssertionError("setOnError called more than once."));
      }
    }

    void setCancelled() {
      if (!cancelled.compareAndSet(false, true)) {
        assertionError.compareAndSet(null,
                                     new AssertionError("setOnCancel called more than once."));
      }
    }

    EnvoyError getEnvoyError() { return envoyError.get(); }

    ResponseHeaders getHeaders() { return headers.get(); }

    String getBodyAsString() {
      int totalSize = bodies.stream().mapToInt(ByteBuffer::limit).sum();
      byte[] body = new byte[totalSize];
      int pos = 0;
      for (ByteBuffer buffer : bodies) {
        int bytesToRead = buffer.limit();
        buffer.get(body, pos, bytesToRead);
        pos += bytesToRead;
      }
      return new String(body);
    }

    void throwAssertionErrorIfAny() {
      if (assertionError.get() != null) {
        throw assertionError.get();
      }
    }
  }
}
