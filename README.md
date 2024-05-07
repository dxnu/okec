```text
   __  __ _  ____  ___ 
  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
 (  O ))  (  ) _)( (__  version 1.0.1
  \__/(__\_)(____)\___) A Realistic, Versatile, and Easily Customizable Edge Computing Simulator
```

## Install
```console
$ git clone https://github.com/lkimuk/okec.git
$ cd okec
$ cmake -S . -B build -DCMAKE_PREFIX_PATH:STRING=/absolute/path/to/your/libtorch
$ cmake --build build
$ cmake --install ./build
```

**Run examples**
```console
$ cd examples
$ cmake -S . -B build
$ cmake --build build
$ ./wf-async
```

## Features

- Dynamic network modeling.
- Mobility.
- Multi-MEC architectures.
- Dynamic Task/Resource attributes.
- Resource monitoring.
- Device interaction.
- Decision engine
  - Non-maching learning based offloading algorithms.
  - Maching learning based offloading algorithms.
- Linear/Discrete simulation.
- Network topology visualization.
- Results visualization.
- Multi-layer scenarios.
- Interated datasets.
- ...

## Examples
### Create heterogeneous devices with custom resources
In this trivial example, we create a base station connecting several edge servers. All heterogeneous devices initialize network communication using the `multiple_and_single_LAN_WLAN_network_model`. Additionally, we randomly generate some resources and install them on these edge servers.

```cpp
#include <okec/okec.hpp>


int main()
{
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

    // Print resources
    okec::print("{:rs}", resources);

    // Install each resource on each edge server.
    edge_servers.install_resources(resources);

    // Run the simulator
    sim.run();
}
```

### Create Tasks
**Generate tasks randomly**
```cpp
#include <okec/okec.hpp>

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
    okec::task t;
    generate_task(t, 10, "dummy");

    okec::print("{:t}", t);
}
```

The potential output:
```text
[  1] cpu: 1.02 deadline: 3 group: dummy task_id: CC5855F2FB5922492B34F37B994CD5D
[  2] cpu: 1.14 deadline: 4 group: dummy task_id: 13E009115B674D4A50DD60CE847DFC2
[  3] cpu: 0.76 deadline: 4 group: dummy task_id: 1F276AAFF89D17AAE219E987DA7998A
[  4] cpu: 0.34 deadline: 1 group: dummy task_id: 5D3DDEB066A080C9AC3A9B9DF752A91
[  5] cpu: 0.81 deadline: 2 group: dummy task_id: 3FC95B4E167FEF99957143D35D21463
[  6] cpu: 1.00 deadline: 4 group: dummy task_id: 82D8F41470CD15DB51D28E1D3A859AF
[  7] cpu: 1.03 deadline: 3 group: dummy task_id: 4273D1952CB9B0099D36904978E9B28
[  8] cpu: 0.74 deadline: 3 group: dummy task_id: 8A201C54390FD95BA6A467F1426048B
[  9] cpu: 0.29 deadline: 3 group: dummy task_id: F3FD22F2A9196DE93915ACEF0A612FA
[ 10] cpu: 0.52 deadline: 4 group: dummy task_id: 072EA1D4AB870B2B15ABCC5DE036FBE
```

**Save tasks and Load them from files**
```cpp
#include <okec/okec.hpp>

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
    okec::task t1;
    generate_task(t1, 5, "dummy");
    t1.save_to_file("task.json");

    okec::task t2;
    t2.load_from_file("task.json");

    okec::print("t1:\n{:t}\n", t1);
    okec::print("t2:\n{:t}", t2);
}
```

The potential output:
```text
t1:
[  1] cpu: 0.94 deadline: 3 group: dummy task_id: C8487480083EF6FA51A07F6786112B2
[  2] cpu: 0.87 deadline: 3 group: dummy task_id: AA6E00D9D1D676999810EC793D4091F
[  3] cpu: 0.70 deadline: 3 group: dummy task_id: D7C9B93D88725A6B0A7F24C8D5576E9
[  4] cpu: 0.27 deadline: 4 group: dummy task_id: D8A72C409FC97BD9B8C6478CD69D23A
[  5] cpu: 0.30 deadline: 3 group: dummy task_id: 05019AFC1C46E9581755D2B819B5092

t2:
[  1] cpu: 0.94 deadline: 3 group: dummy task_id: C8487480083EF6FA51A07F6786112B2
[  2] cpu: 0.87 deadline: 3 group: dummy task_id: AA6E00D9D1D676999810EC793D4091F
[  3] cpu: 0.70 deadline: 3 group: dummy task_id: D7C9B93D88725A6B0A7F24C8D5576E9
[  4] cpu: 0.27 deadline: 4 group: dummy task_id: D8A72C409FC97BD9B8C6478CD69D23A
[  5] cpu: 0.30 deadline: 3 group: dummy task_id: 05019AFC1C46E9581755D2B819B5092
```
