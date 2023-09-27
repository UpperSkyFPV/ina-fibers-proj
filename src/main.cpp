#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <tsl/robin_set.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <span>
#include <thread>
#include <utility>

namespace uppr {

class Scheduler;

using std::chrono::steady_clock;
using duration = steady_clock::duration;
using time_point = steady_clock::time_point;

class Timer {
public:
    using callback = std::function<void(Scheduler* s, Timer* t)>;

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

    [[nodiscard]] constexpr auto is_flagged() const -> bool {
        return timeout == duration::max();
    }

    void run(Scheduler* s, time_point now) {
        cb(s, this);
        mark_run(now);
    }

    constexpr void               mark_run(time_point now) { last_run = now; }
    constexpr void               set_timeout(duration t) { timeout = t; }
    [[nodiscard]] constexpr auto get_timeout() const -> duration {
        return timeout;
    }

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

    auto run_once(duration max_sleep = std::chrono::milliseconds{2000})
        -> duration {
        auto time = duration::max();

        for (auto& timer : timers) {
            auto n = steady_clock::now();
            // fmt::println(" - timer {} with timeout {} has {}",
            //              fmt::ptr(timer.get()), timer->get_timeout(),
            //              timer->to_next_run(uppr::steady_clock::now()));
            if (timer->should_run(n)) timer->run(this, n);

            time = std::min(timer->to_next_run(steady_clock::now()), time);
        }

        // remove pending timers
        for (auto it = timers.begin(); it != timers.end();) {
            auto const& t = *it;
            if (auto sit = todel.find(t.get()); sit != todel.end())
                it = timers.erase(it);
            else
                it++;
        }

        todel.clear();

        // TODO: fix this
        return std::min(max_sleep, time);
    }

    void del(Timer const* t) { todel.insert(t); }

    [[nodiscard]] auto timer_count() const -> std::size_t {
        return timers.size();
    }

    [[nodiscard]] auto get_timers() const
        -> std::span<const std::shared_ptr<Timer>> {
        return timers;
    }

private:
    std::vector<std::shared_ptr<Timer>> timers;
    tsl::robin_set<void const*>         todel;

    std::atomic<bool> running = false;
};
}  // namespace uppr

auto main() -> int {
    using namespace std::chrono_literals;

    uppr::Scheduler sched;
    sched.add_timer(
        [](uppr::Scheduler* s, uppr::Timer*) { fmt::println("timer 1!"); }, 1s);
    sched.add_timer(
        [](uppr::Scheduler* s, uppr::Timer*) { fmt::println("timer 2!"); },
        500ms);

    auto timer = sched.add_timer(
        [](uppr::Scheduler* s, uppr::Timer* t) {
            fmt::println("RUNNING after setting flag!");

            s->del(t);
        },
        uppr::duration::max());

    sched.add_timer(
        [timer](uppr::Scheduler* s, uppr::Timer* t) {
            fmt::println("Timer 3 (del) wakup @{}", fmt::ptr(timer.get()));
            timer->set_timeout(0ns);

            s->del(t);
        },
        2s);

    sched.run();

    return 0;
}
