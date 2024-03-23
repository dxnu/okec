#include "okec.hpp"

void generate_task(okec::task &t, int number, const std::string& group) {
    for (auto _ : std::views::iota(0, number)) {
        t.emplace_back({
            { "task_id", okec::task::get_unique_id() },
            { "group", group },
            { "cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(1.1, 1.2).item<double>()) },
            { "deadline", fmt::format("{:.2f}", torch::rand({1}).uniform_(10, 100).item<double>()) },
        });
    }
}

int main(int argc, char **argv)
{
    okec::simulator simulator(Seconds(10000));

    // Create 1 base station
    okec::base_station_container bs(1);
    // Create 5 edge servers
    okec::edge_device_container edge_servers(8);
    // Create 2 user devices
    okec::client_device_container user_devices(2);

    // Connect the bs and edge servers
    bs.connect_device(edge_servers);

    // Set the network model for every device
    okec::multiple_and_single_LAN_WLAN_network_model model;
    okec::network_initializer(model, user_devices, bs.get(0));

    // Initialize the resources for each edge server.
    okec::resource_container edge_resources(edge_servers.size());
    edge_resources.initialize([](auto res) {
        res->attribute("cpu", fmt::format("{:.2f}", torch::rand({1}).uniform_(2.4, 2.5).item<double>()));
    });
    edge_resources.print();

    // Install each resource on each edge server.
    edge_servers.install_resources(edge_resources);

    // Set decision engine
    auto decision_engine = std::make_shared<okec::scene1_decision_engine>(&user_devices, &bs);
    decision_engine->initialize();

    std::vector<std::string> groups = { "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten",
          "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen", "twenty",
          "twenty-one", "twenty-two", "twenty-three", "twenty-four", "twenty-five", "twenty-six", "twenty-seven", "twenty-eight", "twenty-nine", "thirty" };

    // Initialize a task
    std::vector<okec::task> tasks(10);
    std::vector<double> time_total_points;
    std::vector<double> time_average_points;
    std::vector<int> x_points;

    // Client request someone to handle the task.
    auto user = user_devices.get_device(0);
    user->when_done([&time_total_points, &time_average_points, &x_points, task_size = tasks.size()](okec::response response) {
        fmt::print("{0:=^{1}}\n", "Response Info", 180);
        double finished = 0;
        int index = 1;
        std::vector<double> time_points;
        for (const auto& item : response.data()) {
            fmt::print("[{:>3}] ", index++);
            fmt::print("task_id: {}, device_type: {:>5}, device_address: {:>10}, group: {}, time_consuming: {:>11}s, finished: {}\n",
                item["task_id"], item["device_type"], item["device_address"], item["group"], item["time_consuming"], item["finished"]);
            if (item["finished"] == "Y") {
                finished++;
                time_points.push_back(TO_DOUBLE(item["time_consuming"]));
            }
        }
        
        auto total_time = std::accumulate(time_points.begin(), time_points.end(), .0);
        fmt::print("Task completion rate: {:2.0f}%\n", finished / response.size() * 100);
        fmt::print("Total processing time: {:.9f}s\n", total_time);
        fmt::print("Average processing time: {:.9f}s\n", total_time / time_points.size());

        fmt::print("{0:=^{1}}\n", "", 180);

        // okec::draw(time_points, "Time Comsumption(Seconds)");
        time_total_points.push_back(total_time);
        time_average_points.push_back(total_time / time_points.size());

        if (time_total_points.size() == task_size) {
            fmt::print("time_total_points: {}\n", time_total_points);
            fmt::print("time_average_points: {}\n", time_average_points);
            fmt::print("x_points: {}\n", x_points);

            okec::draw(x_points, time_total_points, "Number of tasks", "Total Processing Time(Seconds)");
            okec::draw(x_points, time_average_points, "Number of tasks", "Average Processing Time(Seconds)");
        }
    });


    int base = 50;
    for (auto i = 0uz; i < tasks.size(); ++i, base += 50) {
        generate_task(tasks[i], base, groups[i]);
        x_points.push_back(tasks[i].size());
        user->send(tasks[i]);
    }

    simulator.run();
}