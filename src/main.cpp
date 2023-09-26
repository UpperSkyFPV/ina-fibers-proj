#include <fmt/format.h>
#include <tsl/robin_set.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <utility>

#include "fmt/core.h"

namespace uppr {

class Scheduler;

using std::chrono::steady_clock;
using duration = steady_clock::duration;
using time_point = steady_clock::time_point;

class Timer {
public:
    using callback = std::function<void(Scheduler* s)>;

    Timer(callback cb, duration timeout, time_point last_run)
        : cb{std::move(cb)}, timeout{timeout}, last_run{last_run} {}

    Timer(callback cb, duration timeout)
        : cb{std::move(cb)}, timeout{timeout}, last_run{steady_clock::now()} {}

    [[nodiscard]] constexpr auto should_run(time_point now) const -> bool {
        return since_last_run(now) > timeout;
    }

    [[nodiscard]] constexpr auto to_next_run(time_point now) const -> duration {
        return timeout - since_last_run(now);
    }

    [[nodiscard]] constexpr auto since_last_run(time_point now) const
        -> duration {
        return now - last_run;
    }

    void run(Scheduler* s, time_point now) {
        cb(s);
        mark_run(now);
    }

    constexpr void mark_run(time_point now) { last_run = now; }

private:
    duration   timeout;
    time_point last_run;
    callback   cb;
};

class Scheduler {
public:
    auto add_timer(Timer::callback cb, duration timeout)
        -> std::shared_ptr<Timer> {
        auto t = std::make_shared<Timer>(cb, timeout);
        add_fiber(t);

        return t;
    }

    void add_fiber(std::shared_ptr<Timer> const& timer) {
        timers.push_back(timer);
    }

    void run() {
        running = true;

        while (running) {
            auto st = run_once();
            std::this_thread::sleep_for(st);
        }
    }

    auto run_once() -> duration {
        auto time = duration::max();

        for (auto& timer : timers) {
            auto n = steady_clock::now();
            if (timer->should_run(n)) timer->run(this, n);

            time = std::min(timer->since_last_run(steady_clock::now()), time);
        }

        for (auto it = timers.begin(); it != timers.end();) {
            auto const& t = *it;
            if (auto sit = todel.find(t.get()); sit != todel.end())
                it = timers.erase(it);
            else
                it++;
        }

        // TODO: fix this
        return time;
    }

    void del(Timer const* t) { todel.insert(t); }

private:
    std::vector<std::shared_ptr<Timer>> timers;
    tsl::robin_set<void const*>         todel;

    std::atomic<bool> running = false;
};
}  // namespace uppr

auto main() -> int {
    using namespace std::chrono_literals;

    uppr::Scheduler sched;
    sched.add_timer([](uppr::Scheduler* s) { fmt::println("timer 1!"); }, 1s);
    sched.add_timer([](uppr::Scheduler* s) { fmt::println("timer 2!"); },
                    500ms);

    sched.run();

    return 0;
}
