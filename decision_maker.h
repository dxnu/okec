#ifndef OKEC_DECISION_MAKER_H
#define OKEC_DECISION_MAKER_H
#include "task.h"
#include "resource.h"


namespace okec
{

class base_station_container;
class cloud_server;


class decision_maker
{
public:
    auto make_decision(task_header header) -> std::pair<std::string, uint16_t>;

    auto initialize_device(const base_station_container* bs_container, const cloud_server* cs) -> void;

private:
    auto query_information() -> void; // 构造函数里面定时执行该函数，以更新缓存信息

private:
    std::map<std::pair<std::string, uint16_t>, resource> m_device_cache;
    std::pair<ns3::Ipv4Address, uint16_t> m_cs_address;
};

} // namespace okec

#endif // OKEC_DECISION_MAKER_H