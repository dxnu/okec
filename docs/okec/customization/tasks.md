# Customizing tasks

The okec library can define tasks by setting custom attributes and values.

## Task header and task body
A task consists of a header and a body. The header contains various information required for task offloading, such as cpu demand, memory demand, source address, etc. The body, which is generally optional, contains the actual set of instructions. In most cases, only the task header is necessary.


```cpp
okec::task t;
t.emplace_back({
    { "id", okec::task::unique_id() }
}, {
    { "instructions", "set of instructions" }
});

okec::print("{:t}", t);
```

This will create one task with a header and a body:

```text
[1] id: 14CD07884A6DD4685B00CC9950EDA4F instructions: set of instructions
```

You can retrieve values from the header or body using the `get_header()` and `get_body()` methods.

```cpp
okec::print("id: {}\n", t[0].get_header("id"));
okec::print("instructions: {}\n", t[0].get_body("instructions"));
```

The output would be:

```text
id: 57D01B1F4802E95834B48DE31F27999
instructions: set of instructions
```

As you can see, a task is essentially a container holding multiple task elements. To manipulate any of these elements, you can directly use the syntax `[index]` or the method `at(index)`.

## Task attributes
All attributes in the task header and task body are customizable. You should tailor your task according to the specific offloading algorithms.

```cpp
okec::task t;
t.emplace_back({
    { "id", okec::task::unique_id() },
    { "cpu", okec::rand_range(1.2, 2.4).to_string() },
    { "memory", okec::rand_range(1, 4).to_string() },
    { "deadline", okec::rand_range(2, 5).to_string() },
    { "...", "..." }
});
```

Note that the types of all attributes and values are strings.
