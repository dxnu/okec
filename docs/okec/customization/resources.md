# Customizing resources

## Custom okec::resource
A single resource can be customized and retrieved using the `attribute()` and `get_value()` methods, respectively.

```cpp
auto res = okec::make_resource();
res->attribute("cpu", okec::rand_range(1.2, 2.4).to_string());

okec::print("resource cpu: {}\n", res->get_value("cpu"));
```

However, this approach is generally not recommended. Typically, you should prefer using the `okec::resource_container` to create and initialize resources.

## Custom okec::resource_container
As the name suggests, `okec::resource_container` is the container version of `okec::resource`. Using it to create tasks is more straightforward.


```cpp
// Create 10 resources
okec::resource_container resources(10);
resources.initialize([](auto res) {
    res->attribute("cpu", okec::rand_range(2.1, 2.2).to_string());
});

okec::print("{:rs}", resources);
```

The output would be:

```text
[ 1] cpu: 2.19 
[ 2] cpu: 2.17 
[ 3] cpu: 2.14 
[ 4] cpu: 2.20 
[ 5] cpu: 2.15 
[ 6] cpu: 2.18 
[ 7] cpu: 2.15 
[ 8] cpu: 2.16 
[ 9] cpu: 2.17 
[10] cpu: 2.18
```

## Resource attributes
Similar to the task type, all attributes in the resource type are customizable, and you should tailor your resource according to the specific offloading algorithms.

Resources can encompass various device properties, such as device memory, price, resource utilization rates, disk size, and more.

Note that the types of all attributes and values are strings.