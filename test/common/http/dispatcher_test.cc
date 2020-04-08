#include <atomic>

#include "common/buffer/buffer_impl.h"
#include "common/http/context_impl.h"

#include "test/common/http/common.h"
#include "test/mocks/buffer/mocks.h"
#include "test/mocks/event/mocks.h"
#include "test/mocks/http/api_listener.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/local_info/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "library/common/buffer/utility.h"
#include "library/common/http/dispatcher.h"
#include "library/common/http/header_utility.h"
#include "library/common/types/c_types.h"

using testing::_;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;
using testing::SaveArg;
using testing::WithArg;

namespace Envoy {
namespace Http {

// Based on Http::Utility::toRequestHeaders() but only used for these tests.
ResponseHeaderMapPtr toResponseHeaders(envoy_headers headers) {
  ResponseHeaderMapPtr transformed_headers = std::make_unique<ResponseHeaderMapImpl>();
  for (envoy_header_size_t i = 0; i < headers.length; i++) {
    transformed_headers->addCopy(LowerCaseString(Utility::convertToString(headers.headers[i].key)),
                                 Utility::convertToString(headers.headers[i].value));
  }
  // The C envoy_headers struct can be released now because the headers have been copied.
  release_envoy_headers(headers);
  return transformed_headers;
}

class DispatcherTest : public testing::Test {
public:
  void ready() { http_dispatcher_.ready(event_dispatcher_, api_listener_); }

  typedef struct {
    uint32_t on_headers_calls;
    uint32_t on_data_calls;
    uint32_t on_trailers_calls;
    uint32_t on_complete_calls;
    uint32_t on_error_calls;
    uint32_t on_cancel_calls;
  } callbacks_called;

  MockApiListener api_listener_;
  MockRequestDecoder request_decoder_;
  ResponseEncoder* response_encoder_{};
  NiceMock<Event::MockDispatcher> event_dispatcher_;
  envoy_http_callbacks bridge_callbacks_;
  std::atomic<envoy_network_t> preferred_network_{ENVOY_NET_GENERIC};
  Dispatcher http_dispatcher_{preferred_network_};
};

TEST_F(DispatcherTest, SetDestinationCluster) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers. Sending multiple headers is illegal and the upstream codec would not
  // accept it. However, given we are just trying to test preferred network headers and using mocks
  // this is fine.

  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  preferred_network_.store(ENVOY_NET_GENERIC);
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, false);

  TestRequestHeaderMapImpl expected_headers{
      {":scheme", "http"},
      {":method", "GET"},
      {":authority", "host"},
      {":path", "/"},
      {"x-envoy-mobile-cluster", "base"},
      {"x-forwarded-proto", "https"},
  };
  EXPECT_CALL(request_decoder_, decodeHeaders_(HeaderMapEqual(&expected_headers), false));
  send_headers_post_cb();

  TestRequestHeaderMapImpl headers2;
  HttpTestUtility::addDefaultHeaders(headers2);
  envoy_headers c_headers2 = Utility::toBridgeHeaders(headers2);

  preferred_network_.store(ENVOY_NET_WLAN);
  Event::PostCb send_headers_post_cb2;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb2));
  http_dispatcher_.sendHeaders(stream, c_headers2, false);

  TestRequestHeaderMapImpl expected_headers2{
      {":scheme", "http"},
      {":method", "GET"},
      {":authority", "host"},
      {":path", "/"},
      {"x-envoy-mobile-cluster", "base_wlan"},
      {"x-forwarded-proto", "https"},
  };
  EXPECT_CALL(request_decoder_, decodeHeaders_(HeaderMapEqual(&expected_headers2), false));
  send_headers_post_cb2();

  TestRequestHeaderMapImpl headers3;
  HttpTestUtility::addDefaultHeaders(headers3);
  envoy_headers c_headers3 = Utility::toBridgeHeaders(headers3);

  preferred_network_.store(ENVOY_NET_WWAN);
  Event::PostCb send_headers_post_cb3;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb3));
  http_dispatcher_.sendHeaders(stream, c_headers3, true);

  TestRequestHeaderMapImpl expected_headers3{
      {":scheme", "http"},
      {":method", "GET"},
      {":authority", "host"},
      {":path", "/"},
      {"x-envoy-mobile-cluster", "base_wwan"},
      {"x-forwarded-proto", "https"},
  };
  EXPECT_CALL(request_decoder_, decodeHeaders_(HeaderMapEqual(&expected_headers3), true));
  send_headers_post_cb3();

  // Encode response headers.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, SetDestinationClusterUpstreamProtocol) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers. Sending multiple headers is illegal and the upstream codec would not
  // accept it. However, given we are just trying to test preferred network headers and using mocks
  // this is fine.

  TestRequestHeaderMapImpl headers{{"x-envoy-mobile-upstream-protocol", "http2"}};
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  preferred_network_.store(ENVOY_NET_GENERIC);
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, false);

  TestResponseHeaderMapImpl expected_headers{
      {":scheme", "http"},
      {":method", "GET"},
      {":authority", "host"},
      {":path", "/"},
      {"x-envoy-mobile-cluster", "base_h2"},
      {"x-forwarded-proto", "https"},
  };
  EXPECT_CALL(request_decoder_, decodeHeaders_(HeaderMapEqual(&expected_headers), false));
  send_headers_post_cb();

  TestRequestHeaderMapImpl headers2{{"x-envoy-mobile-upstream-protocol", "http2"}};
  HttpTestUtility::addDefaultHeaders(headers2);
  envoy_headers c_headers2 = Utility::toBridgeHeaders(headers2);

  preferred_network_.store(ENVOY_NET_WLAN);
  Event::PostCb send_headers_post_cb2;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb2));
  http_dispatcher_.sendHeaders(stream, c_headers2, false);

  TestResponseHeaderMapImpl expected_headers2{
      {":scheme", "http"},
      {":method", "GET"},
      {":authority", "host"},
      {":path", "/"},
      {"x-envoy-mobile-cluster", "base_wlan_h2"},
      {"x-forwarded-proto", "https"},
  };
  EXPECT_CALL(request_decoder_, decodeHeaders_(HeaderMapEqual(&expected_headers2), false));
  send_headers_post_cb2();

  TestRequestHeaderMapImpl headers3{{"x-envoy-mobile-upstream-protocol", "http2"}};
  HttpTestUtility::addDefaultHeaders(headers3);
  envoy_headers c_headers3 = Utility::toBridgeHeaders(headers3);

  preferred_network_.store(ENVOY_NET_WWAN);
  Event::PostCb send_headers_post_cb3;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb3));
  http_dispatcher_.sendHeaders(stream, c_headers3, true);

  TestResponseHeaderMapImpl expected_headers3{
      {":scheme", "http"},
      {":method", "GET"},
      {":authority", "host"},
      {":path", "/"},
      {"x-envoy-mobile-cluster", "base_wwan_h2"},
      {"x-forwarded-proto", "https"},
  };
  EXPECT_CALL(request_decoder_, decodeHeaders_(HeaderMapEqual(&expected_headers3), true));
  send_headers_post_cb3();

  // Setting http1.
  TestRequestHeaderMapImpl headers4{{"x-envoy-mobile-upstream-protocol", "http1"}};
  HttpTestUtility::addDefaultHeaders(headers4);
  envoy_headers c_headers4 = Utility::toBridgeHeaders(headers4);

  preferred_network_.store(ENVOY_NET_WWAN);
  Event::PostCb send_headers_post_cb4;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb4));
  http_dispatcher_.sendHeaders(stream, c_headers4, true);

  TestResponseHeaderMapImpl expected_headers4{
      {":scheme", "http"},
      {":method", "GET"},
      {":authority", "host"},
      {":path", "/"},
      {"x-envoy-mobile-cluster", "base_wwan"},
      {"x-forwarded-proto", "https"},
  };
  EXPECT_CALL(request_decoder_, decodeHeaders_(HeaderMapEqual(&expected_headers4), true));
  send_headers_post_cb4();

  // Encode response headers.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, Queueing) {
  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // These two stream operations will get queued up in the Http::Dispatcher's queue awaiting for the
  // call to ready. Create a stream.
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);
  // Send request headers.
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  // After ready is called the queue will be flushed and two events will be posted to the
  // event_dispatcher.
  Event::PostCb start_stream_post_cb;
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_))
      .WillOnce(SaveArg<0>(&start_stream_post_cb))
      .WillOnce(SaveArg<0>(&send_headers_post_cb));
  ready();
  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Encode response headers.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, BasicStreamHeaders) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Encode response headers.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, BasicStreamData) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_data = [](envoy_data c_data, bool end_stream, void* context) -> void {
    ASSERT_TRUE(end_stream);
    ASSERT_EQ(Http::Utility::convertToString(c_data), "response body");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_data_calls++;
    c_data.release(c_data.context);
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Build body data
  Buffer::OwnedImpl request_data = Buffer::OwnedImpl("request body");
  envoy_data c_data = Buffer::Utility::toBridgeData(request_data);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the
  // dispatcher API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request data. Although HTTP would need headers before data this unit test only wants to
  // test data functionality.
  Event::PostCb data_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&data_post_cb));
  http_dispatcher_.sendData(stream, c_data, true);

  EXPECT_CALL(request_decoder_, decodeData(BufferStringEqual("request body"), true));
  data_post_cb();

  // Encode response data.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  Buffer::InstancePtr response_data{new Buffer::OwnedImpl("response body")};
  response_encoder_->encodeData(*response_data, true);
  ASSERT_EQ(cc.on_data_calls, 1);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, BasicStreamTrailers) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_trailers = [](envoy_headers c_trailers, void* context) -> void {
    ResponseHeaderMapPtr response_trailers = toResponseHeaders(c_trailers);
    EXPECT_EQ(response_trailers->get(LowerCaseString("x-test-trailer"))->value().getStringView(),
              "test_trailer");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_trailers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Build a set of request trailers.
  TestRequestTrailerMapImpl trailers;
  envoy_headers c_trailers = Utility::toBridgeHeaders(trailers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the
  // dispatcher API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request trailers. Although HTTP would need headers before trailers this unit test only
  // wants to test trailers functionality.
  Event::PostCb trailers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&trailers_post_cb));
  http_dispatcher_.sendTrailers(stream, c_trailers);

  EXPECT_CALL(request_decoder_, decodeTrailers_(_));
  trailers_post_cb();

  // Encode response trailers.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseTrailerMapImpl response_trailers{{"x-test-trailer", "test_trailer"}};
  response_encoder_->encodeTrailers(response_trailers);
  ASSERT_EQ(cc.on_trailers_calls, 1);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, MultipleDataStream) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_FALSE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_data = [](envoy_data data, bool, void* context) -> void {
    // TODO: assert end_stream and contents of c_data for multiple calls of on_data.
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_data_calls++;
    data.release(data.context);
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Build first body data
  Buffer::OwnedImpl request_data = Buffer::OwnedImpl("request body");
  envoy_data c_data = Buffer::Utility::toBridgeData(request_data);

  // Build second body data
  Buffer::OwnedImpl request_data2 = Buffer::OwnedImpl("request body2");
  envoy_data c_data2 = Buffer::Utility::toBridgeData(request_data2);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, false);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, false));
  headers_post_cb();

  // Send request data.
  Event::PostCb data_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&data_post_cb));
  http_dispatcher_.sendData(stream, c_data, false);

  EXPECT_CALL(request_decoder_, decodeData(BufferStringEqual("request body"), false));
  data_post_cb();

  // Send second request data.
  Event::PostCb data_post_cb2;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&data_post_cb2));
  http_dispatcher_.sendData(stream, c_data2, true);

  EXPECT_CALL(request_decoder_, decodeData(BufferStringEqual("request body2"), true));
  data_post_cb2();

  // Encode response headers and data.
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, false);
  ASSERT_EQ(cc.on_headers_calls, 1);
  Buffer::InstancePtr response_data{new Buffer::OwnedImpl("response body")};
  response_encoder_->encodeData(*response_data, false);
  ASSERT_EQ(cc.on_data_calls, 1);

  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  Buffer::InstancePtr response_data2{new Buffer::OwnedImpl("response body2")};
  response_encoder_->encodeData(*response_data2, true);
  ASSERT_EQ(cc.on_data_calls, 2);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, MultipleStreams) {
  ready();

  envoy_stream_t stream1 = 1;
  envoy_stream_t stream2 = 2;
  // Start stream1.
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream1, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream1, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Start stream2.
  // Setup bridge_callbacks to handle the response headers.
  NiceMock<MockRequestDecoder> request_decoder2;
  ResponseEncoder* response_encoder2{};
  envoy_http_callbacks bridge_callbacks2;
  callbacks_called cc2 = {0, 0, 0, 0, 0, 0};
  bridge_callbacks2.context = &cc2;
  bridge_callbacks2.on_headers = [](envoy_headers c_headers, bool end_stream,
                                    void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    bool* on_headers_called2 = static_cast<bool*>(context);
    *on_headers_called2 = true;
  };
  bridge_callbacks2.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers2;
  HttpTestUtility::addDefaultHeaders(headers2);
  envoy_headers c_headers2 = Utility::toBridgeHeaders(headers2);

  // Create a stream.
  Event::PostCb start_stream_post_cb2;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb2));
  EXPECT_EQ(http_dispatcher_.startStream(stream2, bridge_callbacks2), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder2 = &encoder;
        return request_decoder2;
      }));
  start_stream_post_cb2();

  // Send request headers.
  Event::PostCb send_headers_post_cb2;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb2));
  http_dispatcher_.sendHeaders(stream2, c_headers2, true);

  EXPECT_CALL(request_decoder2, decodeHeaders_(_, true));
  send_headers_post_cb2();

  // Finish stream 2.
  Event::PostCb stream_deletion_post_cb2;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb2));
  TestResponseHeaderMapImpl response_headers2{{":status", "200"}};
  response_encoder2->encodeHeaders(response_headers2, true);
  ASSERT_EQ(cc2.on_headers_calls, 1);
  stream_deletion_post_cb2();
  // Ensure that the on_headers on the bridge_callbacks was called.
  ASSERT_EQ(cc2.on_complete_calls, 1);

  // Finish stream 1.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  stream_deletion_post_cb();
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, EnvoyLocalReply) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_CONNECTION_FAILURE);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Encode response headers. A non-200 code triggers an on_error callback chain. In particular, a
  // 503 should have an ENVOY_CONNECTION_FAILURE error code.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "503"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 0);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 0);
  ASSERT_EQ(cc.on_error_calls, 1);
}

TEST_F(DispatcherTest, EnvoyLocalReplyNon503) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_UNDEFINED_ERROR);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Encode response headers. A non-200 code triggers an on_error callback chain. In particular, a
  // non-503 should have an ENVOY_UNDEFINED_ERROR error code.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "504"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 0);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 0);
  ASSERT_EQ(cc.on_error_calls, 1);
}

TEST_F(DispatcherTest, EnvoyLocalReplyWithData) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_CONNECTION_FAILURE);
    ASSERT_EQ(Http::Utility::convertToString(error.message), "error message");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
    error.message.release(error.message.context);
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Encode response headers. A non-200 code triggers an on_error callback chain. In particular, a
  // 503 should have an ENVOY_CONNECTION_FAILURE error code. However, do not end the stream yet.
  TestResponseHeaderMapImpl response_headers{{":status", "503"}};
  response_encoder_->encodeHeaders(response_headers, false);
  ASSERT_EQ(cc.on_headers_calls, 0);

  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  Buffer::InstancePtr response_data{new Buffer::OwnedImpl("error message")};
  response_encoder_->encodeData(*response_data, true);
  ASSERT_EQ(cc.on_data_calls, 0);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 0);
  ASSERT_EQ(cc.on_error_calls, 1);
}

TEST_F(DispatcherTest, ResetStreamLocal) {
  ready();

  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_error = [](envoy_error, void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_cancel = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_cancel_calls++;
  };

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  Event::PostCb reset_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&reset_stream_post_cb));
  ASSERT_EQ(http_dispatcher_.resetStream(stream), ENVOY_SUCCESS);
  // The callback happens synchronously outside of the reset_stream_post_cb().
  ASSERT_EQ(cc.on_cancel_calls, 1);

  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  reset_stream_post_cb();
  stream_deletion_post_cb();

  ASSERT_EQ(cc.on_error_calls, 0);
  ASSERT_EQ(cc.on_complete_calls, 0);
}

TEST_F(DispatcherTest, RemoteResetAfterStreamStart) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_FALSE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_STREAM_RESET);
    ASSERT_EQ(error.message.length, 0);
    // This will use envoy_noop_release.
    error.message.release(error.message.context);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_cancel = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_cancel_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Used to verify that when a reset is received, the Http::Dispatcher::DirectStream fires
  // runResetCallbacks. The Http::ConnectionManager depends on the Http::Dispatcher::DirectStream
  // firing this tight loop to let the Http::ConnectionManager clean up its stream state.
  Http::MockStreamCallbacks callbacks;
  response_encoder_->getStream().addCallbacks(callbacks);

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Encode response headers.
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, false);
  ASSERT_EQ(cc.on_headers_calls, 1);

  Event::PostCb stream_deletion_post_cb;
  // Expect that when a reset is received, the Http::Dispatcher::DirectStream fires
  // runResetCallbacks. The Http::ConnectionManager depends on the Http::Dispatcher::DirectStream
  // firing this tight loop to let the Http::ConnectionManager clean up its stream state.
  EXPECT_CALL(callbacks, onResetStream(StreamResetReason::RemoteReset, _));
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  response_encoder_->getStream().resetStream(StreamResetReason::RemoteReset);
  stream_deletion_post_cb();
  // Ensure that the on_error on the bridge_callbacks was called.
  ASSERT_EQ(cc.on_error_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 0);
}

TEST_F(DispatcherTest, StreamResetAfterOnComplete) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Encode response headers.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  stream_deletion_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_complete_calls, 1);

  // Cancellation should have no effect as the stream should have already been cleaned up.
  ASSERT_EQ(http_dispatcher_.resetStream(stream), ENVOY_FAILURE);
}

TEST_F(DispatcherTest, ResetStreamLocalHeadersRemoteRaceLocalWins) {
  ready();

  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_STREAM_RESET);
    ASSERT_EQ(error.message.length, 0);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_cancel = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_cancel_calls++;
  };

  http_dispatcher_.synchronizer().enable();

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Start a thread to encode response headers. This will wait pre-dispatchable call.
  http_dispatcher_.synchronizer().waitOn("dispatch_encode_headers");
  std::thread t1([&] {
    TestResponseHeaderMapImpl response_headers{{":status", "200"}};
    response_headers.setEnvoyUpstreamServiceTime(20);
    response_encoder_->encodeHeaders(response_headers, true);
  });
  // Wait until the thread is actually waiting.
  http_dispatcher_.synchronizer().barrierOn("dispatch_encode_headers");

  // reset the stream from the client side. This should succeed.
  Event::PostCb reset_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&reset_stream_post_cb));
  ASSERT_EQ(http_dispatcher_.resetStream(stream), ENVOY_SUCCESS);
  // The callback happens synchronously outside of the reset_stream_post_cb().
  ASSERT_EQ(cc.on_cancel_calls, 1);

  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  reset_stream_post_cb();

  // Now signal the thread to continue. Dispatchable should return false and prevent on_headers and
  // on_complete from being called.
  http_dispatcher_.synchronizer().signal("dispatch_encode_headers");
  t1.join();

  stream_deletion_post_cb();

  ASSERT_EQ(cc.on_headers_calls, 0);
  ASSERT_EQ(cc.on_complete_calls, 0);
}

TEST_F(DispatcherTest, ResetStreamLocalHeadersRemoteRemoteWinsDeletesStream) {
  ready();

  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_STREAM_RESET);
    ASSERT_EQ(error.message.length, 0);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_cancel = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_cancel_calls++;
  };

  http_dispatcher_.synchronizer().enable();

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Start a thread to reset stream. This will wait pre-dispatchable call. But after getting the
  // stream stream.
  http_dispatcher_.synchronizer().waitOn("getStream_on_cancel");
  std::thread t1([&] {
    // This should fail synchronously because remote cleaned up the stream before the local reset
    // ran getStream.
    ASSERT_EQ(http_dispatcher_.resetStream(stream), ENVOY_FAILURE);
  });
  // Wait until the thread is actually waiting.
  http_dispatcher_.synchronizer().barrierOn("getStream_on_cancel");

  // Now encode headers. This will go through.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_headers.setEnvoyUpstreamServiceTime(20);
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  stream_deletion_post_cb();

  // Now signal the thread to continue. Dispatchable should return false and prevent on_headers and
  // on_complete from being called.
  http_dispatcher_.synchronizer().signal("getStream_on_cancel");
  t1.join();

  // The cancellation callback was not dispatchable.
  ASSERT_EQ(cc.on_cancel_calls, 0);
  ASSERT_EQ(cc.on_headers_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, ResetStreamLocalHeadersRemoteRemoteWins) {
  ready();

  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_STREAM_RESET);
    ASSERT_EQ(error.message.length, 0);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_cancel = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_cancel_calls++;
  };

  http_dispatcher_.synchronizer().enable();

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Start a thread to reset stream. This will wait pre-dispatchable call. But after getting the
  // stream stream.
  http_dispatcher_.synchronizer().waitOn("dispatch_on_cancel");
  std::thread t1([&] {
    // This should succeed because the stream was still present. However, the assertion at the end
    // of the test shows that the callback was not fired.
    ASSERT_EQ(http_dispatcher_.resetStream(stream), ENVOY_SUCCESS);
  });
  // Wait until the thread is actually waiting.
  http_dispatcher_.synchronizer().barrierOn("dispatch_on_cancel");

  // Now encode headers. This will go through.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_headers.setEnvoyUpstreamServiceTime(20);
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  stream_deletion_post_cb();

  // Now signal the thread to continue. Dispatchable should return false and prevent on_headers and
  // on_complete from being called.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(0);
  EXPECT_CALL(event_dispatcher_, post(_)).Times(0);
  http_dispatcher_.synchronizer().signal("dispatch_on_cancel");
  t1.join();

  // The cancellation callback was not dispatchable.
  ASSERT_EQ(cc.on_cancel_calls, 0);
  ASSERT_EQ(cc.on_headers_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, ResetStreamLocalResetRemoteRaceLocalWins) {
  ready();

  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_STREAM_RESET);
    ASSERT_EQ(error.message.length, 0);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_cancel = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_cancel_calls++;
  };

  http_dispatcher_.synchronizer().enable();

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Start a thread to remote reset stream. This will wait pre-dispatchable call. But after getting
  // the stream stream.
  http_dispatcher_.synchronizer().waitOn("dispatch_on_error");
  std::thread t1(
      [&] { response_encoder_->getStream().resetStream(StreamResetReason::RemoteReset); });
  // Wait until the thread is actually waiting.
  http_dispatcher_.synchronizer().barrierOn("dispatch_on_error");

  // Now local reset the stream. This will go through.
  Event::PostCb reset_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&reset_stream_post_cb));
  ASSERT_EQ(http_dispatcher_.resetStream(stream), ENVOY_SUCCESS);
  // The callback happens synchronously outside of the reset_stream_post_cb().
  ASSERT_EQ(cc.on_cancel_calls, 1);

  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  reset_stream_post_cb();

  // Now signal the thread to continue. The remote reset will not run.
  http_dispatcher_.synchronizer().signal("dispatch_on_error");
  t1.join();

  stream_deletion_post_cb();

  ASSERT_EQ(cc.on_error_calls, 0);
  ASSERT_EQ(cc.on_cancel_calls, 1);
}

TEST_F(DispatcherTest, ResetStreamLocalResetRemoteRemoteWinsDeletesStream) {
  ready();

  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_STREAM_RESET);
    ASSERT_EQ(error.message.length, 0);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_cancel = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_cancel_calls++;
  };

  http_dispatcher_.synchronizer().enable();

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Start a thread to locally reset stream. This will wait pre-dispatchable call. But after getting
  // the stream stream.
  http_dispatcher_.synchronizer().waitOn("getStream_on_cancel");
  std::thread t1([&] {
    // This should fail synchronously because remote cleaned up the stream before the local reset
    // ran getStream.
    ASSERT_EQ(http_dispatcher_.resetStream(stream), ENVOY_FAILURE);
  });
  // Wait until the thread is actually waiting.
  http_dispatcher_.synchronizer().barrierOn("getStream_on_cancel");

  // Now remote reset the stream. This will go through.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  response_encoder_->getStream().resetStream(StreamResetReason::RemoteReset);
  ASSERT_EQ(cc.on_error_calls, 1);
  stream_deletion_post_cb();

  // Now signal the thread to continue. The local reset will not run.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(0);
  EXPECT_CALL(event_dispatcher_, post(_)).Times(0);
  http_dispatcher_.synchronizer().signal("getStream_on_cancel");
  t1.join();

  // The callback was not dispatchable.
  ASSERT_EQ(cc.on_cancel_calls, 0);
}

TEST_F(DispatcherTest, ResetStreamLocalResetRemoteRemoteWins) {
  ready();

  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_error = [](envoy_error error, void* context) -> void {
    ASSERT_EQ(error.error_code, ENVOY_STREAM_RESET);
    ASSERT_EQ(error.message.length, 0);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_cancel = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_cancel_calls++;
  };

  http_dispatcher_.synchronizer().enable();

  // Build a set of request headers.
  TestRequestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(request_decoder_, decodeHeaders_(_, true));
  send_headers_post_cb();

  // Start a thread to locally reset stream. This will wait pre-dispatchable call. But after getting
  // the stream stream.
  http_dispatcher_.synchronizer().waitOn("dispatch_on_cancel");
  std::thread t1([&] {
    // This should succeed because the stream was still present. However, the assertion at the end
    // of the test shows that the callback was not fired.
    ASSERT_EQ(http_dispatcher_.resetStream(stream), ENVOY_SUCCESS);
  });
  // Wait until the thread is actually waiting.
  http_dispatcher_.synchronizer().barrierOn("dispatch_on_cancel");

  // Now remote reset the stream. This will go through.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  response_encoder_->getStream().resetStream(StreamResetReason::RemoteReset);
  ASSERT_EQ(cc.on_error_calls, 1);
  stream_deletion_post_cb();

  // Now signal the thread to continue. The local reset will not run.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(0);
  EXPECT_CALL(event_dispatcher_, post(_)).Times(0);
  http_dispatcher_.synchronizer().signal("dispatch_on_cancel");
  t1.join();

  // The callback was not dispatchable.
  ASSERT_EQ(cc.on_cancel_calls, 0);
}

TEST_F(DispatcherTest, ResetWhenRemoteClosesBeforeLocal) {
  ready();

  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Create a stream.
  Event::PostCb start_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&start_stream_post_cb));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Grab the response encoder in order to dispatch responses on the stream.
  // Return the request decoder to make sure calls are dispatched to the decoder via the dispatcher
  // API.
  EXPECT_CALL(api_listener_, newStream(_, _))
      .WillOnce(Invoke([&](ResponseEncoder& encoder, bool) -> RequestDecoder& {
        response_encoder_ = &encoder;
        return request_decoder_;
      }));
  start_stream_post_cb();

  // Encode response headers.
  Event::PostCb stream_deletion_post_cb;
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&stream_deletion_post_cb));
  TestResponseHeaderMapImpl response_headers{{":status", "200"}};
  response_encoder_->encodeHeaders(response_headers, true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 1);

  // Fire stream reset because Envoy does not allow half-open streams on the local side.
  response_encoder_->getStream().resetStream(StreamResetReason::RemoteReset);
  stream_deletion_post_cb();
  ASSERT_EQ(cc.on_error_calls, 0);
}

} // namespace Http
} // namespace Envoy
