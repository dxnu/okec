#include "okec.hpp"

int main(int argc, char **argv)
{
    okec::simulator simulator;

    // Create 1 base station
    okec::base_station_container bs(1);
    // Create 5 edge servers
    okec::edge_device_container edge_servers(5);
    // Create 2 user devices
    okec::client_device_container user_devices(2);

    // Connect the bs and edge servers
    bs.connect_device(edge_servers);

    // Set the network model for every device
    okec::multiple_and_single_LAN_WLAN_network_model model;
    okec::network_initializer(model, user_devices, bs.get(0));

    // Initialize the resources for each edge server.
    okec::resource_container edge_resources(edge_servers.size());
    edge_resources.initialize([](auto res) {
        res->attribute("cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(0.1, 0.5).item<double>()));
    });
    edge_resources.print();

    // Install each resource on each edge server.
    edge_servers.install_resources(edge_resources);

    // Set decision engine
    auto decision_engine = std::make_shared<okec::scene1_decision_engine>(&user_devices, &bs);
    decision_engine->initialize();

    // Initialize a task
    okec::task t;
    t.emplace_back({
        { "task_id", okec::task::get_unique_id() },
        { "group", "one" },
        { "cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(0, 0.5).item<double>()) }
    });

    // Client request someone to handle the task.
    auto user = user_devices.get_device(0);
    user->send(t);


    simulator.run();
}