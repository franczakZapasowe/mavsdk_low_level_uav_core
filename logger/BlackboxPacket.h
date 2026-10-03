#pragma once
#include "..//telemetry/TelemetryFrame.h"
#include <cstdint>
struct BlackboxPacket {
    uint16_t magic = 0xAE47;
    uint64_t timestamp_us;
    TelemetryFrame telemetry_frame;
    uint32_t crc32;
};

static_assert(std::is_standard_layout<BlackboxPacket>());