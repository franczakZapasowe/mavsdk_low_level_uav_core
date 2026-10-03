#pragma once
#include "..//core/RingBuffer.h"
#include "telemetry/TelemetryFrame.h"
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/mavsdk.h>
#include <iostream>
#include <atomic>
#include <thread>
enum class Flagi {
    OK,
    LOST
};
static uint64_t get_current_time() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    uint64_t time = duration.count();
    return time;
}
class TelemetryIngestor {
    mavsdk::Telemetry telemetry;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    std::atomic <float> current_battery{0.0f};
    std::atomic <float> current_yaw{0.0f};
    std::atomic <uint64_t> last_time;
    std::atomic<Flagi> flag{Flagi::OK};
    RingBuffer<TelemetryFrame,256>&buffer_;
    std::jthread watchdogThread;

    public:
    TelemetryIngestor(const std::shared_ptr<mavsdk::System>&system, RingBuffer<TelemetryFrame,256>&buffer)
        :watchdogThread([this](std::stop_token st) {
            while (!st.stop_requested()) {
                auto raw_last_time = last_time.load(std::memory_order_acquire);
                std::chrono::milliseconds time_since_epoch(raw_last_time);
                std::chrono::steady_clock::time_point lastPush{time_since_epoch};
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastPush);
                if (duration.count()>500) {
                    flag.store(Flagi::LOST,std::memory_order_release);
                }else {
                    flag.store(Flagi::OK,std::memory_order_release);
                }
                 std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }}), telemetry(system),buffer_(buffer), last_time(get_current_time())
        {
        auto batteryRateResult = telemetry.set_rate_battery(1);
        if (batteryRateResult != mavsdk::Telemetry::Result::Success) {
            std::cerr<<"Failed to set rate battery to 1\n";
        }
        telemetry.subscribe_battery([this](mavsdk::Telemetry::Battery battery) {
            current_battery.store(battery.remaining_percent,std::memory_order_relaxed);
        });

        auto positionAndVelocityResult = telemetry.set_rate_position_velocity_ned(20);
        if (positionAndVelocityResult != mavsdk::Telemetry::Result::Success) {
            std::cerr<<"Failed to set rate position and velocity to 20\n";
        }
        telemetry.subscribe_position_velocity_ned([this](mavsdk::Telemetry::PositionVelocityNed position_velocity_ned) {
            TelemetryFrame telemetry_frame{};
            telemetry_frame.vel_down_m_s = position_velocity_ned.velocity.down_m_s;
            telemetry_frame.vel_east_m_s = position_velocity_ned.velocity.east_m_s;
            telemetry_frame.vel_north_m_s = position_velocity_ned.velocity.north_m_s;
            telemetry_frame.down_m = position_velocity_ned.position.down_m;
            telemetry_frame.east_m = position_velocity_ned.position.east_m;
            telemetry_frame.north_m = position_velocity_ned.position.north_m;
            telemetry_frame.battery_percentage = current_battery.load(std::memory_order_relaxed);
            telemetry_frame.yaw_deg = current_yaw.load(std::memory_order_relaxed);
            auto now = std::chrono::steady_clock::now();
            telemetry_frame.timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(now -startTime);
            buffer_.tryPush(telemetry_frame);
            auto lastPush = std::chrono::steady_clock::now(); //timepoint
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(lastPush.time_since_epoch());
            uint64_t time = duration.count();
            last_time.store(time,std::memory_order_release);
        });

        auto yawResult = telemetry.set_rate_attitude_euler(10);
        if (yawResult != mavsdk::Telemetry::Result::Success) {
            std::cerr<<"Failed to set rate attitude euler to 10\n";
        }
        telemetry.subscribe_attitude_euler([this](mavsdk::Telemetry::EulerAngle euler) {
            current_yaw.store(euler.yaw_deg,std::memory_order_relaxed);
        });
    }

    ~TelemetryIngestor() {
        watchdogThread.request_stop();
        telemetry.subscribe_battery(nullptr);
        telemetry.subscribe_position_velocity_ned(nullptr);
        telemetry.subscribe_attitude_euler(nullptr);
    }
};