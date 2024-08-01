# Task offloading

## Specify the default offloading strategy
```cpp
#include <okec/okec.hpp>

namespace olog = okec::log;


int main()
{
    olog::set_level(olog::level::debug);
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

    // Run the simulator
    sim.run();
}
```

When your program runs, the decision engine will automatically gather resource information from all edge servers that have installed resources.

```text
[+0.00000000s] █ The decision engine received resource information from edge server(10.1.1.2).
[+0.00000000s] █ The decision engine received resource information from edge server(10.1.2.2).
[+0.00000000s] █ The decision engine received resource information from edge server(10.1.2.3).
[+0.00000000s] █ The decision engine received resource information from edge server(10.1.2.4).
[+0.00000000s] █ The decision engine received resource information from edge server(10.1.2.5).
```

## Asynchronously offload your first set of tasks using the worst-fit decision engine with callbacks
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
    user1->async_send(std::move(t));
    user1->async_read([](auto resp) {
        olog::success("received response.");

        okec::print("{:r}", resp);
    });


    // Run the simulator
    sim.run();
}
```

## Asynchronously offload your first set of tasks using the worst-fit decision engine with coroutines 
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
    olog::debug("offloading begin");

    co_await user->async_send(std::move(t));
    auto resp = co_await user->async_read();
    olog::success("received response.");

    okec::print("{:r}", resp);
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
![offloading-your-first-set-of-tasks-using-the-worst-fit-decision-engine](https://github.com/okecsim/okec/raw/main/images/offloading-your-first-set-of-tasks-using-the-worst-fit-decision-engine.png)

## Discretely offload the task using the DQN decision engine

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
    auto decision_engine = std::make_shared<okec::DQN_decision_engine>(&user_devices, &base_stations);
    decision_engine->initialize();


    // Discretely offload the task using the DQN decision engine.
    okec::task t;
    generate_task(t, 5, "1st");
    int episode = 5;
    decision_engine->train(t, episode);


    // Run the simulator
    sim.run();
}
```

Output:
![discretely-offload-the-task-using-the-dqn-decision-engine](https://github.com/okecsim/okec/raw/main/images/discretely-offload-the-task-using-the-dqn-decision-engine.png)