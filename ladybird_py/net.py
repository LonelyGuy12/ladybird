from __future__ import annotations

from typing import Any, Dict, Optional

from .bridge import call, call_async


class NetError(Exception):
    pass


def request(method: str, url: str, *, headers: Optional[Dict[str, str]] = None, data: Optional[bytes | str] = None, json: Optional[Any] = None, timeout: Optional[float] = None) -> Dict[str, Any]:
    payload: Dict[str, Any] = {
        "method": method.upper(),
        "url": url,
        "headers": headers or {},
    }
    if data is not None:
        payload["body"] = data if isinstance(data, str) else data.decode("utf-8", errors="ignore")
    if json is not None:
        payload["json"] = json
    if timeout is not None:
        payload["timeout"] = timeout

    res = call("net.request", payload)
    if not res.get("ok", False):
        raise NetError(res.get("error", "network error"))
    return res


async def request_async(method: str, url: str, *, headers: Optional[Dict[str, str]] = None, data: Optional[bytes | str] = None, json: Optional[Any] = None, timeout: Optional[float] = None) -> Dict[str, Any]:
    payload: Dict[str, Any] = {
        "method": method.upper(),
        "url": url,
        "headers": headers or {},
    }
    if data is not None:
        payload["body"] = data if isinstance(data, str) else data.decode("utf-8", errors="ignore")
    if json is not None:
        payload["json"] = json
    if timeout is not None:
        payload["timeout"] = timeout

    res = await call_async("net.request", payload)
    if not res.get("ok", False):
        raise NetError(res.get("error", "network error"))
    return res
