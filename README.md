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

    // Connect the base station and edge servers
    base_stations.connect_device(edge_servers);

    // Set the network model for every device
    okec::multiple_and_single_LAN_WLAN_network_model model;
    okec::network_initializer(model, user_devices, base_stations.get(0));

    // Initialize the resources for each edge server.
    okec::resource_container resources(edge_servers.size());
    resources.initialize([](auto res) {
        res->attribute("cpu", okec::rand_range<double>(2.1, 2.2).to_string());
    });

    // Print resources
    okec::print("{:rs}", resources);

    // Install each resource on each edge server.
    edge_servers.install_resources(resources);

    // Run the simulator
    sim.run();
}
```

In this trivial example, we create a base station connecting several edge servers. All heterogeneous devices initialize network communication using the `multiple_and_single_LAN_WLAN_network_model`. Additionally, we randomly generate some resources and install them on these edge servers.

### Create Tasks
**Generate tasks randomly**

**Save tasks and Load them from files**
