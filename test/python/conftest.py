import sys
from http.server import HTTPServer
from threading import Event
from threading import Thread

import pytest

import envoy_requests
from tests.echo_server import EchoServerHandler


@pytest.fixture(scope="session")
def http_server_url():
    with open("tests/test_envoy_config_template.yaml", "r") as f:
        # TODO: generalize to all implementations
        envoy_requests.common.engine.config_template_override = f.read()

    ip = "127.0.0.1"
    port = 9876

    kill_server = Event()

    def _run_http_server():
        server = HTTPServer((ip, port), EchoServerHandler)
        server.timeout = 0.25
        while True:
            if kill_server.is_set():
                break
            try:
                server.handle_request()
            except Exception as e:
                print(f"Encountered exception: {str(e)}", file=sys.stderr)

    # it's safe to spawn a daemon thread here
    # because the process will exist alongside the thread
    # and the OS will release the resource
    # even if the thread does not
    server = Thread(target=_run_http_server)
    server.start()
    yield f"http://{ip}:{port}/"
    kill_server.set()
    server.join()
