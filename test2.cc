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

//     okec::base_station_container base_stations(2);
//     okec::cloud_server cs;
//     okec::client_device_container client_devices(4);
//     okec::edge_device_container edge_devices1(15);
//     okec::edge_device_container edge_devices2(10);
//     if (!base_stations.connect_device(edge_devices1, edge_devices2))
//         return EXIT_FAILURE;
//     okec::initialize_communication(client_devices, base_stations, cs);

//     // 设置用户设备位置
//     client_devices.get_device(1)->set_position(0, 20, 30);
//     client_devices.get_device(1)->set_position(1, 1, 1);
//     client_devices.get_device(2)->set_position(2, 2, 2);
//     client_devices.get_device(3)->set_position(4, 4, 4);
//     // 设置基站位置
//     base_stations.get(0)->set_position(50, 50, 50);
//     base_stations.get(1)->set_position(80, 50, 50);
//     // 设置云服务器位置
//     cs.set_position(1000, 1000, 1000);


//     // 配置资源
//     okec::resource_container client_rcontainer(client_devices.size());
//     okec::resource_container edge1_rcontainer(edge_devices1.size());
//     okec::resource_container edge2_rcontainer(edge_devices2.size());

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
//     edge2_rcontainer.initialize([&cpus, &uid](auto res) {
//         auto index = uid(dre);
//         res->attribute("cpu_cycle", cpus[index][8]);
//         res->attribute("TDP", cpus[index][5]);
//     });
//     edge1_rcontainer.print("Edge Device 1 Resources");
//     edge2_rcontainer.print("Edge Device 2 Resources");


//     client_devices.install_resources(client_rcontainer); // 一键为所有用户设备配置资源
//     edge_devices1.install_resources(edge1_rcontainer);   // 一键为所有边缘设备安装资源
//     edge_devices2.install_resources(edge2_rcontainer);   // 一键为所有边缘设备安装资源

//     auto cloud_resource = okec::make_resource();
//     cloud_resource->attribute("cpu_cycle", "20000");
//     cloud_resource->attribute("memory", "50000");
//     cloud_resource->attribute("TDP", "55.0");
//     cs.install_resource(cloud_resource);

//     auto decision_engine = std::make_shared<okec::default_decision_engine>(&client_devices, &base_stations, &cs);
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
//     device_1->send_to(t3);
//     device_1->when_done([](okec::response res) {
//         fmt::print("{0:=^{1}}\n", "Response Info", 180);
//         double es_count{};
//         int index{1};
//         std::vector<double> points;
//         std::vector<double> power_consumption_points;
//         for (const auto& item : res.data()) {
//             fmt::print("[{:>3}] ", index++);
//             fmt::print("id: {}, device_type: {:>5}, device_address: {:>10}, group: {}, time_consuming: {}s, send_time: {:>8}s, power_consumption: {:>8}Watts, finished: {}\n",
//                 item["task_id"], item["device_type"], item["device_address"], item["group"], item["time_consuming"], item["send_time"], item["power_consumption"], item["finished"]);
//             if (item["device_type"] == "es" || item["device_type"] == "local") {
//                 points.push_back(std::stod(item["time_consuming"].template get<std::string>()) + std::stod(item["send_time"].template get<std::string>()));
//                 power_consumption_points.push_back(std::stod(item["power_consumption"].template get<std::string>()));
//                 es_count++;
//             }
//         }
//         fmt::print("Task completion rate: {:2.0f}%\n", es_count / res.size() * 100);

//         auto total_time = std::accumulate(points.begin(), points.end(), .0);
//         auto total_power_consumption = std::accumulate(power_consumption_points.begin(), power_consumption_points.end(), .0);
//         fmt::print("Total processing time: {:.9f}s\n", total_time);
//         fmt::print("Average processing time: {:.9f}s\n", total_time / points.size());
//         fmt::print("Total power consumption: {:.3f}Watts\n", total_power_consumption);
//         fmt::print("Average power consumption: {:.3f}Watts\n", total_power_consumption / power_consumption_points.size());
//         fmt::print("{0:=^{1}}\n", "", 180);

//         okec::draw(points, "Time Comsumption(Seconds)");
//         okec::draw(power_consumption_points, "Power Comsumption(Watts)");
//     });


//     Simulator::Stop(Seconds(300));
//     Simulator::Run();
//     Simulator::Destroy();
// }