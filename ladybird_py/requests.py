from __future__ import annotations

import json as _json
from typing import Any, Dict, Optional

from . import net


class Response:
    def __init__(self, raw: Dict[str, Any]):
        self._raw = raw
        self.status_code: int = int(raw.get("status", 0))
        self.headers: Dict[str, str] = {k.lower(): v for k, v in (raw.get("headers") or {}).items()}
        self._body: str = raw.get("body", "")

    @property
    def ok(self) -> bool:
        return 200 <= self.status_code < 300

    @property
    def text(self) -> str:
        return self._body

    def json(self) -> Any:
        if not self._body:
            return None
        return _json.loads(self._body)


def request(method: str, url: str, *, headers: Optional[Dict[str, str]] = None, data: Optional[bytes | str] = None, json: Optional[Any] = None, timeout: Optional[float] = None) -> Response:
    res = net.request(method, url, headers=headers, data=data, json=json, timeout=timeout)
    return Response(res)


def get(url: str, **kwargs: Any) -> Response:
    return request("GET", url, **kwargs)


def post(url: str, **kwargs: Any) -> Response:
    return request("POST", url, **kwargs)


def put(url: str, **kwargs: Any) -> Response:
    return request("PUT", url, **kwargs)


def delete(url: str, **kwargs: Any) -> Response:
    return request("DELETE", url, **kwargs)
