#ifndef OKEC_MOBILITY_HPP_
#define OKEC_MOBILITY_HPP_

#include <okec/devices/base_station.h>
#include <okec/devices/client_device.h>
#include <okec/devices/cloud_server.h>
#include <okec/devices/edge_device.h>
#include <okec/utils/log.h>
#include "ns3/ns2-mobility-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/rectangle.h"

#include <chrono>


namespace okec
{

template<typename Derived>
struct mobility {
    auto install(auto& devices) -> void {
        auto& self = *static_cast<Derived*>(this);
        self.install(devices);
    }

    auto test(client_device_container& clients, std::string filename) -> void {
        ns3::NodeContainer wifiStaNodes;
        clients.get_nodes(wifiStaNodes);

        if (!std::filesystem::exists(filename)) {
            log::error("{} does not exist!(Current working directory:{})", filename, std::filesystem::current_path().string());
            exit(1);
        }

        auto start = std::chrono::high_resolution_clock::now();
        okec::println("Loading mobility model from {}...", filename);

        ns3::Ns2MobilityHelper trace(std::move(filename));
        trace.Install(wifiStaNodes.Begin(), wifiStaNodes.End());

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        okec::println("Mobility model loaded in {} seconds.", duration.count());
    }

    auto set_position(std::shared_ptr<client_device> client, double x, double y, double z) -> void {
        client->set_position(x, y, z);
    }

    auto set_position(std::shared_ptr<base_station> bs, double x, double y, double z) -> void {
        bs->set_position(x, y, z);
    }

    auto set_position(cloud_server& cloud, double x, double y, double z) -> void {
        cloud.set_position(x, y, z);
    }
};


} // namespace okec

#endif // OKEC_MOBILITY_HPP_