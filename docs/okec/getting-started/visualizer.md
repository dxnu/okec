# Visualizer

## Response Visualizer

```cpp
#include <okec/okec.hpp>

namespace olog =  okec::log;

void generate_task(okec::task& t, int number, std::string const& group)
{
    for (auto i = number; i-- > 0;)
    {
        t.emplace_back({
            { "task_id", okec::task::get_unique_id() },
            { "group", group },
            { "cpu", okec::rand_range(0.2, 1.2).to_string() },
            { "deadline", okec::rand_range(1, 5).to_string() },
        });
    }
}

okec::awaitable offloading(auto user, okec::task t) {
    std::vector<int> x_points(t.size());
    std::ranges::iota(x_points, 1);

    co_await user->async_send(std::move(t));
    auto resp = co_await user->async_read();
    olog::success("received response.");

    okec::print("{:r}", resp);
    double finished = 0;
    std::vector<double> time_points;
    for (const auto& item : resp.data()) {
        if (item["finished"] == "Y") {
            finished++;
            time_points.push_back(TO_DOUBLE(item["time_consuming"]));
        }
    }

    auto total_time = std::accumulate(time_points.begin(), time_points.end(), .0);
    okec::print("Task completion rate: {:2.0f}%\n", finished / resp.size() * 100);
    okec::print("Total processing time: {:.6f}\n", total_time);
    okec::print("Average processing time: {:.6f}\n", total_time / time_points.size());

    okec::draw(x_points, time_points, "Tasks", "Processing Time(Seconds)");
}

int main()
{
    olog::set_level(olog::level::all);
    okec::simulator sim;

    // Create 1 base station
    okec::base_station_container base_stations(sim, 1);
    // Create 5 edge servers
    okec::edge_device_container edge_servers(sim, 5);
    // Create 2 user devices
    okec::client_device_container user_devices(sim, 2);

    // Connect the base stations and edge servers
    base_stations.connect_device(edge_servers);

    // Set the network model for every device
    okec::multiple_and_single_LAN_WLAN_network_model model;
    okec::network_initializer(model, user_devices, base_stations.get(0));

    // Initialize the resources for each edge server.
    okec::resource_container resources(edge_servers.size());
    resources.initialize([](auto res) {
        res->attribute("cpu", okec::rand_range(2.1, 2.2).to_string());
    });

    // Install each resource on each edge server.
    edge_servers.install_resources(resources);

    // Specify the default offloading strategy
    auto decision_engine = std::make_shared<okec::worst_fit_decision_engine>(&user_devices, &base_stations);
    decision_engine->initialize();


    // Offload tasks
    okec::task t;
    generate_task(t, 5, "1st");
    auto user1 = user_devices.get_device(0);
    co_spawn(sim, offloading(user1, t));


    // Run the simulator
    sim.run();
}
```

Output:
![Response Visualizer](https://github.com/okecsim/okec/raw/main/images/response-visualizer.png)
![Response Draw](https://github.com/okecsim/okec/raw/main/images/response-visualization-demo.png)