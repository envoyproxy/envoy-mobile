#include "test/mocks/http/mocks.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"
#include "library/common/api/external.h"
#include "library/common/extensions/filters/http/platform_bridge/filter.h"
#include "library/common/extensions/filters/http/platform_bridge/filter.pb.h"

using testing::ByMove;
using testing::Return;

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace PlatformBridge {
namespace {

std::string to_string(envoy_data data) {
  return std::string(reinterpret_cast<const char*>(data.bytes), data.length);
}

class PlatformBridgeFilterTest : public testing::Test {
public:
  void setUpFilter(std::string&& yaml, envoy_http_filter* platform_filter) {
    envoymobile::extensions::filters::http::platform_bridge::PlatformBridge config;
    TestUtility::loadFromYaml(yaml, config);
    Api::External::registerApi(config.platform_filter_name(), platform_filter);

    config_ = std::make_shared<PlatformBridgeFilterConfig>(config);
    filter_ = std::make_unique<PlatformBridgeFilter>(config_);
    filter_->setDecoderFilterCallbacks(decoder_callbacks_);
    filter_->setEncoderFilterCallbacks(encoder_callbacks_);
  }

  typedef struct {
    unsigned int init_filter_calls;
    unsigned int on_request_headers_calls;
    unsigned int on_request_data_calls;
    unsigned int on_request_trailers_calls;
    unsigned int on_response_headers_calls;
    unsigned int on_response_data_calls;
    unsigned int on_response_trailers_calls;
    unsigned int release_filter_calls;
  } filter_invocations;

  PlatformBridgeFilterConfigSharedPtr config_{};
  std::unique_ptr<PlatformBridgeFilter> filter_{};
  NiceMock<Http::MockStreamDecoderFilterCallbacks> decoder_callbacks_;
  NiceMock<Http::MockStreamEncoderFilterCallbacks> encoder_callbacks_;
};

TEST_F(PlatformBridgeFilterTest, NullImplementation) {
  envoy_http_filter* null_filter =
      static_cast<envoy_http_filter*>(safe_calloc(1, sizeof(envoy_http_filter)));
  setUpFilter("platform_filter_name: NullImplementation\n", null_filter);

  Http::TestRequestHeaderMapImpl request_headers{{":authority", "test.code"}};
  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->decodeHeaders(request_headers, false));

  Buffer::OwnedImpl request_data = Buffer::OwnedImpl("request body");
  EXPECT_EQ(Http::FilterDataStatus::Continue, filter_->decodeData(request_data, false));

  Http::TestRequestTrailerMapImpl request_trailers{{"x-test-trailer", "test trailer"}};
  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter_->decodeTrailers(request_trailers));

  Http::TestResponseHeaderMapImpl response_headers{{":status", "test.code"}};
  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->encodeHeaders(response_headers, false));

  Buffer::OwnedImpl response_data = Buffer::OwnedImpl("response body");
  EXPECT_EQ(Http::FilterDataStatus::Continue, filter_->encodeData(response_data, false));

  Http::TestResponseTrailerMapImpl response_trailers{{"x-test-trailer", "test trailer"}};
  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter_->encodeTrailers(response_trailers));

  filter_->onDestroy();

  free(null_filter);
}

TEST_F(PlatformBridgeFilterTest, PartialNullImplementation) {
  envoy_http_filter* noop_filter =
      static_cast<envoy_http_filter*>(safe_calloc(1, sizeof(envoy_http_filter)));
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  noop_filter->static_context = &invocations;
  noop_filter->init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  noop_filter->release_filter = [](const void* context) -> void {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->release_filter_calls++;
  };
  setUpFilter("platform_filter_name: PartialNullImplementation\n", noop_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Http::TestRequestHeaderMapImpl request_headers{{":authority", "test.code"}};
  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->decodeHeaders(request_headers, false));

  Buffer::OwnedImpl request_data = Buffer::OwnedImpl("request body");
  EXPECT_EQ(Http::FilterDataStatus::Continue, filter_->decodeData(request_data, false));

  Http::TestRequestTrailerMapImpl request_trailers{{"x-test-trailer", "test trailer"}};
  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter_->decodeTrailers(request_trailers));

  Http::TestResponseHeaderMapImpl response_headers{{":status", "test.code"}};
  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->encodeHeaders(response_headers, false));

  Buffer::OwnedImpl response_data = Buffer::OwnedImpl("response body");
  EXPECT_EQ(Http::FilterDataStatus::Continue, filter_->encodeData(response_data, false));

  Http::TestResponseTrailerMapImpl response_trailers{{"x-test-trailer", "test trailer"}};
  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter_->encodeTrailers(response_trailers));

  filter_->onDestroy();
  EXPECT_EQ(invocations.release_filter_calls, 1);

  free(noop_filter);
}

TEST_F(PlatformBridgeFilterTest, BasicContinueOnRequestHeaders) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_request_headers = [](envoy_headers c_headers, bool end_stream,
                                          const void* context) -> envoy_filter_headers_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    EXPECT_EQ(c_headers.length, 1);
    EXPECT_EQ(to_string(c_headers.headers[0].key), ":authority");
    EXPECT_EQ(to_string(c_headers.headers[0].value), "test.code");
    EXPECT_TRUE(end_stream);
    invocations->on_request_headers_calls++;
    return {kEnvoyFilterHeadersStatusContinue, c_headers};
  };

  setUpFilter(R"EOF(
platform_filter_name: BasicContinueOnRequestHeaders
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Http::TestRequestHeaderMapImpl request_headers{{":authority", "test.code"}};

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->decodeHeaders(request_headers, true));
  EXPECT_EQ(invocations.on_request_headers_calls, 1);
}

TEST_F(PlatformBridgeFilterTest, BasicContinueOnRequestData) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_request_data = [](envoy_data c_data, bool end_stream,
                                       const void* context) -> envoy_filter_data_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    EXPECT_EQ(to_string(c_data), "request body");
    EXPECT_TRUE(end_stream);
    invocations->on_request_data_calls++;
    return {kEnvoyFilterDataStatusContinue, c_data, nullptr};
  };

  setUpFilter(R"EOF(
platform_filter_name: BasicContinueOnRequestData
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Buffer::OwnedImpl request_data = Buffer::OwnedImpl("request body");

  EXPECT_EQ(Http::FilterDataStatus::Continue, filter_->decodeData(request_data, true));
  EXPECT_EQ(invocations.on_request_data_calls, 1);
}

TEST_F(PlatformBridgeFilterTest, StopAndBufferOnRequestData) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_request_data = [](envoy_data c_data, bool end_stream,
                                       const void* context) -> envoy_filter_data_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    std::string expected_data[3] = {"A", "AB", "ABC"};
    EXPECT_EQ(to_string(c_data), expected_data[invocations->on_request_data_calls++]);
    EXPECT_FALSE(end_stream);
    c_data.release(c_data.context);
    return {kEnvoyFilterDataStatusStopIterationAndBuffer, envoy_nodata, nullptr};
  };

  Buffer::OwnedImpl decoding_buffer;
  EXPECT_CALL(decoder_callbacks_, decodingBuffer())
      .Times(3)
      .WillRepeatedly(Return(&decoding_buffer));
  EXPECT_CALL(decoder_callbacks_, modifyDecodingBuffer(_))
      .Times(3)
      .WillRepeatedly(Invoke([&](std::function<void(Buffer::Instance&)> callback) -> void {
        callback(decoding_buffer);
      }));

  setUpFilter(R"EOF(
platform_filter_name: StopAndBufferOnRequestData
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Buffer::OwnedImpl first_chunk = Buffer::OwnedImpl("A");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationAndBuffer,
            filter_->decodeData(first_chunk, false));
  // Since the return code can't be handled in a unit test, manually update the buffer here.
  decoding_buffer.move(first_chunk);
  EXPECT_EQ(invocations.on_request_data_calls, 1);

  Buffer::OwnedImpl second_chunk = Buffer::OwnedImpl("B");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(second_chunk, false));
  // Manual update not required, because once iteration is stopped, data is added directly.
  EXPECT_EQ(invocations.on_request_data_calls, 2);

  Buffer::OwnedImpl third_chunk = Buffer::OwnedImpl("C");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer, filter_->decodeData(third_chunk, false));
  // Manual update not required, because once iteration is stopped, data is added directly.
  EXPECT_EQ(invocations.on_request_data_calls, 3);
}

TEST_F(PlatformBridgeFilterTest, StopNoBufferOnRequestData) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_request_data = [](envoy_data c_data, bool end_stream,
                                       const void* context) -> envoy_filter_data_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    std::string expected_data[3] = {"A", "B", "C"};
    EXPECT_EQ(to_string(c_data), expected_data[invocations->on_request_data_calls++]);
    EXPECT_FALSE(end_stream);
    c_data.release(c_data.context);
    return {kEnvoyFilterDataStatusStopIterationNoBuffer, envoy_nodata, nullptr};
  };

  setUpFilter(R"EOF(
platform_filter_name: StopNoBufferOnRequestData
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Buffer::OwnedImpl first_chunk = Buffer::OwnedImpl("A");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer, filter_->decodeData(first_chunk, false));
  EXPECT_EQ(invocations.on_request_data_calls, 1);

  Buffer::OwnedImpl second_chunk = Buffer::OwnedImpl("B");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(second_chunk, false));
  EXPECT_EQ(invocations.on_request_data_calls, 2);

  Buffer::OwnedImpl third_chunk = Buffer::OwnedImpl("C");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer, filter_->decodeData(third_chunk, false));
  EXPECT_EQ(invocations.on_request_data_calls, 3);
}

TEST_F(PlatformBridgeFilterTest, BasicContinueOnRequestTrailers) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_request_trailers = [](envoy_headers c_trailers,
                                           const void* context) -> envoy_filter_trailers_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    EXPECT_EQ(c_trailers.length, 1);
    EXPECT_EQ(to_string(c_trailers.headers[0].key), "x-test-trailer");
    EXPECT_EQ(to_string(c_trailers.headers[0].value), "test trailer");
    invocations->on_request_trailers_calls++;
    return {kEnvoyFilterTrailersStatusContinue, c_trailers, nullptr, nullptr};
  };

  setUpFilter(R"EOF(
platform_filter_name: BasicContinueOnRequestTrailers
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Http::TestRequestTrailerMapImpl request_trailers{{"x-test-trailer", "test trailer"}};

  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter_->decodeTrailers(request_trailers));
  EXPECT_EQ(invocations.on_request_trailers_calls, 1);
}

// DIVIDE

TEST_F(PlatformBridgeFilterTest, BasicContinueOnResponseHeaders) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_response_headers = [](envoy_headers c_headers, bool end_stream,
                                           const void* context) -> envoy_filter_headers_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    EXPECT_EQ(c_headers.length, 1);
    EXPECT_EQ(to_string(c_headers.headers[0].key), ":status");
    EXPECT_EQ(to_string(c_headers.headers[0].value), "test.code");
    EXPECT_TRUE(end_stream);
    invocations->on_response_headers_calls++;
    return {kEnvoyFilterHeadersStatusContinue, c_headers};
  };

  setUpFilter(R"EOF(
platform_filter_name: BasicContinueOnResponseHeaders
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Http::TestResponseHeaderMapImpl response_headers{{":status", "test.code"}};

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->encodeHeaders(response_headers, true));
  EXPECT_EQ(invocations.on_response_headers_calls, 1);
}

TEST_F(PlatformBridgeFilterTest, BasicContinueOnResponseData) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_response_data = [](envoy_data c_data, bool end_stream,
                                        const void* context) -> envoy_filter_data_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    EXPECT_EQ(to_string(c_data), "response body");
    EXPECT_TRUE(end_stream);
    invocations->on_response_data_calls++;
    return {kEnvoyFilterDataStatusContinue, c_data, nullptr};
  };

  setUpFilter(R"EOF(
platform_filter_name: BasicContinueOnResponseData
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Buffer::OwnedImpl response_data = Buffer::OwnedImpl("response body");

  EXPECT_EQ(Http::FilterDataStatus::Continue, filter_->encodeData(response_data, true));
  EXPECT_EQ(invocations.on_response_data_calls, 1);
}

TEST_F(PlatformBridgeFilterTest, StopAndBufferOnResponseData) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_response_data = [](envoy_data c_data, bool end_stream,
                                        const void* context) -> envoy_filter_data_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    std::string expected_data[3] = {"A", "AB", "ABC"};
    EXPECT_EQ(to_string(c_data), expected_data[invocations->on_response_data_calls++]);
    EXPECT_FALSE(end_stream);
    c_data.release(c_data.context);
    return {kEnvoyFilterDataStatusStopIterationAndBuffer, envoy_nodata, nullptr};
  };

  Buffer::OwnedImpl encoding_buffer;
  EXPECT_CALL(encoder_callbacks_, encodingBuffer())
      .Times(3)
      .WillRepeatedly(Return(&encoding_buffer));
  EXPECT_CALL(encoder_callbacks_, modifyEncodingBuffer(_))
      .Times(3)
      .WillRepeatedly(Invoke([&](std::function<void(Buffer::Instance&)> callback) -> void {
        callback(encoding_buffer);
      }));

  setUpFilter(R"EOF(
platform_filter_name: StopAndBufferOnResponseData
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Buffer::OwnedImpl first_chunk = Buffer::OwnedImpl("A");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationAndBuffer,
            filter_->encodeData(first_chunk, false));
  // Since the return code can't be handled in a unit test, manually update the buffer here.
  encoding_buffer.move(first_chunk);
  EXPECT_EQ(invocations.on_response_data_calls, 1);

  Buffer::OwnedImpl second_chunk = Buffer::OwnedImpl("B");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->encodeData(second_chunk, false));
  // Manual update not required, because once iteration is stopped, data is added directly.
  EXPECT_EQ(invocations.on_response_data_calls, 2);

  Buffer::OwnedImpl third_chunk = Buffer::OwnedImpl("C");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer, filter_->encodeData(third_chunk, false));
  // Manual update not required, because once iteration is stopped, data is added directly.
  EXPECT_EQ(invocations.on_response_data_calls, 3);
}

TEST_F(PlatformBridgeFilterTest, StopNoBufferOnResponseData) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_response_data = [](envoy_data c_data, bool end_stream,
                                        const void* context) -> envoy_filter_data_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    std::string expected_data[3] = {"A", "B", "C"};
    EXPECT_EQ(to_string(c_data), expected_data[invocations->on_response_data_calls++]);
    EXPECT_FALSE(end_stream);
    c_data.release(c_data.context);
    return {kEnvoyFilterDataStatusStopIterationNoBuffer, envoy_nodata, nullptr};
  };

  setUpFilter(R"EOF(
platform_filter_name: StopNoBufferOnResponseData
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Buffer::OwnedImpl first_chunk = Buffer::OwnedImpl("A");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer, filter_->encodeData(first_chunk, false));
  EXPECT_EQ(invocations.on_response_data_calls, 1);

  Buffer::OwnedImpl second_chunk = Buffer::OwnedImpl("B");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->encodeData(second_chunk, false));
  EXPECT_EQ(invocations.on_response_data_calls, 2);

  Buffer::OwnedImpl third_chunk = Buffer::OwnedImpl("C");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer, filter_->encodeData(third_chunk, false));
  EXPECT_EQ(invocations.on_response_data_calls, 3);
}

TEST_F(PlatformBridgeFilterTest, BasicContinueOnResponseTrailers) {
  envoy_http_filter platform_filter;
  filter_invocations invocations = {0, 0, 0, 0, 0, 0, 0, 0};
  platform_filter.static_context = &invocations;
  platform_filter.init_filter = [](const void* context) -> const void* {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    invocations->init_filter_calls++;
    return context;
  };
  platform_filter.on_response_trailers = [](envoy_headers c_trailers,
                                            const void* context) -> envoy_filter_trailers_status {
    filter_invocations* invocations = static_cast<filter_invocations*>(const_cast<void*>(context));
    EXPECT_EQ(c_trailers.length, 1);
    EXPECT_EQ(to_string(c_trailers.headers[0].key), "x-test-trailer");
    EXPECT_EQ(to_string(c_trailers.headers[0].value), "test trailer");
    invocations->on_response_trailers_calls++;
    return {kEnvoyFilterTrailersStatusContinue, c_trailers, nullptr, nullptr};
  };

  setUpFilter(R"EOF(
platform_filter_name: BasicContinueOnResponseTrailers
)EOF",
              &platform_filter);
  EXPECT_EQ(invocations.init_filter_calls, 1);

  Http::TestResponseTrailerMapImpl response_trailers{{"x-test-trailer", "test trailer"}};

  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter_->encodeTrailers(response_trailers));
  EXPECT_EQ(invocations.on_response_trailers_calls, 1);
}

} // namespace
} // namespace PlatformBridge
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
