/**
 * @file csv_reader.cpp
 * @brief Реализация чтения CSV-файлов.
 */

#include "csv_reader.hpp"

#include <algorithm>
#include <future>
#include <thread>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <queue>

namespace csv_median_calculator {
    namespace fs = std::filesystem;



    namespace {

        bool filename_matches(const fs::path& filename_, const std::vector<std::string>& masks_) {
            if (masks_.empty()) {
                return true;
            }
            std::string name = filename_.string();
            return std::any_of(masks_.begin(), masks_.end(),
                [&](const std::string& mask) {
                    return name.find(mask) != std::string::npos;
                });
        }

        bool validate_csv_header(const std::string& header_) {
            return header_.find("receive_ts") != std::string::npos &&
                header_.find("price") != std::string::npos;
        }
    

    } // unnamed namespace

    /**
 * @brief Собирает пути CSV-файлов в директории, соответствующих маскам.
 *
 * Проходит по всем файлам в указанной директории, отбирает регулярные файлы
 * с расширением .csv, имя которых содержит хотя бы одну из заданных масок.
 * Выполняет первичную проверку: читает заголовок и первую строку данных,
 * проверяет наличие обязательных колонок и минимальное количество полей.
 * Файлы, не прошедшие проверку, исключаются из результата с логированием ошибки.
 *
 * @param input_dir Путь к входной директории.
 * @param masks Список подстрок для фильтрации.
 * @return Вектор путей к файлам, прошедшим первичную проверку.
 */
    std::vector<fs::path> collect_csv_files(const fs::path& input_dir, const std::vector<std::string>& masks) {
        std::vector<fs::path> result;
        if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
            spdlog::error("Input directory does not exist or is not a directory: {}", input_dir.string());
            return result;
        }

        for (const auto& entry : fs::directory_iterator(input_dir)) {
            if (!entry.is_regular_file()) continue;
            const auto& path = entry.path();
            if (path.extension() != ".csv") continue;
            if (!filename_matches(path.filename(), masks)) continue;

            // --- Первичная проверка файла ---
            std::ifstream file(path);
            if (!file.is_open()) {
                spdlog::error("Cannot open file for validation: {}", path.string());
                continue;
            }

            std::string line;
            // Проверка заголовка
            if (!std::getline(file, line)) {
                spdlog::error("Empty file (no header): {}", path.string());
                continue;
            }
            if (!validate_csv_header(line)) {
                spdlog::error("Invalid CSV header in file {}: {}", path.string(), line);
                continue;
            }

            // Проверка наличия хотя бы одной строки данных
            if (!std::getline(file, line)) {
                spdlog::error("File has no data rows (only header): {}", path.string());
                continue;
            }

            // Минимальная проверка количества колонок в первой строке данных
            std::stringstream ss(line);
            std::string field;
            std::vector<std::string> fields;
            while (std::getline(ss, field, ';')) {
                fields.push_back(field);
            }
            if (fields.size() < 5) {
                spdlog::error("File {}: first data row has insufficient columns ({}), skipping file",
                    path.string(), fields.size());
                continue;
            }

            // Если все проверки пройдены, добавляем файл в список
            result.push_back(path);
        }
        return result;
    }

} 