/**
 * @file config_parser.cpp
 * @brief Реализация парсинга TOML-конфигурации.
 */

#include "config_parser.hpp"

#include <toml++/toml.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

namespace csv_median_calculator {

    /**
 * @brief Парсит TOML-конфигурацию из файла.
 *
 * Функция загружает указанный файл, извлекает секцию [main] и заполняет
 * структуру config. Если поле output не задано, устанавливается значение "output".
 * При ошибках парсинга выбрасывает исключение с описанием.
 *
 * @param config_path_ Путь к конфигурационному файлу.
 * @return Заполненная структура config.
 * @throws std::runtime_error если отсутствует секция [main] или поле input,
 *         или при ошибках парсинга TOML.
 */
    config parse_config(const std::string& config_path_) {
        try {
            // Загружаем и парсим TOML-файл целиком.
            auto table = toml::parse_file(config_path_);

            // Проверяем наличие секции [main].
            auto main_section = table["main"];
            if (!main_section) {
                throw std::runtime_error("Missing [main] section in config");
            }

            config cfg;

            // Поле input обязательно.
            auto input_opt = main_section["input"].value<std::string>();
            if (!input_opt) {
                throw std::runtime_error("Missing 'input' in [main]");
            }
            cfg.input_dir = *input_opt;

            // Поле output опционально. Если не задано, используем "output".
            auto output_opt = main_section["output"].value<std::string>();
            if (output_opt) {
                cfg.output_dir = *output_opt;
            }
            else {
                cfg.output_dir = "output";   // директория по умолчанию
            }

            // Поле filename_mask опционально, может быть массивом строк.
            auto masks_array = main_section["filename_mask"].as_array();
            if (masks_array) {
                for (auto&& elem : *masks_array) {
                    if (auto str_opt = elem.value<std::string>()) {
                        cfg.filename_masks.push_back(*str_opt);
                    }
                    else {
                        spdlog::warn("Non-string element in filename_mask, skipped");
                    }
                }
            }

            return cfg;
        }
        catch (const toml::parse_error& err) {
            spdlog::error("Failed to parse TOML: {} at line {}",
                err.description(), err.source().begin.line);
            throw;
        }
    }

} 