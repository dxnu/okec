# Task

## Generate tasks randomly

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

    okec::print("{:t}", t);
}
```

The potential output:

```text
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

## Save tasks and Load them from files
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

The potential output:
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

## Iterate through tasks
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

## Append attributes to tasks and modify the task attribute values
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

The potential output:
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