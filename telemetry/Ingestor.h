#pragma once
#include "..//core/RingBuffer.h"
#include <mavsdk/mavsdk.h>
#include <chrono>
#include <thread>
#include <mavsdk/plugins/telemetry/telemetry.h>
using std::chrono::milliseconds;
using std::this_thread::sleep_for;
using namespace mavsdk;

class Ingestor {

    std::atomic<std::shared_ptr<TelemetryFrame>>telemetryFrame;

public:
    Ingestor(std::shared_ptr<System>& system, RingBuffer<TelemetryFrame,256>&buffer){
        auto telemetry = Telemetry{system};
        TelemetryFrame teleFrame{};

        // poziom baterii
        auto batteryHzResult = telemetry.set_rate_battery(1);
        if (batteryHzResult != Telemetry::Result::Success) {
            std::cerr <<"Fail to set 1Hz frequency on battery telemetry\n";
            exit(1);
        }
        telemetry.subscribe_battery([&teleFrame](Telemetry::Battery battery) {

            auto staryStan = telemetryFrame.load();
            auto nowyStan = std::make_shared<TelemetryFrame>(*staryStan);

            nowyStan. .battery_percentage = battery.remaining_percent;
        });

        // pozycja i predkosc
        auto positionHzResult = telemetry.set_rate_position_velocity_ned(20);
        if (positionHzResult != Telemetry::Result::Success) {
            std::cerr <<"Fail to set 20Hz on position & velocity ned\n";
            exit(1);
        }
        telemetry.subscribe_position_velocity_ned([&teleFrame](Telemetry::PositionVelocityNed position_ned) {
            teleFrame.north_m = position_ned.position.north_m;
            teleFrame.east_m = position_ned.position.east_m;
            teleFrame.down_m = position_ned.position.down_m;
            teleFrame.vel_north_m_s = position_ned.velocity.north_m_s;
            teleFrame.vel_east_m_s = position_ned.velocity.east_m_s;
            teleFrame.vel_down_m_s = position_ned.velocity.down_m_s;
        });

        // kąt
        telemetry.subscribe_attitude_euler([&teleFrame](Telemetry::EulerAngle euler_angle) {
           teleFrame.yaw_deg = euler_angle.yaw_deg;
        });

    }
};