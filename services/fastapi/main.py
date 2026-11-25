import asyncio
import os
from typing import Annotated

from fastapi import FastAPI, Query, Request, Response

SERVICE_NAME = os.getenv("SERVICE_NAME", "fastapi")

MAX_DELAY_MS = 60_000
MAX_PAYLOAD_KB = 100 * 1024  # 100 Mb

app = FastAPI(
    title=f"test-service ({SERVICE_NAME})",
    docs_url=None,
    redoc_url=None,
)


@app.get("/fast")
async def fast() -> dict[str, str]:
    return {
        "service": SERVICE_NAME,
        "status": "ok",
    }


@app.get("/delay")
async def delay(ms: Annotated[int, Query(ge=0, le=MAX_DELAY_MS)] = 100) -> dict[str, str | int]:
    await asyncio.sleep(ms / 1000)
    return {
        "service": SERVICE_NAME,
        "delay_ms": ms,
    }


@app.get("/payload")
async def payload(kb: Annotated[int, Query(ge=0, le=MAX_PAYLOAD_KB)] = 1) -> Response:
    return Response(
        content=b"x" * (kb * 1024),
        media_type="application/octet-stream",
    )


@app.post("/echo")
async def echo(request: Request) -> Response:
    body = await request.body()
    content_type = request.headers.get("content-type", "application/octet-stream")

    return Response(content=body, media_type=content_type)


@app.get("/health")
async def health() -> dict[str, str]:
    return {"status": "ok"}


@app.api_route("/headers", methods=["GET", "POST", "PUT", "PATCH", "DELETE"])
async def headers(request: Request) -> dict[str, str | object]:
    return {
        "service": SERVICE_NAME,
        "method": request.method,
        "path": request.url.path,
        "query": request.url.query,
        "client": request.client.host if request.client else None,
        "headers": dict(request.headers),
    }
