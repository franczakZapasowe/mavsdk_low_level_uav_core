#include "..//core/RingBuffer.h"
#include "..//telemetry/Ingestor.h"
#include <mavsdk/mavsdk.h>

int main(){
    RingBuffer<TelemetryFrame,256> buffer;
    Mavsdk mavsdk {Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    ConnectionResult connection_result = mavsdk.add_any_connection("udp://:14540");
    if (connection_result != ConnectionResult::Success) {
        std::cerr<<"[ERROR] Failed to connect to the udp server!\n";
        exit(1);
    }
    auto systems = mavsdk.systems();
    while (systems.empty()) {
        sleep_for(milliseconds(100));
        systems = mavsdk.systems();
    }

    const std::shared_ptr<System> system = systems.at(0);
    Ingestor ingestor(system,buffer);

    return 0;
}
