# C64 Miner

![C64 Miner](https://img.shields.io/badge/C64_Miner-v0.2.1-blue)

**C64 Miner** is the CPU mining software for [C64 Chain](https://github.com/oxynaz/c64chain), forked from [XMRig](https://github.com/xmrig/xmrig).

Features an integrated Commodore 64-themed ncurses TUI with real-time hashrate display, accepted/rejected blocks, and Datasette loading animation.

## Features

- 🖥️ **C64-themed ncurses TUI** — built into the binary
- ⚡ **Optimized CPU mining** — RandomX (rx/c64 variant)
- 📊 **Real-time stats** — hashrate, accepted, rejected, difficulty
- 🎮 **Datasette animation** on startup
- 🚫 **No dev fee** — 0% donation to XMRig developers (removed from fork)

## ⚠️ Current Status: TESTNET

> **C64 Chain is currently in TESTNET phase.** The mainnet has not launched yet.
> All coins mined on testnet have no value.

## Prerequisites

Before mining, you need:
1. **A running C64 Chain node** — see [c64chain](https://github.com/oxynaz/c64chain) to install and run the node
2. **A C64 wallet address** — create one with `c64wallet` (see the c64chain README)

The miner connects to your local node via RPC. **The node must be running before you start the miner.**

## Quick Start

### Option A: Pre-compiled binary (Ubuntu 24.04 x86_64)

Download `c64miner` from [Releases](https://github.com/oxynaz/c64miner/releases/tag/v0.2.1), then:
```bash
wget https://github.com/oxynaz/c64miner/releases/download/v0.2.1/c64miner-v0.2.1-ubuntu24-x86_64.tar.gz
tar xzf c64miner-v0.2.1-ubuntu24-x86_64.tar.gz
mkdir -p ~/c64miner
mv c64miner ~/c64miner/
```

### Option B: Build from source
```bash
sudo apt update
sudo apt install -y build-essential cmake libuv1-dev libssl-dev \
    libncurses5-dev libncursesw5-dev libhwloc-dev git

git clone https://github.com/oxynaz/c64miner.git
cd c64miner
mkdir build && cd build
cmake .. -DWITH_OPENCL=OFF -DWITH_CUDA=OFF -DWITH_HWLOC=OFF
make -j$(nproc)
```

### Updating to a new version
```bash
cd ~/c64miner
git pull --tags
rm -rf build
mkdir build && cd build
cmake .. -DWITH_OPENCL=OFF -DWITH_CUDA=OFF -DWITH_HWLOC=OFF
make -j$(nproc)
```

## Configuration

Create a `config.json` in `~/c64miner/`:
```json
{
    "autosave": false,
    "donate-level": 0,
    "cpu": {
        "enabled": true
    },
    "opencl": false,
    "cuda": false,
    "pools": [
        {
            "url": "127.0.0.1:29641",
            "user": "YOUR_C64_WALLET_ADDRESS_HERE",
            "algo": "rx/c64",
            "coin": "c64chain",
            "daemon": true,
            "daemon-poll-interval": 1000
        }
    ],
    "print-time": 5
}
```

> ⚠️ **Important notes about the config:**
> - `"daemon": true` is **required**. Without it the miner tries stratum mode and fails.
> - `"coin": "c64chain"` is **required**. This tells the miner to use the rx/c64 algorithm.
> - `"url"` must point to your running node's RPC port (`127.0.0.1:29641` for testnet).

### LAN Mining

If another machine on your local network is running the node, point the miner at its local IP instead:
```json
{
    "url": "192.168.X.X:29641",
    "daemon": true
}
```

The node machine must be started with `--rpc-bind-ip=0.0.0.0 --confirm-external-bind`.
No node needed on the miner-only machines.

### Config parameters reference

| Parameter | Value | Description |
|-----------|-------|-------------|
| `url` | `127.0.0.1:29641` | Node RPC address (testnet) |
| `user` | `9...` | Your C64 Chain wallet address |
| `algo` | `rx/c64` | Mining algorithm |
| `coin` | `c64chain` | **Required** — identifies the coin and algorithm |
| `daemon` | `true` | **Required** — enables daemon RPC mode (not stratum) |
| `daemon-poll-interval` | `1000` | How often to poll for new blocks (ms) |
| `-t N` | number | Number of CPU threads (leave 1-2 for system) |

## Run

> ⚠️ **Make sure your C64 Chain node is running first!**

If you built from source:
```bash
cd ~/c64miner
sudo ./build/c64miner -c config.json -t $(nproc)
```

If you downloaded the pre-compiled binary:
```bash
cd ~/c64miner
sudo ./c64miner -c config.json -t $(nproc)
```

> **Always run with `sudo`** for best performance. This enables huge pages and MSR register optimizations, which can improve hashrate by 30-50%.

### Run in background with screen
```bash
screen -dmS miner bash -c "cd ~/c64miner && sudo ./build/c64miner -c config.json -t \$(nproc)"
```

View the miner TUI: `screen -r miner` (detach with Ctrl+A then D)

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| Miner exits immediately | Node not running | Start the node first |
| `connect error` | Wrong RPC port | Check `url` in config.json (testnet: 29641) |
| `login failed` | Missing `daemon: true` | Add `"daemon": true` in your config.json |
| `algorithm not found` | Missing `coin` field | Add `"coin": "c64chain"` in your config.json |
| Low hashrate | Not running as root | Run with `sudo` for huge pages |
| `config not found` | Wrong directory | config.json must be in ~/c64miner/, not in build/ |

> **Note:** The old values `"coin": "wownero"` and `"algo": "rx/wow"` are still supported for backward compatibility, but `"coin": "c64chain"` and `"algo": "rx/c64"` are recommended.

## Community

- Block Explorer: [c64chain.com](https://c64chain.com)
- Discord: [discord.gg/MTRgHT8r45](https://discord.gg/MTRgHT8r45)
- Node & Wallet: [github.com/oxynaz/c64chain](https://github.com/oxynaz/c64chain)

## Credits & License

C64 Miner is a fork of [XMRig](https://github.com/xmrig/xmrig).

Licensed under the **GNU General Public License v3.0** — see [LICENSE](LICENSE).

Modifications from XMRig:
- Integrated ncurses TUI with C64 aesthetics
- Added rx/c64 algorithm alias and c64chain coin support
- XMRig dev fee removed (0%)
- Fixed duplicate log lines in TUI
- Rebranded to C64 Miner

All original XMRig copyrights remain intact.
