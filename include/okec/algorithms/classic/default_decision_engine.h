#ifndef OKEC_DEFAULT_DECISION_ENGINE_H_
#define OKEC_DEFAULT_DECISION_ENGINE_H_

#include <okec/algorithms/decision_engine.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/number.hpp>


namespace okec
{

class client_device;
class client_device_container;
class edge_device;

namespace mp = boost::multiprecision;

class default_decision_engine : public decision_engine
{
    using this_type = default_decision_engine;
    using big_float = mp::number<mp::cpp_dec_float<9>>;

public:
    default_decision_engine() = default;
    default_decision_engine(client_device_container* client_devices, base_station_container* bs_container, cloud_server* cs);

    auto make_decision(const task_element& header) -> result_t override;

    auto local_test(const task_element& header, client_device* client) -> bool override;

    auto send(task_element& t, client_device* client) -> bool override;

private:
    auto on_bs_decision_message(base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

    auto on_bs_response_message(base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;
    
    auto on_cs_handling_message(cloud_server* cs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;
    
    auto on_es_handling_message(edge_device* es, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;
    
    auto on_clients_reponse_message(client_device* client, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

};


} // namespace okec

#endif // OKEC_DEFAULT_DECISION_ENGINE_H_