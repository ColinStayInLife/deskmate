#!/usr/bin/env python3
"""DeskMate architecture diagram — dark theme, clean style matching FishOn-S3."""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch
import os

# ── Colors (Tokyo Night inspired) ──────────────────────────────────────
BG        = '#1a1b26'
CARD      = '#24283b'
BLUE      = '#7aa2f7'
GREEN     = '#9ece6a'
RED       = '#f7768e'
YELLOW    = '#e0af68'
PURPLE    = '#bb9af7'
CYAN      = '#7dcfff'
ORANGE    = '#ff9e64'
WHITE     = '#c0caf5'
DIM       = '#565f89'

FONT      = 'WenQuanYi Micro Hei'

import matplotlib.font_manager as fm
plt.rcParams.update({
    'font.family': 'sans-serif',
    'font.sans-serif': [FONT, 'DejaVu Sans'],
    'axes.edgecolor': DIM,
    'axes.labelcolor': WHITE,
    'xtick.color': WHITE,
    'ytick.color': WHITE,
    'text.color': WHITE,
})

# ── Figure setup ──────────────────────────────────────────────────────
fig, ax = plt.subplots(1, 1, figsize=(14, 9))
fig.patch.set_facecolor(BG)
ax.set_facecolor(BG)
ax.set_xlim(0, 14)
ax.set_ylim(0, 9)
ax.axis('off')

def card(x, y, w, h, color, label, sub='', fg=WHITE, lw=1.5):
    rx = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.12,rounding_size=0.08",
                         facecolor=CARD, edgecolor=color, linewidth=lw, zorder=2)
    ax.add_patch(rx)
    ax.text(x + w/2, y + h - 0.2, label, ha='center', va='top',
            fontsize=11, fontweight='bold', color=fg, zorder=3)
    if sub:
        ax.text(x + w/2, y + 0.12, sub, ha='center', va='bottom',
                fontsize=7.5, color=DIM, zorder=3)
    return rx

def subcard(x, y, w, h, icon, label, color):
    rx = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.08,rounding_size=0.05",
                         facecolor=CARD, edgecolor=color, linewidth=1.0, zorder=2)
    ax.add_patch(rx)
    ax.text(x + w/2, y + h*0.62, icon, ha='center', va='center',
            fontsize=14, zorder=3)
    ax.text(x + w/2, y + h*0.22, label, ha='center', va='center',
            fontsize=8, color=WHITE, zorder=3)

def pt(x, y, text, size=8, color=DIM, ha='center', va='center', bold=False):
    ax.text(x, y, text, fontsize=size, color=color, ha=ha, va=va,
            fontweight='bold' if bold else 'normal', zorder=4)

def arrow(x1, y1, x2, y2, color=BLUE, lw=1.5, style='arc3,rad=0.'):
    ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                arrowprops=dict(arrowstyle='->', color=color, lw=lw,
                                connectionstyle=style), zorder=1)

# ── Title ──────────────────────────────────────────────────────────
pt(7, 8.7, 'DeskMate — Desktop AI Terminal Architecture', 16, WHITE, bold=True)
pt(7, 8.35, 'M5Stack CoreS3 SE  ·  DeepSeek API  ·  ClawCorp Agent', 9, DIM)

# ── Layer 1: Cloud ──────────────────────────────────────────────────
card(9.8, 7.0, 3.8, 1.3, PURPLE, 'Cloud Services',
     'DeepSeek API  ·  iFinD  ·  Feishu', PURPLE)
subcard(10.0, 7.15, 1.0, 1.0, '~D', 'DeepSeek', PURPLE)
subcard(11.2, 7.15, 1.0, 1.0, '~F', 'iFinD', PURPLE)
subcard(12.4, 7.15, 1.0, 1.0, '~L', 'Feishu', PURPLE)

# ── Layer 2: PC/Server ─────────────────────────────────────────────
card(9.8, 4.4, 3.8, 2.2, GREEN, 'ClawCorp Agent System (PC/Server)',
     'WebSocket Bridge  ·  API Proxy  ·  Orchestrator', GREEN)
subcard(10.0, 4.6, 1.6, 1.7, '[WS]', 'WS Bridge\nclaw-s3-server', GREEN)
subcard(11.8, 4.6, 1.6, 1.7, '[A]', 'Agent\nOrchestrator', GREEN)

# ── Layer 3: CoreS3 SE ─────────────────────────────────────────────
card(0.3, 0.8, 9.0, 5.2, BLUE, 'M5Stack CoreS3 SE',
     'ESP32-S3  |  1.9" IPS Touch  |  Mic  |  Speaker  |  SD Card', BLUE)

# ── CoreS3 components ──────────────────────────────────────────────
# Row 0: main screens
subcard(0.5, 1.2, 2.6, 1.9, '>>', 'Agent Console\nStatus / Tasks / History', CYAN)
subcard(3.3, 1.2, 2.6, 1.9, '~)', 'Voice Assistant\nSpeak / Listen / Reply', YELLOW)
subcard(6.1, 1.2, 2.6, 1.9, '[!]', 'Notification Center\nAlerts / Reminders / TTS', RED)

# Row 1: system
subcard(0.5, 3.3, 2.6, 1.9, '<->', 'WebSocket Client\nReal-time Agent Comms', BLUE)
subcard(3.3, 3.3, 2.6, 1.9, '(())', 'Audio Pipeline\nRecord / STT / TTS', GREEN)
subcard(6.1, 3.3, 2.6, 1.9, '[*]', 'System Services\nWiFi / SD / OTA / Config', GREEN)

# ── Connections ────────────────────────────────────────────────────
arrow(9.3, 5.8, 9.8, 5.8, GREEN)  # CoreS3 -> PC
arrow(9.3, 5.2, 9.8, 5.2, GREEN)  # CoreS3 -> PC
arrow(9.3, 4.0, 9.8, 4.0, BLUE)   # CoreS3 -> PC

arrow(10.8, 7.0, 10.8, 6.6, PURPLE)  # PC -> Cloud
arrow(12.8, 7.0, 12.8, 6.6, PURPLE)  # PC -> Cloud

# Protocol labels
for px, py, txt, clr in [
    (9.15, 6.3, 'WebSocket', GREEN),
    (9.15, 4.3, 'HTTP', BLUE),
]:
    ax.text(px, py, txt, ha='right', va='center', fontsize=7.5,
            color=clr, style='italic', zorder=4,
            bbox=dict(boxstyle='round,pad=0.15', facecolor=BG,
                      edgecolor=clr, linewidth=0.6))

# ── Phase legend ───────────────────────────────────────────────────
ly = 0.35
phases = [
    ('Phase 1: Base UI / WiFi / Status', BLUE),
    ('Phase 2: Voice Pipeline', GREEN),
    ('Phase 3: Agent Console + Approvals', YELLOW),
    ('Phase 4: Notifications + Quick Tools', RED),
    ('Phase 5: Integration + Deployment', PURPLE),
]
for i, (txt, clr) in enumerate(phases):
    x0 = 0.4 + i*2.6
    ax.plot([x0, x0+0.5], [ly, ly], color=clr, linewidth=3, zorder=3)
    pt(x0+0.65, ly, txt, 7, DIM, ha='left')

# ── Save ───────────────────────────────────────────────────────────
out = '/home/clawdbot/.openclaw/workspace_user1/projects/deskmate/docs/deskmate-architecture.png'
plt.tight_layout(pad=1.5)
plt.savefig(out, dpi=200, facecolor=BG, bbox_inches='tight')
print(f'Saved -> {out}  ({os.path.getsize(out)} bytes)')
