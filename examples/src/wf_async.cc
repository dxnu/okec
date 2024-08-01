#include <okec/okec.hpp>

using namespace okec;

void generate_task(okec::task &t, int number, const std::string& group) {
    for ([[maybe_unused]] auto _ : std::views::iota(0, number)) {
        t.emplace_back({
            { "task_id", okec::task::unique_id() },
            { "group", group },
            { "cpu", okec::rand_range(0.2, 1.2).to_string() },
            { "deadline", okec::rand_range(10, 100).to_string() },
        });
    }
}

void my_monitor(std::string_view address, std::string_view attr, std::string_view old_val, std::string_view new_val) {
    static std::ofstream file;
    if (!file.is_open()) {
        file.open("my_monitor.csv", std::ios::out/* | std::ios::app*/);
        if (!file.is_open()) {
            return;
        }
    }

    file << okec::format("At time {:.2f}s,{},{},{}\n", okec::now::seconds(), address, old_val, new_val);
}

okec::awaitable offloading(auto user, okec::task t) {
    log::debug("offloading begin");
    
    co_await user->async_send(std::move(t));
    auto resp = co_await user->async_read();
    log::success("received resp.");

    okec::print("{:r}", resp);


    // double finished = 0;
    // int index = 1;
    // std::vector<double> time_points;
    // okec::print("{0:=^{1}}\n", "Response Info", okec::get_winsize().col);
    // for (const auto& item : resp.data()) {
    //     okec::print("[{:>3}] ", index++);
    //     okec::print("task_id: {}, device_type: {:>5}, device_address: {:>10}, group: {}, time_consuming: {:>11}s, finished: {}\n",
    //         item["task_id"], item["device_type"], item["device_address"], item["group"], item["time_consuming"], item["finished"]);
    //     if (item["finished"] == "Y") {
    //         finished++;
    //         time_points.push_back(TO_DOUBLE(item["time_consuming"]));
    //     }
    // }

    // auto total_time = std::accumulate(time_points.begin(), time_points.end(), .0);
    // okec::print("Task completion rate: {:2.0f}%\n", finished / resp.size() * 100);
    // okec::print("Total processing time: {:.6f}\n", total_time);
    // okec::print("Average processing time: {:.6f}\n", total_time / time_points.size());
    // okec::print("{0:=^{1}}", "", okec::get_winsize().col);
}

int main(int argc, char **argv)
{
    log::set_level(log::level::all);

    okec::simulator sim;

    // Create 1 base station
    okec::base_station_container bs(sim, 1);
    // Create 5 edge servers
    okec::edge_device_container edge_servers(sim, 5);
    // Create 2 user devices
    okec::client_device_container user_devices(sim, 2);

    // Connect the bs and edge servers
    bs.connect_device(edge_servers);

    // Set the network model for every device
    okec::multiple_and_single_LAN_WLAN_network_model model;
    okec::network_initializer(model, user_devices, bs.get(0));

    // Initialize the resources for each edge server.
    okec::resource_container edge_resources(edge_servers.size());
    edge_resources.initialize([](auto res) {
        res->attribute("cpu", okec::rand_range<double>(2.1, 2.2).to_string());
    });
    okec::print("{:rs}", edge_resources);

    // Install each resource on each edge server.
    edge_servers.install_resources(edge_resources);

    edge_resources.trace_resource(); // 先捕捉初始值
    edge_resources.set_monitor([&edge_resources](std::string_view address, std::string_view attr, std::string_view old_val, std::string_view new_val) {
        edge_resources.trace_resource();
    });

    // Set decision engine
    auto decision_engine = std::make_shared<okec::worst_fit_decision_engine>(&user_devices, &bs);
    decision_engine->initialize();

    okec::task t1;
    generate_task(t1, 100, "1st");
    auto user1 = user_devices.get_device(0);
    co_spawn(sim, offloading(user1, t1));

    log::debug("main-1--");

    okec::task t2;
    generate_task(t2, 1, "2st");
    auto user2 = user_devices.get_device(1);
    co_spawn(sim, offloading(user2, t2));

    log::debug("main-2--");

    // sim.enable_visualizer();
    sim.run();
}