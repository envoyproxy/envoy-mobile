# TODO: don't automatically prefer gevent
# maybe use threading by default
# let the user explicitly choose between
# envoy_requests.gevent and envoy_requests.asyncio
# for async libraries
from envoy_requests.gevent import *
from envoy_requests.response import Response
