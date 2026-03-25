// DecoderTimeoutGuard.cpp — Per-Decoder Watchdog Timeout Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
#include "DecoderTimeoutGuard.h"

#include <atomic>
#include <future>
#include <stdexcept>
#include <thread>

namespace ExplorerLens {
namespace Engine {

TimeoutResult DecoderTimeoutGuard::RunWithTimeout(const std::function<void()>& work) const
{
    std::atomic<bool> done{ false };
    std::exception_ptr exc = nullptr;

    std::thread workerThread([&]() {
        try
        {
            work();
        }
        catch (...)
        {
            exc = std::current_exception();
        }
        done.store(true, std::memory_order_release);
    });

    const auto deadline = Clock::now() + m_timeout;

    while (!done.load(std::memory_order_acquire))
    {
        if (Clock::now() >= deadline)
        {
            workerThread.detach();
            return TimeoutResult::TimedOut;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (workerThread.joinable()) workerThread.join();

    if (exc) std::rethrow_exception(exc);

    return TimeoutResult::Completed;
}

} // namespace Engine
} // namespace ExplorerLens
