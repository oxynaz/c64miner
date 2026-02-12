# C64 Miner

![C64 Miner](https://img.shields.io/badge/C64_Miner-v0.1-blue)

**C64 Miner** is the CPU mining software for [C64 Chain](https://github.com/oxynaz/c64chain), forked from [XMRig](https://github.com/xmrig/xmrig).

Features an integrated Commodore 64-themed ncurses TUI with real-time hashrate display, accepted/rejected blocks, and Datasette loading animation.

## Features

- 🖥️ **C64-themed ncurses TUI** — built into the binary
- ⚡ **Optimized CPU mining** — RandomX (RandomWOW variant)
- 📊 **Real-time stats** — hashrate, accepted, rejected, difficulty
- 🎮 **Datasette animation** on startup
- 🚫 **No dev fee** — 0% donation to XMRig developers (removed from fork)

## ⚠️ Current Status: TESTNET

> **C64 Chain is currently in TESTNET phase.** The mainnet has not launched yet.
> All coins mined on testnet have no value.

## Prerequisites

Before mining, you need:
1. **A running C64 Chain node** — see [c64chain](https://github.com/oxynaz/c64chain) to install and run the node
2. **A C64 wallet address** — create one with `c64wallet` (see the c64chain README, step 4)

The miner connects to your local node via RPC. **The node must be running before you start the miner.**

## Quick Start

### Option A: Pre-compiled binary (Ubuntu 24.04 x86_64)

```bash
# Download from GitHub Releases
wget https://github.com/oxynaz/c64miner/releases/download/v0.1/c64miner-v0.1-ubuntu24-x86_64.tar.gz
tar xzf c64miner-v0.1-ubuntu24-x86_64.tar.gz
mkdir -p ~/c64miner
mv c64miner ~/c64miner/
```

### Option B: Build from source

```bash
sudo apt update
sudo apt install -y build-essential cmake libuv1-dev libssl-dev \
    libncurses5-dev libncursesw5-dev libhwloc-dev

git clone https://github.com/oxynaz/c64miner.git
cd c64miner
mkdir build && cd build
cmake .. -DWITH_OPENCL=OFF -DWITH_CUDA=OFF
make -j$(nproc)
```

## Configuration

### Create your config.json

```bash
cd ~/c64miner
cp config.example.json config.json
```

Edit `config.json` and replace `YOUR_C64_WALLET_ADDRESS_HERE` with your wallet address:

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
            "algo": "rx/wow",
            "coin": "wownero",
            "daemon": true,
            "daemon-poll-interval": 1000
        }
    ],
    "print-time": 5
}
```

> ⚠️ **Important notes about the config:**
> - `"daemon": true` is **required**. Without it the miner tries stratum mode and fails.
> - `"coin": "wownero"` is **required**. This tells XMRig to use the RandomWOW algorithm. C64 Chain uses the same mining algorithm as Wownero.
> - `"url"` must point to your running node's RPC port (`127.0.0.1:29641` for testnet).
> - The `config.json` file must be in `~/c64miner/` (not inside `~/c64miner/build/`).

### Config parameters reference

| Parameter | Value | Description |
|-----------|-------|-------------|
| `url` | `127.0.0.1:29641` | Node RPC address (testnet) |
| `user` | `9...` | Your C64 Chain wallet address |
| `algo` | `rx/wow` | Mining algorithm (RandomWOW) |
| `coin` | `wownero` | **Required** — identifies RandomWOW algorithm |
| `daemon` | `true` | **Required** — enables daemon RPC mode (not stratum) |
| `daemon-poll-interval` | `1000` | How often to poll for new blocks (ms) |
| `-t N` | number | Number of CPU threads (leave 1-2 for system) |

## Run

> ⚠️ **Make sure your C64 Chain node is running first!**

```bash
cd ~/c64miner
sudo ./build/c64miner -c config.json -t 2
```

If you downloaded the pre-compiled binary:
```bash
cd ~/c64miner
sudo ./c64miner -c config.json -t 2
```

Replace `-t 2` with the number of threads you want to use. Leave 1-2 threads free for the system.

> **Always run with `sudo`** for best performance. This enables huge pages and MSR register optimizations, which can improve hashrate by 30-50%.

### Run in background with screen

```bash
cd ~/c64miner && screen -dmS miner sudo ./build/c64miner -c config.json -t 2
```

View the miner TUI: `screen -r miner` (detach with Ctrl+A then D)

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| Miner exits immediately | Node not running | Start the node first |
| `connect error` | Wrong RPC port | Check `url` in config.json (testnet: 29641) |
| `login failed` | Missing `daemon: true` | Add `"daemon": true` in your config.json |
| `algorithm not found` | Missing `coin` field | Add `"coin": "wownero"` in your config.json |
| Low hashrate | Not running as root | Run with `sudo` for huge pages |
| `config not found` | Wrong directory | config.json must be in ~/c64miner/, not in build/ |

## Community

- Block Explorer: [c64chain.com](https://c64chain.com)
- Discord: [discord.gg/MTRgHT8r45](https://discord.gg/MTRgHT8r45)
- Node & Wallet: [github.com/oxynaz/c64chain](https://github.com/oxynaz/c64chain)

## Credits & License

C64 Miner is a fork of [XMRig](https://github.com/xmrig/xmrig).

Licensed under the **GNU General Public License v3.0** — see [LICENSE](LICENSE).

Modifications from XMRig:
- Integrated ncurses TUI with C64 aesthetics
- XMRig dev fee removed (0%)
- Rebranded to C64 Miner

All original XMRig copyrights remain intact.
