#ifndef READ_CSV_H_
#define READ_CSV_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>


namespace okec
{

using dataset_sequence_type = std::vector<std::vector<std::string>>;

auto read_csv(std::string_view file, std::string_view type = "", std::string_view delimiter = ",")
    -> std::optional<dataset_sequence_type>;

} // namespace okec


#endif // READ_CSV_H_