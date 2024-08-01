///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_RL_BRAIN_HPP_
#define OKEC_RL_BRAIN_HPP_

#include <okec/utils/visualizer.hpp>
#include <torch/torch.h>


namespace okec {

class Network : public torch::nn::Module
{
public:
    Network(int n_features, int n_actions, int n_neuron = 10)
    {
        // Define layers
        net = register_module("net", torch::nn::Sequential(
            torch::nn::Linear(torch::nn::LinearOptions(n_features, n_neuron).bias(true)),
            torch::nn::Linear(torch::nn::LinearOptions(n_neuron, n_actions).bias(true)),
            torch::nn::ReLU()
        ));
    }

    torch::Tensor forward(torch::Tensor& s) {
        return net->forward(s);
    }

private:
    torch::nn::Sequential net;
};


class DeepQNetwork : public torch::nn::Module
{
public:
    DeepQNetwork(int n_actions,
                 int n_features,
                 double learning_rate = 0.01,
                 double reward_decay = 0.9,
                 double e_greedy = 0.9,
                 int replace_target_iter = 300,
                 int memory_size = 500,
                 int batch_size=32,
                 double e_greedy_increment = 0)
      : n_actions(n_actions),
        n_features(n_features),
        lr(learning_rate),
        gamma(reward_decay),
        epsilon_max(e_greedy),
        replace_target_iter(replace_target_iter),
        memory_size(memory_size),
        batch_size(batch_size),
        epsilon_increment(e_greedy_increment),
        epsilon(e_greedy_increment ? 0 : epsilon_max),
        learn_step_counter(0), // total learning step
        memory_counter(0),
        memory(torch::zeros({memory_size, n_features * 2 + 2})),
        eval_net(n_features, n_actions),
        target_net(n_features, n_actions),
        loss_function(),
        optimizer(eval_net.parameters(), torch::optim::AdamOptions(lr))
      {}

    void store_transition(const torch::Tensor& s, int a, float r, const torch::Tensor& s_)
    {
        torch::Tensor transition = torch::cat({s, torch::stack({torch::tensor({a}), torch::tensor({r})}, 1), s_}, 1);
        // std::cout << "transition: " << transition << "\n";

        // replace the old memory with new memory
        int index = this->memory_counter % this->memory_size;
        this->memory.index_put_({index}, transition);

        this->memory_counter += 1;
    }

    int choose_action(torch::Tensor& observation) {
        if (torch::rand({1}).item<double>() < epsilon) {
            observation = observation.to(torch::kFloat);

            torch::Tensor actions_value = this->eval_net.forward(observation);
            int action = torch::argmax(actions_value).item<int>();
            return action;
        } else {
            return torch::randint(n_actions, {1}).item<int>();
        }
    }

    void load_state_dict(torch::nn::Module& model, torch::nn::Module& target_model) {
        torch::autograd::GradMode::set_enabled(false); // make parameters copying possible
        auto new_params = target_model.named_parameters();
        auto params = model.named_parameters(true /*recurse*/);
        auto buffers = model.named_buffers(true /*recurse*/);
        for (auto& val : new_params) {
            auto name = val.key();
            auto t = params.find(name);
            if (t != nullptr) {
                t->copy_(val.value());
            } else {
                t = buffers.find(name);
                if (t != nullptr) {
                    t->copy_(val.value());
                }
            }
        }

        torch::autograd::GradMode::set_enabled(true); // recovery grad
    }

    void replace_target_params() {
        load_state_dict(target_net, eval_net);
    }

    void learn() {
        torch::autograd::GradMode::set_enabled(true);

        if (this->learn_step_counter % this->replace_target_iter == 0) {
            this->replace_target_params();
            std::cout << "target params replaced\n";
        }

        // sample batch memory from all memory
        torch::Tensor batch_memory;
        if (this->memory_counter > this->memory_size) {
            batch_memory = memory.index({torch::randperm(memory_size, torch::dtype(torch::kLong)).slice(0, 0, batch_size)});
        } else {
            batch_memory = memory.index({torch::randint(0, memory_size, {batch_size}, torch::dtype(torch::kLong))});
        }

        // run the nextwork
        torch::Tensor s = batch_memory.index({torch::indexing::Slice(), torch::indexing::Slice(torch::indexing::None, n_features)});
        torch::Tensor s_ = batch_memory.index({torch::indexing::Slice(), torch::indexing::Slice(-n_features, batch_memory.size(1))});
        torch::Tensor q_eval = this->eval_net.forward(s);
        torch::Tensor q_next = this->target_net.forward(s_);

        torch::Tensor q_target = q_eval.clone();

        torch::Tensor batch_index = torch::arange(batch_size, torch::dtype(torch::kInt));
        torch::Tensor eval_act_index = batch_memory.index({torch::indexing::Slice(), n_features}).to(torch::kInt);
        torch::Tensor reward = batch_memory.index({torch::indexing::Slice(), (n_features + 1)}).to(torch::kFloat);

        q_target.index_put_({batch_index, eval_act_index}, reward + gamma * std::get<0>(torch::max(q_next, 1)));

        // train eval network
        // torch::Tensor loss = torch::nn::functional::mse_loss(q_target, q_eval);
        // torch::Tensor loss = torch::mse_loss(q_target, q_eval);
        torch::Tensor loss = loss_function(q_target, q_eval);
        optimizer.zero_grad();
        loss.backward();

        // 记录每一步训练的损失值（loss）
        cost_his.push_back(loss.item<float>());

        // increasing epsilon
        epsilon = epsilon < epsilon_max ? epsilon + epsilon_increment : epsilon_max;
        learn_step_counter += 1;
    }

    void plot_cost() {
        // plt::plot(cost_his);
        // plt::show();
        okec::draw(cost_his, "cost hist");
    }

    void print_memory() {
        std::cout << "memory:\n" << this->memory;

        // auto q_table = eval_net.parameters();
        // std::cout << "Q Table:" << std::endl;
        // std::cout << q_table << std::endl;
    }
    
private:
    int n_actions;
    int n_features;
    double lr;
    double gamma;
    double epsilon_max;
    int replace_target_iter;
    int memory_size;
    int batch_size;
    double epsilon_increment;
    double epsilon;
    int learn_step_counter;
    int memory_counter;
    torch::Tensor memory;
    Network eval_net;
    Network target_net;
    torch::nn::MSELoss loss_function; // 均方误差（MSE）损失函数
    torch::optim::Adam optimizer;
    std::vector<float> cost_his;
};

} // namespace okec

#endif // OKEC_RL_BRAIN_HPP_