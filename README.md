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

## Usage samples

### Basic usage

