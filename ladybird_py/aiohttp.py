from __future__ import annotations

import json as _json
from typing import Any, Dict, Optional

from .net import request_async


class ClientResponse:
    def __init__(self, raw: Dict[str, Any]):
        self.status: int = int(raw.get("status", 0))
        self.headers: Dict[str, str] = {k.lower(): v for k, v in (raw.get("headers") or {}).items()}
        self._body: str = raw.get("body", "")

    @property
    def ok(self) -> bool:
        return 200 <= self.status < 300

    async def text(self) -> str:
        return self._body

    async def json(self) -> Any:
        if not self._body:
            return None
        return _json.loads(self._body)


class ClientSession:
    async def __aenter__(self) -> "ClientSession":
        return self

    async def __aexit__(self, exc_type, exc, tb) -> None:
        return None

    async def request(self, method: str, url: str, **kwargs: Any) -> ClientResponse:
        raw = await request_async(method, url, headers=kwargs.get("headers"), data=kwargs.get("data"), json=kwargs.get("json"), timeout=kwargs.get("timeout"))
        return ClientResponse(raw)

    async def get(self, url: str, **kwargs: Any) -> ClientResponse:
        return await self.request("GET", url, **kwargs)

    async def post(self, url: str, **kwargs: Any) -> ClientResponse:
        return await self.request("POST", url, **kwargs)
