# 🤖 DeskMate — 桌面 AI Agent 终端

> 基于 M5Stack CoreS3 SE + DeepSeek API + ClawCorp Agent 的物理 AI 助手

**放在桌上的 AI Agent 物理控制台。语音问它、触屏操作、实时看 Agent 状态、不用打开电脑。**

---

## ✨ 功能

| 功能 | 说明 |
|:----|:-----|
| **Agent 控制台** | 实时查看 Agent 任务状态、进度条、历史 |
| **触屏审批** | 直接在屏幕上批准/驳回 Agent 操作 |
| **语音助手** | 对 CoreS3 说话，DeepSeek 理解并执行 |
| **通知中心** | 任务完成、飞书消息、股价异动 → 屏幕弹出 + 语音播报 |
| **快捷工具** | 语音速记、快速搜索、倒计时、桌面时钟 |

## 🏗️ 系统架构

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│              │     │              │     │              │
│  CoreS3 SE   │◄───►│ ClawCorp     │◄───►│ DeepSeek     │
│  桌面终端    │  WS │ Agent 系统   │ HTTP│ iFinD Feishu │
│              │     │ (PC/Server)  │     │              │
└──────────────┘     └──────────────┘     └──────────────┘
```

详见 [docs/deskmate-architecture.png](docs/deskmate-architecture.png)

## 📦 硬件需求

- **M5Stack CoreS3 SE** — 1.9" IPS 触控、麦克风、喇叭、WiFi、SD 卡
- **USB-C 电源** — 5V/2A 长期供电
- **WiFi 网络** — 2.4GHz，连接 ClawCorp + DeepSeek

**无需额外硬件**，CoreS3 SE 自带的就是全部。

## 🚀 快速开始

### 1. 烧录固件

```bash
cd firmware/cores3-se
cp src/secrets.h.example src/secrets.h
# 编辑 secrets.h 填入 WiFi 密码、DeepSeek API Key、Agent WS URL
pio run -t upload
```

### 2. 启动服务端

```bash
cd server
pip install -r requirements.txt
cp .env.example .env
# 编辑 .env 填入 API Key 等配置
python claw-s3-server.py
```

### 3. 使用

CoreS3 SE 开机自动：
1. 连接 WiFi
2. 连接 ClawCorp WebSocket
3. 显示 Agent 控制台
4. 等待语音指令或 Agent 推送

## 📁 项目结构

```
deskmate/
├── README.md
├── CODE_REVIEW.md          # 代码审查 + 改进建议
├── docs/
│   ├── deskmate-architecture.png
│   └── generate_architecture.py
├── firmware/cores3-se/     # CoreS3 SE 固件 (PlatformIO + Arduino)
│   ├── platformio.ini
│   └── src/
│       ├── main.cpp
│       ├── config.h
│       ├── secrets.h.example
│       ├── wifi_manager.h
│       ├── websocket_client.h
│       ├── ui_manager.h
│       ├── audio_manager.h
│       ├── message_handler.h
│       └── system_manager.h
└── server/                  # ClawCorp 端代理服务
    ├── requirements.txt
    ├── .env.example
    └── claw-s3-server.py
```

## 📋 开发路线

| 阶段 | 内容 | 时间 |
|:----|:-----|:----|
| **Phase 1** | CoreS3 SE 基础 UI + 连接 ClawCorp | 3天 |
| **Phase 2** | 语音采集 + DeepSeek 语音理解 | 3天 |
| **Phase 3** | Agent 状态推送 + 触屏审批 | 3天 |
| **Phase 4** | 通知中心 + 快捷工具 | 2天 |
| **Phase 5** | 集成测试 + 桌面部署 | 2天 |

## 🔧 技术栈

| 组件 | 技术 |
|:----|:-----|
| MCU | ESP32-S3 @ 240MHz |
| UI 框架 | LVGL 8.3+ |
| 通信 | WebSocket ↔ ClawCorp |
| 语音 | DeepSeek API (语音理解 + TTS) |
| 音频 | ES7210 (Mic) + MAX98357 (Speaker) |

## 📄 相关项目

- [FishOn-S3](https://github.com/ColinStayInLife/FishOn-S3) — 智能鱼口监控系统
- [ClawCorp Agent](https://github.com/ColinStayInLife/Event-Driven-Investing) — 事件驱动投资 Agent

---

<p align="center">⚙️ 由 ClawCorp 驱动</p>
