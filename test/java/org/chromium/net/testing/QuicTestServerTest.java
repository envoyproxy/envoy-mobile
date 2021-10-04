package org.chromium.net.testing;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.fail;
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
import io.envoyproxy.envoymobile.UpstreamHttpProtocol;
import io.envoyproxy.envoymobile.engine.AndroidJniLibrary;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.ByteBuffer;
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
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class QuicTestServerTest {
  private static final String hcmType =
      "type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager";

  private static final String quicDownstreamType =
      "type.googleapis.com/envoy.extensions.transport_sockets.quic.v3.QuicDownstreamTransport";

  private static final String quicUpstreamType =
      "type.googleapis.com/envoy.extensions.transport_sockets.quic.v3.QuicUpstreamTransport";
  private static final String config =
      "static_resources:\n"
      + " listeners:\n"
      + " - name: h3_remote_listener\n"
      + "   address:\n"
      + "     socket_address: { protocol: UDP, address: 127.0.0.1, port_value: 10101 }\n"
      + "   reuse_port: true\n"
      + "   udp_listener_config:\n"
      + "     quic_options: {}\n"
      + "     downstream_socket_config:\n"
      + "       prefer_gro: true\n"
      + "   filter_chains:\n"
      + "     transport_socket:\n"
      + "       name: envoy.transport_sockets.quic\n"
      + "       typed_config:\n"
      + "         \"@type\": " + quicDownstreamType + "\n"
      + "         downstream_tls_context:\n"
      + "           common_tls_context:\n"
      + "             alpn_protocols: h3\n"
      + "             tls_certificates:\n"
      + "               certificate_chain:\n"
      + "                 inline_string: |\n"
      + "                   -----BEGIN CERTIFICATE-----\n"
      + "                   MIIEPjCCAyagAwIBAgIUEuy1WgSCzX6mojPirk7Th6uhNHowDQYJKoZIhvcNAQEL\n"
      + "                   BQAwfzELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExFjAUBgNVBAcM\n"
      + "                   DVNhbiBGcmFuY2lzY28xDTALBgNVBAoMBEx5ZnQxGTAXBgNVBAsMEEx5ZnQgRW5n\n"
      + "                   aW5lZXJpbmcxGTAXBgNVBAMMEFRlc3QgVXBzdHJlYW0gQ0EwHhcNMjAwODA1MTkx\n"
      + "                   NjAyWhcNMjIwODA1MTkxNjAyWjCBgzELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNh\n"
      + "                   bGlmb3JuaWExFjAUBgNVBAcMDVNhbiBGcmFuY2lzY28xDTALBgNVBAoMBEx5ZnQx\n"
      + "                   GTAXBgNVBAsMEEx5ZnQgRW5naW5lZXJpbmcxHTAbBgNVBAMMFFRlc3QgVXBzdHJl\n"
      + "                   YW0gU2VydmVyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtpiYA4/I\n"
      + "                   NuflkPe4L/GTslmngNQUCo8TzPXG0gt7uoxr4FeuVy7AaD28S2/hwhbl+bDtHTQY\n"
      + "                   mvBUwNMsYzpND2eQ3sSIumdeLzBEKP2mnnZ9gE/Zd2TIuZl686RpDq0B6ZdZSpCu\n"
      + "                   bqQmmPFLiRNH8JViJZMN5yqMt7T5oq+DnCYQZllqmpAwd6NnhKALrYmZ87oqc0zh\n"
      + "                   kf+5amP7zMYKkwQuRwcx4QPZkEp3+qhszolpAJ52dFGJ+pLuUVDg0Gf0cnxLjFKc\n"
      + "                   6vcTlj4tsymR4ci58MHRt4EdGdhShw0oaj67gRRfU4Vj61I2ZAVH07kL0mjO2TZT\n"
      + "                   EKrOEJJ7/dtxdwIDAQABo4GsMIGpMAwGA1UdEwEB/wQCMAAwCwYDVR0PBAQDAgXg\n"
      + "                   MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATAtBgNVHREEJjAkggoqLmx5\n"
      + "                   ZnQuY29thwR/AAABhxAAAAAAAAAAAAAAAAAAAAABMB0GA1UdDgQWBBQeoC5wxwX5\n"
      + "                   k3ggIN/844/6jKx9czAfBgNVHSMEGDAWgBQ7Zh1TopMm7SY1RCOEO8L1G8IAZDAN\n"
      + "                   BgkqhkiG9w0BAQsFAAOCAQEA18wEg8LnPm99cIouFUFMAO+BpiY2KVa9Bu6x07m9\n"
      + "                   quNFv7/4mLt87sk/umD3LH/tDjqW0D84vhG9a+0yDq7ZrD/P5eK3R+yBINwhe4/x\n"
      + "                   obJlThEsbcZF1FkMnq1rt53izukyQLLQwoVxidQl3HCg3hosWmpH1VBPgwoize6V\n"
      + "                   aAhKLW0n+JSfIE1d80nvZdYlHuCnS6UhLmAbTBCnwT0aGTfzT0Dd4KlYiY8vGZRu\n"
      + "                   tXOw4MzKtJcOL3t7Zpz2mhqN25dyiuyvKEhLXdx48aemwa2t6ISfFKsd0/glnNe/\n"
      + "                   PFZMakzKv1G0xLGURjsInCZ0kePAmerfZN6CBZDo4laYEg==\n"
      + "                   -----END CERTIFICATE-----\n"
      + "               private_key:\n"
      + "                 inline_string: |\n"
      + "                   -----BEGIN RSA PRIVATE KEY-----\n"
      + "                   MIIEpAIBAAKCAQEAtpiYA4/INuflkPe4L/GTslmngNQUCo8TzPXG0gt7uoxr4Feu\n"
      + "                   Vy7AaD28S2/hwhbl+bDtHTQYmvBUwNMsYzpND2eQ3sSIumdeLzBEKP2mnnZ9gE/Z\n"
      + "                   d2TIuZl686RpDq0B6ZdZSpCubqQmmPFLiRNH8JViJZMN5yqMt7T5oq+DnCYQZllq\n"
      + "                   mpAwd6NnhKALrYmZ87oqc0zhkf+5amP7zMYKkwQuRwcx4QPZkEp3+qhszolpAJ52\n"
      + "                   dFGJ+pLuUVDg0Gf0cnxLjFKc6vcTlj4tsymR4ci58MHRt4EdGdhShw0oaj67gRRf\n"
      + "                   U4Vj61I2ZAVH07kL0mjO2TZTEKrOEJJ7/dtxdwIDAQABAoIBACz6E1+1N/0GTA7U\n"
      + "                   ZgMxP09MNC1QkAs1yQvQcoPknjqKQjxFfMUu1+gVZN80FOjpGQbTJOTvoyvvDQFe\n"
      + "                   Qu3CO58SxKWKxZ8cvR9khTWPnU4lI67KfGejZKoK+zUuh049IV53kGAEmWLZfkRo\n"
      + "                   E1IVdL/3G/DjcyZA3d6WbnM7RnDcqORPnig4lq5HxN76eBdssbxtrAi3Sjy3ChMy\n"
      + "                   BLInnryF4UtaT5xqR26YjgtFmYkunrgXTe1i/ewQgBBkSPXcNr7or69hCCv0SG9e\n"
      + "                   vRsv1r+Uug3/iRZDjEhKBmXWNAZJ/IsDF37ywiyeBdUY+klDX+mWz+0BB0us8b4u\n"
      + "                   LxoZQTECgYEA2Gu9EVC0RMrQ9FF5AgKKJWmZKkOn346RkPrtbl5lbuUgnVdBXJjr\n"
      + "                   wfMZVTD/8E/tMN4EMSGmC9bxCpRRzhrphrm7SHGD6b9O30DH9q0TV0r0A8IG/bMO\n"
      + "                   xJLYjrYVxtEE+KckzvyvfIefbDG7wYkI3u+ObmjBg9t6jcErKlI/PdkCgYEA1/1E\n"
      + "                   T+cpR16iOPz1mz+f/GU4YmPkdSDj/PrjMv0c1OTDvdPiZPpLkhLUKiICnKSKbYjX\n"
      + "                   Ko8fdZc3cmakgD1dXtAfR7Tf/hXQIR5+iHD48I5e9DVlkqMNDObfj82ohTFKVe/P\n"
      + "                   ZSwgDiAPTMFxWr26u/GzY5D3adCQYJyKE2wTh88CgYEAu7vpzGVnmu0ciXNLNvUh\n"
      + "                   BQcvODxsGT9BArTI1Z7I+oOD4TjZmAuHJz1L0lypB7stk+BjXoND2K1hdr3moJUz\n"
      + "                   0gy3a0YdGd07++nkDBVi26xHNCNRkS2MN/TyKgnFpiuW1mOXSH5lc+7p2h7iMiY/\n"
      + "                   LbQ8p4Xzp//xtZnFafbiqTECgYEAwDN5KZ1r5z24H/xCVv+cT46HSU7ZCr3VA9cC\n"
      + "                   fOouUOitouu9J9xviTJGKKQRLPFi2awOxKmN9ic1SRE7y35P60JKw5WaSdGBXydy\n"
      + "                   s9nMPMyEhM5Lb9y2jUeZo68ACl5dZvG63a4RbGBtHQF67KOvWvXvi2eCM2BMShyi\n"
      + "                   5jujeZMCgYAjewq1hVqL1FOD8sIFpmndsH3+Dfc7BJ/erqGOX9bQYGvJO4nCe+7K\n"
      + "                   4o8qFQf4jwdxu0iNxYJIMdn+l4/pz2e7GUFHjgMduUclf27Qj1p+8EyYqp6cmkzM\n"
      + "                   8mcwRkYo3aM70EmUu0Xxi3d5O5F1bIJ5MkgXaX/zSF2N02B3jXroxQ==\n"
      + "                   -----END RSA PRIVATE KEY-----\n"
      + "     filters:\n"
      + "     - name: envoy.filters.network.http_connection_manager\n"
      + "       typed_config:\n"
      + "         \"@type\": " + hcmType + "\n"
      + "         codec_type: HTTP3\n"
      + "         stat_prefix: remote_hcm\n"
      + "         route_config:\n"
      + "           name: remote_route\n"
      + "           virtual_hosts:\n"
      + "           - name: remote_service\n"
      + "             domains: [\"*\"]\n"
      + "             routes:\n"
      + "             - match: { prefix: \"/\" }\n"
      + "               route: { host_rewrite_literal: 127.0.0.1, cluster: h3_remote }\n"
      + "         http3_protocol_options:\n"
      + "         http_filters:\n"
      + "         - name: envoy.router\n"
      + "           typed_config:\n"
      + "             \"@type\": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router\n"
      + " - name: base_api_listener\n"
      + "   address:\n"
      + "     socket_address: { protocol: TCP, address: 0.0.0.0, port_value: 10000 }\n"
      + "   api_listener:\n"
      + "     api_listener:\n"
      + "       \"@type\": " + hcmType + "\n"
      + "       stat_prefix: api_hcm\n"
      + "       route_config:\n"
      + "         name: api_router\n"
      + "         virtual_hosts:\n"
      + "         - name: api\n"
      + "           domains: [\"*\"]\n"
      + "           routes:\n"
      + "           - match: { prefix: \"/\" }\n"
      + "             route: { host_rewrite_literal: www.lyft.com, cluster: h3_remote }\n"
      + "       http_filters:\n"
      + "       - name: envoy.router\n"
      + "         typed_config:\n"
      + "           \"@type\": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router\n"
      + " clusters:\n"
      + " - name: h3_remote\n"
      + "   connect_timeout: 10s\n"
      + "   type: STATIC\n"
      + "   dns_lookup_family: V4_ONLY\n"
      + "   lb_policy: ROUND_ROBIN\n"
      + "   load_assignment:\n"
      + "     cluster_name: h3_remote\n"
      + "     endpoints:\n"
      + "     - lb_endpoints:\n"
      + "       - endpoint:\n"
      + "           address:\n"
      + "             socket_address: { address: 127.0.0.1, port_value: 10101 }\n"
      + "   typed_extension_protocol_options:\n"
      + "     envoy.extensions.upstreams.http.v3.HttpProtocolOptions:\n"
      + "       \"@type\": type.googleapis.com/envoy.extensions.upstreams.http.v3.HttpProtocolOptions\n"
      + "       explicit_http_config:\n"
      + "         http3_protocol_options: {}\n"
      + "       common_http_protocol_options:\n"
      + "         idle_timeout: 10s\n"
      + "   transport_socket:\n"
      + "     name: envoy.transport_sockets.quic\n"
      + "     typed_config:\n"
      + "       \"@type\": " + quicUpstreamType + "\n"
      + "       upstream_tls_context:\n"
      + "         sni: www.lyft.com";

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
    System.err.println(response.getBodyAsString());
    System.err.println(response.getHeaders());
    assertThat(response.getHeaders().getHttpStatus()).isEqualTo(200);
    assertThat(response.getBodyAsString()).isEqualTo("hello, world");
    assertThat(response.getEnvoyError()).isNull();
  }

  private QuicTestServerTest.Response
  sendRequest(QuicTestServerTest.RequestScenario requestScenario) throws Exception {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<QuicTestServerTest.Response> response =
        new AtomicReference<>(new QuicTestServerTest.Response());

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
    private final boolean closeBodyStream = false;

    RequestHeaders getHeaders() {
      RequestHeadersBuilder requestHeadersBuilder =
          new RequestHeadersBuilder(method, url.getProtocol(), url.getAuthority(), url.getPath());
      headers.forEach(entry -> requestHeadersBuilder.add(entry.getKey(), entry.getValue()));
      // requestHeadersBuilder.add("x-envoy-mobile-upstream-protocol", "http3");
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
