```text
   __  __ _  ____  ___ 
  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
 (  O ))  (  ) _)( (__  version 1.0.1
  \__/(__\_)(____)\___) A Realistic, Versatile, and Easily Customizable Edge Computing Simulator
```

[![Build status](https://ci.appveyor.com/api/projects/status/8b08rootot5dfrh2?svg=true)](https://ci.appveyor.com/project/lkimuk/okec)
[![License](https://img.shields.io/github/license/lkimuk/okec.svg)](https://github.com/lkimuk/okec/blob/main/LICENSE)
![Language](https://img.shields.io/github/languages/top/lkimuk/okec.svg)
![Last commit](https://img.shields.io/github/last-commit/lkimuk/okec.svg)

- [Prerequisites](#prerequisites)
- [Install](#install)
- [Features](#features)
- [Examples](#examples)
  - [Create heterogeneous devices with custom resources](#create-heterogeneous-devices-with-custom-resources)
  - [Create Tasks](#create-tasks)
  - [Iterate through tasks](#iterate-through-tasks)
  - [Append attributes to tasks and modify the task attribute values](#append-attributes-to-tasks-and-modify-the-task-attribute-values)

## Prerequisites
|Library|Version|
|---|---|
|[NS-3](https://www.nsnam.org/releases/ns-3-41/)|3.41|
|[libtorch](https://pytorch.org/)|cxx11 ABI|
|[fmtlib](https://github.com/fmtlib/fmt)|N/A|
|[nlohmann\_json](https://github.com/nlohmann/json)|N/A|
|[matplotlib-cpp](https://github.com/lava/matplotlib-cpp)|N/A|

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

### Iterate through tasks
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

    for (auto const& item : t.elements())
    {
        okec::print("task_id: {} ", item.get_header("task_id"));
        okec::print("group: {} ", item.get_header("group"));
        okec::print("cpu: {} ", item.get_header("cpu"));
        okec::print("deadline: {}\n", item.get_header("deadline"));
    }
}
```

The potential output:
```text
task_id: E99E1850C94F616A2E7A2F01FEA4F43 group: dummy cpu: 1.15 deadline: 4
task_id: 2E54F007DCA13C89621C9D554F7203B group: dummy cpu: 1.05 deadline: 4
task_id: B430A892935CE16A2DD458A0E73013E group: dummy cpu: 0.46 deadline: 4
task_id: 8FEE0B90F2ABF3280B573C1517CF487 group: dummy cpu: 0.83 deadline: 2
task_id: CFAF12948696F9AA25D2EA45D99F3CC group: dummy cpu: 0.87 deadline: 4
task_id: B89CF926127909C91641670DCF59E16 group: dummy cpu: 0.87 deadline: 1
task_id: A75B4F6C8828E7B98550FA8E59232FB group: dummy cpu: 0.24 deadline: 1
task_id: A838D2B4E534D9E8556B7F5228A5F0D group: dummy cpu: 0.65 deadline: 4
task_id: 42CC12E43AEF2E9AC50C47E675050C1 group: dummy cpu: 0.77 deadline: 4
task_id: 5BEE99DFF8F012C9FB10DA9E81C5DF9 group: dummy cpu: 0.76 deadline: 3
```

### Append attributes to tasks and modify the task attribute values
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

    okec::print("Before:\n{:t}\n", t);

    for (auto& item : t.elements_view())
    {
        item.set_header("memory", okec::rand_range(10.0, 100.0).to_string());
    }

    t.at(2).set_header("deadline", "20");

    okec::print("After:\n{:t}", t);
}
```

The potential output:
```text
Before:
[  1] cpu: 1.14 deadline: 4 group: dummy task_id: 9AD1D88CE2E541198061EDEEAB10497 
[  2] cpu: 0.91 deadline: 4 group: dummy task_id: 7CABEFFCD3065A384461F000B0225D5 
[  3] cpu: 0.61 deadline: 4 group: dummy task_id: A41D4073CCFD0A0A815FD93D1EBFB85 
[  4] cpu: 0.31 deadline: 2 group: dummy task_id: D83E435EF572337AAED1742252D218B 
[  5] cpu: 0.87 deadline: 4 group: dummy task_id: 50C236D2D6D5EFB863B089CAAFD16DC 
[  6] cpu: 0.99 deadline: 1 group: dummy task_id: 039D54B8EFB551B9AFE1BFD52E2FAD7 
[  7] cpu: 0.41 deadline: 3 group: dummy task_id: F59EC4AF406CC6C9B8E7CED4457324A 
[  8] cpu: 0.24 deadline: 2 group: dummy task_id: E8BFD48C35299B099FC7CE5C9A98784 
[  9] cpu: 1.01 deadline: 1 group: dummy task_id: 1036DE54D22172C9CE3BD8A48353C78 
[ 10] cpu: 1.08 deadline: 3 group: dummy task_id: 4C94DB3A661E42B9597E1E0373B62FE 

After:
[  1] cpu: 1.14 deadline: 4 group: dummy memory: 63.53 task_id: 9AD1D88CE2E541198061EDEEAB10497 
[  2] cpu: 0.91 deadline: 4 group: dummy memory: 17.95 task_id: 7CABEFFCD3065A384461F000B0225D5 
[  3] cpu: 0.61 deadline: 20 group: dummy memory: 51.57 task_id: A41D4073CCFD0A0A815FD93D1EBFB85 
[  4] cpu: 0.31 deadline: 2 group: dummy memory: 39.79 task_id: D83E435EF572337AAED1742252D218B 
[  5] cpu: 0.87 deadline: 4 group: dummy memory: 35.01 task_id: 50C236D2D6D5EFB863B089CAAFD16DC 
[  6] cpu: 0.99 deadline: 1 group: dummy memory: 35.07 task_id: 039D54B8EFB551B9AFE1BFD52E2FAD7 
[  7] cpu: 0.41 deadline: 3 group: dummy memory: 65.44 task_id: F59EC4AF406CC6C9B8E7CED4457324A 
[  8] cpu: 0.24 deadline: 2 group: dummy memory: 78.54 task_id: E8BFD48C35299B099FC7CE5C9A98784 
[  9] cpu: 1.01 deadline: 1 group: dummy memory: 92.91 task_id: 1036DE54D22172C9CE3BD8A48353C78 
[ 10] cpu: 1.08 deadline: 3 group: dummy memory: 92.70 task_id: 4C94DB3A661E42B9597E1E0373B62FE
```
