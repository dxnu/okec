// #include "okec.hpp"

// void generate_task(okec::task &t, int number, const std::string& group) {
//     // for ([[maybe_unused]] auto _ : std::views::iota(0, number)) {
//     //     t.emplace_back({
//     //         { "task_id", okec::task::get_unique_id() },
//     //         { "group", group },
//     //         { "size", fmt::format("{:.2f}", torch::rand({1}).uniform_(2, 5).item<double>()) },
//     //         { "cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(0.5, 1.5).item<double>()) },
//     //         { "deadline", fmt::format("{:.2f}", torch::rand({1}).uniform_(2.0, 2.5).item<double>()) },
//     //     });
//     // }

//     // t.save_to_file("task-scene3-" + std::to_string(number) + ".json");
//     t.load_from_file("task-scene3-" + std::to_string(number) + ".json");
// }

// int main(int argc, char **argv)
// {
//     okec::simulator simulator;

//     // Create 2 base stations and connect them with some edge serves.
//     okec::base_station_container base_stations(1);
//     okec::edge_device_container edge_devices1(10);
//     base_stations.connect_device(edge_devices1);

//     // Create 2 user groups
//     okec::client_device_container user_devices(2);

//     // Initialize the network
//     okec::multiple_and_single_LAN_WLAN_network_model net_model;
//     okec::network_initializer(net_model, user_devices, base_stations.get(0));

//     // Create resources.
//     okec::resource_container edge_resources1(edge_devices1.size());
//     // edge_resources1.initialize([](auto res) {
//     //     res->attribute("cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(10, 10).item<double>()));
//     // });
//     // edge_resources1.save_to_file("resource-scene3-" + std::to_string(edge_resources1.size()) + ".json");
//     edge_resources1.load_from_file("resource-scene3-" + std::to_string(edge_resources1.size()) + ".json");
//     edge_resources1.print();

//     // Install resources on edge servers.
//     edge_devices1.install_resources(edge_resources1);   // 一键为所有边缘设备安装资源

//     auto decision_engine = std::make_shared<okec::DQN_decision_engine2>(&user_devices, &base_stations);
//     decision_engine->initialize();

//     okec::task t;
//     generate_task(t, 50, "dummy");
//     // t.print();

//     auto device_1 = user_devices.get_device(0);
    
//     decision_engine->train(t, 1);



//     simulator.run();
// }