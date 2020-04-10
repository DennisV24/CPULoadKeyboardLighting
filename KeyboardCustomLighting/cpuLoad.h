#pragma once

#include <cstdint>
#include <thread>
#include <mutex>
#include <time.h>

#include <Windows.h>

#include "ringBuffer.h"

template <unsigned int NUM_SAMPLES>
class CpuLoad
{
public:
    CpuLoad(unsigned int sampleTimeMs) :
        sampleTimeMs(sampleTimeMs),
        buffer(),
        previousBusyTicks(0),
        previousIdleTicks(0),
        lock(),
        diagnosticThread([=] {runLoadCalculations();})
    {}

    // add an option for number of samples
    float getCpuLoad() const
    {
        return buffer.averageData();
    }

private:
    static uint64_t fileTimeToUInt64(const FILETIME& filetime) 
    {
        uint64_t retval;
        uint64_t upper = filetime.dwHighDateTime;
        uint64_t lower = filetime.dwLowDateTime;
        retval = upper << 32 | lower;
        return retval;
    }

    float calculateCpuLoad(uint64_t idleTicks, uint64_t busyTicks)
    {
        uint64_t totalBusyTicksSinceLastTime = busyTicks - previousBusyTicks;
        uint64_t totalIdleTicksSinceLastTime = idleTicks - previousIdleTicks;
        float overhead;
        if (totalBusyTicksSinceLastTime > 0) {
            overhead = (float)totalIdleTicksSinceLastTime / totalBusyTicksSinceLastTime;
        }
        else {
            overhead = 0.;
        }

        float ret = 1.0f - overhead;
        previousBusyTicks = busyTicks;
        previousIdleTicks = idleTicks;
        return ret;
    }

    void runLoadCalculations() 
    {
        while (1) {
            float load = getCPULoad() * 100.f;
            lock.lock();
            buffer.addVal(load);
            lock.unlock();
            Sleep(sampleTimeMs);
        }
    }

    float getCPULoad()
    {
        FILETIME idleTime, kernelTime, userTime;
        bool success = GetSystemTimes(&idleTime, &kernelTime, &userTime);
        float retval;
        if (success) {
            retval = calculateCpuLoad(fileTimeToUInt64(idleTime), fileTimeToUInt64(kernelTime) + fileTimeToUInt64(userTime));
        }
        else {
            retval = -1.0f;
        }
        return retval;
    }

private:
    unsigned int sampleTimeMs;
    RingBuffer<float, NUM_SAMPLES> buffer;
    uint64_t previousBusyTicks;
    uint64_t previousIdleTicks;
    std::mutex lock;
    std::thread diagnosticThread;
};

