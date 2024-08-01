#task::task

|||
|----|----|
|`#!cpp task() = default;`|(1)|
|`#!cpp task(json other);`|(2)|

Constructs a task object.

- 1) default constructor.
- 2) Constructs a task object from a valid json file.

## Parameters
- **other**: a valid json file to copy from

## Example
```cpp
#include <okec/okec.hpp>

int main() {
    okec::task t;
}
```