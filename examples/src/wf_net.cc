#include <okec/okec.hpp>

void generate_task(okec::task &t, int number, const std::string& group) {
    for ([[maybe_unused]] auto _ : std::views::iota(0, number)) {
        t.emplace_back({
            { "task_id", okec::task::unique_id() },
            { "group", group },
            { "cpu", okec::rand_range(0.2, 1.2).to_string() },
            { "deadline", okec::rand_range(10, 100).to_string() }
        });
    }

    // t.save_to_file("task-" + std::to_string(number) + ".json");
    // t.load_from_file("data/task-" + std::to_string(number) + ".json");
}

void my_monitor(std::string_view address, std::string_view attr, std::string_view old_val, std::string_view new_val) {
    static std::ofstream file;
    if (!file.is_open()) {
        file.open("my_monitor.csv", std::ios::out/* | std::ios::app*/);
        if (!file.is_open()) {
            return;
        }
    }

    file << okec::format("At time {:.2f}s,{},{},{}\n", ns3::Simulator::Now().GetSeconds(), address, old_val, new_val);
}

int main(int argc, char **argv)
{
    okec::log::set_level(okec::log::level::debug | okec::log::level::success);
    okec::log::set_level(okec::log::level::debug, false);
    okec::log::set_level(okec::log::level::info);

    okec::simulator sim;
    sim.enable_visualizer();

    // Create 1 base station
    okec::base_station_container bs(sim, 1);
    // Create 5 edge servers
    okec::edge_device_container edge_servers(sim, 5);
    // Create 2 user devices
    okec::client_device_container user_devices(sim, 2);

    // Connect the bs and edge servers
    bs.connect_device(edge_servers);

    // Set the network model for every device
    okec::multiple_and_single_LAN_WLAN_network_model model;
    okec::network_initializer(model, user_devices, bs.get(0));

    // Initialize the resources for each edge server.
    okec::resource_container edge_resources(edge_servers.size());
    edge_resources.initialize([](auto res) {
        res->attribute("cpu", okec::format("{:.2f}", torch::rand({1}).uniform_(2.1, 2.2).item<double>()));
    });
    // edge_resources.save_to_file("resource-" + std::to_string(edge_resources.size()) + ".json");
    // edge_resources.load_from_file("data/resource-" + std::to_string(edge_resources.size()) + ".json");
    
    // edge_resources.print();

    // Install each resource on each edge server.
    edge_servers.install_resources(edge_resources);

    edge_resources.trace_resource(); // 先捕捉初始值
    edge_resources.set_monitor([&edge_resources](std::string_view address, std::string_view attr, std::string_view old_val, std::string_view new_val) {
        // edge_resources.print();
        edge_resources.trace_resource();
    });

    // Set decision engine
    auto decision_engine = std::make_shared<okec::worst_fit_decision_engine>(&user_devices, &bs);
    decision_engine->initialize();

    std::vector<std::string> groups = { "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten",
          "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen", "twenty",
          "twenty-one", "twenty-two", "twenty-three", "twenty-four", "twenty-five", "twenty-six", "twenty-seven", "twenty-eight", "twenty-nine", "thirty" };

    // Initialize a task
    std::vector<okec::task> tasks(1); // 10 batch of tasks
    std::vector<double> time_total_points;
    std::vector<double> time_average_points;
    std::vector<int> x_points;

    // Client request someone to handle the task.
    auto user = user_devices.get_device(0);
    user->async_read([&time_total_points, &time_average_points, &x_points, task_size = tasks.size()](okec::response response) {
        okec::print("{0:=^{1}}\n", "Response Info", 180);
        double finished = 0;
        int index = 1;
        std::vector<double> time_points;
        for (const auto& item : response.data()) {
            okec::print("[{:>3}] ", index++);
            okec::print("task_id: {}, device_type: {:>5}, device_address: {:>10}, group: {}, time_consuming: {:>11}s, finished: {}\n",
                TO_STR(item["task_id"]), TO_STR(item["device_type"]), TO_STR(item["device_address"]), TO_STR(item["group"]), TO_STR(item["time_consuming"]), TO_STR(item["finished"]));
            if (item["finished"] == "Y") {
                finished++;
                time_points.push_back(TO_DOUBLE(item["time_consuming"]));
            }
        }
        
        auto total_time = std::accumulate(time_points.begin(), time_points.end(), .0);
        okec::print("Task completion rate: {:2.0f}%\n", finished / response.size() * 100);
        okec::print("Total processing time: {:.6f}\n", total_time);
        okec::print("Average processing time: {:.6f}\n", total_time / time_points.size());

        okec::print("{0:=^{1}}\n", "", 180);

        // okec::draw(time_points, "Time Comsumption(Seconds)");
        time_total_points.push_back(total_time);
        time_average_points.push_back(total_time / time_points.size());

        if (time_total_points.size() == task_size) {
            // std::format doesn't format ranges in C++20.
            // Comment below 3 lines for compiling.
            // okec::print("time_total_points: {}\n", time_total_points);
            // okec::print("time_average_points: {}\n", time_average_points);
            // okec::print("x_points: {}\n", x_points);


            // okec::draw(x_points, time_total_points, "Number of tasks", "Total Processing Time(Seconds)");
            // okec::draw(x_points, time_average_points, "Number of tasks", "Average Processing Time(Seconds)");
        }
    });

    // int step = 50;
    // for (auto i = 0uz; i < tasks.size(); ++i, step += 50) {
    //     generate_task(tasks[i], step, groups[i]);
    //     x_points.push_back(tasks[i].size());
    //     user->send(tasks[i]);
    // }
    // int step = 50;
    // int index = step / 50 - 1;
    // generate_task(tasks[0], step, groups[index]);
    // x_points.push_back(tasks[0].size());
    // user->send(tasks[0]);

    // !!! 目前记得修改 worse_fit 267 行的任务数，以保证执行效果不受网络影响
    generate_task(tasks[0], 50, "dummy");
    x_points.push_back(tasks[0].size());
    user->send(tasks[0]);

    okec::task t2; // 10 batch of tasks
    generate_task(t2, 3, "2nd");
    user->send(t2);
    

    sim.run();
}