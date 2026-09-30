#pragma once
#include <chrono>

// Struktura opisujaca stan drona w układzie lokalnym NED
struct TelemetryFrame {
    // pozycja
    float north_m;
    float east_m;
    float down_m;
    // prędkości
    float vel_north_m_s;
    float vel_east_m_s;
    float vel_down_m_s;
    // kąt odchyelnia
    float yaw_deg;
    // poziom naladowania bateri - procenty
    float battery_percentage;
    // znacznik czasu w mikrosekundach
    std::chrono::steady_clock timestamp_us;
};

static_assert(std::is_standard_layout<TelemetryFrame>());
static_assert(std::is_trivially_copyable_v<TelemetryFrame>);
