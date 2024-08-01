# Customizing responses

The response type is very similar to the task type. Here's an example:

```cpp
// Create responses
okec::response resp;
resp.emplace_back({
    { "time_consuming", okec::rand_value<float>().to_string() },
    { "finished", "Y" }
});

okec::print("{:r}", resp);
```

The output would be:

```text
[1] finished: Y time_consuming: 0.27
```