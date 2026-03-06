/**
 * @file config_parser.hpp
 * @brief Парсинг конфигурационного TOML-файла.
 */

#pragma once

#include <string>
#include <vector>

namespace csv_median_calculator {

    struct config {
        std::string input_dir;                  ///< Путь к директории с входными CSV
        std::string output_dir;                  ///< Путь к директории для выходного файла
        std::vector<std::string> filename_masks; ///< Список подстрок для фильтрации файлов
    };

    [[nodiscard]] config parse_config(const std::string& config_path_);

} 