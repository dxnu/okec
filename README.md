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
### Create Heterogeneous devices

### Create Tasks
**Generate tasks randomly**
**Generate tasks from files**
