#ifndef OKEC_AP_STA_MOBILITY_HPP_
#define OKEC_AP_STA_MOBILITY_HPP_

#include <okec/mobility/mobility.hpp>


namespace okec
{

class ap_sta_mobility : public mobility<ap_sta_mobility> {
public:
    ap_sta_mobility() {
        mobility_.SetPositionAllocator("ns3::GridPositionAllocator",
                                    "MinX",
                                    ns3::DoubleValue(0.0),
                                    "MinY",
                                    ns3::DoubleValue(0.0),
                                    "DeltaX",
                                    ns3::DoubleValue(5.0),
                                    "DeltaY",
                                    ns3::DoubleValue(10.0),
                                    "GridWidth",
                                    ns3::UintegerValue(100),
                                    "LayoutType",
                                    ns3::StringValue("RowFirst"));
    }

    auto install(client_device_container& devices) -> void {
        ns3::NodeContainer wifiStaNodes;
        devices.get_nodes(wifiStaNodes);

        // 设置节点移动速度范围
        ns3::Ptr<ns3::UniformRandomVariable> speedVar = ns3::CreateObject<ns3::UniformRandomVariable>();
        speedVar->SetAttribute("Min", ns3::DoubleValue(5.0));
        speedVar->SetAttribute("Max", ns3::DoubleValue(10.0));

        mobility_.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds",
                                ns3::RectangleValue(ns3::Rectangle(-100, 100, -100, 100)),
                                "Speed",
                                ns3::PointerValue(speedVar));

        mobility_.Install(wifiStaNodes);
    }

    auto install(std::shared_ptr<base_station> bs) -> void {
        mobility_.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility_.Install(bs->get_node()); // ap node
    }

private:
    ns3::MobilityHelper mobility_;
};

} // namespace okec

#endif // OKEC_AP_STA_MOBILITY_HPP_