from envoy_requests import get


def test_send_headers():
    response = get("https://api.lyft.com/ping")
    assert response.status == 200
    assert response.headers["content-type"] == "application/json"
    assert response.body == b"{}"
