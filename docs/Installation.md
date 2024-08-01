#Installation
## Prerequisites

|Library|Version|Compiler|Version|Language|Version|
|---|---|---|---|---|---|
|[NS-3](https://www.nsnam.org/releases/ns-3-41/)|3.41|GCC|13.0 above|C++|23|
|[libtorch](https://pytorch.org/)|cxx11 ABI|Clang|N/A|||
|[fmtlib](https://github.com/fmtlib/fmt)|N/A|MSVC|N/A|||
|[nlohmann\_json](https://github.com/nlohmann/json)|N/A|||||
|[matplotlib-cpp](https://github.com/lava/matplotlib-cpp)|N/A|||||

## Install

```console
$ git clone https://github.com/lkimuk/okec.git
$ cd okec
$ cmake -S . -B build -DCMAKE_PREFIX_PATH:STRING=/absolute/path/to/your/libtorch
$ cmake --build build
$ cmake --install ./build
```

If your prerequisite libraries are not installed in standard directories, you may need to specify multiple paths as follows:

```console hl_lines="3"
$ git clone https://github.com/lkimuk/okec.git
$ cd okec
$ cmake -S . -B build -DCMAKE_PREFIX_PATH:STRING="/absolute/path/to/your/libtorch;/absolute/path/to/your/other/libraries"
$ cmake --build build
$ cmake --install ./build
```

## Run examples

```console
$ cd examples
$ cmake -S . -B build
$ cmake --build build
$ ./wf-async
$ ./wf_discrete
$ ./wf_net
$ ./rf_discrete
```