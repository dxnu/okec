#ifndef OKEC_CLOUD_EDGE_END_DEFAULT_DECISION_ENGINE_H_
#define OKEC_CLOUD_EDGE_END_DEFAULT_DECISION_ENGINE_H_

#include <okec/algorithms/decision_engine.h>


namespace okec
{

class client_device;
class client_device_container;
class edge_device;
class cloud_server;


class cloud_edge_end_default_decision_engine : public decision_engine
{
    using this_type = cloud_edge_end_default_decision_engine;

public:
    cloud_edge_end_default_decision_engine() = default;
    cloud_edge_end_default_decision_engine(client_device_container* clients, base_station_container* base_stations, cloud_server* cloud);

    auto make_decision(const task_element& header) -> result_t override;

    auto local_test(const task_element& header, client_device* client) -> bool override;

    auto send(task_element t, std::shared_ptr<client_device> client) -> bool override;

    auto initialize() -> void override;

    auto train(const task& train_task, int episode = 1) -> void;

    auto handle_next() -> void override;

private:
    auto on_bs_decision_message(base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

    auto on_bs_response_message(base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;
    
    auto on_es_handling_message(edge_device* es, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;
    
    auto on_cloud_handling_message(cloud_server* cs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

    auto on_clients_reponse_message(client_device* client, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

private:
    client_device_container* clients_{};
    std::vector<client_device_container>* clients_container_{};
    base_station_container* base_stations_{};
};


} // namespace okec

#endif // OKEC_CLOUD_EDGE_END_DEFAULT_DECISION_ENGINE_H_