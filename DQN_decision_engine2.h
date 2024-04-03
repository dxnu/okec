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
    using this_type       = Env;
    using done_callback_t = std::function<void(const task&, const device_cache&)>;

public:
    Env(const device_cache& cache, const task& t, std::shared_ptr<DeepQNetwork> RL);

    auto next_observation() -> torch::Tensor;

    auto train() -> void;
    auto train_next() -> void;

    auto when_done(done_callback_t callback) -> void;

    auto print_cache() -> void;

    auto learn(std::size_t step) -> void;

private:
    task t_;
    device_cache cache_;
    std::shared_ptr<DeepQNetwork> RL_;
    std::vector<double> state_; // 初始状态
    torch::Tensor observation_;
    done_callback_t done_fn_;
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

    auto train(const task& train_task, int episode = 1) -> void;

    auto initialize() -> void override;

    auto handle_next() -> void override;

private:
    auto on_bs_decision_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;

    auto on_bs_response_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_cs_handling_message(cloud_server* cs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_es_handling_message(edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_clients_reponse_message(client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void;

    // episode: current episode_all: total episode
    auto train_start(const task& train_task, int episode, int episode_all) -> void;

private:
    client_device_container* clients_{};
    std::vector<client_device_container>* clients_container_{};
    base_station_container* base_stations_{};

    std::shared_ptr<DeepQNetwork> RL;
};


} // namespace okec

#endif // DQN_DECISION_ENGINE_H_