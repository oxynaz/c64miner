#ifndef C64MINER_TUI_H
#define C64MINER_TUI_H

#include "base/kernel/interfaces/ILogBackend.h"

#include <string>
#include <deque>
#include <mutex>
#include <atomic>
#include <chrono>

// ncurses defines macros (ERR, OK, etc.) that conflict with XMRig enums
// Only include ncurses in the .cpp file, not the header
// Forward declare what we need
typedef struct _win_st WINDOW;
typedef struct screen SCREEN;

namespace c64tui {

struct MinerStats {
    std::atomic<uint64_t> height{0};
    std::atomic<uint64_t> difficulty{0};
    std::atomic<uint64_t> accepted{0};
    std::atomic<uint64_t> rejected{0};
    std::atomic<uint64_t> threads{0};

    std::mutex mtx;
    std::string hashrate_10s = "n/a";
    std::string hashrate_60s = "n/a";
    std::string hashrate_15m = "n/a";
    std::string hashrate_max = "n/a";
    std::string cpu_name;
    std::string status = "INITIALIZING...";
    std::deque<std::pair<std::string, int>> log_lines;

    void add_log(const std::string& line, int color = 0);
    void set_status(const std::string& s);
};

MinerStats& get_stats();

void play_datasette_animation();
void run_tui(std::atomic<bool>& stop_signal);

enum LogColor {
    LOG_NORMAL = 0,
    LOG_SUCCESS = 1,
    LOG_ERROR = 2,
    LOG_WARNING = 3,
    LOG_INFO = 4
};

} // namespace c64tui

namespace xmrig {

class C64MinerTuiLog : public ILogBackend
{
public:
    C64MinerTuiLog() = default;
    ~C64MinerTuiLog() override = default;

    void print(uint64_t timestamp, int level, const char *line, size_t offset, size_t size, bool colors) override;
};

} // namespace xmrig

#endif // C64MINER_TUI_H
