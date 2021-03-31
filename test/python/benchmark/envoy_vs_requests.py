import gevent.monkey

gevent.monkey.patch_all()

import gevent
import pytest
import requests
from gevent.pool import Group

from envoy_requests import gevent as envoy_requests
from envoy_requests.gevent.engine import make_gevent_envoy_engine


@pytest.fixture
def premake_envoy_engine():
    make_gevent_envoy_engine()


def ping_api(requests_impl, url: str, concurrent_requests: int):
    group = Group()
    for _ in range(concurrent_requests):
        group.spawn(requests_impl.get, url)
    group.join()


@pytest.mark.parametrize("implementation", [requests, envoy_requests])
@pytest.mark.parametrize("concurrent_requests", [1, 10, 100])
def test_performance(
    premake_envoy_engine, benchmark, implementation, concurrent_requests
):
    benchmark(ping_api, implementation, "https://www.google.com/", concurrent_requests)
