#include <iostream>

#include "..//core/RingBuffer.h"
#include <mavsdk/mavsdk.h>
#include "..//telemetry/TelemetryFrame.h"
#include "telemetry/Ingestor.h"
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using namespace mavsdk;
int main(){

    Mavsdk mavsdk {Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    ConnectionResult connection_result = mavsdk.add_any_connection("udp://:14540");
    if (connection_result != ConnectionResult::Success) {
        std::cerr<<"Failed to connect to the udp system\n";
        return 1;
    }
    auto systems = mavsdk.systems();
    while (systems.empty()) {
        sleep_for(milliseconds(100));
        systems = mavsdk.systems();
    }
    auto system = systems.at(0);

    RingBuffer<TelemetryFrame,256>buffer;
    TelemetryIngestor telemetry_ingestor (system,buffer);
    return 0;
}
