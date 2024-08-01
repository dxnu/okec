# Log

This logging module is inspired by [Stargirl](https://x.com/theavalkyrie/status/1768787170137940141).

```cpp
#include <okec/okec.hpp>

namespace olog =  okec::log;

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
    olog::set_level(olog::level::all);

    olog::debug("this is a debug message");
    olog::info("this is a info message");
    olog::warning("watch out, this is a warning message");
    olog::success("oh nice, this one is success");
    olog::error("oops, this one is an error");


    olog::info("{0:-^{1}}", "", okec::get_winsize().col - olog::indent_size());

    // Print tasks
    okec::task t;
    generate_task(t, 5, "dummy");
    okec::print("task:\n{:t}", t);

    olog::info("{0:-^{1}}", "", okec::get_winsize().col - olog::indent_size());

    // Print resources
    okec::resource_container resources(5);
    resources.initialize([](auto res) {
        res->attribute("cpu", okec::rand_range(2.1, 2.2).to_string());
        res->attribute("memory", okec::rand_range(1, 4).to_string());
    });
    okec::print("resource:\n{:rs}", resources);

    olog::info("{0:-^{1}}", "", okec::get_winsize().col - olog::indent_size());
}
```

Output:
![Log](https://github.com/okecsim/okec/raw/main/images/log.png)