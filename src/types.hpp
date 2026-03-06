/**
 * @file types.hpp
 * @brief Общие типы данных для проекта.
 *
 * Содержит структуру Record, используемую для хранения одной записи из CSV.
 */

#pragma once

#include <cstdint>

namespace csv_median_calculator {

    /**
     * @brief Структура, представляющая одну запись из CSV-файла после фильтрации.
     *
     * Нам нужны только временная метка получения и цена.
     */
    struct record {
        std::uint64_t receive_ts;   ///< Временная метка получения данных (микросекунды)
        double price;                ///< Цена (для level или trade)
    };

} // namespace csv_median_calculator