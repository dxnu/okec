#ifndef OKEC_DEFAULT_DECISION_ENGINE
#define OKEC_DEFAULT_DECISION_ENGINE

#include "decision_engine.h"

namespace okec
{

class edge_device;


class default_decision_engine : public decision_engine
{
    using this_type = default_decision_engine;

public:
    default_decision_engine() = default;
    default_decision_engine(base_station_container* bs_container, cloud_server* cs);

    auto make_decision(const task_element& header) -> result_t override;

private:
    auto on_bs_decision_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_cs_handling_message(cloud_server* cs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_es_handling_message(edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void;

};


} // namespace okec

#endif // OKEC_DEFAULT_DECISION_ENGINE