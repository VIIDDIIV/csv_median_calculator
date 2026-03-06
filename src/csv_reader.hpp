/**
 * @file csv_reader.hpp
 * @brief Чтение и валидация CSV-файлов.
 */

#pragma once

#include "types.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace csv_median_calculator {

    // Собирает пути всех CSV-файлов в директории, соответствующих маскам (без чтения)
    [[nodiscard]] std::vector<std::filesystem::path> collect_csv_files(
        const std::filesystem::path& input_dir,
        const std::vector<std::string>& masks);

} // namespace csv_median_calculator