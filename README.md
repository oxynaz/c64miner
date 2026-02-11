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

## Quick Start

### Pre-compiled binary (Ubuntu 24.04 x86_64)

Download `c64miner` from [Releases](../../releases).

### Configure

Copy `config.example.json` to `config.json` and set your wallet address:
```bash
cp config.example.json config.json
# Edit config.json and replace YOUR_C64_WALLET_ADDRESS_HERE with your address
```

### Run
```bash
./c64miner -c config.json -t $(nproc)
```

`-t` sets the number of CPU threads to use.

## Build from source

### Dependencies (Ubuntu 24.04)
```bash
sudo apt update
sudo apt install -y build-essential cmake libuv1-dev libssl-dev \
    libncurses5-dev libncursesw5-dev libhwloc-dev
```

### Compile
```bash
git clone https://github.com/oxynaz/c64miner.git
cd c64miner
mkdir build && cd build
cmake .. -DWITH_OPENCL=OFF -DWITH_CUDA=OFF
make -j$(nproc)
```

## Configuration

| Parameter | Description |
|-----------|-------------|
| `url` | Node RPC address (default: `127.0.0.1:29641` for testnet) |
| `user` | Your C64 Chain wallet address |
| `algo` | `rx/wow` |
| `-t N` | Number of mining threads |

## Credits & License

C64 Miner is a fork of [XMRig](https://github.com/xmrig/xmrig).

Licensed under the **GNU General Public License v3.0** — see [LICENSE](LICENSE).

Modifications from XMRig:
- Integrated ncurses TUI with C64 aesthetics
- XMRig dev fee removed (0%)
- Rebranded to C64 Miner

All original XMRig copyrights remain intact.
