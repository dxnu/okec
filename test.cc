// #include "default_decision_engine.h"
// #include "qlearning_decision_engine.h"
// #include "okec.hpp"
// #include "read_csv.h"
// #include "response_visulizer.hpp"
// #include <random>


// using namespace ns3;

// auto main(int argc, char **argv) -> int
// {
//     CommandLine cmd;
//     cmd.Parse(argc, argv);

//     fmt::print("C++ version: {}\n", __cplusplus);

//     Time::SetResolution(Time::NS);
//     LogComponentEnable("udp_application", LOG_LEVEL_INFO);

//     okec::base_station_container base_stations(1);
//     okec::cloud_server cs;
//     okec::client_device_container client_devices(4);
//     okec::edge_device_container edge_devices1(2);
//     if (!base_stations.connect_device(edge_devices1))
//         return EXIT_FAILURE;
//     okec::initialize_communication(client_devices, base_stations, cs);

//     // 设置用户设备位置
//     client_devices.get_device(1)->set_position(0, 20, 30);
//     client_devices.get_device(1)->set_position(1, 1, 1);
//     client_devices.get_device(2)->set_position(2, 2, 2);
//     client_devices.get_device(3)->set_position(4, 4, 4);
//     // 设置基站位置
//     base_stations.get(0)->set_position(50, 50, 50);
//     // 设置云服务器位置
//     cs.set_position(1000, 1000, 1000);


//     // 配置资源
//     okec::resource_container client_rcontainer(client_devices.size());
//     okec::resource_container edge1_rcontainer(edge_devices1.size());

//     // 初始化用户设备资源
//     client_rcontainer.initialize([](auto res) {
//         res->attribute("cpu_cycle", "400");
//         res->attribute("TDP", "20");
//     });
//     client_rcontainer.print("Client Device Resources:");


//     // 加载数据集
//     auto chip = okec::read_csv("./datasets/chip_dataset.csv", "CPU");
//     if (!chip) {
//         fmt::print("failed to open dataset\n");
//         return EXIT_FAILURE;
//     }

//     // 以数据集初始化边缘设备资源
//     const auto& cpus = chip.value();
//     using rng = std::default_random_engine;
//     static rng dre{ (rng::result_type)time(0) };
//     std::uniform_int_distribution<int> uid(0, cpus.size());
//     edge1_rcontainer.initialize([&cpus, &uid](auto res) {
//         auto index = uid(dre);
//         res->attribute("cpu_cycle", cpus[index][8]);
//         res->attribute("TDP", cpus[index][5]);
//     });
//     edge1_rcontainer.print("Edge Device 1 Resources");

//     client_devices.install_resources(client_rcontainer); // 一键为所有用户设备配置资源
//     edge_devices1.install_resources(edge1_rcontainer);   // 一键为所有边缘设备安装资源

//     auto cloud_resource = okec::make_resource();
//     cloud_resource->attribute("cpu_cycle", "20000");
//     cloud_resource->attribute("memory", "50000");
//     cloud_resource->attribute("TDP", "55.0");
//     cs.install_resource(cloud_resource);

//     auto decision_engine = std::make_shared<okec::qlearning_decision_engine>(&client_devices, &base_stations, &cs);
//     base_stations.set_decision_engine(decision_engine);
//     client_devices.set_decision_engine(decision_engine);


//     okec::task t3;
//     // if (!t3.read_from_file("task.json")) {
//     //     fmt::print("Failed to read task data\n");
//     //     return EXIT_FAILURE;
//     // }
//     for (int i = 0; i < 10; ++i)
//     {
//         t3.emplace_back({
//             { "task_id", okec::task::get_unique_id() },
//             { "group", "one" },
//             { "cpu_cycle", okec::task::get_random_number(10000, 10000000000) },
//             { "deadline", okec::task::get_random_number(1, 5) },
//             { "input_size", okec::task::get_random_number(10000, 100000) }
//         }, {
//             { "instructions", "xxx" }
//         });
//     }
//     t3.print();

//     // t3.save_to_file("task.json");



//     auto device_1 = client_devices.get_device(1);
//     decision_engine->train(t3, device_1, &base_stations, &cs);




//     Simulator::Stop(Seconds(300));
//     Simulator::Run();
//     Simulator::Destroy();
// }

// #include <torch/torch.h>

// int main(int argc, char **argv)
// {

// }