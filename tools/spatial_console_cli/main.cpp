#include "../../modules/spatial/ImmersiveBusManager.h"
#include "../../modules/spatial/SpatialCrosstalkEngine.h"
#include <iostream>
#include <vector>

int main()
{
    ImmersiveBusManager buses;

    buses.addBus({"Main Atmos Bed", 12});
    buses.addBus({"FX Objects", 16});

    std::vector<float> l(512, 0.1f);
    std::vector<float> r(512, 0.05f);
    std::vector<float> c(512, 0.08f);

    std::vector<float*> channels = {
        l.data(),
        r.data(),
        c.data()
    };

    SpatialCrosstalkEngine engine;
    engine.process(channels, 512, 0.5f);

    std::cout << "ConsoleCore Spatial Console CLI\n";
    std::cout << "Bus Count: " << buses.buses().size() << "\n";
    std::cout << "Processed Spatial Crosstalk\n";

    return 0;
}