#!/usr/bin/env python3
import asyncio

from ladybird_py.backends import set_urllib_backend_default


def test_requests_sync():
    set_urllib_backend_default()
    import requests

    r = requests.get("https://example.com")
    assert r.status_code == 200
    assert isinstance(r.text, str) and len(r.text) > 0


def test_aiohttp_async():
    set_urllib_backend_default()
    import ladybird_py.aiohttp as aiohttp

    async def main():
        async with aiohttp.ClientSession() as session:
            resp = await session.get("https://example.com")
            assert resp.status == 200
            txt = await resp.text()
            assert isinstance(txt, str) and len(txt) > 0

    asyncio.run(main())


if __name__ == "__main__":
    test_requests_sync()
    test_aiohttp_async()
    print("OK")
