# okec::task

*Defined in header `<okec/common/task.h>`*<br>
*Defined in namespace okec*

```cpp
class task : public ns3::SimpleRefCount<task> {};
```

## Member types

|Member type|Definition|
|----|----|
|attribute_t<br><span style="color: green">(private)</span>|`#!cpp std::pair<std::string, std::string>`|
|attributes_t<br><span style="color: green">(private)</span>|`#!cpp std::initializer_list<attribute_t>`|
|task_header<br><span style="color: green">(public)</span>|`#!cpp attributes_t`|
|task_body<br><span style="color: green">(public)</span>|`#!cpp attributes_t`|

## Member functions
|||
|-------------|--------|
|[(constructor)](#tasktask)|constructs a task object<br><span style="color: green">(public member function)</span>|
|(destructor)<span style="color: green">(implicitly declared)</span>|destructs a task<br><span style="color: green">(public member function)|
|[from_packet](#taskfrom_packet)|constructs a task object from a packet<br><span style="color: green">(public static member function)|
|[from_msg_packet](#taskfrom_msg_packet)|constructs a task object from a message packet.<br><span style="color: green">(public static member function)|
|[emplace_back](#taskemplace_back)|insert a new task into the task object<br><span style="color: green">(public member function)|
|[dump](#taskdump)|dump the task into a string<br><span style="color: green">(public member function)|
|[elements_view](#taskelements_view)|accesses the task elements through views<br><span style="color: green">(public member function)|
|[elements](#taskelements)|safely accesses the task elements through a copy<br><span style="color: green">(public member function)|
|[at](#taskat)|retrieve the task element at the specified index<br><span style="color: green">(public member function)|
|[data](#taskdata)|gets the task data<br><span style="color: green">(public member function)|
|[j_data](#taskj_data)|`data` returns only the task elements, whereas `j_data` returns the original JSON data of the task<br><span style="color: green">(public member function)|
|[is_null](#taskis_null)|checks if the task is null<br><span style="color: green">(public member function)|
|[size](#tasksize)|the size of the task<br><span style="color: green">(public member function)|
|[empty](#taskempty)|checks if the task is empty<br><span style="color: green">(public member function)|
|[set_if](#taskset_if)|if the attributes of the task meet the specified criteria, set the task information<br><span style="color: green">(public member function)|
|[unique_id](#taskunique_id)|generates a unique task id<br><span style="color: green">(public member function)|
|[save_to_file](#tasksave_to_file)|saves task to a file<br><span style="color: green">(public member function)|
|[load_from_file](#taskload_from_file)|loads task from a file<br><span style="color: green">(public member function)|


### task::task

|||
|----|----|
|`#!cpp task() = default;`|(1)|
|`#!cpp task(json other);`|(2)|

Constructs a task object.

- 1) default constructor.
- 2) Constructs a task object from a valid json file.

#### Parameters
- **other**: a valid json file to copy from

### task::from_packet
||
|----|
|`#!cpp static auto from_packet(ns3::Ptr<ns3::Packet> packet) -> task;`|
||

Constructs a task object from a ns3 packet.

#### Parameters
- **packet**: a valid task packet

#### Return value
A task object that contains some tasks.

### task::from_msg_packet
||
|----|
|`#!cpp auto task::from_msg_packet(ns3::Ptr<ns3::Packet> packet) -> task`|
||

Constructs a task object from a message packet.

#### Parameters
- **packet**: a valid task packet with a message wrapper

#### Return value
A task object that contains some tasks.

### task::emplace_back
||
|----|
|`#!cpp auto emplace_back(task_header_t header_attrs, task_body_t body_attrs = {}) -> void;`|
||

Insert a new task into the task object. 

#### Parameters
- **header_attrs**: task header attributes
- **body_attrs**: task body attributes

### task::dump
||
|----|
|`#!cpp auto dump(int indent = -1) const -> std::string;`|
||

Dump the task into a string.

#### Parameters
- **indent**: the indentation level of the exported content

#### Return value
A string containing task information in JSON format.

### task::elements_view
||
|----|
|`#!cpp auto elements_view() -> std::vector<task_element>;`|
||

Accesses the task elements through views.

#### Return value
A list of element views for accessing and modifying task information.

### task::elements
||
|----|
|`#!cpp auto elements() const -> std::vector<task_element>;`|
||

Safely accesses the task elements through a copy.

#### Return value
A list of task elements for accessing task information.

### task::at
||
|----|
|`#!cpp auto at(std::size_t index) noexcept -> task_element;`|
||

Retrieve the task element at the specified index.

#### Parameters
- **index**: The index of the task element to retrieve

#### Return value
The task element located at the given index.

### task::data
||
|----|
|`#!cpp auto data() const -> json;`|
||

Gets the task data.

#### Return value
Task data formatted in JSON.

### task::j_data
||
|----|
|`#!cpp auto j_data() const -> json;`|
||

`task::data` returns only the task elements, whereas `task::j_data` returns the original JSON data of the task.

### task::is_null
||
|----|
|`#!cpp auto is_null() const -> bool;`|
||

Checks if the task is null.

### task::size
||
|----|
|`#!cpp auto size() const -> std::size_t;`|
||

The size of the task.

### task::empty
||
|----|
|`#!cpp auto empty() -> bool;`|
||

Checks if the task is empty.

### task::set_if
||
|----|
|`#!cpp auto set_if(attributes_t values, auto f) -> void;`|
||

If the attributes of the task meet the specified criteria, set the task information.

### task::unique_id
||
|----|
|`#!cpp static auto unique_id() -> std::string;`|
||

Generates a unique task id.

### task::save_to_file
||
|----|
|`#!cpp auto save_to_file(const std::string& file_name) -> void;`|
||

### task::load_from_file
||
|----|
|`#!cpp auto load_from_file(const std::string& file_name) -> bool;`|
||

## Example
### Generate tasks randomly
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

Output:

```cpp
[ 1] cpu: 1.02 deadline: 3 group: dummy task_id: CC5855F2FB5922492B34F37B994CD5D
[ 2] cpu: 1.14 deadline: 4 group: dummy task_id: 13E009115B674D4A50DD60CE847DFC2
[ 3] cpu: 0.76 deadline: 4 group: dummy task_id: 1F276AAFF89D17AAE219E987DA7998A
[ 4] cpu: 0.34 deadline: 1 group: dummy task_id: 5D3DDEB066A080C9AC3A9B9DF752A91
[ 5] cpu: 0.81 deadline: 2 group: dummy task_id: 3FC95B4E167FEF99957143D35D21463
[ 6] cpu: 1.00 deadline: 4 group: dummy task_id: 82D8F41470CD15DB51D28E1D3A859AF
[ 7] cpu: 1.03 deadline: 3 group: dummy task_id: 4273D1952CB9B0099D36904978E9B28
[ 8] cpu: 0.74 deadline: 3 group: dummy task_id: 8A201C54390FD95BA6A467F1426048B
[ 9] cpu: 0.29 deadline: 3 group: dummy task_id: F3FD22F2A9196DE93915ACEF0A612FA
[10] cpu: 0.52 deadline: 4 group: dummy task_id: 072EA1D4AB870B2B15ABCC5DE036FBE
```

### Save tasks and Load them from files
```cpp
#include <okec/okec.hpp>

void generate_task(okec::task& t, int number, std::string const& group)
{
    for (auto i = number; i-- > 0;)
    {
        t.emplace_back({
            { "task_id", okec::task::unique_id() },
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

Output:
```text
t1:
[1] cpu: 0.94 deadline: 3 group: dummy task_id: C8487480083EF6FA51A07F6786112B2
[2] cpu: 0.87 deadline: 3 group: dummy task_id: AA6E00D9D1D676999810EC793D4091F
[3] cpu: 0.70 deadline: 3 group: dummy task_id: D7C9B93D88725A6B0A7F24C8D5576E9
[4] cpu: 0.27 deadline: 4 group: dummy task_id: D8A72C409FC97BD9B8C6478CD69D23A
[5] cpu: 0.30 deadline: 3 group: dummy task_id: 05019AFC1C46E9581755D2B819B5092

t2:
[1] cpu: 0.94 deadline: 3 group: dummy task_id: C8487480083EF6FA51A07F6786112B2
[2] cpu: 0.87 deadline: 3 group: dummy task_id: AA6E00D9D1D676999810EC793D4091F
[3] cpu: 0.70 deadline: 3 group: dummy task_id: D7C9B93D88725A6B0A7F24C8D5576E9
[4] cpu: 0.27 deadline: 4 group: dummy task_id: D8A72C409FC97BD9B8C6478CD69D23A
[5] cpu: 0.30 deadline: 3 group: dummy task_id: 05019AFC1C46E9581755D2B819B5092
```

### Iterate through tasks
```cpp
#include <okec/okec.hpp>

void generate_task(okec::task& t, int number, std::string const& group)
{
    for (auto i = number; i-- > 0;)
    {
        t.emplace_back({
            { "task_id", okec::task::unique_id() },
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

Output:
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
            { "task_id", okec::task::unique_id() },
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

Output:
```text
Before:
[ 1] cpu: 0.91 deadline: 2 group: dummy task_id: D759A6B6161BCE3B25C7AC79064D082
[ 2] cpu: 0.36 deadline: 1 group: dummy task_id: F058239672AD6108DF9CFBDA74B5628
[ 3] cpu: 0.27 deadline: 2 group: dummy task_id: 109DC14559C70AB880E2AE44588E2B6
[ 4] cpu: 1.05 deadline: 2 group: dummy task_id: 1D55F6D4EC004FBB0D449EF3FC325A2
[ 5] cpu: 0.45 deadline: 4 group: dummy task_id: 5FC152A187472BCB51FEC24C1B34808
[ 6] cpu: 1.03 deadline: 4 group: dummy task_id: 3811B94C6FB3E20817FA648746EEFB7
[ 7] cpu: 0.43 deadline: 4 group: dummy task_id: 009A7DA8A7E0643B84391C790D0562B
[ 8] cpu: 0.74 deadline: 2 group: dummy task_id: 486E9DD50B5AE6BA4DB76CB6CCAD057
[ 9] cpu: 0.93 deadline: 3 group: dummy task_id: 770DE1C994C61ACBAAF8C708C0A90D8
[10] cpu: 0.89 deadline: 1 group: dummy task_id: 026D7FF78ADDEC098EF62A6316DD75C

After:
[ 1] cpu: 0.91 deadline: 2 group: dummy memory: 12.29 task_id: D759A6B6161BCE3B25C7AC79064D082
[ 2] cpu: 0.36 deadline: 1 group: dummy memory: 47.81 task_id: F058239672AD6108DF9CFBDA74B5628
[ 3] cpu: 0.27 deadline: 20 group: dummy memory: 99.64 task_id: 109DC14559C70AB880E2AE44588E2B6
[ 4] cpu: 1.05 deadline: 2 group: dummy memory: 17.39 task_id: 1D55F6D4EC004FBB0D449EF3FC325A2
[ 5] cpu: 0.45 deadline: 4 group: dummy memory: 90.99 task_id: 5FC152A187472BCB51FEC24C1B34808
[ 6] cpu: 1.03 deadline: 4 group: dummy memory: 45.24 task_id: 3811B94C6FB3E20817FA648746EEFB7
[ 7] cpu: 0.43 deadline: 4 group: dummy memory: 41.11 task_id: 009A7DA8A7E0643B84391C790D0562B
[ 8] cpu: 0.74 deadline: 2 group: dummy memory: 84.72 task_id: 486E9DD50B5AE6BA4DB76CB6CCAD057
[ 9] cpu: 0.93 deadline: 3 group: dummy memory: 82.40 task_id: 770DE1C994C61ACBAAF8C708C0A90D8
[10] cpu: 0.89 deadline: 1 group: dummy memory: 37.67 task_id: 026D7FF78ADDEC098EF62A6316DD75C
```