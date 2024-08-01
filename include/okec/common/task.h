///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_TASK_H_
#define OKEC_TASK_H_

#include <okec/utils/packet_helper.h>
#include <string>


namespace okec
{

class task_element
{
public:
    task_element(json* item) noexcept;
    task_element(json item) noexcept;
    task_element(const task_element& other) noexcept;
    task_element& operator=(const task_element& other) noexcept;
    task_element(task_element&& other) noexcept;
    task_element& operator=(task_element&& other) noexcept;
    ~task_element();

    auto get_header(const std::string& key) const -> std::string;
    auto set_header(std::string_view key, std::string_view value) -> bool;

    auto get_body(const std::string& key) const -> std::string;
    auto set_body(std::string_view key, std::string_view value) -> bool;

    auto j_data() const -> json;

    auto empty() const -> bool;

    static auto from_msg_packet(ns3::Ptr<ns3::Packet> packet) -> task_element;

    auto dump(int indent = -1) const -> std::string;

private:
    json* elem_;
    bool is_dynamically_allocated;
};

class task : public ns3::SimpleRefCount<task>
{
    using attribute_t      = std::pair<std::string, std::string>;
    using attributes_t     = std::initializer_list<attribute_t>;

public:
    using task_header = attributes_t;
    using task_body = attributes_t;

public:
    task() = default;
    task(json other);

    // construct task from packet
    static auto from_packet(ns3::Ptr<ns3::Packet> packet) -> task;

    static auto from_msg_packet(ns3::Ptr<ns3::Packet> packet) -> task;

    auto emplace_back(task_header, task_body = {}) -> void;
    
    auto dump(int indent = -1) const -> std::string;

    auto elements_view() -> std::vector<task_element>;
    auto elements() const -> std::vector<task_element>;

    auto at(std::size_t index) noexcept -> task_element;
    auto at(std::size_t index) const noexcept -> task_element;
    
    auto data() const -> json;

    auto j_data() const -> json;

    auto is_null() const -> bool;

    auto size() const -> std::size_t;

    auto empty() -> bool;

    auto set_if(attributes_t values, auto f) -> void;

    auto find_if(attributes_t values) -> task;
    auto find_if(attribute_t value) -> task;

    auto contains(attributes_t values) -> bool;
    auto contains(attribute_t value) -> bool;

    static auto get_header(const json& element, const std::string& key) -> std::string;
    static auto get_body(const json& element, const std::string& key) -> std::string;

    static auto unique_id() -> std::string;

    auto save_to_file(const std::string& file_name) -> void;
    auto load_from_file(const std::string& file_name) -> bool;

    auto operator[](std::size_t index) noexcept -> task_element;
    auto operator[](std::size_t index) const noexcept -> task_element;

private:
    auto push_back(const json& sub) -> void;

private:
    json m_task;
};


} // namespace okec

#endif // OKEC_TASK_H_