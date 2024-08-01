# Simulation time

*Defined in header `<okec/common/simulator.h>`*<br>
*Defined in namespace okec::now*

|||
|-------------|--------|
|[years](#years)|returns a year in the simulation time<br><span style="color: green">(function)</span>|
|[days](#days)|returns a day of a month in the simulation time<br><span style="color: green">(function)</span>|
|[hours](#hours)|returns an hour of a day in the simulation time<br><span style="color: green">(function)</span>|
|[minutes](#minutes)|returns a minute in the simulation time<br><span style="color: green">(function)</span>|
|[seconds](#seconds)|returns a second in the simulation time<br><span style="color: green">(function)</span>|
|[milli_seconds](#milli_seconds)|returns a millisecond in the simulation time<br><span style="color: green">(function)</span>|
|[micro_seconds](#micro_seconds)|returns a microsecond in the simulation time<br><span style="color: green">(function)</span>|
|[nano_seconds](#nano_seconds)|returns a nanosecond in the simulation time<br><span style="color: green">(function)</span>|
|[pico_seconds](#pico_seconds)|returns a picosecond in the simulation time<br><span style="color: green">(function)</span>|
|[femto_seconds](#femto_seconds)|returns a femtosecond in the simulation time<br><span style="color: green">(function)</span>|

## Functions

### years

```cpp
inline auto years() -> double;
```

### days
```cpp
inline auto days() -> double;
```

### hours
```cpp
inline auto hours() -> double;
```

### minutes
```cpp
inline auto minutes() -> double;
```

### seconds
```cpp
inline auto seconds() -> double;
```

### milli_seconds
```cpp
inline auto milli_seconds() -> double;
```

### micro_seconds
```cpp
inline auto micro_seconds() -> double;
```

### nano_seconds
```cpp
inline auto nano_seconds() -> double;
```

### pico_seconds
```cpp
inline auto pico_seconds() -> double;
```

### femto_seconds
```cpp
inline auto femto_seconds() -> double;
```

## Example
```cpp
#include <okec/okec.hpp>


int main() {
    okec::print("years: {}\n", okec::now::years());
    okec::print("days: {}\n", okec::now::days());
    okec::print("hours: {}\n", okec::now::hours());
    okec::print("minutes: {}\n", okec::now::minutes());
    okec::print("seconds: {}\n", okec::now::seconds());
    okec::print("milli_seconds: {}\n", okec::now::milli_seconds());
    okec::print("micro_seconds: {}\n", okec::now::micro_seconds());
    okec::print("nano_seconds: {}\n", okec::now::nano_seconds());
    okec::print("pico_seconds: {}\n", okec::now::pico_seconds());
    okec::print("femto_seconds: {}\n", okec::now::femto_seconds());
}
```

Output:
```text
years: 0
days: 0
hours: 0
minutes: 0
seconds: 0
milli_seconds: 0
micro_seconds: 0
nano_seconds: 0
pico_seconds: 0
femto_seconds: 0
```