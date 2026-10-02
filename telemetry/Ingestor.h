#pragma once
#include "..//core/RingBuffer.h"
#include "telemetry/TelemetryFrame.h"
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/mavsdk.h>
#include <iostream>
#include <atomic>
#include <thread>
using namespace mavsdk;
enum class Flagi {
    OK,
    LOST
};
using std::this_thread::sleep_for;
class TelemetryIngestor {
    Telemetry telemetry;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    std::atomic<float> current_battery{0.0f};
    std::atomic<float> current_yaw{0.0f};
    std::atomic<std::chrono::time_point<uint64_t,std::chrono::milliseconds>> last_time{};
    Flagi flag{Flagi::OK};
    std::jthread watchdogThread;
    public:
    TelemetryIngestor(std::shared_ptr<System>&system, RingBuffer<TelemetryFrame,256>&buffer)
        :watchdogThread([this,&buffer](std::stop_token st) {
            while (!st.stop_requested()) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_time.load(std::memory_order_acquire));
                if (duration.count()>500) {
                    flag = Flagi::LOST;
                }
                sleep_for(std::chrono::milliseconds(100));
            }}), telemetry(system)
        {
        auto batteryRateResult = telemetry.set_rate_battery(1);
        if (batteryRateResult != Telemetry::Result::Success) {
            std::cerr<<"Failed to set rate battery to 1\n";
        }
        telemetry.subscribe_battery([this](Telemetry::Battery battery) {
            current_battery.store(battery.remaining_percent,std::memory_order_relaxed);
        });

        auto positionAndVelocityResult = telemetry.set_rate_position_velocity_ned(20);
        if (positionAndVelocityResult != Telemetry::Result::Success) {
            std::cerr<<"Failed to set rate position and velocity to 20\n";
        }
        telemetry.subscribe_position_velocity_ned([this,&buffer](Telemetry::PositionVelocityNed position_velocity_ned) {
            TelemetryFrame telemetry_frame{};
            telemetry_frame.vel_down_m_s = position_velocity_ned.velocity.down_m_s;
            telemetry_frame.vel_east_m_s = position_velocity_ned.velocity.east_m_s;
            telemetry_frame.vel_north_m_s = position_velocity_ned.velocity.north_m_s;
            telemetry_frame.down_m = position_velocity_ned.position.down_m;
            telemetry_frame.east_m = position_velocity_ned.position.east_m;
            telemetry_frame.north_m = position_velocity_ned.position.north_m;
            telemetry_frame.battery_percentage = current_battery.load();
            telemetry_frame.yaw_deg = current_yaw.load();
            auto now = std::chrono::steady_clock::now();
            telemetry_frame.timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(now -startTime);
            buffer.tryPush(telemetry_frame);
            auto pushTime = std::chrono::steady_clock::now();
            last_time.store(pushTime,std::memory_order_release);
        });

        auto yawResult = telemetry.set_rate_attitude_euler(10);
        if (yawResult != Telemetry::Result::Success) {
            std::cerr<<"Failed to set rate attitude euler to 10\n";
        }
        telemetry.subscribe_attitude_euler([this](Telemetry::EulerAngle euler) {
            current_yaw.store(euler.yaw_deg,std::memory_order_relaxed);
        });
    }

    ~TelemetryIngestor() {
        watchdogThread.request_stop();
    }
};