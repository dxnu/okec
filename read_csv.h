#ifndef OKEC_READ_CSV
#define OKEC_READ_CSV

#include <vector>
#include <string>
#include <string_view>
#include <optional>


namespace okec
{

using dataset_sequence_type = std::vector<std::vector<std::string>>;

auto read_csv(std::string_view file, std::string_view type = "", std::string_view delimiter = ",")
    -> std::optional<dataset_sequence_type>;

} // namespace okec


#endif // OKEC_READ_CSV