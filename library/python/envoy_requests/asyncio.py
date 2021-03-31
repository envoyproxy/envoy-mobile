import asyncio

from envoy_requests.common.core import make_stream
from envoy_requests.common.core import send_request
from envoy_requests.common.engine import make_envoy_engine
from envoy_requests.common.executor import AsyncioExecutor
from envoy_requests.response import Response


# TODO: add better typing to this (and functions that use it)
async def request(*args, **kwargs) -> Response:
    response = Response()
    stream_complete = asyncio.Event()

    stream = make_stream(
        make_envoy_engine(), AsyncioExecutor(), response, stream_complete
    )
    send_request(stream, *args, **kwargs)
    await stream_complete.wait()
    return response


async def delete(*args, **kwargs) -> Response:
    return await request("delete", *args, **kwargs)


async def get(*args, **kwargs) -> Response:
    return await request("get", *args, **kwargs)


async def head(*args, **kwargs) -> Response:
    return await request("head", *args, **kwargs)


async def options(*args, **kwargs) -> Response:
    return await request("options", *args, **kwargs)


async def patch(*args, **kwargs) -> Response:
    return await request("patch", *args, **kwargs)


async def post(*args, **kwargs) -> Response:
    return await request("post", *args, **kwargs)


async def put(*args, **kwargs) -> Response:
    return await request("put", *args, **kwargs)


async def trace(*args, **kwargs) -> Response:
    return await request("trace", *args, **kwargs)
