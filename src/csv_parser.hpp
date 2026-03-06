#pragma once
#include "types.hpp"
#include <optional>
#include <string>

namespace csv_median_calculator {
    std::optional<record> parse_record(const std::string& line);
}