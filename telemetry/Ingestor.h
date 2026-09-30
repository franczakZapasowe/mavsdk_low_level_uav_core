#pragma once
#include "..//core/RingBuffer.h"
#include <mavsdk/mavsdk.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <mavsdk/plugins/telemetry/telemetry.h>
using std::chrono::milliseconds;
using std::this_thread::sleep_for;
using namespace mavsdk;

class Ingestor {
    Telemetry telemetry_;
    std::atomic<std::shared_ptr<TelemetryFrame>>telemetryFrame{std::make_shared<TelemetryFrame>()};
public:
    Ingestor(const std::shared_ptr<System>& system, RingBuffer<TelemetryFrame,256>&buffer)
        :telemetry_(system)
    {
        // poziom baterii
        auto batteryHzResult = telemetry_.set_rate_battery(1);
        if (batteryHzResult != Telemetry::Result::Success) {
            std::cerr <<"Fail to set 1Hz frequency on battery telemetry\n";
            exit(1);
        }
        telemetry_.subscribe_battery([this, &buffer](Telemetry::Battery battery) {
            auto staryStan = telemetryFrame.load(std::memory_order_acquire);
            auto nowyStan = std::make_shared<TelemetryFrame>(*staryStan);
            do {
                nowyStan->battery_percentage = battery.remaining_percent;
                telemetryFrame.compare_exchange_weak(staryStan, nowyStan, std::memory_order_release);
            }while (! telemetryFrame.compare_exchange_weak(staryStan, nowyStan, std::memory_order_release,std::memory_order_acquire));
            buffer.try_push(*staryStan);
        });

        // pozycja i predkosc
        auto positionHzResult = telemetry_.set_rate_position_velocity_ned(20);
        if (positionHzResult != Telemetry::Result::Success) {
            std::cerr <<"Fail to set 20Hz on position & velocity ned\n";
            exit(1);
        }
        telemetry_.subscribe_position_velocity_ned([this, &buffer](Telemetry::PositionVelocityNed position_ned) {
            auto staryStan = telemetryFrame.load(std::memory_order_release);
            auto nowyStan = std::make_shared<TelemetryFrame>(*staryStan);
            do {
                nowyStan->north_m = position_ned.position.north_m;
                nowyStan->east_m = position_ned.position.east_m;
                nowyStan->down_m = position_ned.position.down_m;
                nowyStan->vel_north_m_s = position_ned.velocity.north_m_s;
                nowyStan->vel_east_m_s = position_ned.velocity.east_m_s;
                nowyStan->vel_down_m_s = position_ned.velocity.down_m_s;
            } while (! telemetryFrame.compare_exchange_weak(staryStan, nowyStan,std::memory_order_release,std::memory_order::acquire));
            buffer.try_push(*nowyStan);
        });

        // kąt
        telemetry_.subscribe_attitude_euler([this,&buffer](Telemetry::EulerAngle euler_angle) {
            auto staryStan = telemetryFrame.load(std::memory_order_acquire);
            auto nowyStan = std::make_shared<TelemetryFrame>(*staryStan);
            do {
                nowyStan->yaw_deg = euler_angle.yaw_deg;
            }while (! telemetryFrame.compare_exchange_weak(staryStan, nowyStan,std::memory_order_release,std::memory_order::acquire));
            buffer.try_push(*nowyStan);
        });
    }

    ~Ingestor() {
        telemetry_.subscribe_battery(nullptr);
        telemetry_.subscribe_position_velocity_ned(nullptr);
        telemetry_.subscribe_attitude_euler(nullptr);
    }
};