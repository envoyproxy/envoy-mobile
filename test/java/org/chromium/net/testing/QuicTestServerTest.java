package org.chromium.net.testing;

import static org.assertj.core.api.Assertions.assertThat;
import static org.chromium.net.testing.CronetTestRule.getContext;

import io.envoyproxy.envoymobile.AndroidEngineBuilder;
import io.envoyproxy.envoymobile.Custom;
import io.envoyproxy.envoymobile.Engine;
import io.envoyproxy.envoymobile.EnvoyError;
import io.envoyproxy.envoymobile.RequestHeaders;
import io.envoyproxy.envoymobile.RequestHeadersBuilder;
import io.envoyproxy.envoymobile.RequestMethod;
import io.envoyproxy.envoymobile.ResponseHeaders;
import io.envoyproxy.envoymobile.ResponseTrailers;
import io.envoyproxy.envoymobile.Stream;
import io.envoyproxy.envoymobile.engine.AndroidJniLibrary;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.AbstractMap.SimpleImmutableEntry;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class QuicTestServerTest {
  private static String hcmType = "type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager";

  private static String quicDownstreamType = "type.googleapis.com/envoy.extensions.transport_sockets.quic.v3.QuicDownstreamTransport";

  private static String quicUpstreamType = "type.googleapis.com/envoy.extensions.transport_sockets.quic.v3.QuicUpstreamTransport";
  private static String config =
      "static_resources:\n" +
          " listeners:\n" +
          " - name: h3_remote_listener\n" +
          "   address:\n" +
          "     socket_address: { protocol: UDP, address: 127.0.0.1, port_value: 10101 }\n" +
          "   reuse_port: true\n" +
          "   udp_listener_config:\n" +
          "     quic_options: {}\n" +
          "     downstream_socket_config:\n" +
          "       prefer_gro: true\n" +
          "   filter_chains:\n" +
          "     transport_socket:\n" +
          "       name: envoy.transport_sockets.quic\n" +
          "       typed_config:\n" +
          "         \"@type\": " + quicDownstreamType + "\n" +
          "         downstream_tls_context:\n" +
          "           common_tls_context:\n" +
          "             alpn_protocols: h3\n" +
          "             tls_certificates:\n" +
          "               certificate_chain:\n" +
          "                 inline_string: |\n" +
          "                   -----BEGIN CERTIFICATE-----\n" +
          "                   MIIEbDCCA1SgAwIBAgIUJuVBh0FKfFgIcO++ljWm7D47eYUwDQYJKoZIhvcNAQEL\n" +
          "                   BQAwdjELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExFjAUBgNVBAcM\n" +
          "                   DVNhbiBGcmFuY2lzY28xDTALBgNVBAoMBEx5ZnQxGTAXBgNVBAsMEEx5ZnQgRW5n\n" +
          "                   aW5lZXJpbmcxEDAOBgNVBAMMB1Rlc3QgQ0EwHhcNMjAwODA1MTkxNjAxWhcNMjIw\n" +
          "                   ODA1MTkxNjAxWjCBpjELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWEx\n" +
          "                   FjAUBgNVBAcMDVNhbiBGcmFuY2lzY28xDTALBgNVBAoMBEx5ZnQxGTAXBgNVBAsM\n" +
          "                   EEx5ZnQgRW5naW5lZXJpbmcxGjAYBgNVBAMMEVRlc3QgQmFja2VuZCBUZWFtMSQw\n" +
          "                   IgYJKoZIhvcNAQkBFhViYWNrZW5kLXRlYW1AbHlmdC5jb20wggEiMA0GCSqGSIb3\n" +
          "                   DQEBAQUAA4IBDwAwggEKAoIBAQC9JgaI7hxjPM0tsUna/QmivBdKbCrLnLW9Teak\n" +
          "                   RH/Ebg68ovyvrRIlybDT6XhKi+iVpzVY9kqxhGHgrFDgGLBakVMiYJ5EjIgHfoo4\n" +
          "                   UUAHwIYbunJluYCgANzpprBsvTC/yFYDVMqUrjvwHsoYYVm36io994k9+t813b70\n" +
          "                   o0l7/PraBsKkz8NcY2V2mrd/yHn/0HAhv3hl6iiJme9yURuDYQrae2ACSrQtsbel\n" +
          "                   KwdZ/Re71Z1awz0OQmAjMa2HuCop+Q/1QLnqBekT5+DH1qKUzJ3Jkq6NRkERXOpi\n" +
          "                   87j04rtCBteCogrO67qnuBZ2lH3jYEMb+lQdLkyNMLltBSdLAgMBAAGjgcAwgb0w\n" +
          "                   DAYDVR0TAQH/BAIwADALBgNVHQ8EBAMCBeAwHQYDVR0lBBYwFAYIKwYBBQUHAwIG\n" +
          "                   CCsGAQUFBwMBMEEGA1UdEQQ6MDiGHnNwaWZmZTovL2x5ZnQuY29tL2JhY2tlbmQt\n" +
          "                   dGVhbYIIbHlmdC5jb22CDHd3dy5seWZ0LmNvbTAdBgNVHQ4EFgQU2XcTZbc0xKZf\n" +
          "                   gNVKSvAbMZJCBoYwHwYDVR0jBBgwFoAUlkvaLFO0vpXGk3Pip6SfLg1yGIcwDQYJ\n" +
          "                   KoZIhvcNAQELBQADggEBAFW05aca3hSiEz/g593GAV3XP4lI5kYUjGjbPSy/HmLr\n" +
          "                   rdv/u3bGfacywAPo7yld+arMzd35tIYEqnhoq0+/OxPeyhwZXVVUatg5Oknut5Zv\n" +
          "                   2+8l+mVW+8oFCXRqr2gwc8Xt4ByYN+HaNUYfoucnjDplOPukkfSuRhbxqnkhA14v\n" +
          "                   Lri2EbISX14sXf2VQ9I0dkm1hXUxiO0LlA1Z7tvJac9zPSoa6Oljke4D1iH2jzwF\n" +
          "                   Yn7S/gGvVQgkTmWrs3S3TGyBDi4GTDhCF1R+ESvXz8z4UW1MrCSdYUXbRtsT7sbE\n" +
          "                   CjlFYuUyxCi1oe3IHCeXVDo/bmzwGQPDuF3WaDNSYWU=\n" +
          "                   -----END CERTIFICATE-----\n" +
          "               private_key:\n" +
          "                 inline_string: |\n" +
          "                   -----BEGIN RSA PRIVATE KEY-----\n" +
          "                   MIIEpAIBAAKCAQEAvSYGiO4cYzzNLbFJ2v0JorwXSmwqy5y1vU3mpER/xG4OvKL8\n" +
          "                   r60SJcmw0+l4Sovolac1WPZKsYRh4KxQ4BiwWpFTImCeRIyIB36KOFFAB8CGG7py\n" +
          "                   ZbmAoADc6aawbL0wv8hWA1TKlK478B7KGGFZt+oqPfeJPfrfNd2+9KNJe/z62gbC\n" +
          "                   pM/DXGNldpq3f8h5/9BwIb94ZeooiZnvclEbg2EK2ntgAkq0LbG3pSsHWf0Xu9Wd\n" +
          "                   WsM9DkJgIzGth7gqKfkP9UC56gXpE+fgx9ailMydyZKujUZBEVzqYvO49OK7QgbX\n" +
          "                   gqIKzuu6p7gWdpR942BDG/pUHS5MjTC5bQUnSwIDAQABAoIBADEMwlcSAFSPuNln\n" +
          "                   hzJ9udj0k8md4T8p5Usw/2WLyeJDdBjg30wjQniAJBXgDmyueWMNmFz4iYgdP1CG\n" +
          "                   /vYOEPV7iCZ7Da/TDZd77hYKo+MevuhD4lSU1VEoyCDjNA8OxKyHJB77BwmlYS+0\n" +
          "                   nE3UOPLji47EOVfUTbvnRBSmn3DCSHkQiRIUP1xMivoiZgKJn+D+FxSMwwiq2pQR\n" +
          "                   5tdo7nh2A8RxlYUbaD6i4poUB26HVm8vthXahNEkLpXQOz8MWRzs6xOdDHRzi9kT\n" +
          "                   ItRLa4A/3LIATqviQ2EpwcALHXcULcNUMTHORC1EHPvheWR5nLuRllYzN4ReoeHC\n" +
          "                   3+A5KEkCgYEA52rlh/22/rLckCWugjyJic17vkg46feSOGhjuP2LelrIxNlg491y\n" +
          "                   o28n8lQPSVnEp3/sT7Y3quVvdboq4DC9LTzq52f6/mCYh9UQRpljuSmFqC2MPG46\n" +
          "                   Zl5KLEVLzhjC8aTWkhVINSpz9vauXderOpFYlPW32lnRTjJWE276kj8CgYEA0T2t\n" +
          "                   ULnn7TBvRSpmeWzEBA5FFo2QYkYvwrcVe0pfUltV6pf05xUmMXYFjpezSTEmPhh6\n" +
          "                   +dZdhwxDk+6j8Oo61rTWucDsIqMj5ZT1hPNph8yQtb5LRlRbLGVrirU9Tp7xTgMq\n" +
          "                   3uRA2Eka1d98dDBsEbMIVFSZ2MX3iezSGRL6j/UCgYEAxZQ82HjEDn2DVwb1EXjC\n" +
          "                   LQdliTZ8cTXQf5yQ19aRiSuNkpPN536ga+1xe7JNQuEDx8auafg3Ww98tFT4WmUC\n" +
          "                   f2ctX9klMJ4kXISK2twHioVq+gW5X7b04YXLajTX3eTCPDHyiNLmzY2raMWAZdrG\n" +
          "                   9MA3kyafjCt3Sn4rg3gTM10CgYEAtJ8WRpJEd8aQttcUIItYZdvfnclUMtE9l0su\n" +
          "                   GwCnalN3xguol/X0w0uLHn0rgeoQhhfhyFtY3yQiDcg58tRvODphBXZZIMlNSnic\n" +
          "                   vEjW9ygKXyjGmA5nqdpezB0JsB2aVep8Dm5g35Ozu52xNCc8ksbGUO265Jp3xbMN\n" +
          "                   5iEw9CUCgYBmfoPnJwzA5S1zMIqESUdVH6p3UwHU/+XTY6JHAnEVsE+BuLe3ioi7\n" +
          "                   6dU4rFd845MCkunBlASLV8MmMbod9xU0vTVHPtmANaUCPxwUIxXQket09t19Dzg7\n" +
          "                   A23sE+5myXtcfz6YrPhbLkijV4Nd7fmecodwDckvpBaWTMrv52/Www==\n" +
          "                   -----END RSA PRIVATE KEY-----\n" +
          "     filters:\n" +
          "     - name: envoy.filters.network.http_connection_manager\n" +
          "       typed_config:\n" +
          "         \"@type\": "+ hcmType + "\n" +
          "         codec_type: HTTP3\n" +
          "         stat_prefix: remote_hcm\n" +
          "         route_config:\n" +
          "           name: remote_route\n" +
          "           virtual_hosts:\n" +
          "           - name: remote_service\n" +
          "             domains: [\"*\"]\n" +
          "             routes:\n" +
          "             - match: { prefix: \"/\" }\n" +
          "               direct_response: { status: 200 }\n" +
          "         http3_protocol_options:\n" +
          "         http_filters:\n" +
          "         - name: envoy.router\n" +
          "           typed_config:\n" +
          "             \"@type\": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router\n" +
          " - name: base_api_listener\n" +
          "   address:\n" +
          "     socket_address: { protocol: TCP, address: 0.0.0.0, port_value: 10000 }\n" +
          "   api_listener:\n" +
          "     api_listener:\n" +
          "       \"@type\": " + hcmType + "\n" +
          "       stat_prefix: api_hcm\n" +
          "       route_config:\n" +
          "         name: api_router\n" +
          "         virtual_hosts:\n" +
          "         - name: api\n" +
          "           domains: [\"*\"]\n" +
          "           routes:\n" +
          "           - match: { prefix: \"/\" }\n" +
          "             route: { host_rewrite_literal: example.com, cluster: h3_remote }\n" +
          "       http_filters:\n" +
          "       - name: envoy.router\n" +
          "         typed_config:\n" +
          "           \"@type\": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router\n" +
          " clusters:\n" +
          " - name: h3_remote\n" +
          "   connect_timeout: 10s\n" +
          "   type: STATIC\n" +
          "   dns_lookup_family: V4_ONLY\n" +
          "   lb_policy: ROUND_ROBIN\n" +
          "   load_assignment:\n" +
          "     cluster_name: h3_remote\n" +
          "     endpoints:\n" +
          "     - lb_endpoints:\n" +
          "       - endpoint:\n" +
          "           address:\n" +
          "             socket_address: { address: 127.0.0.1, port_value: 10101 }\n" +
          "   typed_extension_protocol_options:\n" +
          "     envoy.extensions.upstreams.http.v3.HttpProtocolOptions:\n" +
          "       \"@type\": type.googleapis.com/envoy.extensions.upstreams.http.v3.HttpProtocolOptions\n" +
          "       explicit_http_config:\n" +
          "         http3_protocol_options: {}\n" +
          "       common_http_protocol_options:\n" +
          "         idle_timeout: 1s\n" +
          "   transport_socket:\n" +
          "     name: envoy.transport_sockets.quic\n" +
          "     typed_config:\n" +
          "       \"@type\": " + quicUpstreamType + "\n" +
          "       upstream_tls_context:\n" +
          "         sni: example.com";

  private static Engine engine;

  @BeforeClass
  public static void loadJniLibrary() {
    AndroidJniLibrary.loadTestLibrary();
    System.loadLibrary("quic_test_server_jni");
  }

  @AfterClass
  public static void shutdownEngine() {
    if (engine != null) {
      engine.terminate();
    }
  }

  @Before
  public void setUpEngine() throws Exception {
    if (engine == null) {
      CountDownLatch latch = new CountDownLatch(1);
      engine = new AndroidEngineBuilder(getContext().getApplicationContext(), new Custom(config))
          .setOnEngineRunning(() -> {
            latch.countDown();
            return null;
          })
          .build();
      latch.await(); // Don't launch a request before initialization has completed.
    }
  }

  @After
  public void shutdownMockWebServer() throws IOException {
    QuicTestServer.shutdownQuicTestServer();
  }

  @Test
  public void get_simple() throws Exception {
    QuicTestServer.startQuicTestServer(getContext());
    QuicTestServerTest.RequestScenario requestScenario = new QuicTestServerTest.RequestScenario()
        .setHttpMethod(RequestMethod.GET)
        .setUrl(QuicTestServer.getServerURL());

    QuicTestServerTest.Response response = sendRequest(requestScenario);

    assertThat(response.getHeaders().getHttpStatus()).isEqualTo(200);
    assertThat(response.getBodyAsString()).isEqualTo("hello, world");
    assertThat(response.getEnvoyError()).isNull();
  }

  private QuicTestServerTest.Response sendRequest(
      QuicTestServerTest.RequestScenario requestScenario) throws Exception {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<QuicTestServerTest.Response> response = new AtomicReference<>(new QuicTestServerTest.Response());

    Stream stream = engine.streamClient()
        .newStreamPrototype()
        .setOnResponseHeaders((responseHeaders, endStream, ignored) -> {
          response.get().setHeaders(responseHeaders);
          if (endStream) {
            latch.countDown();
          }
          return null;
        })
        .setOnResponseData((data, endStream, ignored) -> {
          response.get().addBody(data);
          if (endStream) {
            latch.countDown();
          }
          return null;
        })
        .setOnResponseTrailers((trailers, ignored) -> {
          response.get().setTrailers(trailers);
          latch.countDown();
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
        .start(Executors.newSingleThreadExecutor())
        .sendHeaders(requestScenario.getHeaders(), !requestScenario.hasBody());
    requestScenario.getBodyChunks().forEach(stream::sendData);
    requestScenario.getClosingBodyChunk().ifPresent(stream::close);

    latch.await();
    response.get().throwAssertionErrorIfAny();
    return response.get();
  }

  private static class RequestScenario {
    private URL url;
    private RequestMethod method = null;
    private final List<ByteBuffer> bodyChunks = new ArrayList<>();
    private final List<Map.Entry<String, String>> headers = new ArrayList<>();
    private boolean closeBodyStream = false;

    RequestHeaders getHeaders() {
      RequestHeadersBuilder requestHeadersBuilder =
          new RequestHeadersBuilder(method, url.getProtocol(), url.getAuthority(), url.getPath());
      headers.forEach(entry -> requestHeadersBuilder.add(entry.getKey(), entry.getValue()));
      // HTTP1 is the only way to send HTTP requests (not HTTPS)
      return requestHeadersBuilder.build();
    }

    List<ByteBuffer> getBodyChunks() {
      return closeBodyStream
          ? Collections.unmodifiableList(bodyChunks.subList(0, bodyChunks.size() - 1))
          : Collections.unmodifiableList(bodyChunks);
    }

    Optional<ByteBuffer> getClosingBodyChunk() {
      return closeBodyStream ? Optional.of(bodyChunks.get(bodyChunks.size() - 1))
          : Optional.empty();
    }

    boolean hasBody() { return !bodyChunks.isEmpty(); }

    QuicTestServerTest.RequestScenario setHttpMethod(RequestMethod requestMethod) {
      this.method = requestMethod;
      return this;
    }

    QuicTestServerTest.RequestScenario setUrl(String url) throws MalformedURLException {
      this.url = new URL(url);
      return this;
    }

    QuicTestServerTest.RequestScenario addBody(byte[] requestBodyChunk) {
      ByteBuffer byteBuffer = ByteBuffer.allocateDirect(requestBodyChunk.length);
      byteBuffer.put(requestBodyChunk);
      bodyChunks.add(byteBuffer);
      return this;
    }

    QuicTestServerTest.RequestScenario addBody(String requestBodyChunk) {
      return addBody(requestBodyChunk.getBytes());
    }

    QuicTestServerTest.RequestScenario addHeader(String key, String value) {
      headers.add(new SimpleImmutableEntry<>(key, value));
      return this;
    }

    QuicTestServerTest.RequestScenario closeBodyStream() {
      closeBodyStream = true;
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

    ResponseTrailers getTrailers() { return trailers.get(); }

    boolean isCancelled() { return cancelled.get(); }

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

    int getNbResponseChunks() { return bodies.size(); }

    void throwAssertionErrorIfAny() {
      if (assertionError.get() != null) {
        throw assertionError.get();
      }
    }
  }
}


