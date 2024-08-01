///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/common/task.h>
#include <okec/utils/format_helper.hpp>
#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <ns3/ptr.h>



namespace okec
{

task_element::task_element(json* item) noexcept
{
    if (item->contains("/header"_json_pointer)) {
        elem_ = item; // ref
        is_dynamically_allocated = false;
    }
    else
        elem_ = nullptr;
}

task_element::task_element(json item) noexcept
{
    if (item.contains("/header"_json_pointer)) {
        elem_ = new json(std::move(item)); // copy
        is_dynamically_allocated = true;
    }
    else
        elem_ = nullptr;
}

task_element::task_element(const task_element& other) noexcept
{
    if (this != &other) {
        elem_ = new json(*other.elem_); // copy
        is_dynamically_allocated = true;
    }
}

task_element& task_element::operator=(const task_element& other) noexcept
{
    if (this != &other) {
        elem_ = new json(*other.elem_); // copy
        is_dynamically_allocated = true;
    }

    return *this;
}

task_element::task_element(task_element&& other) noexcept
    : elem_ { std::exchange(other.elem_, nullptr) }
    , is_dynamically_allocated { std::exchange(other.is_dynamically_allocated, false) }
{
}

task_element& task_element::operator=(task_element&& other) noexcept
{
    elem_ = std::exchange(other.elem_, nullptr);
    is_dynamically_allocated = std::exchange(other.is_dynamically_allocated, false);
    return *this;
}

task_element::~task_element()
{
    if (elem_ != nullptr && is_dynamically_allocated)
        delete elem_;
}

auto task_element::get_header(const std::string& key) const -> std::string
{
    std::string result{};
    json::json_pointer j_key{ "/header/" + key };
    if (elem_ && elem_->contains(j_key))
        elem_->at(j_key).get_to(result);
    
    return result;
}

auto task_element::set_header(std::string_view key, std::string_view value) -> bool
{
    if (elem_) {
        json::json_pointer j_key{ "/header/" + std::string(key) };
        (*elem_)[j_key] = value;
        return true;
    }

    return false;
}

auto task_element::get_body(const std::string& key) const -> std::string
{
    std::string result{};
    json::json_pointer j_key{ "/body/" + key };
    if (elem_ && elem_->contains(j_key))
        elem_->at(j_key).get_to(result);
    
    return result;
}

auto task_element::set_body(std::string_view key, std::string_view value) -> bool
{
    if (elem_) {
        json::json_pointer j_key{ "/body/" + std::string(key) };
        (*elem_)[j_key] = value;
        return true;
    }

    return false;
}

auto task_element::j_data() const -> json
{
    return elem_ ? *elem_ : json{};
}

auto task_element::empty() const -> bool
{
    return elem_;
}

auto task_element::from_msg_packet(ns3::Ptr<ns3::Packet> packet) -> task_element
{
    json j = packet_helper::to_json(packet);
    if (!j.is_null() && j.contains("/content/header"_json_pointer))
        return task_element(j["content"]);
    
    return task_element{nullptr};
}

auto task_element::dump(int indent) const -> std::string
{
    std::string result{};
    if (!elem_->is_null())
        result = elem_->dump(indent);
    return result;
}

task::task(json other)
{
    if (other.contains("/task/items"_json_pointer))
        m_task = std::move(other);
}

auto task::from_packet(ns3::Ptr<ns3::Packet> packet) -> task
{
    json j = packet_helper::to_json(packet);
    return j.is_null() ? task{} : task(j);
}

auto task::from_msg_packet(ns3::Ptr<ns3::Packet> packet) -> task
{
    json j = packet_helper::to_json(packet);
    if (!j.is_null() && j.contains("/content/task"_json_pointer))
        return task(j["content"]);
    
    return task{};
}

auto task::emplace_back(task_header header_attrs, task_body body_attrs) -> void
{
    json item;
    // Set header attributes
    for (auto [key, value] : header_attrs) {
        item["header"][key] = value;
    }

    // Set body attributes
    for (auto [key, value] : body_attrs) {
        item["body"][key] = value;
    }

    m_task["task"]["items"].emplace_back(std::move(item));
}

auto task::dump(int indent) const -> std::string
{
    return m_task.dump(indent);
}

auto task::elements_view() -> std::vector<task_element>
{
    std::vector<task_element> items;
    items.reserve(this->size());
    for (json& item : m_task["task"]["items"])
        items.emplace_back(task_element(&item));

    return items;
}

auto task::elements() const -> std::vector<task_element>
{
    std::vector<task_element> items;
    items.reserve(this->size());

    for (const json& item : m_task["task"]["items"])
        items.emplace_back(task_element(item));

    return items;
}

auto task::at(std::size_t index) noexcept -> task_element
{
    return task_element(&m_task["task"]["items"].at(index));
}

auto task::at(std::size_t index) const noexcept -> task_element
{
    return task_element(m_task["task"]["items"].at(index));
}

auto task::data() const -> json
{
    return m_task["task"]["items"];
}

auto task::j_data() const -> json
{
    return m_task;
}

auto task::is_null() const -> bool
{
    return m_task.is_null();
}

auto task::size() const -> std::size_t
{
    if (m_task.contains("/task/items"_json_pointer))
        return m_task["task"]["items"].size();

    return 0;
}

auto task::set_if(attributes_t values, auto f) -> void
{
    for (auto& item : m_task["task"]["items"]) {
        bool cond{true};
        for (auto [key, value] : values) {
            if (item[key] != value)
                cond = false;
        }

        if (cond) {
            f(item);
            break;
        }
    }
}

auto task::find_if(attributes_t values) -> task
{
    task result{};
    for (const auto& item : m_task["task"]["items"]) {
        bool cond{true};
        for (auto [key, value] : values) {
            if (item[key] != value)
                cond = false;
        }

        if (cond) {
            result.push_back(item);
        }
    }
    return result;
}

auto task::find_if(attribute_t value) -> task
{
    return find_if({value});
}
// status 0
auto task::contains(attributes_t values) -> bool
{
    for (auto& item : m_task["task"]["items"]) {
        for (auto [key, value] : values) {
            if (item["header"][key] == value)
                return true;
        }
    }

    return false;
}

auto task::contains(attribute_t value) -> bool
{
    return contains({value});
}

auto task::get_header(const json& element, const std::string& key) -> std::string
{
    std::string result{};
    json::json_pointer j_key{ "/header/" + key };
    if (element.contains(j_key))
        element.at(j_key).get_to(result);
    
    return result;
}

auto task::get_body(const json& element, const std::string& key) -> std::string
{
    std::string result{};
    json::json_pointer j_key{ "/body/" + key };
    if (element.contains(j_key))
        element.at(j_key).get_to(result);
    
    return result;
}

auto task::unique_id() -> std::string
{
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    int i;
    ss << std::hex << std::uppercase;
    for (i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    // ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    // ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    // ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    // ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis(gen);
    };
    return ss.str();
}

auto task::save_to_file(const std::string& file_name) -> void
{
    std::ofstream fout(file_name);
    fout << std::setw(4) << m_task << std::endl;
}

auto task::load_from_file(const std::string& file_name) -> bool
{
    std::ifstream fin(file_name);
    if (!fin.is_open())
        return false;

    json data;
    fin >> data;

    if (!data.contains("/task/items"_json_pointer))
        return false;
    
    m_task = std::move(data);
    return true;
}

auto task::operator[](std::size_t index) noexcept -> task_element
{
    return this->at(index);
}

auto task::operator[](std::size_t index) const noexcept -> task_element
{
    return this->at(index);
}

auto task::push_back(const json &sub) -> void
{
    m_task.push_back(sub);
}


} // namespace okec