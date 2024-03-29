#include "okec.hpp"


void generate_task(okec::task &t, int number, const std::string& group) {
    // for ([[maybe_unused]] auto _ : std::views::iota(0, number)) {
    //     t.emplace_back({
    //         { "task_id", okec::task::get_unique_id() },
    //         { "group", group },
    //         { "cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(0.2, 1.2).item<double>()) },
    //         { "deadline", fmt::format("{:.2f}", torch::rand({1}).uniform_(10, 100).item<double>()) },
    //     });
    // }

    // t.save_to_file("task-" + std::to_string(number) + ".json");
    t.load_from_file("task-" + std::to_string(number) + ".json");
}

int main(int argc, char **argv)
{
    okec::simulator simulator;

    // Create 2 base stations and connect them with some edge serves.
    okec::base_station_container base_stations(2);
    okec::edge_device_container edge_devices1(5);
    okec::edge_device_container edge_devices2(8);
    base_stations.connect_device(edge_devices1, edge_devices2);

    // Create 2 user groups
    okec::client_device_container client_devices1(2);
    okec::client_device_container client_devices2(2);
    std::vector<okec::client_device_container> client_devices;
    client_devices.push_back(std::move(client_devices1));
    client_devices.push_back(std::move(client_devices2));

    // Initialize the network
    okec::multiple_LAN_WLAN_network_model net_model;
    okec::network_initializer(net_model, client_devices, base_stations);

    // Create resources.
    okec::resource_container edge_resources1(edge_devices1.size());
    okec::resource_container edge_resources2(edge_devices2.size());
    edge_resources1.load_from_file("resource-" + std::to_string(edge_resources1.size()) + ".json");
    edge_resources2.load_from_file("resource-" + std::to_string(edge_resources2.size()) + ".json");
    edge_resources1.print();
    edge_resources2.print();

    // Install resources on edge servers.
    edge_devices1.install_resources(edge_resources1);   // 一键为所有边缘设备安装资源
    edge_devices2.install_resources(edge_resources2);   // 一键为所有边缘设备安装资源

    auto decision_engine = std::make_shared<okec::DQN_decision_engine>(&client_devices, &base_stations);
    decision_engine->initialize();

    okec::task t;
    generate_task(t, 10, "dummy");

    auto device_1 = client_devices[0].get_device(0);
    // device_1->send(t);
    decision_engine->train(t);
    // decision_engine->train(t, device_1, base_stations);
    // device_1->send(t);
    // device_1->when_done([](okec::response res) {
    
    //     fmt::print("task is done!\n");
    // });



    simulator.run();
}