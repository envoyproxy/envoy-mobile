import threading
import unittest

from library.python import envoy_engine
from test.python.integration.common import CONFIG_TEMPLATE


class TestSendHeaders(unittest.TestCase):
    def test_send_headers(self):
        engine_running = threading.Event()
        engine = (
            envoy_engine.EngineBuilder(CONFIG_TEMPLATE)
            .add_log_level(envoy_engine.LogLevel.Error)
            .set_on_engine_running(lambda: engine_running.set())
            .build()
        )
        engine_running.wait()

        response_lock = threading.Lock()
        response = {"status": None, "end_stream": None}

        def _on_headers(response_headers: envoy_engine.ResponseHeaders, end_stream: bool):
            with response_lock:
                response["status"] = response_headers.http_status()
                response["end_stream"] = end_stream

        stream_complete = threading.Event()
        stream = (
            engine
            .stream_client()
            .new_stream_prototype()
            .set_on_headers(_on_headers)
            # unused:
            # .set_on_metadata(on_metadata)
            # .set_on_trailers(on_trailers)
            .set_on_complete(lambda: stream_complete.set())
            .set_on_error(lambda _: stream_complete.set())
            .set_on_cancel(lambda: stream_complete.set())
            .start()
        )
        stream.send_headers(
            envoy_engine.RequestHeadersBuilder(
                envoy_engine.RequestMethod.GET,
                "https",
                "example.com",
                "/test",
            )
            .add_upstream_http_protocol(envoy_engine.UpstreamHttpProtocol.HTTP2)
            .build(),
            True,
        )
        stream_complete.wait()
        engine.terminate()

        assert response == {
            "status": 200,
            "end_stream": True,
        }

if __name__ == "__main__":
    unittest.main()
