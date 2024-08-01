#include <okec/okec.hpp>

namespace olog = okec::log;


void generate_task(okec::task& t, int number, std::string const& group)
{
    // for (auto i = number; i-- > 0;)
    // {
    //     t.emplace_back({
    //         { "task_id", okec::task::unique_id() },
    //         { "group", group },
    //         { "size",  okec::rand_range(20, 25).to_string() },
    //         { "cpu", okec::rand_range(0.5, 1.5).to_string() },
    //         { "deadline", okec::rand_range(2.0, 2.5).to_string() },
    //     });
    // }
    // t.save_to_file("task-" + std::to_string(number) + ".json");
    t.load_from_file("data/task-" + std::to_string(number) + ".json");
}

okec::awaitable offloading(auto user, okec::task t) {
    // olog::debug("offloading begin");

    co_await user->async_send(std::move(t));
    auto resp = co_await user->async_read();
    olog::success("received response.\n");

    // okec::print("{}\n", resp.dump(2));
    okec::print("{:r}", resp);
    double finished = 0;
    for (const auto& item : resp.data()) {
        if (item["finished"] == "Y") {
            finished++;
        }
    }
    okec::print("Task completion rate: {:2.0f}%\n\n", finished / resp.size() * 100);
}

int main(int argc, char **argv)
{
    std::size_t edge_num = 3;
    std::size_t task_num = 10;

    ns3::CommandLine cmd;
	cmd.AddValue("edge_num", "edge number", edge_num);
	cmd.AddValue("task_num", "task number", task_num);
	cmd.Parse(argc, argv);

    okec::print("edge_num: {}, task_num: {}\n", edge_num, task_num);

    olog::set_level(olog::level::all);

    okec::simulator sim;

    // Create 1 base station
    okec::base_station_container base_stations(sim, 1);
    // Create 5 edge servers
    okec::edge_device_container edge_servers(sim, edge_num);
    // Create 2 user devices
    okec::client_device_container user_devices(sim, 2);
    // Create a cloud
    okec::cloud_server cloud(sim);

    // std::vector<okec::client_device_container> client_groups;
    // client_groups.push_back(std::move(user_devices));

    // Connect the bs and edge servers
    base_stations.connect_device(edge_servers);

    // Set the network model for every device
    okec::cloud_edge_end_model model;
    okec::network_initializer(model, user_devices, base_stations.get(0), cloud);

    // Set the location
    base_stations.get(0)->set_position(0, 0, 0);
    cloud.set_position(100, 0, 0);

    // Initialize the resources for each edge server.
    okec::resource_container resources(edge_servers.size());
    resources.initialize([](auto res) {
        res->attribute("cpu", okec::rand_range(2.4, 2.8).to_string());
    });
    // resources.save_to_file("resource-" + std::to_string(resources.size()) + ".json");
    // resources.load_from_file("data/resource-" + std::to_string(resources.size()) + ".json");
    okec::print("resources:\n{:rs}\n", resources);

    // Install each resource on each edge server.
    edge_servers.install_resources(resources);

    resources.trace_resource(); // 先捕捉初始值
    resources.set_monitor([&resources](std::string_view address, std::string_view attr, std::string_view old_val, std::string_view new_val) {
        resources.trace_resource();
    });

    // Install resource on cloud server
    auto cloud_res = okec::make_resource();
    cloud_res->attribute("cpu", "20");
    cloud.install_resource(cloud_res);

    // Specify the default offloading strategy
    auto decision_engine = std::make_shared<okec::cloud_edge_end_default_decision_engine>(&user_devices, &base_stations, &cloud);
    decision_engine->initialize();


    okec::task t;
    generate_task(t, task_num, "dummy");
    okec::print("task:\n{:t}\n", t);

    auto user1 = user_devices.get_device(0);
    co_spawn(sim, offloading(user1, t));


    sim.enable_visualizer();
    sim.run();
}