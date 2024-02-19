#ifndef OKEC_QLEARNING_DECISION_ENGINE
#define OKEC_QLEARNING_DECISION_ENGINE

#include "decision_engine.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/number.hpp>
#include "RL_brain.hpp"
#include "format_helper.hpp"


namespace okec
{

class client_device;
class client_device_container;
class edge_device;

namespace mp = boost::multiprecision;


class Env {
public:
    Env(device_cache& cache) : action_space(cache) {
        features = action_space.size();

        initialization_state.reserve(action_space.size());
        for (auto action : action_space) {
            initialization_state.push_back(TO_INT(action["cpu_cycle"]));
        }
    }

    auto n_actions() const -> std::size_t {
        return action_space.size();
    }

    auto n_features() const -> std::size_t {
        return features;
    }

    auto reset() -> torch::Tensor {
        auto observation = torch::from_blob(initialization_state.data(), {static_cast<long>(action_space.size())}, torch::kInt);
        return observation.unsqueeze(0);
    }

    auto step(int action) {
        auto& server =  action_space.view().at(action);
        std::cout << "server: " << server << "\n";
        auto available_capacity = TO_INT(server["cpu_cycle"]);
        if (available_capacity >= 10) {
            available_capacity -= 10;
            server["cpu_cycle"] = fmt::format("{}", available_capacity);
        }

        std::vector<int> flattened_state;
        for (const auto& action : action_space) {
            flattened_state.push_back(TO_INT(action["cpu_cycle"]));
        }

        auto new_state = torch::tensor(flattened_state, torch::dtype(torch::kInt)).unsqueeze(0);
        float reward;
        bool done;

        // reward function
        if (std::all_of(action_space.begin(), action_space.end(), [](const auto& server) {
            return TO_INT(server["cpu_cycle"]) == 0;
        })) {
            reward = 1;
            done = true;
        } else {
            reward = 0;
            done = false;
        }

        return std::make_tuple(new_state, reward, done);
    }

private:
    int features;
    device_cache& action_space;
    std::vector<int> initialization_state;
};


class qlearning_decision_engine : public decision_engine
{
    using this_type = qlearning_decision_engine;
    using big_float = mp::number<mp::cpp_dec_float<9>>;

public:
    qlearning_decision_engine() = default;
    qlearning_decision_engine(client_device_container* client_devices, base_station_container* bs_container, cloud_server* cs);

    auto make_decision(const task_element& header) -> result_t override;

    auto local_test(const task_element& header, client_device* client) -> bool override;

    auto send(task_element& t, client_device* client) -> bool override;

    auto train(const task& t, std::shared_ptr<client_device> client, base_station_container* bs_container, cloud_server* cs) -> void;

private:
    auto on_bs_decision_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;

    auto on_bs_response_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_cs_handling_message(cloud_server* cs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_es_handling_message(edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_clients_reponse_message(client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void;

};


} // namespace okec

#endif // OKEC_QLEARNING_DECISION_ENGINE