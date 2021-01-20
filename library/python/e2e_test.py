import threading

from library.python import envoy_engine


def main():
    on_engine_running_cond = threading.Condition(threading.Lock())

    def on_engine_running():
        with on_engine_running_cond:
            on_engine_running_cond.notify()

    engine = (
        envoy_engine.EngineBuilder()
        .add_log_level(envoy_engine.LogLevel.Info)
        .set_on_engine_running(on_engine_running)
        .build()
    )

    with on_engine_running_cond:
        on_engine_running_cond.wait()

    stream_complete_cond = threading.Condition(threading.Lock())

    def on_headers(response_headers: envoy_engine.ResponseHeaders, end_stream: bool):
        # TODO: provide iter for header types
        print(response_headers)

    def on_data(data: bytes, end_stream: bool):
        print(data)

    def on_complete():
        with stream_complete_cond:
            stream_complete_cond.notify()

    def on_error(error: envoy_engine.EnvoyError):
        with stream_complete_cond:
            stream_complete_cond.notify()

    def on_cancel():
        with stream_complete_cond:
            stream_complete_cond.notify()

    stream = (
        engine
        .stream_client()
        .new_stream_prototype()
        .set_on_headers(on_headers)
        .set_on_data(on_data)
        # unused:
        # .set_on_metadata(on_metadata)
        # .set_on_trailers(on_trailers)
        .set_on_complete(on_complete)
        .set_on_error(on_error)
        .set_on_cancel(on_cancel)
        .start()
    )

    stream.send_headers(
        envoy_engine.RequestHeadersBuilder(
            envoy_engine.RequestMethod.GET,
            "https",
            "www.google.com",
            "/",
        )
        .build(),
        True,
    )

    with stream_complete_cond:
        stream_complete_cond.wait()

if __name__ == "__main__":
    main()
