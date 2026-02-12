#include "C64MinerTui.h"
#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <algorithm>

namespace c64tui {

static MinerStats g_stats;
MinerStats& get_stats() { return g_stats; }

void MinerStats::add_log(const std::string& line, int color) {
    std::lock_guard<std::mutex> lock(mtx);
    log_lines.push_back({line, color});
    if (log_lines.size() > 500)
        log_lines.pop_front();
}

// ---------- Datasette Animation ----------

void play_datasette_animation() {
    const char* colors[] = {
        "\033[41m", "\033[42m", "\033[43m", "\033[44m",
        "\033[45m", "\033[46m", "\033[47m", "\033[44m",
        "\033[41m", "\033[43m", "\033[42m", "\033[45m",
        "\033[46m", "\033[41m", "\033[44m", "\033[47m"
    };
    const char* reset = "\033[0m";

    struct winsize ws;
    int tw = 60;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
        tw = ws.ws_col - 4;

    fprintf(stderr, "\033[2J\033[H");
    fprintf(stderr, "\n\033[36m    PRESS PLAY ON TAPE\033[0m\n\n");
    fflush(stderr);
    usleep(1500000);
    fprintf(stderr, "\033[36m    OK\033[0m\n\n");
    fflush(stderr);
    usleep(500000);

    srand(time(NULL));
    for (int f = 0; f < 80; f++) {
        std::string ln = "    ";
        for (int x = 0; x < tw; x++) {
            int ci = (x + f * 3 + (rand() % 3)) % 16;
            ln += colors[ci];
            ln += " ";
        }
        ln += reset;
        fprintf(stderr, "%s\n", ln.c_str());
        fflush(stderr);
        usleep(50000);
    }

    fprintf(stderr, "\n");
    fflush(stderr);
    usleep(300000);
    fprintf(stderr, "\033[36m    FOUND C64MINER\033[0m\n");
    fflush(stderr);
    usleep(800000);
    fprintf(stderr, "\033[36m    LOADING...\033[0m\n");
    fflush(stderr);
    usleep(1000000);
    fprintf(stderr, "\033[36m    READY.\033[0m\n\n");
    fflush(stderr);
    usleep(300000);
}

// ---------- ncurses TUI ----------

enum {
    CP_NORMAL = 1, CP_HEADER = 2, CP_SUCCESS = 3,
    CP_ERROR = 4, CP_WARNING = 5, CP_LOG_GREEN = 6, CP_LOG_BLUE = 7
};

static void init_colors() {
    start_color();
    if (can_change_color()) {
        init_color(20, 260, 220, 560);
        init_color(21, 420, 370, 710);
        init_color(22, 1000, 1000, 1000);
        init_color(23, 700, 1000, 700);
        init_color(24, 1000, 400, 400);
        init_color(25, 1000, 1000, 700);
        init_color(26, 400, 900, 400);
        init_color(27, 400, 600, 1000);
        init_pair(CP_NORMAL, 22, 20);
        init_pair(CP_HEADER, 21, 20);
        init_pair(CP_SUCCESS, 23, 20);
        init_pair(CP_ERROR, 24, 20);
        init_pair(CP_WARNING, 25, 20);
        init_pair(CP_LOG_GREEN, 26, 20);
        init_pair(CP_LOG_BLUE, 27, 20);
    } else {
        init_pair(CP_NORMAL, COLOR_WHITE, COLOR_BLUE);
        init_pair(CP_HEADER, COLOR_CYAN, COLOR_BLUE);
        init_pair(CP_SUCCESS, COLOR_GREEN, COLOR_BLUE);
        init_pair(CP_ERROR, COLOR_RED, COLOR_BLUE);
        init_pair(CP_WARNING, COLOR_YELLOW, COLOR_BLUE);
        init_pair(CP_LOG_GREEN, COLOR_GREEN, COLOR_BLUE);
        init_pair(CP_LOG_BLUE, COLOR_CYAN, COLOR_BLUE);
    }
}

void run_tui(std::atomic<bool>& stop_signal) {
    auto start_time = std::chrono::steady_clock::now();

    FILE* tty = fopen("/dev/tty", "r+");
    SCREEN* scr = newterm(NULL, tty, tty);
    set_term(scr);
    clear();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    keypad(stdscr, TRUE);

    init_colors();
    bkgd(COLOR_PAIR(CP_NORMAL));

    MinerStats& stats = get_stats();

    while (!stop_signal.load()) {
        int h, w;
        getmaxyx(stdscr, h, w);
        erase();

        // Border
        attron(COLOR_PAIR(CP_NORMAL));
        mvaddstr(0, 0, "+");
        for (int x = 1; x < w - 1; x++) mvaddch(0, x, '-');
        mvaddch(0, w - 1, '+');
        for (int row = 1; row < h - 1; row++) {
            mvaddch(row, 0, '|');
            mvaddch(row, w - 1, '|');
        }
        mvaddch(h - 1, 0, '+');
        for (int x = 1; x < w - 1; x++) mvaddch(h - 1, x, '-');
        try { mvaddch(h - 1, w - 1, '+'); } catch (...) {}
        attroff(COLOR_PAIR(CP_NORMAL));

        int y = 2;

        // Header
        {
            const char* title = "**** C64 MINER V0.1 ****";
            int tx = (w - (int)strlen(title)) / 2;
            attron(COLOR_PAIR(CP_HEADER) | A_BOLD);
            mvaddstr(y++, tx, title);
            attroff(A_BOLD);
            const char* sub = "COMMODORE 64 CHAIN - CPU MINING SYSTEM";
            tx = (w - (int)strlen(sub)) / 2;
            attron(COLOR_PAIR(CP_NORMAL));
            mvaddstr(y++, tx, sub);
            attroff(COLOR_PAIR(CP_NORMAL));
        }
        y += 2;

        attron(COLOR_PAIR(CP_HEADER) | A_BOLD);
        mvaddstr(y++, 3, "READY.");
        attroff(A_BOLD);
        attron(COLOR_PAIR(CP_NORMAL));
        mvaddstr(y++, 3, "RUN C64MINER");
        attroff(COLOR_PAIR(CP_NORMAL));
        y++;

        // Big hashrate
        {
            std::lock_guard<std::mutex> lock(stats.mtx);
            char buf[128];
            snprintf(buf, sizeof(buf), ">>> HASHRATE: %s H/s <<<", stats.hashrate_10s.c_str());
            attron(COLOR_PAIR(CP_SUCCESS) | A_BOLD);
            mvaddstr(y, 3, buf);
            attroff(COLOR_PAIR(CP_SUCCESS) | A_BOLD);
        }
        y += 2;

        // Stats
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        int uptime = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
        int hrs = uptime / 3600, mins = (uptime % 3600) / 60, secs = uptime % 60;

        char buf[256], tmp[128];
        auto print_stat = [&](int idx, const char* key, const char* val, int cp) {
            snprintf(buf, sizeof(buf), "%2d  %-14s %s", idx, key, val);
            attron(COLOR_PAIR(cp));
            mvaddnstr(y++, 3, buf, w - 6);
            attroff(COLOR_PAIR(cp));
        };

        std::string status_str;
        {
            std::lock_guard<std::mutex> lock(stats.mtx);
            status_str = stats.status;
        }
        print_stat(0, "STATUS:", status_str.c_str(), status_str == "MINING" ? CP_SUCCESS : CP_NORMAL);

        snprintf(tmp, sizeof(tmp), "%lu", stats.height.load());
        print_stat(1, "HEIGHT:", tmp, CP_NORMAL);

        snprintf(tmp, sizeof(tmp), "%lu", stats.difficulty.load());
        print_stat(2, "DIFFICULTY:", tmp, CP_NORMAL);

        snprintf(tmp, sizeof(tmp), "%lu", stats.accepted.load());
        print_stat(3, "ACCEPTED:", tmp, stats.accepted.load() > 0 ? CP_SUCCESS : CP_NORMAL);

        snprintf(tmp, sizeof(tmp), "%lu", stats.rejected.load());
        print_stat(4, "REJECTED:", tmp, stats.rejected.load() > 0 ? CP_ERROR : CP_NORMAL);

        {
            std::lock_guard<std::mutex> lock(stats.mtx);
            snprintf(tmp, sizeof(tmp), "%s", stats.cpu_name.empty() ? "DETECTING..." : stats.cpu_name.c_str());
        }
        print_stat(5, "CPU:", tmp, CP_NORMAL);

        snprintf(tmp, sizeof(tmp), "%lu", stats.threads.load());
        print_stat(6, "THREADS:", tmp, CP_NORMAL);

        print_stat(7, "ALGO:", "RANDOMWOW (CPU-ONLY)", CP_NORMAL);

        {
            std::lock_guard<std::mutex> lock(stats.mtx);
            snprintf(tmp, sizeof(tmp), "%s / %s / %s",
                stats.hashrate_10s.c_str(), stats.hashrate_60s.c_str(), stats.hashrate_15m.c_str());
        }
        print_stat(8, "10s/60s/15m:", tmp, CP_NORMAL);

        {
            std::lock_guard<std::mutex> lock(stats.mtx);
            snprintf(tmp, sizeof(tmp), "%s H/s", stats.hashrate_max.c_str());
        }
        print_stat(9, "MAX HASH:", tmp, CP_NORMAL);

        snprintf(tmp, sizeof(tmp), "%02d:%02d:%02d", hrs, mins, secs);
        print_stat(10, "UPTIME:", tmp, CP_NORMAL);
        y++;

        // Mining animation
        if (status_str == "MINING") {
            const char anim[] = "|/-\\";
            char ch = anim[(int)(time(NULL) * 4) % 4];
            snprintf(buf, sizeof(buf), "  [%c] MINING BLOCK %lu...", ch, stats.height.load());
            attron(COLOR_PAIR(CP_SUCCESS) | A_BOLD);
            mvaddstr(y, 3, buf);
            attroff(COLOR_PAIR(CP_SUCCESS) | A_BOLD);
        }
        y += 2;

        // Log section
        attron(COLOR_PAIR(CP_HEADER) | A_BOLD);
        mvaddstr(y++, 3, "LOG:");
        attroff(COLOR_PAIR(CP_HEADER) | A_BOLD);

        int available = h - y - 3;
        if (available > 0) {
            std::lock_guard<std::mutex> lock(stats.mtx);
            int start = (int)stats.log_lines.size() - available;
            if (start < 0) start = 0;
            for (int i = start; i < (int)stats.log_lines.size() && y < h - 3; i++) {
                auto& entry = stats.log_lines[i];
                int cp = CP_NORMAL;
                switch (entry.second) {
                    case LOG_SUCCESS: cp = CP_LOG_GREEN; break;
                    case LOG_ERROR:   cp = CP_ERROR;     break;
                    case LOG_WARNING: cp = CP_WARNING;   break;
                    case LOG_INFO:    cp = CP_LOG_BLUE;  break;
                }
                attron(COLOR_PAIR(cp));
                mvaddnstr(y++, 3, entry.first.c_str(), w - 6);
                attroff(COLOR_PAIR(cp));
            }
        }

        // Blinking cursor
        {
            const char* cursor = (time(NULL) % 2) ? "READY. _" : "READY.  ";
            attron(COLOR_PAIR(CP_HEADER) | A_BOLD);
            mvaddstr(h - 3, 3, cursor);
            attroff(COLOR_PAIR(CP_HEADER) | A_BOLD);
        }

        // Footer
        {
            const char* footer = "CTRL+C TO QUIT  -  C64 MINER (C) 2026";
            int fx = (w - (int)strlen(footer)) / 2;
            attron(COLOR_PAIR(CP_HEADER) | A_BOLD);
            mvaddstr(h - 2, fx, footer);
            attroff(COLOR_PAIR(CP_HEADER) | A_BOLD);
        }

        refresh();

        for (int i = 0; i < 5 && !stop_signal.load(); i++) {
            int ch = getch();
            if (ch == 'q' || ch == 'Q' || ch == 27) {
                stop_signal.store(true);
                break;
            }
            usleep(50000);
        }
    }

    endwin();
    if (scr) delscreen(scr);
    if (tty) fclose(tty);
}

} // namespace c64tui

// ---------- XMRig Log Backend ----------

void xmrig::C64MinerTuiLog::print(uint64_t, int level, const char *line, size_t offset, size_t size, bool colors) {
    if (!line || size == 0) return;
    if (colors) return;  // Skip colored duplicate — we only process the plain text call

    // Get the clean text (without color codes) starting at offset
    std::string msg(line + offset, size - offset);

    // Remove ANSI color codes
    std::string clean;
    clean.reserve(msg.size());
    bool in_esc = false;
    for (char c : msg) {
        if (c == '\033') { in_esc = true; continue; }
        if (in_esc) { if (c == 'm') in_esc = false; continue; }
        if (c >= 0x01 && c <= 0x1f && c != '\n') continue;
        clean += c;
    }

    c64tui::MinerStats& stats = c64tui::get_stats();

    // --- Parse stats ---

    // Hashrate: "10s/60s/15m 1178.4 n/a n/a H/s max 1197.8 H/s"
    {
        auto p = clean.find(" H/s max ");
        if (p != std::string::npos) {
            // Find hashrate values before " H/s max"
            auto speed_pos = clean.find("10s/60s/15m");
            if (speed_pos != std::string::npos) {
                auto vals_start = speed_pos + 12;
                auto vals = clean.substr(vals_start, p - vals_start);
                // Parse "1178.4 n/a n/a"
                char h10[32] = "n/a", h60[32] = "n/a", h15[32] = "n/a";
                sscanf(vals.c_str(), "%31s %31s %31s", h10, h60, h15);
                // Parse max
                char hmax[32] = "n/a";
                sscanf(clean.c_str() + p + 9, "%31s", hmax);
                std::lock_guard<std::mutex> lock(stats.mtx);
                stats.hashrate_10s = h10;
                stats.hashrate_60s = h60;
                stats.hashrate_15m = h15;
                stats.hashrate_max = hmax;
            }
        }
    }

    // Accepted: "accepted (123/0)"
    {
        auto p = clean.find("accepted (");
        if (p != std::string::npos) {
            unsigned a = 0, r = 0;
            if (sscanf(clean.c_str() + p + 10, "%u/%u", &a, &r) == 2) {
                stats.accepted.store(a);
                stats.rejected.store(r);
                stats.set_status("MINING");
            }
        }
    }
    // Rejected
    if (clean.find("rejected") != std::string::npos) {
        auto p = clean.find("(");
        if (p != std::string::npos) {
            unsigned a = 0, r = 0;
            if (sscanf(clean.c_str() + p + 1, "%u/%u", &a, &r) == 2) {
                stats.accepted.store(a);
                stats.rejected.store(r);
            }
        }
    }

    // New job: "diff XXXX... height YYYY"
    {
        auto dp = clean.find("diff ");
        auto hp = clean.find("height ");
        if (dp != std::string::npos && hp != std::string::npos) {
            try {
                stats.difficulty.store(std::stoull(clean.substr(dp + 5)));
                stats.height.store(std::stoull(clean.substr(hp + 7)));
                stats.set_status("MINING");
            } catch (...) {}
        }
    }

    // CPU name
    if (clean.find("CPU") != std::string::npos) {
        auto p = clean.find("CPU");
        // "CPU  AMD Athlon 3000G..."
        if (p != std::string::npos && clean.size() > p + 5) {
            auto name_start = clean.find_first_not_of(" ", p + 3);
            if (name_start != std::string::npos) {
                auto paren = clean.find("(", name_start);
                if (paren != std::string::npos) {
                    std::lock_guard<std::mutex> lock(stats.mtx);
                    stats.cpu_name = clean.substr(name_start, paren - name_start);
                }
            }
        }
    }

    // Threads: "READY threads X/Y"
    {
        auto p = clean.find("threads ");
        if (p != std::string::npos && clean.find("READY") != std::string::npos) {
            unsigned t = 0;
            if (sscanf(clean.c_str() + p + 8, "%u", &t) == 1)
                stats.threads.store(t);
        }
    }

    // Dataset ready
    if (clean.find("dataset ready") != std::string::npos) {
        std::lock_guard<std::mutex> lock(stats.mtx);
        stats.status = "DATASET READY";
    }
    if (clean.find("init dataset") != std::string::npos) {
        std::lock_guard<std::mutex> lock(stats.mtx);
        stats.status = "INIT DATASET...";
    }

    // MinerStats::set_status helper
    auto set_status_fn = [&](const std::string& s) {
        std::lock_guard<std::mutex> lock(stats.mtx);
        stats.status = s;
    };

    // --- Filter + display ---

    // Skip noisy lines
    if (clean.find("ABOUT") != std::string::npos) return;
    if (clean.find("LIBS") != std::string::npos) return;
    if (clean.find("HUGE PAGES") != std::string::npos) return;
    if (clean.find("1GB PAGES") != std::string::npos) return;
    if (clean.find("DONATE") != std::string::npos) return;
    if (clean.find("POOL #") != std::string::npos) return;
    if (clean.find("COMMANDS") != std::string::npos) return;
    if (clean.find("OPENCL") != std::string::npos) return;
    if (clean.find("CUDA") != std::string::npos) return;
    if (clean.find("ASSEMBLY") != std::string::npos) return;
    if (clean.find("MSR") != std::string::npos) return;
    if (clean.find("MEMORY") != std::string::npos) return;
    if (clean.find("argon2") != std::string::npos) return;

    // Rebrand
    std::string display = clean;
    {
        size_t pos;
        while ((pos = display.find("XMRig")) != std::string::npos)
            display.replace(pos, 5, "C64Miner");
        while ((pos = display.find("xmrig")) != std::string::npos)
            display.replace(pos, 5, "c64miner");
    }

    // Simplify labels
    {
        size_t pos;
        while ((pos = display.find("  net      ")) != std::string::npos)
            display.replace(pos, 11, " NET  ");
        while ((pos = display.find("  cpu      ")) != std::string::npos)
            display.replace(pos, 11, " CPU  ");
        while ((pos = display.find("  miner    ")) != std::string::npos)
            display.replace(pos, 11, " SYS  ");
        while ((pos = display.find("  randomx  ")) != std::string::npos)
            display.replace(pos, 11, " RNG  ");
    }

    // Determine color
    int color = c64tui::LOG_NORMAL;
    if (clean.find("accepted") != std::string::npos)
        color = c64tui::LOG_SUCCESS;
    else if (clean.find("rejected") != std::string::npos || level <= 3)
        color = c64tui::LOG_ERROR;
    else if (clean.find("new job") != std::string::npos)
        color = c64tui::LOG_INFO;
    else if (clean.find("speed") != std::string::npos)
        color = c64tui::LOG_WARNING;
    else if (clean.find("READY") != std::string::npos || clean.find("dataset") != std::string::npos)
        color = c64tui::LOG_SUCCESS;

    stats.add_log(display, color);
}

void c64tui::MinerStats::set_status(const std::string& s) {
    std::lock_guard<std::mutex> lock(mtx);
    status = s;
}
