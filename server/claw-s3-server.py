"""
claw-s3-server.py — DeskMate ClawCorp-side WebSocket bridge

Acts as the server-side proxy between CoreS3 SE and:
  - ClawCorp Agent system
  - DeepSeek API (voice understanding, chat, TTS)
  - iFinD API (market data)
  - Feishu (notifications)

Protocol:
  CoreS3 SE ← WebSocket → this server ← internal → ClawCorp Agents

Phase 1: WebSocket echo + status push
Phase 2: Voice relay to DeepSeek API
Phase 3: Agent status + approval routing
"""

import asyncio
import json
import logging
import os
import time
from datetime import datetime, timezone, timedelta

try:
    import websockets
except ImportError:
    print("Install: pip install websockets python-dotenv")
    raise

from dotenv import load_dotenv

# ── Config ────────────────────────────────────────────────────────────
load_dotenv()
HOST = os.getenv("WS_HOST", "0.0.0.0")
PORT = int(os.getenv("WS_PORT", "8765"))
DEEPSEEK_API_KEY = os.getenv("DEEPSEEK_API_KEY", "")
DEEPSEEK_MODEL = os.getenv("DEEPSEEK_MODEL", "deepseek-chat")

log = logging.getLogger("claw-s3-server")
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)

TZ = timezone(timedelta(hours=8), "CST")


# ── Client Manager ────────────────────────────────────────────────────
class ClientManager:
    """Manages connected CoreS3 SE devices."""

    def __init__(self):
        self.clients: dict[str, websockets.WebSocketServerProtocol] = {}

    def register(self, ws: websockets.WebSocketServerProtocol) -> str:
        client_id = f"deskmate-{id(ws):x}"
        self.clients[client_id] = ws
        log.info(f"Client registered: {client_id} (total: {len(self.clients)})")
        return client_id

    def unregister(self, client_id: str):
        self.clients.pop(client_id, None)
        log.info(f"Client unregistered: {client_id} (total: {len(self.clients)})")

    async def broadcast(self, message: dict):
        """Send a message to all connected devices."""
        payload = json.dumps(message, ensure_ascii=False)
        disconnected = []
        for cid, ws in self.clients.items():
            try:
                await ws.send(payload)
            except websockets.exceptions.ConnectionClosed:
                disconnected.append(cid)
        for cid in disconnected:
            self.unregister(cid)

    async def send_to(self, client_id: str, message: dict):
        """Send a message to a specific device."""
        ws = self.clients.get(client_id)
        if ws:
            try:
                await ws.send(json.dumps(message, ensure_ascii=False))
            except websockets.exceptions.ConnectionClosed:
                self.unregister(client_id)


clients = ClientManager()


# ── Message Handlers ──────────────────────────────────────────────────
def make_message(msg_type: str, payload: dict) -> dict:
    return {
        "type": msg_type,
        "id": f"svr_{int(time.time() * 1000)}",
        "payload": payload,
        "timestamp": datetime.now(TZ).isoformat(),
    }


async def handle_voice_query(client_id: str, payload: dict):
    """Phase 2: Relay voice data to DeepSeek API."""
    log.info(f"[{client_id}] Voice query received ({len(payload.get('data', ''))} bytes)")


async def handle_agent_command(client_id: str, payload: dict):
    """Route agent commands to ClawCorp task system."""
    agent = payload.get("agent", "unknown")
    action = payload.get("action", "unknown")
    log.info(f"[{client_id}] Agent command: {agent}/{action}")


async def handle_approval_response(client_id: str, payload: dict):
    """Route approval response to the corresponding agent."""
    task_id = payload.get("task_id", "unknown")
    approved = payload.get("approved", False)
    log.info(f"[{client_id}] Approval response: task={task_id} approved={approved}")


ROUTING = {
    "voice:query": handle_voice_query,
    "agent:command": handle_agent_command,
    "agent:approval_response": handle_approval_response,
}


# ── WebSocket Handler ─────────────────────────────────────────────────
async def handler(ws: websockets.WebSocketServerProtocol):
    client_id = clients.register(ws)

    # Send welcome / config
    await clients.send_to(client_id, make_message("system:config", {
        "fw_version": "v0.1.0",
        "server_time": datetime.now(TZ).isoformat(),
    }))

    try:
        async for raw in ws:
            try:
                msg = json.loads(raw)
            except json.JSONDecodeError:
                log.warning(f"[{client_id}] Invalid JSON: {raw[:200]}")
                continue

            msg_type = msg.get("type", "")
            msg_id = msg.get("id", "?")
            payload = msg.get("payload", {})
            log.debug(f"[{client_id}] RX: {msg_type}/{msg_id}")

            # Handle ping/pong
            if msg_type == "ping":
                await clients.send_to(client_id, {"type": "pong"})
                continue

            # Route to handler
            handler_fn = ROUTING.get(msg_type)
            if handler_fn:
                await handler_fn(client_id, payload)
            else:
                log.warning(f"[{client_id}] Unknown type: {msg_type}")

    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        clients.unregister(client_id)


# ── Agent Integration ─────────────────────────────────────────────────
async def push_agent_status(client_id: str, status: dict):
    """Push agent status update to a device."""
    await clients.send_to(client_id, make_message("agent:status", status))


async def push_approval(client_id: str, approval: dict):
    """Push approval request to a device."""
    await clients.send_to(client_id, make_message("agent:approval", approval))


async def push_notification(client_id: str, notification: dict):
    """Push notification to a device."""
    await clients.send_to(client_id, make_message("notification:alert", notification))


# ── Main ──────────────────────────────────────────────────────────────
async def main():
    log.info(f"Starting claw-s3-server on {HOST}:{PORT}")
    async with websockets.serve(handler, HOST, PORT):
        await asyncio.Future()  # run forever


if __name__ == "__main__":
    asyncio.run(main())
