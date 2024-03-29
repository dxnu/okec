#ifndef DQN_DECISION_ENGINE_H_
#define DQN_DECISION_ENGINE_H_

#include "decision_engine.h"
#include "RL_brain.hpp"
#include "format_helper.hpp"


namespace okec
{

class client_device;
class client_device_container;
class edge_device;


class Env : public std::enable_shared_from_this<Env> {
    using this_type = Env;

public:
    Env(const device_cache& cache, const task& t)
        : cache_(cache)
        , t_(t)
    {
        // Save the initialization state.
        state_.reserve(cache.size());
        for (auto it = cache.cbegin(); it != cache.cend(); ++it)
            state_.push_back(TO_DOUBLE((*it)["cpu"])); // edge resources
        
        // for (const auto& item : t.elements())
        //     state_.push_back(std::stod(item.get_header("cpu"))); // tasks   
    }

    auto n_actions() const -> std::size_t {
        return cache_.size();
    }

    auto n_features() const -> std::size_t {
        return cache_.size(); // + t_.size();
    }

    auto reset() -> torch::Tensor {
        // torch::tensor makes a copy, from_blob does not (but torch::from_blob(vector).clone() does)
        auto observation = torch::from_blob(state_.data(), {static_cast<long>(state_.size())}, torch::kFloat64);
        return observation.unsqueeze(0);
    }

    auto step2(int action) {
        static std::size_t task_index = 0;

        auto& edge_cache = cache_.view();
        auto& server = edge_cache.at(action);
        auto t = t_.at(task_index);
        auto cpu_supply = TO_DOUBLE(server["cpu"]);
        auto cpu_demand = std::stod(t.get_header("cpu"));

        fmt::print("server: {}\n", edge_cache[action].dump());
        fmt::print("task: {}\n", t.dump());
        fmt::print("cpu_supply: {}, cpu_demand: {}\n", cpu_supply, cpu_demand);

        float reward;
        if (cpu_supply < cpu_demand) {
            t.set_header("status", "N");
            reward = -1.0f;
        } else {
            t.set_header("status", "Y");
            reward = 1.0f;
            server["cpu"] = std::to_string(cpu_supply - cpu_demand);
            fmt::print(fg(fmt::color::red), "[{}] 消耗资源：{} --> {}\n", TO_STR(server["ip"]), cpu_supply, TO_DOUBLE(server["cpu"]));
        }

        bool done;
        if (task_index++ == t_.size() - 1) {
            done = true;
        } else {
            done = false;
        }

        std::vector<double> flattened_state;
        for (const auto& edge : edge_cache) {
            flattened_state.push_back(TO_DOUBLE(edge["cpu"]));
        }

        auto new_state = torch::tensor(flattened_state, torch::dtype(torch::kFloat64)).unsqueeze(0);

        return std::make_tuple(new_state, reward, done);
    }

    auto step(int action) {
        static std::size_t task_index = 0;

        auto& edge_cache = cache_.view();
        auto& server = edge_cache.at(action);
        auto t = t_.at(task_index);
        auto cpu_supply = TO_DOUBLE(server["cpu"]);
        auto cpu_demand = std::stod(t.get_header("cpu"));

        fmt::print("server: {}\n", edge_cache[action].dump());
        double available_cpu = TO_DOUBLE(server["cpu"]);
        fmt::print("available_cpu: {}\n", available_cpu);
        fmt::print("task: {}\n", t.dump());
        fmt::print("cpu_supply: {}, cpu_demand: {}\n", cpu_supply, cpu_demand);

        float reward;
        bool handled = false;
        if (cpu_supply < cpu_demand) {
            reward = -1.0f;
        } else {
            reward = 1.0f;
            handled = true;
            server["cpu"] = std::to_string(cpu_supply - cpu_demand);
            fmt::print("new cpu: {}\n", TO_DOUBLE(edge_cache.at(action)["cpu"]));
            fmt::print(fg(fmt::color::red), "[{}] 消耗资源：{} --> {}\n", TO_STR(server["ip"]), cpu_supply, TO_DOUBLE(server["cpu"]));
            
            double processing_time = cpu_demand / cpu_supply;
            // auto self = shared_from_this();
            // Simulator::Schedule(Seconds(processing_time), [self, action, cpu_demand]() {
            //     auto& edge_cache = self->cache_.view();
            //     auto& server = edge_cache.at(action);
            //     double cur_cpu = TO_DOUBLE(server["cpu"]);
            //     double new_cpu = cur_cpu + cpu_demand;
            //     print_info(fmt::format("[{}] 恢复资源：{} --> {:.2f}(demand: {})\n", TO_STR(server["ip"]), cur_cpu, new_cpu, cpu_demand));
            //     server["cpu"] = std::to_string(cur_cpu + cpu_demand);
            // });
        }

        bool done;
        if (handled) { // 处理成功
            if (task_index == t_.size() - 1) {
                task_index = 0;
                done = true;
            } else {
                task_index++;
                done = false;
            }
        } else { // 处理失败
            done = false;
        }

        std::vector<double> flattened_state;
        for (const auto& edge : edge_cache) {
            flattened_state.push_back(TO_DOUBLE(edge["cpu"]));
        }

        auto new_state = torch::tensor(flattened_state, torch::dtype(torch::kFloat64)).unsqueeze(0);
        // std::cout << new_state << std::endl;

        return std::make_tuple(new_state, reward, done);
    }

    auto print_cache() -> void {
        fmt::print("print_cache:\n{}\n", cache_.dump(4));
        fmt::print("tasks:\n{}\n", t_.dump(4));
    }

private:
    task t_;
    device_cache cache_;
    std::vector<double> state_; // initialization state
    int n_ = 42;
};


class DQN_decision_engine : public decision_engine
{
    using this_type = DQN_decision_engine;

public:
    DQN_decision_engine() = default;
    DQN_decision_engine(client_device_container* clients, base_station_container* base_stations);
    DQN_decision_engine(std::vector<client_device_container>* clients_container, base_station_container* base_stations);

    auto make_decision(const task_element& header) -> result_t override;

    auto local_test(const task_element& header, client_device* client) -> bool override;

    auto send(task_element& t, client_device* client) -> bool override;

    auto train(const task& t) -> void;

    auto initialize() -> void override;

    auto handle_next() -> void override;

private:
    auto on_bs_decision_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;

    auto on_bs_response_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_cs_handling_message(cloud_server* cs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_es_handling_message(edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_clients_reponse_message(client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void;

    auto train_next(torch::Tensor observation) -> void;

private:
    client_device_container* clients_{};
    std::vector<client_device_container>* clients_container_{};
    base_station_container* base_stations_{};

    std::shared_ptr<DeepQNetwork> RL;
    task train_task;
};


} // namespace okec

#endif // DQN_DECISION_ENGINE_H_