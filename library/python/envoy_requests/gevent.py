from gevent.event import Event

from envoy_requests.common.core import make_stream
from envoy_requests.common.core import send_request
from envoy_requests.common.engine import make_envoy_engine
from envoy_requests.common.executor import GeventExecutor
from envoy_requests.response import Response


# TODO: add better typing to this (and functions that use it)
def request(*args, **kwargs) -> Response:
    response = Response()
    stream_complete = Event()

    stream = make_stream(
        make_envoy_engine(), GeventExecutor(), response, stream_complete
    )
    send_request(stream, *args, **kwargs)
    stream_complete.wait()
    return response


def delete(*args, **kwargs) -> Response:
    return request("delete", *args, **kwargs)


def get(*args, **kwargs) -> Response:
    return request("get", *args, **kwargs)


def head(*args, **kwargs) -> Response:
    return request("head", *args, **kwargs)


def options(*args, **kwargs) -> Response:
    return request("options", *args, **kwargs)


def patch(*args, **kwargs) -> Response:
    return request("patch", *args, **kwargs)


def post(*args, **kwargs) -> Response:
    return request("post", *args, **kwargs)


def put(*args, **kwargs) -> Response:
    return request("put", *args, **kwargs)


def trace(*args, **kwargs) -> Response:
    return request("trace", *args, **kwargs)
