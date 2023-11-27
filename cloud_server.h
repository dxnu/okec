#ifndef OKEC_CLOUD_DEVICE_H
#define OKEC_CLOUD_DEVICE_H

#include "task.h"
#include "resource.h"
#include "udp_application.h"
#include "ns3/internet-module.h"
#include "ns3/node-container.h"


namespace okec
{

using ns3::Address;
using ns3::Ipv4;
using ns3::Ipv4Address;
using ns3::Node;
using ns3::NodeContainer;
using ns3::Ptr;
using ns3::Packet;
using ns3::Seconds;
using ns3::Socket;



class cloud_server {
    using callback_type  = std::function<void(cloud_server*, Ptr<Packet>, const Address&)>;

public:
    cloud_server();

    auto get_node() -> Ptr<Node>;

    auto get_nodes(NodeContainer &nodes) -> void;

    // 返回当前设备的IP地址
    auto get_address() const -> Ipv4Address;

    // 返回当前设备的端口号
    auto get_port() const -> uint16_t;

    // 获取当前设备绑定的资源
    auto get_resource() -> Ptr<resource>;

    // 为当前设备安装资源
    auto install_resource(Ptr<resource> res) -> void;

    auto set_position(double x, double y, double z) -> void;
    auto get_position() -> Vector;

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

    auto write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port) const -> void;

private:
    // 处理请求回调函数
    auto on_get_resource_information(Ptr<Packet> packet, const Address& remote_address) -> void;

private:
    Ptr<Node> m_node;
    Ptr<udp_application> m_udp_application;
};


} // namespace okec


#endif // OKEC_CLOUD_DEVICE_H