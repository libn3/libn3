#pragma once

#include <chrono>
#include <optional>
#include <queue>

namespace n3 {

class Timer {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::nanoseconds;

    TimePoint next_wakeup;
    std::optional<Duration> period;
    bool active;
    bool fired;

    void update() noexcept {
        if (!this->active) {
            return;
        }
        const auto have_period = this->period.has_value();
        const auto now = Clock::now();

        if (now >= this->next_wakeup) {
            this->fired = true;
            if (!have_period) {
                this->active = false;
            }
        }
        if (have_period) {
            this->next_wakeup = now + period.value();
        }
    }

public:
    constexpr Timer() noexcept = default;

    constexpr Timer(const Duration& interval) noexcept :
            next_wakeup{[&] { return Clock::now() + interval; }()},
            period{interval},
            active{true},
            fired{false} {
    }
    constexpr Timer(const TimePoint& wakeup,
            const std::optional<Duration>& interval = std::nullopt) noexcept :
            next_wakeup{wakeup},
            period{interval},
            active{true},
            fired{false} {
    }

    constexpr Timer(const Timer&) noexcept = default;
    constexpr Timer(Timer&&) noexcept = default;

    constexpr Timer& operator=(const Timer&) noexcept = default;
    constexpr Timer& operator=(Timer&&) noexcept = default;

    constexpr std::weak_ordering operator<=>(const Timer& other) const noexcept {
        //Next wakeup first
        if (auto cmp = (this->next_wakeup <=> other.next_wakeup); cmp != 0) {
            return cmp;
        }
        return this->period <=> other.period;
        //Comparing status flags makes no sense
        //Is a paused timer "less than" an active one?
    }

    [[nodiscard]] bool is_active() noexcept {
        this->update();
        return this->active;
    }
    [[nodiscard]] bool has_elapsed() noexcept {
        this->update();
        return this->fired;
    }

    constexpr void resume() noexcept {
        this->active = true;
    }
    constexpr void pause() noexcept {
        this->active = false;
    }
    constexpr void stop_repeat() noexcept {
        this->period.reset();
    }
    constexpr void repeat(const Duration& interval) noexcept {
        this->period = interval;
        this->resume();
    }
    constexpr void reset(const TimePoint& wakeup,
            const std::optional<Duration>& interval = std::nullopt) noexcept {
        this->next_wakeup = wakeup;
        this->period = interval;
        this->fired = false;
        this->resume();
    }
};

class TimerList {
    std::priority_queue<Timer> timer_heap;

public:
    TimerList() noexcept = default;
};

} // namespace n3
