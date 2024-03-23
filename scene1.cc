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
        res->attribute("cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(2.4, 2.5).item<double>()));
    });
    edge_resources.print();

    // Install each resource on each edge server.
    edge_servers.install_resources(edge_resources);

    // Set decision engine
    auto decision_engine = std::make_shared<okec::scene1_decision_engine>(&user_devices, &bs);
    decision_engine->initialize();

    // Initialize a task
    okec::task t;
    for (auto _ : std::views::iota(0, 50)) {
        t.emplace_back({
            { "task_id", okec::task::get_unique_id() },
            { "group", "one" },
            { "cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(1.1, 1.2).item<double>()) },
            { "deadline", fmt::format("{:.2f}", torch::rand({1}).uniform_(10, 100).item<double>()) },
        });
    }
    t.print();

    // Client request someone to handle the task.
    auto user = user_devices.get_device(0);
    user->send(t);
    user->when_done([](okec::response response) {
        fmt::print("{0:=^{1}}\n", "Response Info", 180);
        double finished = 0;
        int index = 1;
        std::vector<double> time_points;
        for (const auto& item : response.data()) {
            fmt::print("[{:>3}] ", index++);
            fmt::print("task_id: {}, device_type: {:>5}, device_address: {:>10}, group: {}, time_consuming: {:>11}s, finished: {}\n",
                item["task_id"], item["device_type"], item["device_address"], item["group"], item["time_consuming"], item["finished"]);
            if (item["finished"] == "Y") {
                finished++;
                time_points.push_back(TO_DOUBLE(item["time_consuming"]));
            }
        }
        
        auto total_time = std::accumulate(time_points.begin(), time_points.end(), .0);
        fmt::print("Task completion rate: {:2.0f}%\n", finished / response.size() * 100);
        fmt::print("Total processing time: {:.9f}s\n", total_time);
        fmt::print("Average processing time: {:.9f}s\n", total_time / time_points.size());

        fmt::print("{0:=^{1}}\n", "", 180);

        okec::draw(time_points, "Time Comsumption(Seconds)");
    });

    simulator.run();
}